/*
 * AStarSearch.h
 *
 *  Created on: 09.05.2016
 *      Author: michael
 */

#ifndef TOPKSEARCH_H_
#define TOPKSEARCH_H_

#include "IndexReader.h"
#include "datamodel/Item.h"
#include "datamodel/StatementGroup.h"
#include "Blacklist.h"
#include "State.h"
#include "Initial.h"
#include "datamodel/TopKEntry.h"
#include "TopK.h"
#include "DebugHelpers.h"
#include "Tracker.h"

#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <fstream>

class TopKSearch {
public:

	TopKSearch(IndexReader& reader, int k, int acc, ostream* log,
			Tracker* track);
	~TopKSearch();

	unordered_map<int, TopKEntry> search(int itemId);

protected:

	IndexReader& reader;
	TopK topK;
	int k;
	int acc;
	ostream* log;
	Tracker* track;

	void processInitial(Initial* initial, State& state);
	map<int, double>* hasSimilarity(vector<int> propertyTrail,
			vector<int> itemTrail, double weight, Blacklist& blacklist,
			int& inpenalty);

	void addItemToResult(map<int, double>* result, int itemId,
			vector<int>& itemTrail, double weight, Blacklist* blacklist,
			int& inpenalty);
	bool extendTrails(vector<int>& searchTrailPositions,
			vector<StatementGroup*>& searchTrailTargets, Item& item,
			int propertyId);

};

TopKSearch::TopKSearch(IndexReader& reader, int k, int acc, ostream* log,
		Tracker* track) :
		reader(reader), topK(k, log) {
	this->acc = acc;
	this->k = k;
	this->log = log;
	this->track = track;
}

TopKSearch::~TopKSearch() {
}

unordered_map<int, TopKEntry> TopKSearch::search(int itemId) {

	vector<int> itemTrail;
	itemTrail.push_back(itemId);

	vector<int> propertyTrail;
	Initial* initial = new Initial(reader, itemId, NULL, 1.0, 1.0, itemTrail,
			propertyTrail);

	int debug;
	int maxIteration = 10000000;
	int level = 1;
	bool terminate = false;
	while ((!terminate) && (level <= acc)) {
		(*log) << "Level: " << level << endl;
		int iteration = 0;
		topK.clear();
		track->clearListeners();
		State state = State(&topK, level, log);
		debug = 0;
		state.createNewInitials(initial, NULL, reader, false);
		while ((iteration < maxIteration) && (!terminate)) {
			Initial* init = state.getNextInitial(debug, reader, iteration,
					maxIteration);
			if (init == NULL) {
				break;
			}
			(*log) << "Iteration: " << iteration << endl;
			processInitial(init, state);
			delete init;
			terminate = topK.hasConverged();

		}
		level++;
	}
	(*log) << "Debug:" << debug << endl;
	(*log) << "score table size: " << topK.getContentsSize() << " terminate:  "
			<< terminate << " Global Delta: " << topK.getGlobalDelta() << endl;
	(*log) << "inUseCount: " << reader.getInUseCount() << endl;
	return topK.getTopK();
}

void TopKSearch::processInitial(Initial* initial, State& state) {
	// compute similarities
	int ip = 0;
	map<int, double>* candidates = hasSimilarity(initial->getPropertyTrail(),
			initial->getItemTrail(), initial->getOP(), *initial->getBlacklist(),
			ip);

	// create new initials
	double newOp = state.createNewInitials(initial, initial->getBlacklist(),
			reader, true);

	double oldIp = initial->getBaseIp();
	double oldOp = initial->getOP();
	(*log) << "old OP " << oldOp;
	(*log) << " old IP " << oldIp;
	(*log) << endl;
	if ((ip != 0) && (oldIp > (1.0 / ((double) ip)))) {
		double candidatesReduce = oldOp * oldIp;
		double allReduce = candidatesReduce
				- (oldOp * (1.0 / ((double) ip + 1.0)));
		(*log) << " new OP: " << newOp << " --> allReduce: " << allReduce
				<< " candidatesReduce: " << candidatesReduce << endl;
		if (track != NULL) {
			for (map<int, double>::iterator it = candidates->begin();
					it != candidates->end(); it++) {
				track->notify(it->first, it->second, initial->getItemTrail(),
						initial->getPropertyTrail());
			}
		}
		topK.updateTopK(candidates, allReduce, candidatesReduce,
				*initial->getBlacklist());
	}

	delete candidates;

}

