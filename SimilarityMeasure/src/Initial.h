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

class Initial {
public:
	Initial(IndexReader& reader, int itemId, Blacklist* bl, double baseOP,
			vector<int> itemTrail, vector<int> propertyTrail,
			Item* item = NULL);
	~Initial();
	void clearState();
	bool isProcessed();
	StatementGroup& getNextStmtGr(Item* item = NULL);
//	int getLowesetDegree(Item* item = NULL);
	int getItemDegree();
	void addToItemTrail(int id);
	void addToPropertyTrail(int id);

	Blacklist* getBlacklist();
	int getItemId();
	double getBaseOP();
	vector<int>& getItemTrail();
	vector<int>& getPropertyTrail();

protected:

	// initial values
	double baseOP;
	Blacklist* blacklist;
	int itemId;

	IndexReader& reader;

	// computed values
	int lowest;
	int size;
	int degree;
	vector<int> propertyTrail;
	vector<int> itemTrail;

};

Initial::Initial(IndexReader& reader, int itemId, Blacklist* bl, double baseOP,
		vector<int> itemTrail, vector<int> propertyTrail, Item* item) :
		reader(reader) {
	this->baseOP = baseOP;
	this->blacklist = bl;
	this->itemId = itemId;
	this->itemTrail = itemTrail;
	this->propertyTrail = propertyTrail;
	itemTrail.push_back(itemId);
	if (item == NULL) {
		item = &reader.getItemById(itemId);
	}
	size = item->getStatementGroups().size();
	degree = item->getDegree();
	clearState();
}

Initial::~Initial() {
}

void Initial::clearState() {
	lowest = 0;
}

bool Initial::isProcessed() {
	return (lowest >= size);
}

StatementGroup& Initial::getNextStmtGr(Item* item) {
	if (item == NULL) {
		item = &reader.getItemById(itemId);
	}
	lowest++;
	cout << "read item " << lowest << endl;
	return item->getStatementGroups()[lowest - 1];
}

//int Initial::getLowesetDegree(Item* item) {
//	if (item == NULL){
//		item = &reader.getItemById(itemId);
//	}
//	if (isProcessed(item)) {
//		return -1;
//	}
//	return item->getStatementGroups()[lowest].getTargets().size();
//}

int Initial::getItemDegree() {
	return degree;
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
