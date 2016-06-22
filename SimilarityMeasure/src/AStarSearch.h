/*
 * AStarSearch.h
 *
 *  Created on: 09.05.2016
 *      Author: michael
 */

#ifndef ASTARSEARCH_H_
#define ASTARSEARCH_H_

#include "NodeFinder.h"
#include "Direction.h"
#include "IndexReader.h"
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

class AStarSearch {
public:

	AStarSearch(IndexReader& inReader, IndexReader& outReader);
	~AStarSearch();

	void updateTopK(map<int, double>* candidates, double gReduction,
			double cReduction, Blacklist& bl);
//	void pruneTopK(int min);
	unordered_map<int, TopKEntry>& getTopK();
	int getBestMatch();
	double getBestMatchValue();
	void search(int itemId);
	void processInitial(Initial* initial, State& state);
	map<int, double>* hasSimilarity(vector<int> propertyTrail,
			vector<int> itemTrail, Direction direction, double weight,
			Blacklist& blacklist, int& inpenalty);

protected:

	IndexReader& iReader;
	IndexReader& oReader;
	NodeFinder finder;
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
	IndexReader* alterReaderDirection(Direction& direction);
	template<typename T> void clearVector(vector<T*>& v);
	static bool cmp(const int* a, const int* b);

	// debug
	void printTrail(vector<int>& trail) {
		cout << "[ ";
		for (size_t i = 0; i < trail.size(); i++) {
			cout << trail[i] << " ";
		}
		cout << "]" << endl;
	}

};

AStarSearch::AStarSearch(IndexReader& inReader, IndexReader& outReader) :
		iReader(inReader), oReader(outReader), finder(inReader, outReader) {
	topId = 0;
	topValue = 0;
	globalDelta = 1;
}

AStarSearch::~AStarSearch() {
}

void AStarSearch::updateTopK(map<int, double>* candidates, double gReduction,
		double cReduction, Blacklist& bl) {
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

	for (unordered_map<int, TopKEntry>::iterator it = topK.begin(); it != topK.end();
			it++) {
		if (bl.hasItem(it->first)){
			continue;
		}
		TopKEntry& value = it->second;
		value.delta -= gReduction;
		if (value.delta < 0){
			cout << "################ FEHLER #####################" << endl;
		}
		if (value.delta < (topValue - value.weight)) {

			toErase.push_back(it->first);
		}
	}
	for (size_t i = 0; i < toErase.size(); i++) {
		topK.erase(toErase[i]);
	}
	globalDelta -= gReduction;
}

//void AStarSearch::pruneTopK(int min) {
//	vector<int> ids;
//	for (map<int, double>::iterator ii = topK.begin(); ii != topK.end(); ii++) {
//		if (ii->second < min) {
//			ids.push_back(ii->first);
//		}
//	}
//	for (size_t i = 0; i < ids.size(); i++) {
//		topK.erase(ids[i]);
//	}
//}

unordered_map<int, TopKEntry>& AStarSearch::getTopK() {
	return topK;
}

int AStarSearch::getBestMatch() {
	return topId;
}

double AStarSearch::getBestMatchValue() {
	return topValue;
}

void AStarSearch::search(int itemId) {
	Item& out = oReader.getItemById(itemId);
	Item& in = iReader.getItemById(itemId);

	int inDegree = in.getDegree();
	int outDegree = out.getDegree();

	Initial* initialIn = new Initial(iReader, itemId, NULL,
			(double) inDegree / (inDegree + outDegree), incomming,
			vector<int>(), vector<int>(), &in);
	Initial* initialOut = new Initial(oReader, itemId, NULL,
			(double) outDegree / (inDegree + outDegree), outgoing,
			vector<int>(), vector<int>(), &out);
	initialIn->addToItemTrail(itemId);
	initialOut->addToItemTrail(itemId);

	State state;
	state.addInitial(initialIn);
	state.addInitial(initialOut);

	int maxIteration = 10;
	int iteration = 0;
	bool terminate = false;
	while ((iteration < maxIteration) && (!terminate)) {
		iteration++;
		cout << "Iteration: " << iteration << endl;
		Initial* init = state.getBestChoice();
		cout << "best choise: " << (double) ((double) 1.0 / init->getBaseOP()) * init->getItemDegree()  << " Degree Only: " << init->getItemDegree() << endl;
		processInitial(init, state);
		terminate = ((globalDelta < topValue) && (topK.size() == 1));
	}

	cout << topId << ": " << topValue << endl;
	cout << "TOP K Size: " << topK.size() << " terminate:  " << terminate
			<< " Global Delta: " << globalDelta << endl;
}

