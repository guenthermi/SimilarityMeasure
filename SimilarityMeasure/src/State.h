/*
 * State.h
 *
 *  Created on: 03.06.2016
 *      Author: michael
 */

#ifndef STATE_H_
#define STATE_H_

#include "Initial.h"

#include<vector>

using namespace std;

class State{
public:
	State();
	~State();
	void addInitial(Initial* initial);
	bool isComplete();
	Initial* getBestChoice();

protected:
	vector<Initial*> initials;
	void freeInitials();
};

State::State(){
}

State::~State(){
	freeInitials();
}

void State::addInitial(Initial* initial){
	initials.push_back(initial);
	if (!initial->isProcessed()){
	}
}

bool State::isComplete(){
	for (size_t i; i < initials.size(); i++){
		if (initials[i]->isProcessed() == false){
			return false;
		}
	}
	return true;
}

Initial* State::getBestChoice(){
	double highest = -1;
	Initial* result = NULL;
	for (size_t i = 0; i<initials.size(); i++){
		double op = initials[i]->getBaseOP() / initials[i]->getLowesetDegree();
		if ((op > 0) && (highest < op)){
			result = initials[i];
			highest = op;
		}
	}
	return result;
}

void State::freeInitials(){
	for (size_t i; i<initials.size(); i++){
		delete initials[i];
	}
}

#endif /* STATE_H_ */
