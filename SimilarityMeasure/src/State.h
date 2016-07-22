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
#include "AStarSearch.h"
#include "datamodel/TopKEntry.h"

#include<set>
#include<iostream>
#include<unordered_map>

using namespace std;

class State{
public:

	const double kLevelFactor = 0.1;
	const double kBufferVolume = 1000;

	State(unordered_map<int, TopKEntry>* topK, double* globalDelta, double* topValue);
	~State();
	void addInitial(Initial* initial);
	void deleteInitial(Initial* initial);
	Initial* getBestChoice(IndexReader& reader);
	set<Initial*>& getInitials();
	double createNewInitials(Initial* initial, Blacklist* bl, set<Initial*> destination, IndexReader& reader);

protected:
	set<Initial*> initials;
	set<Initial*> buffer;
	void freeInitials();
	void reduceDeltas(double gReduction, Blacklist* bl);
	double level;
	unordered_map<int, TopKEntry>* topK;
	double* globalDelta;
	double* topValue;
};

State::State(unordered_map<int, TopKEntry>* topK, double* globalDelta, double* topValue){
	level = 0.1;
	this->topK = topK;
	this->globalDelta = globalDelta;
	this->topValue = topValue;
}

State::~State(){
	freeInitials();
}

void State::reduceDeltas(double gReduction, Blacklist* bl){
	vector<int> toErase;

	for (unordered_map<int, TopKEntry>::iterator it = topK->begin();
			it != topK->end(); it++) {
		TopKEntry& value = it->second;
		if (bl){
			if (!bl->hasItem(it->first)) {
				value.delta -= gReduction;
			}
		}
		if (value.delta < 0) {
			cout << "################ FEHLER #####################" << endl;
		}
		if (value.delta < ((*topValue) - value.weight)) {
			toErase.push_back(it->first);
		}
	}
	for (size_t i = 0; i < toErase.size(); i++) {
		topK->erase(toErase[i]);
	}
	(*globalDelta) -= gReduction;
	cout << "globalDelta: " << *globalDelta << endl;
}

void State::addInitial(Initial* initial){
	initials.insert(initial);
}

void State::deleteInitial(Initial* initial){
	delete initial;
	initials.erase(initial);
}

Initial* State::getBestChoice(IndexReader& reader){
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
					reduceDeltas(deltaReduce, (*it)->getBlacklist());
				}
				continue;
			}
			if (value == -2){
				Blacklist* bl = new Blacklist();
				bl->setNext((*it)->getBlacklist());
				createNewInitials((*it), bl, toAdd, reader);
				toRemove.insert((*it));
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
double State::createNewInitials(Initial* initial, Blacklist* bl, set<Initial*> destination, IndexReader& reader){
	Item& item = reader.getItemById(initial->getItemId());
	int degree = item.getDegree();
	double newOp = ((double) (1.0 / (degree -1))) * initial->getOP(); // degree reduced by origin
	vector<int>& origins = initial->getItemTrail();
	vector<StatementGroup>& stmtGrs = item.getStatementGroups();
	vector<int> itemTrail = initial->getItemTrail();
	vector<int> propertyTrail = initial->getPropertyTrail();
	for (int i=0; i<stmtGrs.size(); i++){
		propertyTrail.push_back(stmtGrs[i].getPropertyId());
		vector<int>& targets = stmtGrs[i].getTargets();
		for (int j=0; j<targets.size(); j++){
			bool valid = true;
			for (int k=0; k < origins.size();k++){
				if (origins[k] == targets[j]){
					valid = false;
				}
			}
			if (valid){
				itemTrail.push_back(targets[j]);
				Initial* newInitial = new Initial(reader, targets[j], bl, newOp, 1.0 / ((1.0 / initial->getInpenalty()) +1), itemTrail, propertyTrail);
				addInitial(newInitial);
				itemTrail.pop_back();
			}
		}
		propertyTrail.pop_back();
	}
	return newOp;
}

void State::freeInitials(){
	for (set<Initial*>::iterator it = initials.begin(); it != initials.end(); it++){
		delete *it;
	}
}

#endif /* STATE_H_ */
