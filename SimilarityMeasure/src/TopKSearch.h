/*
 * AStarSearch.h
 *
 *  Created on: 09.05.2016
 *      Author: michael
 */

#ifndef TOPKSEARCH_H_
#define TOPKSEARCH_H_

#include "InMemoryIndexReader.h"
#include "datamodel/Item.h"
#include "datamodel/StatementGroup.h"
#include "Blacklist.h"
#include "State.h"
#include "Initial.h"
#include "datamodel/TopKEntry.h"

#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>

class TopKSearch {
public:

	TopKSearch(InMemoryIndexReader& reader);
	~TopKSearch();

	void updateTopK(map<int, double>* candidates, double gReduction,
			double cReduction, Blacklist& bl);
	unordered_map<int, TopKEntry>& getTopK();
	int getBestMatch();
	double getBestMatchValue();
	void search(int itemId);
	void processInitial(Initial* initial, State& state);
	map<int, double>* hasSimilarity(vector<int> propertyTrail,
			vector<int> itemTrail, double weight, Blacklist& blacklist,
			int& inpenalty);

protected:

	InMemoryIndexReader& reader;
	unordered_map<int, TopKEntry> topK;
	int topId;
	double topValue;
	double globalDelta;

	void addItemToResult(map<int, double>* result, int itemId,
			vector<int>& itemTrail, double weight, Blacklist* blacklist,
			int& inpenalty);
	void extendTrails(vector<int>& searchTrailPositions,
			vector<vector<int>*>& searchTrailTargets, Item& item,
			int propertyId);

	// debug
	void printTrail(vector<int>& trail) {
		cout << "[ ";
		for (size_t i = 0; i < trail.size(); i++) {
			cout << trail[i] << " ";
		}
		cout << "]";
	}

};

TopKSearch::TopKSearch(InMemoryIndexReader& reader) :
		reader(reader) {
	topId = 0;
	topValue = 0;
	globalDelta = 1;
}

TopKSearch::~TopKSearch() {
}

void TopKSearch::updateTopK(map<int, double>* candidates, double gReduction,
		double cReduction, Blacklist& bl) {
	cout << "cReduce: " << cReduction << " gReduce: " << gReduction << " candidates size: " << candidates->size() << endl;
	for (map<int, double>::iterator it = candidates->begin();
			it != candidates->end(); it++) {
		TopKEntry& value = topK[it->first];
		value.weight += it->second;
		if (value.delta == 0) {
			value.delta = globalDelta;
		}
		value.delta -= cReduction;
		if (value.weight > topValue) {
			topValue = value.weight;
			topId = it->first;
		}
	}

	vector<int> toErase;

	for (unordered_map<int, TopKEntry>::iterator it = topK.begin();
			it != topK.end(); it++) {
		TopKEntry& value = it->second;
		if (!bl.hasItem(it->first)) {
			value.delta -= gReduction;
		}
		if (value.delta < 0) {
			cout << "################ FEHLER #####################" << endl;
		}
		if (value.delta < (topValue - value.weight)) {
			if (topId == it->first){
				cout << "####Problem:" << endl;
			}
			toErase.push_back(it->first);
		}
	}
	for (size_t i = 0; i < toErase.size(); i++) {
		topK.erase(toErase[i]);
	}
	globalDelta -= gReduction;
}

unordered_map<int, TopKEntry>& TopKSearch::getTopK() {
	return topK;
}

int TopKSearch::getBestMatch() {
	return topId;
}

double TopKSearch::getBestMatchValue() {
	return topValue;
}

void TopKSearch::search(int itemId) {

	State state(&topK, &globalDelta, &topValue);

	vector<int> itemTrail;
	itemTrail.push_back(itemId);

	vector<int> propertyTrail;

	Initial* initial = new Initial(reader, itemId, NULL, 1.0, 1.0, itemTrail, propertyTrail);
	state.createNewInitials(initial, NULL, reader);

	cout << "#Initials= " << state.getInitials().size() << endl;

	int maxIteration = 10000;
	int iteration = 0;
	bool terminate = false;
	while ((iteration < maxIteration) && (!terminate)) {
		iteration++;
		cout << "Iteration: " << iteration << endl;
		Initial* init = state.getBestChoice(reader);
		processInitial(init, state);
		state.deleteInitial(init);
		terminate = ((globalDelta < topValue) && (topK.size() == 1));
	}
	cout << "initial size: " << state.getInitials().size() << endl;
	cout << topId << ": " << topValue << endl;
	cout << "TOP K Size: " << topK.size() << " terminate:  " << terminate
			<< " Global Delta: " << globalDelta <<  " Top Delta: " << ((topK.size() > 1) ? topK[topId].delta : 0) << endl;
}

