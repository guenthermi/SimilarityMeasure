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
#include "TopKSearch.h"
#include "datamodel/TopKEntry.h"
#include "TopK.h"

#include<set>
#include<iostream>
#include<unordered_map>

using namespace std;

class State{
public:

	const double kLevelFactor = 0.1;
	const double kBufferVolume = 1000;

	State(TopK* topK);
	~State();
	void deleteInitial(Initial* initial);
	Initial* getBestChoice(IndexReader& reader, int& iterationCount, int& maxIteration);
	set<Initial*>& getInitials();
	double createNewInitials(Initial* initial, Blacklist* bl, IndexReader& reader, set<Initial*>* destination=NULL);

protected:
	set<Initial*> initials;
	set<Initial*> buffer;
	void freeInitials();
	double level;
	TopK* topK;
};

State::State(TopK* topK){
	level = 0.1;
	this->topK = topK;
}

State::~State(){
	freeInitials();
}

void State::deleteInitial(Initial* initial){
	delete initial;
	initials.erase(initial);
}

Initial* State::getBestChoice(IndexReader& reader, int& iterationCount, int& maxIteration){
	Initial* result = NULL;
	set<Initial*> toRemove;
	set<Initial*> toAdd;
	if (initials.size() == 0){
		return NULL;
	}

	if (!buffer.empty()){
		result = *buffer.begin();
		buffer.erase(buffer.begin());
		return result;
	}

	while (result == NULL){
		for (set<Initial*>::iterator it = initials.begin(); it != initials.end(); it++){
			double deltaReduce = 0;
			double value = (*it)->getPenalty(&deltaReduce, level);
			if (value == -1){
				if (deltaReduce != 0){
					topK->reduceDeltas(deltaReduce, (*it)->getBlacklist());
					iterationCount++;
					cout << "IterationD: " << iterationCount << endl;
				}

				if (iterationCount > maxIteration){
					return NULL;
				}
				continue;
			}
			if (value == -2){
				Blacklist* bl = new Blacklist();
				bl->setNext((*it)->getBlacklist());
				createNewInitials((*it), bl, reader, &toAdd);
				toRemove.insert((*it));
				iterationCount++;
				cout << "Iteration0: " << iterationCount << endl;
				if (iterationCount > maxIteration){
					cout << "get there" << endl;
					return NULL;
				}
				continue;
			}
			if (value != -1){
//				cout << "found candidate" << endl;
				if (result == NULL){
					result = *it;
				}else{
					buffer.insert(*it);
				}
			}
			if (buffer.size() > kBufferVolume){
				break;
			}
		}
		if (result == NULL){
			level *= kLevelFactor;
			cout << "buffer size: " << buffer.size() << endl;
			cout << "get on the next level: " << level << " Initial size: " << initials.size() << endl;
		}else{
			for (set<Initial*>::iterator it = toRemove.begin(); it != toRemove.end(); it++){
				delete *it;
				initials.erase(*it);
			}
			for (set<Initial*>::iterator it = toAdd.begin(); it != toAdd.end(); it++){
				initials.insert(*it);
			}
			cout << "initial size: " << initials.size() << endl;
			return result;
		}
	}
}

set<Initial*>& State::getInitials(){
	return initials;
}

/**
 * Returns the new op value of the initials which have been created.
 */
double State::createNewInitials(Initial* initial, Blacklist* bl, IndexReader& reader, set<Initial*>* destination){
	if (destination == NULL){
		destination = &initials;
	}

	Item& item = reader.getItemById(initial->getItemId());
	int count=0;
	int degree = item.getDegree();
	if (initial->getItemTrail().size() == 1){
		degree++;
	}
	double newOp = ((double) (1.0 / (degree -1))) * initial->getOP(); // degree reduced by origin
	vector<int>& origins = initial->getItemTrail();
	vector<StatementGroup>& stmtGrs = item.getStatementGroups();
	vector<int> itemTrail = initial->getItemTrail();
	vector<int> propertyTrail = initial->getPropertyTrail();
	cout << "start creating initials" << endl;
	for (int i=0; i<stmtGrs.size(); i++){
		propertyTrail.push_back(stmtGrs[i].getPropertyId());
		int* targets = stmtGrs[i].getTargets();
		for (int j=0; j<stmtGrs[i].size(); j++){
			bool valid = true;
			for (int k=0; k < origins.size();k++){
				if (origins[k] == targets[j]){
					valid = false;
				}
			}
			count++;
			if (valid){
				itemTrail.push_back(targets[j]);
				double ip = (initial->getInpenalty() == 0) ? 1.0 : (1.0 / ((1.0 / initial->getInpenalty()) +1));
				Initial* newInitial = new Initial(reader, targets[j], bl, newOp, ip, itemTrail, propertyTrail);
				destination->insert(newInitial);
				itemTrail.pop_back();
			}
		}
		propertyTrail.pop_back();
	}
	cout << "finish creation of " << count << "initials" << endl;
	return newOp;
}

void State::freeInitials(){
	for (set<Initial*>::iterator it = initials.begin(); it != initials.end(); it++){
		delete *it;
	}
}

#endif /* STATE_H_ */
