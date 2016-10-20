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
#include <algorithm>

using namespace std;

class Initial {
public:

//	static const int maxCacheSize = 7000000;

	Initial(IndexReader& reader, int itemId, Blacklist* bl, double op,
			double baseIp, vector<int> itemTrail, vector<int> propertyTrail);
	~Initial();
	double computePenalty(int& debug, double* deltaReduce, Item* item = NULL, double minIp = 0, int maxEffort = 0x7FFFFFFF);
	double getPenalty(int& debug, double* deltaReduce, double min, int maxEffort);
	double getInpenalty();

	double getBaseIp();
	Blacklist* getBlacklist();
	int getItemId();
	double getOP();
	int getEffort();
	vector<int>& getItemTrail();
	vector<int>& getPropertyTrail();

protected:

	bool extendTrails(vector<int>& searchTrailPositions,
			vector<StatementGroup*>& searchTrailTargets, Item& item,
			int propertyId); // TODO share with AStarSearch

	// initial values
	double op;
	Blacklist* blacklist;
	int itemId;
	double baseIp;

	IndexReader& reader;

	// computed values
	double ipMin;
	int effort;
	bool inpenaltyAvailable;
	vector<int> propertyTrail;
	vector<int> itemTrail;

};

Initial::Initial(IndexReader& reader, int itemId, Blacklist* bl, double op,
		double baseIp, vector<int> itemTrail, vector<int> propertyTrail) :
		reader(reader) {
	if (itemId == 0){
		cout << "error item id == 0" << endl;
		while(1);
	}
	this->op = op;
	this->blacklist = bl;
	this->itemId = itemId;
	this->itemTrail = itemTrail;
	this->propertyTrail = propertyTrail;
	this->inpenaltyAvailable = false;
	this->baseIp = baseIp;
	this->ipMin = baseIp;
	this->effort = 0;
	itemTrail.push_back(itemId);
}

Initial::~Initial() {
	if (blacklist != NULL){
		delete blacklist;
	}
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
		int tmp =0;
		computePenalty(tmp, NULL, NULL, 0, 0x7FFFFFFF);
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
double Initial::computePenalty(int& debug, double* deltaReduce, Item* item, double minIp, int maxEffort) {
	if (inpenaltyAvailable) {
		return ipMin * op;
	}
	if (minIp > ipMin) {
		*deltaReduce = 0;
		return -1;
	}
	if (effort >  maxEffort){
		*deltaReduce = 0;
		return -1;
	}

	vector<int> inUse;
	if (item == NULL) {
		item = &reader.getItemById(itemId);
		if (item->getId() == 0) {
			ipMin = 0;
			inpenaltyAvailable = true;
			cout << "should not get here" << endl;
			return 0;
		}
		reader.setInUseFlag(itemId);
		inUse.push_back(itemId);
	}
	effort = 0;
	int resultNumber = 0;
	vector<StatementGroup*> searchTrailTargets;
	vector<int> searchTrailPositions;

	extendTrails(searchTrailPositions, searchTrailTargets, *item,
			propertyTrail.back());

	if (searchTrailTargets.size() == 0) { // this should not happen
		cout << "ERROR: should not get here" << endl;
		// unset inUse flags
		for (size_t i = 0; i < inUse.size(); i++) {
			reader.unsetInUseFlag(inUse[i]);
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
				Item& nextItem = reader.getItemById(id);
				if (reader.setInUseFlag(id)){
					inUse.push_back(id);
				}else{
					inUse.push_back(0);
				}
				effort++;

				searchTrailPositions.back()++;
				int propertyTrailPosition = propertyTrail.size()
						- searchTrailTargets.size() - 1;

				if (!extendTrails(searchTrailPositions, searchTrailTargets, nextItem,
						propertyTrail[propertyTrailPosition])){
					reader.unsetInUseFlag(inUse.back());
					inUse.pop_back();
				}
				continue;
			}
		}
		if ( effort > (maxEffort* (1.0 / minIp) ) ) {
			debug++;
			if (resultNumber > 1){
				double oldIpMin = ipMin;
				ipMin = (double) 1.0 / (resultNumber-1);
				if ((deltaReduce) && (oldIpMin > ipMin)){
					(*deltaReduce) = (oldIpMin - ipMin)*op;
				}
			}
			// unset inUse flags
			for (size_t i = 0; i < inUse.size(); i++) {
				reader.unsetInUseFlag(inUse[i]);
			}
			inpenaltyAvailable = true;
			return -1;
		}
		if (searchTrailTargets.size() == propertyTrail.size()) {
			resultNumber += searchTrailTargets.back()->size();
			if (resultNumber > 1){
				if ( ((double) 1.0 / (resultNumber-1)) < minIp ) {
					if (resultNumber > 1){
						double oldIpMin = ipMin;
						ipMin = (double) 1.0 / (resultNumber-1);
						if (deltaReduce){
							(*deltaReduce) = (oldIpMin - ipMin)*op;
						}
					}
					// unset inUse flags
					for (size_t i = 0; i < inUse.size(); i++) {
						reader.unsetInUseFlag(inUse[i]);
					}
					inpenaltyAvailable = true;
					return -1;
				}
			}
		}
		// go one layer upper
		searchTrailTargets.pop_back();
		searchTrailPositions.pop_back();
		reader.unsetInUseFlag(inUse.back());
		inUse.pop_back();
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
//	cout << "return not -1 " << endl;
	return ip * op;
}

bool Initial::extendTrails(vector<int>& searchTrailPositions,
		vector<StatementGroup*>& searchTrailTargets, Item& item, int propertyId) {
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

double Initial::getPenalty(int& debug, double* deltaReduce, double min, int maxEffort) {
	double minIp = min / op;
	return computePenalty(debug, deltaReduce, NULL, minIp, maxEffort);
}

double Initial::getBaseIp(){
	return baseIp;
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

int Initial::getEffort(){
	return effort;
}

vector<int>& Initial::getItemTrail() {
	return itemTrail;
}

vector<int>& Initial::getPropertyTrail() {
	return propertyTrail;
}

#endif /* INITIAL_H_ */
