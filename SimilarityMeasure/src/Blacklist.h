/*
 * Blacklist.h
 *
 *  Created on: 28.05.2016
 *      Author: michael
 */

#ifndef BLACKLIST_H_
#define BLACKLIST_H_

#include <unordered_set>
#include <iostream>
#include <algorithm>

using namespace std;

class Blacklist{
public:

	Blacklist();

	bool hasItem(int id);
	Blacklist* getNext();
	unordered_set<int>& getItems();
	void addItem(int id);
//	addItems(set<int> ids)
	void setNext(Blacklist* bl);

protected:

	Blacklist* next;
	unordered_set<int> items;
	bool end;

};


Blacklist::Blacklist(){
	next = NULL;
}

bool Blacklist::hasItem(int id){
	if (items.find(id) != items.end()){
		return true;
	}else{
		if (next == NULL){
			return false;
		}else{
			return next->hasItem(id);
		}
	}
}

Blacklist* Blacklist::getNext(){
	return next;
}


unordered_set<int>& Blacklist::getItems(){
	return items;
}

void Blacklist::addItem(int id){
	items.insert(id);
}

void Blacklist::setNext(Blacklist* bl){
	next = bl;
}

#endif /* BLACKLIST_H_ */
