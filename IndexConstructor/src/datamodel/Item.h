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

using namespace std;

class Item {
public:
	Item();
	Item(int qId);
	Item(int qId, vector<StatementGroup>& stmtGrs);
//	Item(const Item& item);
	void pushStmtGr(StatementGroup stmtGr);
	vector<StatementGroup>& getStatementGroups();
	void setId(int id);
	int getId();
protected:
	int qId;
	vector<StatementGroup> stmtGrs;
};

Item::Item(){
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

void Item::setId(int id){
	qId = id;
}

#endif /* ITEM_H_ */
