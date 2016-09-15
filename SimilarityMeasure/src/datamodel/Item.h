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
	Item(int qId);
	Item(int qId, vector<StatementGroup>& stmtGrs);
	void pushStmtGr(StatementGroup stmtGr);
	vector<StatementGroup>& getStatementGroups();
	void setId(int id);
	int getId();
	int getDegree();
	string toString();
	void sortStmtGrsBySize();
	void clear();
protected:
	int qId;
	vector<StatementGroup> stmtGrs;

	static bool cmp(StatementGroup& a, StatementGroup& b);
};

Item::Item() {
	this->qId = 0;
	this->stmtGrs = vector<StatementGroup>();
}

Item::Item(int qId) {
	this->qId = qId;
	this->stmtGrs = vector<StatementGroup>();
}

Item::Item(int qId, vector<StatementGroup>& stmtGrs) {
	this->qId = qId;
	this->stmtGrs = stmtGrs;
}

//Item::Item(const Item& item) {
//	this->qId = item.qId;
//	this->stmtGrs = item.stmtGrs;
//}

void Item::pushStmtGr(StatementGroup stmtGr) {
	stmtGrs.push_back(stmtGr);
}

vector<StatementGroup>& Item::getStatementGroups() {
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
	for (int i = 0; i < stmtGrs.size(); i++) {
		result += stmtGrs[i].size();
	}
	return result;
}

string Item::toString() {
	stringstream ss;
	ss << "Q" << qId << ": " << endl;

	for (int i = 0; i < stmtGrs.size(); i++) {
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

void Item::sortStmtGrsBySize() {
	sort(stmtGrs.begin(), stmtGrs.end(), cmp);
}

void Item::clear(){
	for (int i=0; i<stmtGrs.size(); i++){
		stmtGrs[i].clear();
	}
}

bool Item::cmp(StatementGroup& a, StatementGroup& b) {
	return (a.size() < b.size());
}

#endif /* ITEM_H_ */