/**
 * Returns possible targets.
 * Arguments:
 * 	propertyTrail	path to the item
 * 	itemTrail		blacklist of items according to the properties in the property trail
 * 	weight			holds the inverted weight for the path in the property trail
 * 	blacklist		blacklist with items that already got a score
 * 	inpenalty		variable to return the inpenalty of the computation
 */
map<int, double>* TopKSearch::hasSimilarity(vector<int> propertyTrail,
		vector<int> itemTrail, double weight, Blacklist& blacklist,
		int& inpenalty) {

	(*log) << "Call hasSimilarity itemTrail ";
	DebugHelpers::printTrail(itemTrail, *log);
	(*log) << " OP: " << weight << " PropertyTrail ";
	DebugHelpers::printTrail(propertyTrail, *log);
	(*log) << endl;
	map<int, double>* result = new map<int, double>();
	if (propertyTrail.empty()) {
		return result;
	}

	vector<int> inUse;

	int itemId = itemTrail.back();

	vector<StatementGroup*> searchTrailTargets;
	vector<int> searchTrailPositions;

	Item& origin = reader.getItemById(itemId); // the item where the two paths end

	if (origin.getId() == 0) {
		return result; // null item -> not valid
	} else {
		reader.setInUseFlag(itemId);
		inUse.push_back(itemId);
	}

	extendTrails(searchTrailPositions, searchTrailTargets, origin,
			propertyTrail.back());

	if (searchTrailTargets.size() == 0) {
		// unset inUse flags
		for (size_t i = 0; i < inUse.size(); i++) {
			reader.unsetInUseFlag(inUse[i]);
		}
		return result;
	}

	// search
	while (!((searchTrailTargets.size() == 1)
			&& (searchTrailPositions[0] == searchTrailTargets[0]->size()))) { // exit condition
		if (searchTrailPositions.back() < searchTrailTargets.back()->size()) {
			// -> there are still elements on this layer

			// read next item
			int id =
					(*searchTrailTargets.back()).getTargets()[searchTrailPositions.back()];

			// set position in search trail to next element
			searchTrailPositions.back()++;

			int
			propertyTrailPosition = propertyTrail.size()
					- searchTrailTargets.size() - 1;
			if (propertyTrail.size() > searchTrailTargets.size()) { // property path is not at the end
				Item& item = reader.getItemById(id);

				if (item.getId() != 0) {
					if (reader.setInUseFlag(id)) {
						inUse.push_back(id);
					} else {
						inUse.push_back(0);
					}

					if (!extendTrails(searchTrailPositions, searchTrailTargets,
							item, propertyTrail[propertyTrailPosition])) {
						reader.unsetInUseFlag(inUse.back());
						inUse.pop_back();
					}
				}
			} else {
				addItemToResult(result, id, itemTrail, weight, &blacklist,
						inpenalty);
			}
		} else {
			// go to upper layer
			searchTrailPositions.pop_back();
			searchTrailTargets.pop_back();
			reader.unsetInUseFlag(inUse.back());
			inUse.pop_back();
		}
	}

	// unset inUse flags
	for (size_t i = 0; i < inUse.size(); i++) {
		reader.unsetInUseFlag(inUse[i]);
	}

	inpenalty--; // ignore origin itself

	// multiply in-penalty
	for (map<int, double>::iterator it = result->begin(); it != result->end();
			it++) {
		it->second *= 1.0 / inpenalty;
	}

	return result;

}

/**
 * Adds an item and its weight to the result list if the proposed item is not in the blacklist.
 */
void TopKSearch::addItemToResult(map<int, double>* result, int itemId,
		vector<int>& itemTrail, double weight, Blacklist* blacklist,
		int& inpenalty) {

	inpenalty++;

	if (itemTrail[0] == itemId) {
		return; // item is origin
	}

	// check blacklist
	if (blacklist->hasItem(itemId)) {
		return;
	} else {
		blacklist->addItem(itemId);
	}

	// add item if not in blacklist
	(*result)[itemId] = weight;
}

bool TopKSearch::extendTrails(vector<int>& searchTrailPositions,
		vector<StatementGroup*>& searchTrailTargets, Item& item,
		int propertyId) {
	StatementGroup* stmtGrs = item.getStatementGroups();
	for (size_t i = 0; i < item.size(); i++) {
		if (stmtGrs[i].getPropertyId() == -propertyId) {
			searchTrailTargets.push_back(&stmtGrs[i]); // evt. & weglassen
		}
	}
	if (searchTrailTargets.size() > searchTrailPositions.size()) { // found an element
		searchTrailPositions.push_back(0);
		return true;
	}
	return false;
}

#endif /* TOPKSEARCH_H_ */
