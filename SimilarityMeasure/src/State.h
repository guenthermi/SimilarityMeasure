/*
 * State.h
 *
 *  Created on: 03.06.2016
 *      Author: michael
 */

#ifndef STATE_H_
#define STATE_H_

#include "Initial.h"
#include "Blacklist.h"
#include "IndexReader.h"
#include "DebugHelpers.h"
#include "datamodel/TopKEntry.h"
#include "TopK.h"

#include <math.h>
#include <set>
#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

class State {
public:

	const double kLevelFactor = 0.1;
	const double kBufferVolume = 1000;

	State(TopK* topK, int level);
	~State();
	Initial* getNextInitial(int& debug, IndexReader& reader,
			int& iterationCount, int& maxIteration);
	vector<Initial*>& getInitials();
	double createNewInitials(Initial* initial, Blacklist* bl,
			IndexReader& reader);

protected:
	vector<Initial*> stack;
	void freeInitials();
	double level;
	TopK* topK;
};

State::State(TopK* topK, int level) {
	this->level = pow(0.1, level);
	this->topK = topK;
}

State::~State() {
	freeInitials();
}

Initial* State::getNextInitial(int& debug, IndexReader& reader,
		int& iterationCount, int& maxIteration) {
	Initial* result = NULL;
	while ((result == NULL) && (stack.size() > 0)) {
		Initial* initial = stack.back();
		stack.pop_back();
		double deltaReduce = 0;
		double value = initial->getPenalty(debug, &deltaReduce, level,
				sqrt(1.0 / level));
		if (value == -1){
			if (deltaReduce != 0) {
				if (deltaReduce < 0){
					cout << "error"<< endl;
					while(1);
				}
				topK->reduceDeltas(deltaReduce, initial->getBlacklist());
			}else{
				iterationCount--;
			}
			delete initial;
		}
		if (value == -2){
			createNewInitials(initial, initial->getBlacklist(), reader);
			delete initial;
		}
		if ((value != -1) && (value != -2)){
			result = initial;
		}
		iterationCount++;
		if (iterationCount > maxIteration) {
			return NULL;
		}
		if ((iterationCount % 100) == 0){
			cout << "Iteration: " << iterationCount << endl;
		}
	}
//	cout << "Problem 2" << endl;
	return result;
}

vector<Initial*>& State::getInitials() {
	return stack;
}

/**
 * Returns the new op value of the initials which have been created.
 */
double State::createNewInitials(Initial* initial, Blacklist* bl,
		IndexReader& reader) {
	Item& item = reader.getItemById(initial->getItemId());
	int count = 0;
	int degree = item.getDegree();
	if (initial->getItemTrail().size() == 1) {
		degree++;
	}
	double newOp = ((double) (1.0 / (degree - 1))) * initial->getOP(); // degree reduced by origin
	vector<int>& origins = initial->getItemTrail();
	StatementGroup* stmtGrs = item.getStatementGroups();
	vector<int> itemTrail = initial->getItemTrail();
//	cout << stmtGrs[0].getPropertyId() << endl;
	vector<int> propertyTrail = initial->getPropertyTrail();
//	cout << "start creating initials" << endl;
	for (int i = 0; i < item.size(); i++) {
		propertyTrail.push_back(stmtGrs[i].getPropertyId());
		int* targets = stmtGrs[i].getTargets();
		for (int j = 0; j < stmtGrs[i].size(); j++) {
			bool valid = true;
			for (int k = 0; k < origins.size(); k++) {
				if (origins[k] == targets[j]) {
					valid = false;
				}
			}
			count++;
			if (valid) {
				itemTrail.push_back(targets[j]);
				double ip =
						(initial->getInpenalty() == 0) ?
								1.0 :
								(1.0 / ((1.0 / initial->getInpenalty()) + 1));
				if ((newOp*ip) > level){
					Blacklist* blacklist = new Blacklist(bl);
					Initial* newInitial = new Initial(reader, targets[j], blacklist, newOp,
							ip, itemTrail, propertyTrail);
					stack.push_back(newInitial);
				}
				itemTrail.pop_back();
			}
		}
		propertyTrail.pop_back();
	}
//	cout << "finish creation of " << count << "initials" << endl;
	return newOp;
}

void State::freeInitials() {
	for (vector<Initial*>::iterator it = stack.begin(); it != stack.end();
			it++) {
		delete *it;
	}
}

#endif /* STATE_H_ */
