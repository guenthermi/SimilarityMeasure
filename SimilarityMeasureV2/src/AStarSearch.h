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

#include <vector>

class AStarSearch {
public:

	static const int kK = 100;

	AStarSearch(IndexReader& inReader, IndexReader& outReader);

	void updateTopK(int* update);
	void updateTopK(int itemId, int value);
	int** getTopK();
	int getBestMatch();
	void search(int itemId);
	vector<int*>* hasSimilarity(vector<int> propertyTrail,
			vector<int> itemTrail, Direction direction, int weight);

protected:

	NodeFinder finder;
	IndexReader& iReader;
	IndexReader& oReader;
	int** topK;
	int* top;

	void addItemToResult(vector<int*>* result, Item& item,
			vector<int>& blacklist, int weight1, int weight2);
	void extendTrails(vector<int>& searchTrailPositions,
			vector<vector<int>*>& searchTrailTargets, vector<int>& degreeTrail,
			Item& item, int propertyId, int& weight);

};

AStarSearch::AStarSearch(IndexReader& inReader, IndexReader& outReader) :
		oReader(outReader), iReader(inReader), finder(inReader, outReader) {
	topK = new int*[kK];
	top = new int[2];
	top[0] = 0;
	top[1] = 0;
}

void AStarSearch::updateTopK(int* update) {
	if (update[1] > top[1]) {
		top = update;
	}
}

void AStarSearch::updateTopK(int itemId, int value) {
	int min = 0;
	int minI = 0;
	int i = 0;
	for (i = 0; i < kK; i++) {
		if (topK[i][0] == 0) {
			topK[i][0] = itemId;
			topK[i][1] = value;
			break;
		}
		if (topK[i][0] == itemId) {
			topK[i][1] = value;
			break;
		}
		if (min > topK[i][1]) {
			min = topK[i][1];
			minI = i;
		}
	}
	if (i == kK) {
		topK[minI][0] = itemId;
		topK[minI][1] = value;
	}
	if (value > top[1]) {
		top = itemId;
	}
}

int** AStarSearch::getTopK() {
	return topK;
}

int AStarSearch::getBestMatch() {
	return top;
}

void AStarSearch::search(int itemId) {
	Item& out = oReader.getItemById(itemId);
	Item& in = iReader.getItemById(itemId);
	int inDegree = in.getDegree();
	int outDegree = out.getDegree();
	vector<StatementGroup>& stmtGrs = out.getStatementGroups();
	for (int i = 0; i < stmtGrs.size(); i++) {
		int pId = stmtGrs[i].pId;
		vector<int>& targets = stmtGrs[i].getTargets();
		// untersuche für jedes target, ob ähnlichkeiten exsistieren
		//	-> Berechne Ähnlichkeit für Target und dividiere durch inDegree+outDegree
		// Ähnlichkeit Berechnen -> get a trail
		//TODO: create ItemTrail and PropertyTrail
		//TODO: call hasSimilarity
		// compute Siliarity (add to Similarity)
	}
}

/**
 * Returns possible targets.
 * Arguments:
 * 	propertyTrail	path to the item
 * 	itemTrail		blacklist of items according to the properties in the property trail
 * 	direction		direction of the first reader to use
 * 	weight			holds the inverted weight of the results after execution
 */
vector<int*>* AStarSearch::hasSimilarity(vector<int> propertyTrail,
		vector<int> itemTrail, Direction direction, int weight) {

	vector<int*>* result = new vector<int*>();
	int weight2 = 0;
	if (propertyTrail.empty()) {
		return result;
	}

	IndexReader* reader;
	Direction direct = direction;
	if (direct == outgoing) {
		reader = &oReader;
		direct = incomming;
	} else {
		reader = &iReader;
		direct = outgoing;
	}

	int itemId = itemTrail[itemTrail.size() - 1];

	vector<vector<int>*> searchTrailTargets;
	vector<int> searchTrailPositions;
	vector<int> degreeTrail;

	Item& origin = reader->getItemById(itemId); // the item where the two paths end

	extendTrails(searchTrailPositions, searchTrailTargets, degreeTrail, origin,
			propertyTrail.back(), weight2);

	if (searchTrailTargets.size() == 0) {
		return result;
	}

	// search
	while (!((searchTrailTargets.size() == 1)
			&& (searchTrailPositions[0] == searchTrailTargets[0]->size()))) { // exit condition

		if (searchTrailPositions.back() < searchTrailTargets.back()->size()) {
			// -> there are still elements on this layer

			// read next item
			Item& item = reader->getItemById(
					(*searchTrailTargets.back())[searchTrailPositions.back()]);

			// set position in search trail to next element
			searchTrailPositions.back()++;

			int
			propertyTrailPosition = propertyTrail.size()
					- searchTrailTargets.size();

			if (propertyTrail.size() > searchTrailTargets.size()) { // property path is not at the end
				extendTrails(searchTrailPositions, searchTrailTargets,
						degreeTrail, item, propertyTrail[propertyTrailPosition],
						weight2);
			} else {
				addItemToResult(result, item, itemTrail, weight, weight2);
			}
		} else {
			// go to upper layer
			weight2 /= degreeTrail.back();
			degreeTrail.pop_back();
			searchTrailPositions.pop_back();
			searchTrailTargets.pop_back();
		}

	}

	return result;

}

/**
 * Adds an item and its weight to the result list if the proposed item is not in the blacklist.
 */
void AStarSearch::addItemToResult(vector<int*>* result, Item& item,
		vector<int>& blacklist, int weight1, int weight2) {

	// check blacklist
	bool inBlacklist = false;
	for (int i = 0; i < blacklist.size(); i++) {
		if (blacklist[i] == item.getId()) {
			inBlacklist = true;
			break;
		}
	}

	// add item
	if (!inBlacklist) {
		// add item to results
		int* elem = new int[2];
		elem[0] = item.getId();
		elem[1] = weight1 * weight2;
		result->push_back(elem);
	}
}

void AStarSearch::extendTrails(vector<int>& searchTrailPositions,
		vector<vector<int>*>& searchTrailTargets, vector<int>& degreeTrail,
		Item& item, int propertyId, int& weight) {
	vector<StatementGroup>& stmtGrs = item.getStatementGroups();
	for (int i = 0; i < stmtGrs.size(); i++) {
		if (stmtGrs[i].getPropertyId() == propertyId) {
			searchTrailTargets.push_back(&stmtGrs[i].getTargets()); // evt. & weglassen
			weight *= item.getDegree();
			degreeTrail.push_back(item.getDegree());
			weight *= degreeTrail.back();
		}
	}
	if (searchTrailTargets.size() > searchTrailPositions.size()) { // found an element
		searchTrailPositions.push_back(0);
	}
}

#endif /* ASTARSEARCH_H_ */
