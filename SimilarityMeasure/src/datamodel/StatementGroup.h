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
	StatementGroup();
	StatementGroup(int pId, int size);
	int getPropertyId();
	int* getTargets();
	int size();
	void clear();
	void reallocate(int pId, int size, int* targets);
protected:
	int pId;
	int sz;
	int* targets;
};

StatementGroup::StatementGroup(){
	this->pId = 0;
	this->sz = 0;
	this->targets = NULL;
}

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
	if (targets != NULL){
		delete[] targets;
	}
}

void StatementGroup::reallocate(int pId, int size, int* targets){
	this->pId = pId;
	this->sz = size;
	if (this->targets != NULL){
		delete[] this->targets;
	}
	this->targets = targets;
}

#endif /* STATEMENTGROUP_H_ */