void TopKSearch::processInitial(Initial* initial, State& state) {
	Blacklist* bl = new Blacklist();
	bl->setNext(initial->getBlacklist());
	cout << "HERE" << endl;
	// create new initials
	double newOp = state.createNewInitials(initial, bl, reader);

	// compute similarities
	int ip = 0;
	map<int, double>* candidates = hasSimilarity(initial->getPropertyTrail(), initial->getItemTrail(), initial->getOP(), *bl, ip);

	double oldIp = initial->getInpenalty();
	double oldOp = initial->getOP();
	cout << "old OP " << oldOp;
	if ((ip != 0) && (candidates->size() != 0)) {
		double candidatesReduce = oldOp * oldIp;
		double allReduce = candidatesReduce -  (oldOp * ( 1.0 / ( (double) ip + 1.0 )));
		cout << "new OP: " << newOp <<  " --> allReduce: " << allReduce << " candidatesReduce: " << candidatesReduce << endl;
		updateTopK(candidates, allReduce, candidatesReduce, *bl);
	}else{
		cout << " oldIP: " << oldIp << " newIp:" << (1.0 / ip) << " --> candidates size == 0" << endl;
	}

	delete candidates;

}

/**
 * Returns possible targets.
 * Arguments:
 * 	propertyTrail	path to the item
 * 	itemTrail		blacklist of items according to the properties in the property trail
 * 	direction		direction of the first reader to use
 * 	weight			holds the inverted weight for the path in the property trail
 */
map<int, double>* TopKSearch::hasSimilarity(vector<int> propertyTrail,
		vector<int> itemTrail, double weight,
		Blacklist& blacklist, int& inpenalty) {

	cout << "Call hasSimilarity itemTrail ";
	printTrail(itemTrail);
	cout << " OP: " << weight << " PropertyTrail ";
	printTrail(propertyTrail);
	cout << endl;
	map<int, double>* result = new map<int, double>();
	if (propertyTrail.empty()) {
		return result;
	}

	vector<int> inUse;

	int itemId = itemTrail.back();

	vector<vector<int>*> searchTrailTargets;
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
			int id = (*searchTrailTargets.back())[searchTrailPositions.back()];

			// set position in search trail to next element
			searchTrailPositions.back()++;

			int
			propertyTrailPosition = propertyTrail.size()
					- searchTrailTargets.size() - 1;
			if (propertyTrail.size() > searchTrailTargets.size()) { // property path is not at the end
				Item& item = reader.getItemById(id);
//				cout << "find: " << item.getId() << endl;
				if (item.getId() != 0) {
					reader.setInUseFlag(id);
					inUse.push_back(id);

					extendTrails(searchTrailPositions, searchTrailTargets, item,
							propertyTrail[propertyTrailPosition]);
				}
			} else {
				addItemToResult(result, id, itemTrail, weight, &blacklist,
						inpenalty);
			}
		} else {
			// go to upper layer
			searchTrailPositions.pop_back();
			searchTrailTargets.pop_back();

//			cout << "go to upper layer" << endl;
		}
	}

	// unset inUse flags
	for (size_t i = 0; i < inUse.size(); i++) {
		reader.unsetInUseFlag(inUse[i]);
	}

	// multiply in-penalty
	for (map<int, double>::iterator it = result->begin(); it != result->end();
			it++) {
		it->second *= 1.0 / inpenalty;
	}

	return result; // TODO just return unordered_set, weight

}

/**
 * Adds an item and its weight to the result list if the proposed item is not in the blacklist.
 */
void TopKSearch::addItemToResult(map<int, double>* result, int itemId,
		vector<int>& itemTrail, double weight, Blacklist* blacklist,
		int& inpenalty) {

	if (itemTrail[0] == itemId) {
		return; // item is origin
	}

	inpenalty++;

	// check blacklist
	if (blacklist->hasItem(itemId)) {
		return;
	} else {
		blacklist->addItem(itemId);
	}

	// add item if not in blacklist
	(*result)[itemId] = weight;
}

void TopKSearch::extendTrails(vector<int>& searchTrailPositions,
		vector<vector<int>*>& searchTrailTargets, Item& item, int propertyId) {
	vector<StatementGroup>& stmtGrs = item.getStatementGroups();
	for (size_t i = 0; i < stmtGrs.size(); i++) {
		if (stmtGrs[i].getPropertyId() == -propertyId) {
			searchTrailTargets.push_back(&stmtGrs[i].getTargets()); // evt. & weglassen
		}
	}
	if (searchTrailTargets.size() > searchTrailPositions.size()) { // found an element
		searchTrailPositions.push_back(0);
	}
}

#endif /* TOPKSEARCH_H_ */
