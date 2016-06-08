/*
 * State.h
 *
 *  Created on: 03.06.2016
 *      Author: michael
 */

#ifndef STATE_H_
#define STATE_H_

#include "Initial.h"

#include<set>

using namespace std;

class State{
public:
	State();
	~State();
	void addInitial(Initial* initial);
	bool isComplete();
	Initial* getBestChoice();

protected:
	set<Initial*> initials;
	void freeInitials();
};

State::State(){
}

State::~State(){
	freeInitials();
}

void State::addInitial(Initial* initial){
	initials.insert(initial);
	if (!initial->isProcessed()){
	}
}

bool State::isComplete(){
	set<Initial*> completed;
	bool result = true;
	for (set<Initial*>::iterator it = initials.begin(); it != initials.end(); it++){
		if ((*it)->isProcessed() == false){
			completed.insert(*it);
		}else{
			result = false;
			break;
		}
	}
	for (set<Initial*>::iterator it = completed.begin(); it != completed.end(); it++){
		initials.erase(*it);
	}
	return result;
}

Initial* State::getBestChoice(){
	double highest = -1;
	Initial* result = NULL;
	for (set<Initial*>::iterator it = initials.begin(); it != initials.end(); it++){
		if (!(*it)->isProcessed()){
			double op = (*it)->getBaseOP() / (*it)->getItemDegree();
			if ((op > 0) && (highest < op)){
				result = *it;
				highest = op;
			}
		}
	}
	return result;
}

void State::freeInitials(){
	for (set<Initial*>::iterator it = initials.begin(); it != initials.end(); it++){
		delete *it;
	}
}

#endif /* STATE_H_ */
