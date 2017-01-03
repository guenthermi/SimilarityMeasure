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
#include <fstream>
#include <unistd.h>

using namespace std;

class State {
public:

	const double kLevelFactor = 0.1;
	const double kBufferVolume = 1000;

	State(TopK* topK, int level, ostream* log);
	~State();
	Initial* getNextInitial(int& debug, IndexReader& reader,
			int& iterationCount, int& maxIteration);
	double createNewInitials(Initial* initial, Blacklist* bl,
			IndexReader& reader, bool hasResults);


protected:

	vector<Initial*> stack;
	void freeInitials();
	double level;
	TopK* topK;
	ostream* log;
	int lastOutput = 0;
};

State::State(TopK* topK, int level, ostream* log) {
	this->level = pow(0.1, level);
	this->topK = topK;
	this->log = log;
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
		if (value == -1) {
			if (deltaReduce != 0) {
				if (deltaReduce < 0) {
					cerr << "error" << endl;
					while (1)
						;
				}
				topK->reduceDeltas(deltaReduce, initial->getBlacklist());
			} else {
				iterationCount--;
			}
			delete initial;
		}
		if (value == -2) {
			createNewInitials(initial, initial->getBlacklist(), reader, false);
			delete initial;
		}
		if ((value != -1) && (value != -2)) {
			result = initial;
		}
		iterationCount++;
		if (iterationCount > maxIteration) {
			return NULL;
		}
		if ((iterationCount % 1000) == 0) {
			if (iterationCount != lastOutput){
				(*log) << "Iteration: " << iterationCount << endl;
			}
			lastOutput = iterationCount;
		}
	}

	return result;
}

/**
 * Returns the new op value of the initials which have been created.
 */
double State::createNewInitials(Initial* initial, Blacklist* bl,
		IndexReader& reader, bool hasResults) {
	Item& item = reader.getItemById(initial->getItemId());
	int degree = item.getDegree();
	double newOp = ((double) (1.0 / (degree))) * initial->getOP();
	StatementGroup* stmtGrs = item.getStatementGroups();
	vector<int> itemTrail = initial->getItemTrail();
	vector<int> propertyTrail = initial->getPropertyTrail();
	for (int i = 0; i < item.size(); i++) {
		propertyTrail.push_back(stmtGrs[i].getPropertyId());
		int* targets = stmtGrs[i].getTargets();
		for (int j = 0; j < stmtGrs[i].size(); j++) {

			itemTrail.push_back(targets[j]);
			int offset = hasResults ? 1 : 0;
			double ip =
					(initial->getInpenalty() == 0) ?
							1.0 :
							(1.0 / ((1.0 / initial->getInpenalty()) + offset));
			if ((newOp * ip) > level) {
				Blacklist* blacklist = new Blacklist(bl);
				Initial* newInitial = new Initial(reader, targets[j], blacklist,
						newOp, ip, itemTrail, propertyTrail);
				stack.push_back(newInitial);
			}
			itemTrail.pop_back();
		}
		propertyTrail.pop_back();
	}
	return newOp;
}

void State::freeInitials() {
	for (vector<Initial*>::iterator it = stack.begin(); it != stack.end();
			it++) {
		delete *it;
	}
}

#endif /* STATE_H_ */
