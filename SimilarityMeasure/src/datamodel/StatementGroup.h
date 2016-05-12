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
	StatementGroup(int pId, vector<int>& targets);
	StatementGroup(int pId);
	int getPropertyId();
	vector<int>& getTargets();
	void pushTarget(int target, bool safemode = false);
protected:
	int pId;
	vector<int> targets;
};

StatementGroup::StatementGroup(int pId, vector<int>& targets) {
	this->pId = pId;
	this->targets = targets;
}

StatementGroup::StatementGroup(int pId) {
	this->pId = pId;
	this->targets = vector<int>();
}

int StatementGroup::getPropertyId() {
	return pId;
}

vector<int>& StatementGroup::getTargets() {
	return targets;
}

void StatementGroup::pushTarget(int target, bool safemode) {
	if (safemode) {
		for (size_t i = 0; i < targets.size(); i++) {
			if (targets[i] == target) {
				return;
			}
		}
	}
	targets.push_back(target);
}

#endif /* STATEMENTGROUP_H_ */
