/*
 * Index.h
 *
 *  Created on: 29.04.2016
 *      Author: michael
 */

#ifndef INDEX_H_
#define INDEX_H_

#include "StatementGroup.h"
#include "Item.h"

#include <vector>
#include <iostream>

using namespace std;

class Index {
public:

	static const int kItemSize = 25000000;
	static const int kNumberOfProperties = 3000;

	Index(int size = kItemSize, int numberOfProperties = kNumberOfProperties);

	vector<StatementGroup>* getStatements(int itemId);
	int getSize();
	vector<StatementGroup>** getData();
	int* getPropertyCounts();

	void pushStatement(int itemId, int propertyId, int target, bool safemode=false);
	void clear();

protected:

	vector<StatementGroup>** data;
	int size;
	int* properties;
	int numberOfProperties;

};

Index::Index(int size, int numberOfProperties) {
	data = new vector<StatementGroup>*[size];
	this->size = size;
	for (int i = 0; i < size; i++) {
		data[i] = NULL;
	}
	properties = new int[numberOfProperties];
	this->numberOfProperties = numberOfProperties;
	for (int i = 0; i < numberOfProperties; i++) {
		data[i] = 0;
	}
}

vector<StatementGroup>* Index::getStatements(int itemId) {
	return data[itemId];
}

int Index::getSize() {
	return size;
}

vector<StatementGroup>** Index::getData() {
	return data;
}

int* Index::getPropertyCounts(){
	return properties;
}

void Index::pushStatement(int itemId, int propertyId, int target, bool safemode) {
	properties[propertyId]++;
	if (data[itemId] == NULL) {
		data[itemId] = new vector<StatementGroup>();
	}
	vector<StatementGroup>* stmtGrs = data[itemId];
	int stmtGr = -1;
	for (size_t i = 0; i < stmtGrs->size(); i++) {
		if ((*stmtGrs)[i].getPropertyId() == propertyId) {
			stmtGr = i;
			break;
		}
	}
	if (stmtGr == -1) {
		StatementGroup statementGroup(propertyId);
		statementGroup.pushTarget(target, safemode);
		stmtGrs->push_back(statementGroup);
	} else {
		StatementGroup* statementGroup = &(*stmtGrs)[stmtGr];
		statementGroup->pushTarget(target, safemode);
	}
}

void Index::clear() {
	for (int i = 0; i < size; i++) {
		if (data[i]) {
			delete data[i];
			data[i] = NULL;
		}
	}
}

#endif /* INDEX_H_ */
