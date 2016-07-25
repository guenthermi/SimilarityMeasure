/*
 * Initial.h
 *
 *  Created on: 03.06.2016
 *      Author: michael
 */

#ifndef INITIAL_H_
#define INITIAL_H_

#include "datamodel/Item.h"
#include "IndexReader.h"


#include <vector>
#include <iostream>

class Initial {
public:
	Initial(IndexReader& reader, int itemId, Blacklist* bl, double op,
			double baseIp, vector<int> itemTrail, vector<int> propertyTrail);
	~Initial();
	void clearState();
	double getPenalty();
	double computePenalty(double* deltaReduce, Item* item = NULL, double minIp = 0);
	double getPenalty(double* deltaReduce, double min);
	double getInpenalty();
	void addToItemTrail(int id);
	void addToPropertyTrail(int id);

	Blacklist* getBlacklist();
	int getItemId();
	double getOP();
	double getBaseIP();
	double getIpMin();
	vector<int>& getItemTrail();
	vector<int>& getPropertyTrail();

protected:

	void extendTrails(vector<int>& searchTrailPositions,
			vector<vector<int>*>& searchTrailTargets, Item& item,
			int propertyId); // TODO share with AStarSearch

	// initial values
	double op;
	double baseIp;
	Blacklist* blacklist;
	int itemId;

	IndexReader& reader;

	// computed values
	double ip;
	double ipMin;
	bool inpenaltyAvailable;
	vector<int> propertyTrail;
	vector<int> itemTrail;

};

Initial::Initial(IndexReader& reader, int itemId, Blacklist* bl, double op,
		double baseIp, vector<int> itemTrail, vector<int> propertyTrail) :
		reader(reader) {
	this->op = op;
	this->baseIp = baseIp;
	this->blacklist = bl;
	this->itemId = itemId;
	this->itemTrail = itemTrail;
	this->propertyTrail = propertyTrail;
	this->inpenaltyAvailable = false;
	this->ipMin = baseIp;
	itemTrail.push_back(itemId);
}

Initial::~Initial() {
}

double Initial::getPenalty() {
	if (inpenaltyAvailable) {
		return ip * op;
	} else {
		return -1;
	}
}

double Initial::getInpenalty() {
	if (inpenaltyAvailable) {
		return ip;
	}else{
		computePenalty(NULL, NULL, 0);
		return ip;
	}
}

/**
 * Returns the penalty value. If the inpenalty is lower than minIp the return value is -1.
 * If there are no candidate solutions the return value is -2.
 * Arguments:
 * 	item	pointer to the target item
 * 	minIp	minimal inpenalty value
 */
double Initial::computePenalty(double* deltaReduce, Item* item, double minIp) {
	int counter = 0; // debug
	if (inpenaltyAvailable) {
		return ip * op;
	}
	if (minIp > ipMin) {
		*deltaReduce = 0;
		return -1;
	}
	vector<int> inUse;
	if (item == NULL) {
		item = &reader.getItemById(itemId);
		if (item->getId() == 0) {
			ip = 0;
			inpenaltyAvailable = true;
			cout << "should not get here" << endl;
			return 0;
		}
		reader.setInUseFlag(itemId);
		inUse.push_back(itemId);
	}
	int resultNumber = 0;
	vector<vector<int>*> searchTrailTargets;
	vector<int> searchTrailPositions;

	extendTrails(searchTrailPositions, searchTrailTargets, *item,
			propertyTrail.back());

	if (searchTrailTargets.size() == 0) { // this should not happen
		cout << "ERROR: should not get here" << endl;
		// unset inUse flags
		for (size_t i = 0; i < inUse.size(); i++) {
			reader.unsetInUseFlag(inUse[i]);
		}
		ip = 0;
		inpenaltyAvailable = true;
		cout << "this should not happen" << endl;
		return 0;
	}
	while (!((searchTrailTargets.size() == 1)
			&& (searchTrailPositions[0] == searchTrailTargets[0]->size()))) { // exit condition
		if (searchTrailTargets.size() < propertyTrail.size()) { // path not at the end
			if (searchTrailPositions.back()
					< searchTrailTargets.back()->size()) { // still elements on this layer
					// go one layer deeper
				int id =
						(*searchTrailTargets.back())[searchTrailPositions.back()];
				Item& nextItem = reader.getItemById(id);
				reader.setInUseFlag(id);
				inUse.push_back(id);

				searchTrailPositions.back()++;int
				propertyTrailPosition = propertyTrail.size()
						- searchTrailTargets.size() - 1;

				extendTrails(searchTrailPositions, searchTrailTargets, nextItem,
						propertyTrail[propertyTrailPosition]);
				continue;
			}
		}
		if (searchTrailTargets.size() == propertyTrail.size()) {
			resultNumber += searchTrailTargets.back()->size();
			if (resultNumber > 1){
				if (((double) 1.0 / (resultNumber-1)) < minIp) {
					double oldIpMin = ipMin;
					ipMin = (double) 1.0 / (resultNumber-1);
					if (deltaReduce){
						// TODO reduce globalDelta
						(*deltaReduce) = (oldIpMin - ipMin)*op;
//						cout << "Reduction: " <<
					}

					return -1;
				}
			}
		}
		// go one layer upper
		searchTrailTargets.pop_back();
		searchTrailPositions.pop_back();
		if (searchTrailTargets.size() == 0) {
			break;
		}
	}
		// unset inUse flags
	for (size_t i = 0; i < inUse.size(); i++) {
		reader.unsetInUseFlag(inUse[i]);
	}
	if (resultNumber == 0) {
		cout << "result number = 0. This should not happen" << endl;
		return 0;
	}

	if (resultNumber < 2) {
		return -2;
	}

	ip = (double) 1.0 / (resultNumber - 1);
	if (ip > baseIp){
		return -2;
	}

	ipMin = ip;
	inpenaltyAvailable = true;
	return ip * op;
}

void Initial::extendTrails(vector<int>& searchTrailPositions,
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

double Initial::getPenalty(double* deltaReduce, double min) {
	double minIp = min / op;
	return computePenalty(deltaReduce, NULL, minIp);
}

void Initial::addToItemTrail(int id) {
	itemTrail.push_back(id);
}

void Initial::addToPropertyTrail(int id) {
	propertyTrail.push_back(id);
}

Blacklist* Initial::getBlacklist() {
	return blacklist;
}
int Initial::getItemId() {
	return itemId;
}
double Initial::getOP() {
	return op;
}
double Initial::getBaseIP(){
	return baseIp;
}

double Initial::getIpMin(){
	return ipMin;
}

vector<int>& Initial::getItemTrail() {
	return itemTrail;
}

vector<int>& Initial::getPropertyTrail() {
	return propertyTrail;
}

#endif /* INITIAL_H_ */
