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

#include <vector>
#include <map>
#include <algorithm>

class AStarSearch {
public:

	AStarSearch(IndexReader& inReader, IndexReader& outReader);
	~AStarSearch();

	void updateTopK(vector<int*>* candidates);
	void pruneTopK(int min);
	map<int, double>& getTopK();
	int getBestMatch();
	double getBestMatchValue();
	void search(int itemId);
	vector<int*>* hasSimilarity(vector<int> propertyTrail,
			vector<int> itemTrail, Direction direction, int weight,
			Blacklist& blacklist);

protected:
	IndexReader& iReader;
	IndexReader& oReader;
	NodeFinder finder;
	map<int, double> topK;
	int topId;
	double topValue;

	void addItemToResult(vector<int*>* result, int itemId,
			vector<int>& itemTrail, int weight, Blacklist* blacklist);
	void extendTrails(vector<int>& searchTrailPositions,
			vector<vector<int>*>& searchTrailTargets, vector<int>& degreeTrail,
			Item& item, int propertyId);
	IndexReader* alterReaderDirection(Direction& direction);
	template<typename T> void clearVector(vector<T*>& v);
	static bool cmp(const int* a, const int* b);

};

AStarSearch::AStarSearch(IndexReader& inReader, IndexReader& outReader) :
		iReader(inReader), oReader(outReader), finder(inReader, outReader) {
	topId = 0;
	topValue = 0;
}

AStarSearch::~AStarSearch() {
}

void AStarSearch::updateTopK(vector<int*>* candidates) {
	for (size_t i = 0; i < candidates->size(); i++) {
		double& value = topK[(*candidates)[i][0]];
		if (value == 0) {
			value = (double) 1 / (*candidates)[i][1];
		} else {
			value += (double) 1 / (*candidates)[i][1];
		}
		if (value > topValue) {
			topValue = value;
			topId = (*candidates)[i][0];
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
	//int inDegree = in.getDegree();
	int outDegree = out.getDegree();
	vector<StatementGroup>& stmtGrs = out.getStatementGroups();
	vector<int> itemTrail;
	itemTrail.push_back(itemId);

	for (size_t i = 0; i < stmtGrs.size(); i++) {
		int pId = stmtGrs[i].getPropertyId();
		vector<int>& targets = stmtGrs[i].getTargets();
		vector<int> propertyTrail;
		propertyTrail.push_back(pId);
		for (size_t j = 0; j < targets.size(); j++) {
			itemTrail.push_back(targets[j]);
			Blacklist blacklist;

			vector<int*>* candidates = hasSimilarity(propertyTrail, itemTrail,
					incomming, outDegree, blacklist);
			cout << "candidates size: " << candidates->size() << endl;
			if (candidates->size() > 1) {
				cout << (*candidates)[0][0] << endl;
			}
			if (candidates->size() == 1) {
				cout << *candidates[0][0] << endl;
			}
			updateTopK(candidates);
			clearVector(*candidates);
			delete candidates;
			itemTrail.pop_back();
		}
	}
	cout << topId << ": " << topValue << endl;
}

/**
 * Returns possible targets.
 * Arguments:
 * 	propertyTrail	path to the item
 * 	itemTrail		blacklist of items according to the properties in the property trail
 * 	direction		direction of the first reader to use
 * 	weight			holds the inverted weight for the path in the property trail
 */
vector<int*>* AStarSearch::hasSimilarity(vector<int> propertyTrail,
		vector<int> itemTrail, Direction direction, int weight,
		Blacklist& blacklist) {
	cout << "Call hasSimilarity " << itemTrail[0] << "W: " << weight
			<< " Property" << propertyTrail[0] << endl;
	vector<int*>* result = new vector<int*>();
	if (propertyTrail.empty()) {
		return result;
	}

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
	vector<int> degreeTrail;

	Item& origin = reader->getItemById(itemId); // the item where the two paths end

	if (origin.getId() == 0) {
		return result; // null item -> not valid
	}

	extendTrails(searchTrailPositions, searchTrailTargets, degreeTrail, origin,
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
					- searchTrailTargets.size();

			if (propertyTrail.size() > searchTrailTargets.size()) { // property path is not at the end
				extendTrails(searchTrailPositions, searchTrailTargets,
						degreeTrail, item,
						propertyTrail[propertyTrailPosition]);
			} else {
				addItemToResult(result, id, itemTrail, weight,
						&blacklist);
			}
		} else {
			// go to upper layer
			degreeTrail.pop_back();
			searchTrailPositions.pop_back();
			searchTrailTargets.pop_back();

			cout << "go to upper layer" << endl;
		}

	}

	// multiply in-penalty
	for (size_t i = 0; i < result->size(); i++) {
		(*result)[i][1] *= result->size();
	}

	return result;

}

/**
 * Adds an item and its weight to the result list if the proposed item is not in the blacklist.
 */
void AStarSearch::addItemToResult(vector<int*>* result, int itemId,
		vector<int>& itemTrail, int weight, Blacklist* blacklist) {

	// check item trail
	bool inItemTrail = false;
	for (size_t i = 0; i < itemTrail.size(); i++) {
		if (itemTrail[i] == itemId) {
			return; // item is in item trail -> do not add
		}
	}

	// check blacklist
	if (blacklist->hasItem(itemId)) {
		return;
	} else {
		blacklist->addItem(itemId);
	}

	// add item if not in item trail or blacklist
	if (!inItemTrail) {
		int* elem = new int[2];
		elem[0] = itemId;
		elem[1] = weight;
		result->push_back(elem);
	}

}

void AStarSearch::extendTrails(vector<int>& searchTrailPositions,
		vector<vector<int>*>& searchTrailTargets, vector<int>& degreeTrail,
		Item& item, int propertyId) {
	vector<StatementGroup>& stmtGrs = item.getStatementGroups();
	for (size_t i = 0; i < stmtGrs.size(); i++) {
		if (stmtGrs[i].getPropertyId() == propertyId) {
			searchTrailTargets.push_back(&stmtGrs[i].getTargets()); // evt. & weglassen
//			cout << "ItemId: " << item.getId() << "propertyId: " << propertyId << ": Degree: " << item.getDegree() << endl;
			degreeTrail.push_back(stmtGrs[i].getTargets().size() - 1);
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
