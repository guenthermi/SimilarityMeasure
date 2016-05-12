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

#include <vector>
#include <algorithm>

class AStarSearch {
public:

	AStarSearch(IndexReader& inReader, IndexReader& outReader);

	void updateTopK(int* update);
	void updateTopK(int itemId, int value);
	void updateTopK(vector<int*>* candidates);
	void pruneTopK(int min);
	vector<int*>& getTopK();
	int* getBestMatch();
	void search(int itemId);
	vector<int*>* hasSimilarity(vector<int> propertyTrail,
			vector<int> itemTrail, Direction direction, int weight);

protected:

	NodeFinder finder;
	IndexReader& iReader;
	IndexReader& oReader;
	vector<int*> topK;
	int* top;

	void addItemToResult(vector<int*>* result, Item& item,
			vector<int>& blacklist, int weight1, int weight2);
	void extendTrails(vector<int>& searchTrailPositions,
			vector<vector<int>*>& searchTrailTargets, vector<int>& degreeTrail,
			Item& item, int propertyId, int& weight);
	IndexReader* alterReaderDirection(Direction& direction);
	template<typename T> void clearVector(vector<T*>& v);
	static bool cmp(const int* a, const int* b);

};

AStarSearch::AStarSearch(IndexReader& inReader, IndexReader& outReader) :
		oReader(outReader), iReader(inReader), finder(inReader, outReader) {
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
	int* elem = new int[2];
	elem[0] = itemId;
	elem[1] = value;
	topK.push_back(elem);
	if (top[1] < elem[1]){
		top = elem;
	}
}

void AStarSearch::updateTopK(vector<int*>* candidates){
	for (int i=0; i<candidates->size(); i++){
		topK.push_back((*candidates)[i]);
	}
}

void AStarSearch::pruneTopK(int min){
	sort(topK.begin(), topK.end(), cmp);
	for (int i=0; i<topK.size(); i++){
		if (topK[i][1] < min){

		}
	}
	while ((topK.back()[1] < min) && (topK.size() > 0)){
		delete[] topK.back();
		topK.pop_back();
	}
}

vector<int*>& AStarSearch::getTopK() {
	return topK;
}

int* AStarSearch::getBestMatch() {
	return top;
}

void AStarSearch::search(int itemId) {
	Item& out = oReader.getItemById(itemId);
	Item& in = iReader.getItemById(itemId);
	int inDegree = in.getDegree();
	int outDegree = out.getDegree();
	vector<StatementGroup>& stmtGrs = out.getStatementGroups();
	vector<int> itemTrail;
	itemTrail.push_back(itemId);
	for (int i = 0; i < stmtGrs.size(); i++) {
		int pId = stmtGrs[i].getPropertyId();
		vector<int>& targets = stmtGrs[i].getTargets();
		vector<int> propertyTrail;
		propertyTrail.push_back(pId);
		for(int j = 0; j < targets.size(); j++){
			itemTrail.push_back(targets[j]);
			vector<int*>* candidates = hasSimilarity(propertyTrail, itemTrail, outgoing, outDegree);
			updateTopK(candidates);
			delete candidates;
			itemTrail.pop_back();
		}
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
	} else {
		reader = &iReader;
	}

	int itemId = itemTrail.back();

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

	// add item if not in blacklist
	if (!inBlacklist) {
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

IndexReader* AStarSearch::alterReaderDirection(Direction& direction){
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

template<typename T> void AStarSearch::clearVector(vector<T*>& v){
	for (size_t i=0; i<v.size();i++){
		delete[] v[i];
	}
}

bool AStarSearch::cmp(const int* a, const int* b){
	if (a[1] > b[1]){
		return true;
	}else{
		return false;
	}
}

#endif /* ASTARSEARCH_H_ */
