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

#include <vector>
#include <map>
#include <algorithm>

class AStarSearch {
public:

	AStarSearch(IndexReader& inReader, IndexReader& outReader);
	~AStarSearch();

	void updateTopK(map<int, double>* candidates);
	void pruneTopK(int min);
	map<int, double>& getTopK();
	int getBestMatch();
	double getBestMatchValue();
	void search(int itemId);
	void processInitial(Initial* initial, State& state);
	map<int, double>* hasSimilarity(vector<int> propertyTrail,
			vector<int> itemTrail, Direction direction, double weight,
			Blacklist& blacklist);

protected:
	IndexReader& iReader;
	IndexReader& oReader;
	NodeFinder finder;
	map<int, double> topK;
	int topId;
	double topValue;

	void addItemToResult(map<int, double>* result, int itemId,
			vector<int>& itemTrail, double weight, Blacklist* blacklist, int& inpenalty);
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
}

AStarSearch::~AStarSearch() {
}

void AStarSearch::updateTopK(map<int, double>* candidates) {
	for (map<int, double>::iterator it = candidates->begin();
			it != candidates->end(); it++) {
		double& value = topK[it->first];
		value += it->second;
		if (value > topValue) {
			topValue = value;
			topId = it->first;
		}
	}
}

void AStarSearch::pruneTopK(int min) {
	vector<int> ids;
	for (map<int, double>::iterator ii = topK.begin(); ii != topK.end(); ii++) {
		if (ii->second < min) {
			ids.push_back(ii->first);
		}
	}
	for (size_t i = 0; i < ids.size(); i++) {
		topK.erase(ids[i]);
	}
}

map<int, double>& AStarSearch::getTopK() {
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

	Initial* initialIn = new Initial(in, NULL,
			(double) inDegree / (inDegree + outDegree), incomming,
			vector<int>(), vector<int>());
	Initial* initialOut = new Initial(out, NULL,
			(double) outDegree / (inDegree + outDegree), outgoing,
			vector<int>(), vector<int>());
	initialIn->addToItemTrail(itemId);
	initialOut->addToItemTrail(itemId);

	State state;
	state.addInitial(initialIn);
	state.addInitial(initialOut);

	int maxIteration = 10;
	int iteration = 0;
	while (iteration <= maxIteration) {
		iteration++;
		cout << "Iteration: " << iteration << endl;
		Initial* init = state.getBestChoice();
		cout << "best choise: " << init->getLowesetDegree() << endl;
		processInitial(init, state);
	}

	cout << topId << ": " << topValue << endl;
}

void AStarSearch::processInitial(Initial* initial, State& state) {
	StatementGroup& stmtGr = initial->getNextStmtGr();
	int pId = stmtGr.getPropertyId();
	vector<int>& targets = stmtGr.getTargets();
	vector<int> itemTrail = initial->getItemTrail();
	vector<int> propertyTrail = initial->getPropertyTrail();
	int itemDegree = initial->getItem().getDegree();
	propertyTrail.push_back(pId);
	for (size_t i = 0; i < targets.size(); i++) {
		Blacklist* bl = new Blacklist();
		bl->setNext(initial->getBlacklist());
		double op = initial->getBaseOP() * (double) (1.0 / itemDegree);
		itemTrail.push_back(targets[i]);

		Item* newItem;
		if (initial->getDirection() == incomming) {
			newItem = &iReader.getItemById(targets[i]);
		} else {
			newItem = &oReader.getItemById(targets[i]);
		}
		Initial* newInitial = new Initial(*newItem, bl,
				initial->getBaseOP() / itemDegree, initial->getDirection(),
				itemTrail, propertyTrail);
		state.addInitial(newInitial);

		int inpenalty = 0;
		map<int, double>* candidates = hasSimilarity(propertyTrail, itemTrail,
				(initial->getDirection() == incomming) ? outgoing : incomming,
				op, *bl);
		cout << "candidates size: " << candidates->size() << endl;
		if (candidates->size() > 0) {
			cout << candidates->begin()->first << endl;
		}
		if (candidates->size() > 1) {
			cout << (++candidates->begin())->first << endl;
		}
		updateTopK(candidates);

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
		Blacklist& blacklist) {

	cout << "Call hasSimilarity itemTrail ";
	printTrail(itemTrail);
	cout << "W: " << weight << " PropertyTrail ";
	printTrail(propertyTrail);
	cout << endl;
	map<int, double>* result = new map<int, double>();
	if (propertyTrail.empty()) {
		return result;
	}

	int inpenalty;

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
			Item& item = reader->getItemById(id);

			// set position in search trail to next element
			searchTrailPositions.back()++;

			int
			propertyTrailPosition = propertyTrail.size()
					- searchTrailTargets.size() - 1;
			if (propertyTrail.size() > searchTrailTargets.size()) { // property path is not at the end
				extendTrails(searchTrailPositions, searchTrailTargets, item,
						propertyTrail[propertyTrailPosition]);
			} else {
				addItemToResult(result, id, itemTrail, weight, &blacklist, inpenalty);
			}
		} else {
			// go to upper layer
			searchTrailPositions.pop_back();
			searchTrailTargets.pop_back();

//			cout << "go to upper layer" << endl;
		}
	}
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
void AStarSearch::addItemToResult(map<int, double>* result, int itemId,
		vector<int>& itemTrail, double weight, Blacklist* blacklist, int& inpenalty) {

	// check item trail
	bool inItemTrail = false;
	for (size_t i = 0; i < itemTrail.size(); i++) {
		if (itemTrail[i] == itemId) {
			return; // item is in item trail -> do not add
		}
	}

	inpenalty++;

	// check blacklist
	if (blacklist->hasItem(itemId)) {
		return;
	} else {
		blacklist->addItem(itemId);
	}

	// add item if not in item trail or blacklist
	if (!inItemTrail) {
		(*result)[itemId] = weight;
	}
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
