/*
 * StatementGroup.h
 *
 *  Created on: 26.04.2016
 *      Author: michael
 */

#ifndef STATEMENTGROUP_H_
#define STATEMENTGROUP_H_

#include <vector>
#include <iostream>

using namespace std;

class StatementGroup {
public:
	StatementGroup(int pId, int size);
	int getPropertyId();
	int* getTargets();
	int size();
	void clear();
protected:
	int pId;
	int sz;
	int* targets;
};

StatementGroup::StatementGroup(int pId, int size) {
	this->pId = pId;
	targets = new int[size];
	sz = size;
}

int StatementGroup::getPropertyId() {
	return pId;
}

int* StatementGroup::getTargets() {
	return targets;
}

int StatementGroup::size(){
	return sz;
}

void StatementGroup::clear(){
	delete[] targets;
}

#endif /* STATEMENTGROUP_H_ */
