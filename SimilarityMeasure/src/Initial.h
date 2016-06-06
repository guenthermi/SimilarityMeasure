/*
 * Initial.h
 *
 *  Created on: 03.06.2016
 *      Author: michael
 */

#ifndef INITIAL_H_
#define INITIAL_H_

#include "datamodel/Item.h"
#include "Direction.h"

#include <vector>

class Initial {
public:
	Initial(Item& item, Blacklist* bl, double baseOP, Direction d, vector<int> itemTrail, vector<int> propertyTrail);
	void clearState();
	bool isProcessed();
	StatementGroup& getNextStmtGr();
	int getLowesetDegree();
	void addToItemTrail(int id);
	void addToPropertyTrail(int id);

	Direction getDirection();
	Blacklist* getBlacklist();
	Item& getItem();
	double getBaseOP();
	vector<int>& getItemTrail();
	vector<int>& getPropertyTrail();

protected:

	// initial values
	double baseOP;
	Direction direction;
	Blacklist* blacklist;
	Item item;

	// computed values
	int lowest;
	vector<int> propertyTrail;
	vector<int> itemTrail;

};

Initial::Initial(Item& item, Blacklist* bl, double baseOP, Direction d, vector<int> itemTrail, vector<int> propertyTrail) :
		item(item) {
	this->baseOP = baseOP;
	this->direction = d;
	this->blacklist = bl;
	this->itemTrail = itemTrail;
	this->propertyTrail = propertyTrail;
	itemTrail.push_back(item.getId());
	clearState();
}

void Initial::clearState() {
	item.sortStmtGrsBySize();
	lowest = 0;
}

bool Initial::isProcessed() {
	return (lowest >= item.getStatementGroups().size());
}

StatementGroup& Initial::getNextStmtGr() {
	lowest++;
	return item.getStatementGroups()[lowest - 1];
}

int Initial::getLowesetDegree() {
	if (isProcessed()) {
		return -1;
	}
	return item.getStatementGroups()[lowest].getTargets().size();
}

void Initial::addToItemTrail(int id) {
	itemTrail.push_back(id);
}

void Initial::addToPropertyTrail(int id) {
	propertyTrail.push_back(id);
}

Direction Initial::getDirection() {
	return direction;
}
Blacklist* Initial::getBlacklist() {
	return blacklist;
}
Item& Initial::getItem() {
	return item;
}
double Initial::getBaseOP() {
	return baseOP;
}

vector<int>& Initial::getItemTrail() {
	return itemTrail;
}

vector<int>& Initial::getPropertyTrail() {
	return propertyTrail;
}

#endif /* INITIAL_H_ */
