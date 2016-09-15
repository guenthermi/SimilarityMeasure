/*
 * Item.h
 *
 *  Created on: 26.04.2016
 *      Author: michael
 */

#ifndef ITEM_H_
#define ITEM_H_

#include "StatementGroup.h"

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

class Item {
public:
	Item();
	Item(int qId, int size);
	Item(int qId, StatementGroup* stmtGrs, int size);
	StatementGroup* getStatementGroups();
	void setId(int id);
	int getId();
	int getDegree();
	string toString();
	int size();
	void clear();
protected:
	int qId;
	int sz;
	StatementGroup* stmtGrs;
};

Item::Item() {
	this->qId = 0;
	this->sz = 0;
	this->stmtGrs = NULL;
}

Item::Item(int qId, int size) {
	this->qId = qId;
	this->sz = size;
	this->stmtGrs = new StatementGroup[size];
}

Item::Item(int qId, StatementGroup* stmtGrs, int size) {
	this->qId = qId;
	this->sz = size;
	this->stmtGrs = stmtGrs;
}

//Item::Item(const Item& item) {
//	this->qId = item.qId;
//	this->stmtGrs = item.stmtGrs;
//}

StatementGroup* Item::getStatementGroups() {
	return stmtGrs;
}

int Item::getId() {
	return qId;
}

void Item::setId(int id) {
	qId = id;
}

int Item::getDegree() {
	int result = 0;
	for (int i = 0; i < sz; i++) {
		result += stmtGrs[i].size();
	}
	return result;
}

string Item::toString() {
	stringstream ss;
	ss << "Q" << qId << ": " << endl;

	for (int i = 0; i < sz; i++) {
		ss << "\tP" << stmtGrs[i].getPropertyId() << ": ";
		for (int j = 0; j < stmtGrs[i].size(); j++) {
			ss << "Q" << stmtGrs[i].getTargets()[j];
			if (j < (stmtGrs[i].size() - 1)) {
				ss << ", ";
			}
		}
		ss << endl;
	}
	ss << endl;
	return ss.str();
}

int Item::size(){
	return sz;
}

void Item::clear(){
	for (int i=0; i<sz; i++){
		stmtGrs[i].clear();
	}
	if (stmtGrs != NULL){
		delete[] stmtGrs;
	}
}

#endif /* ITEM_H_ */