void AStarSearch::processInitial(Initial* initial, State& state) {
	StatementGroup& stmtGr = initial->getNextStmtGr();
	int pId = stmtGr.getPropertyId();
	vector<int>& targetsTmp = stmtGr.getTargets();
	vector<int> targets;
	targets.assign(targetsTmp.begin(), targetsTmp.end());
	vector<int> itemTrail = initial->getItemTrail();
	vector<int> propertyTrail = initial->getPropertyTrail();
	int itemDegree = initial->getItemDegree();
	propertyTrail.push_back(pId);
	for (size_t i = 0; i < targets.size(); i++) {
		Blacklist* bl = new Blacklist();
		bl->setNext(initial->getBlacklist());
		double op = initial->getBaseOP() * (double) (1.0 / itemDegree);
		itemTrail.push_back(targets[i]);

		Item* newItem;
		IndexReader* initialReader;
		if (initial->getDirection() == incomming) {
			initialReader = &iReader;
			newItem = &iReader.getItemById(targets[i]);
		} else {
			initialReader = &oReader;
			newItem = &oReader.getItemById(targets[i]);
		}
		Initial* newInitial = new Initial(*initialReader, newItem->getId(), bl,
				initial->getBaseOP() / itemDegree, initial->getDirection(),
				itemTrail, propertyTrail, newItem);
		state.addInitial(newInitial);

		int ip = 0;

		map<int, double>* candidates = hasSimilarity(propertyTrail, itemTrail,
				(initial->getDirection() == incomming) ? outgoing : incomming,
				op, *bl, ip);
		cout << "candidates size: " << candidates->size() << endl;
		if (candidates->size() > 0) {
			cout << candidates->begin()->first << endl;
		}
		if (candidates->size() > 1) {
			cout << (++candidates->begin())->first << endl;
		}

		if ((ip != 0) && (candidates->size() != 0)) {
			double allReduce = (op
					* (1.0 / (double) (1 + ip - candidates->size())))
					- (op * (1.0 / (double) (1 + ip)));
			double candidatesReduce = (op
					* (1.0 / (double) (1 + ip - candidates->size())));
			cout << "allReduce: " << allReduce << " candidatesReduce: "
					<< candidatesReduce << endl;
			updateTopK(candidates, allReduce, candidatesReduce, *bl);
		}

		delete candidates;
		itemTrail.pop_back();
	}

}

/**
 * Returns possible targets.
 * Arguments:
 * 	propertyTrail	path to the item
 * 	itemTrail		blacklist of items according to the properties in the property trail
 * 	direction		direction of the first reader to use
 * 	weight			holds the inverted weight for the path in the property trail
 */
map<int, double>* AStarSearch::hasSimilarity(vector<int> propertyTrail,
		vector<int> itemTrail, Direction direction, double weight,
		Blacklist& blacklist, int& inpenalty) {

	cout << "Call hasSimilarity itemTrail ";
	printTrail(itemTrail);
	cout << "W: " << weight << " PropertyTrail ";
	printTrail(propertyTrail);
	cout << endl;
	map<int, double>* result = new map<int, double>();
	if (propertyTrail.empty()) {
		return result;
	}

	vector<int> inUse;

	IndexReader* reader;
	IndexReader* invReader;
	Direction direct = direction;
	if (direct == outgoing) {
		reader = &oReader;
		invReader = &iReader;
	} else {
		reader = &iReader;
		invReader = &oReader;
	}

	int itemId = itemTrail.back();

	vector<vector<int>*> searchTrailTargets;
	vector<int> searchTrailPositions;

	Item& origin = reader->getItemById(itemId); // the item where the two paths end

	if (origin.getId() == 0) {
		return result; // null item -> not valid
	}else{
		reader->setInUseFlag(itemId);
		inUse.push_back(itemId);
	}

	extendTrails(searchTrailPositions, searchTrailTargets, origin,
			propertyTrail.back());

	if (searchTrailTargets.size() == 0) {
		return result;
	}

	// search
	while (!((searchTrailTargets.size() == 1)
			&& (searchTrailPositions[0] == searchTrailTargets[0]->size()))) { // exit condition
		if (searchTrailPositions.back() < searchTrailTargets.back()->size()) {
			// -> there are still elements on this layer

			// read next item
			// TODO care about case that there is no item in the index
			int id = (*searchTrailTargets.back())[searchTrailPositions.back()];

			// set position in search trail to next element
			searchTrailPositions.back()++;

			int
			propertyTrailPosition = propertyTrail.size()
					- searchTrailTargets.size() - 1;
			if (propertyTrail.size() > searchTrailTargets.size()) { // property path is not at the end
				Item& item = reader->getItemById(id);
//				cout << "find: " << item.getId() << endl;
				if (item.getId() != 0) {
					reader->setInUseFlag(id);
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
		reader->unsetInUseFlag(inUse[i]);
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
void AStarSearch::addItemToResult(map<int, double>* result, int itemId,
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

void AStarSearch::extendTrails(vector<int>& searchTrailPositions,
		vector<vector<int>*>& searchTrailTargets, Item& item, int propertyId) {
	vector<StatementGroup>& stmtGrs = item.getStatementGroups();
	for (size_t i = 0; i < stmtGrs.size(); i++) {
		if (stmtGrs[i].getPropertyId() == propertyId) {
			searchTrailTargets.push_back(&stmtGrs[i].getTargets()); // evt. & weglassen
		}
	}
	if (searchTrailTargets.size() > searchTrailPositions.size()) { // found an element
		searchTrailPositions.push_back(0);
	}
}

IndexReader* AStarSearch::alterReaderDirection(Direction& direction) {
	IndexReader* reader;
	if (direction == outgoing) {
		reader = &oReader;
		direction = incomming;
	} else {
		reader = &iReader;
		direction = outgoing;
	}
	return reader;
}

template<typename T> void AStarSearch::clearVector(vector<T*>& v) {
	for (size_t i = 0; i < v.size(); i++) {
		delete[] v[i];
	}
}

bool AStarSearch::cmp(const int* a, const int* b) {
	if (a[1] > b[1]) {
		return true;
	} else {
		return false;
	}
}

#endif /* ASTARSEARCH_H_ */
