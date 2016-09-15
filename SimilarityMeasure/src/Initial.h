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
	double computePenalty(double* deltaReduce, Item* item = NULL, double minIp = 0);
	double getPenalty(double* deltaReduce, double min);
	double getInpenalty();

	Blacklist* getBlacklist();
	int getItemId();
	double getOP();
	vector<int>& getItemTrail();
	vector<int>& getPropertyTrail();

protected:

	void extendTrails(vector<int>& searchTrailPositions,
			vector<StatementGroup*>& searchTrailTargets, Item& item,
			int propertyId); // TODO share with AStarSearch

	// initial values
	double op;
	Blacklist* blacklist;
	int itemId;

	IndexReader& reader;

	// computed values
	double ipMin;
	bool inpenaltyAvailable;
	vector<int> propertyTrail;
	vector<int> itemTrail;

};

Initial::Initial(IndexReader& reader, int itemId, Blacklist* bl, double op,
		double baseIp, vector<int> itemTrail, vector<int> propertyTrail) :
		reader(reader) {
	this->op = op;
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

double Initial::getInpenalty() {
	if (inpenaltyAvailable) {
		return ipMin;
	}else{
		if (itemTrail.size() == 1){
			ipMin = 0;
			inpenaltyAvailable = true;
			return ipMin;
		}
		computePenalty(NULL, NULL, 0);
		return ipMin;
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
		return ipMin * op;
	}
	if (minIp > ipMin) {
		*deltaReduce = 0;
		return -1;
	}

	vector<bool*> inUse;
	if (item == NULL) {
		bool* flag;
		item = &reader.getItemById(itemId, flag);
		if (item->getId() == 0) {
			ipMin = 0;
			inpenaltyAvailable = true;
			cout << "should not get here" << endl;
			return 0;
		}
		*flag = true;
		inUse.push_back(flag);
	}
	int resultNumber = 0;
	vector<StatementGroup*> searchTrailTargets;
	vector<int> searchTrailPositions;

	extendTrails(searchTrailPositions, searchTrailTargets, *item,
			propertyTrail.back());

	if (searchTrailTargets.size() == 0) { // this should not happen
		cout << "ERROR: should not get here" << endl;
		// unset inUse flags
		for (size_t i = 0; i < inUse.size(); i++) {
			*inUse[i] = false;
		}
		ipMin = 0;
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
						(*searchTrailTargets.back()).getTargets()[searchTrailPositions.back()];
				bool* flag;
				Item& nextItem = reader.getItemById(id, flag);
				*flag = true;
				inUse.push_back(flag);

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
						(*deltaReduce) = (oldIpMin - ipMin)*op;
					}
					// unset inUse flags
					for (size_t i = 0; i < inUse.size(); i++) {
						*inUse[i] = false;
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
		*inUse[i] = false;
	}
	if (resultNumber == 0) {
		cout << "result number = 0. This should not happen" << endl;
		return 0;
	}

	if (resultNumber < 2) {
		ipMin = 0;
		inpenaltyAvailable = true;
		return -2;
	}

	double ip = (double) 1.0 / (resultNumber - 1);
	if (ip > ipMin){
		ipMin = ip;
		inpenaltyAvailable = true;
		return -2;
	}
	ipMin = ip;
	inpenaltyAvailable = true;
	return ip * op;
}

void Initial::extendTrails(vector<int>& searchTrailPositions,
		vector<StatementGroup*>& searchTrailTargets, Item& item, int propertyId) {
	StatementGroup* stmtGrs = item.getStatementGroups();
	for (size_t i = 0; i < item.size(); i++) {
		if (stmtGrs[i].getPropertyId() == -propertyId) {
			searchTrailTargets.push_back(&stmtGrs[i]); // evt. & weglassen
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

Blacklist* Initial::getBlacklist() {
	return blacklist;
}
int Initial::getItemId() {
	return itemId;
}
double Initial::getOP() {
	return op;
}

vector<int>& Initial::getItemTrail() {
	return itemTrail;
}

vector<int>& Initial::getPropertyTrail() {
	return propertyTrail;
}

#endif /* INITIAL_H_ */
