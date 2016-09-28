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

class Blacklist {
public:

	static const int bitsetSize = 25000000;

	Blacklist();
	Blacklist(Blacklist* bl);
	~Blacklist();

	bool hasItem(int id);
	unordered_set<int>* getItems();
	void addItem(int id);
	void activate();
	int getSize();
	void incrementUsage();
	bool decrementUsage();
	bool inUse();

protected:
	unordered_set<int>* items;
	bool active;
	int usageCount;
	int size;
	Blacklist* pred;

};

Blacklist::Blacklist() {
	active = true;
	usageCount = 0;
	size = 0;
	pred = NULL;
	items = new unordered_set<int>();
}

Blacklist::Blacklist(Blacklist* bl) {
	pred = bl;
	active = false;
	size = 0;
	usageCount = 0;
	if (bl != NULL) {
		bl->incrementUsage();
	}
}

Blacklist::~Blacklist() {
	if (!active) {
		if (pred != NULL) {
			if (pred->decrementUsage()) {
				delete pred;
			}
		}
	} else {
		delete items;
	}
}

void Blacklist::activate() {
	if (!active) {
		items = new unordered_set<int>();
		if (pred != NULL) {
			items->insert(pred->getItems()->begin(), pred->getItems()->end());
			size = pred->getSize();
			if (pred->decrementUsage()) {
				delete pred;
			}
		}
	}
	active = true;
}

bool Blacklist::inUse() {
	return (usageCount > 0);
}

int Blacklist::getSize() {
	return size;
}

void Blacklist::incrementUsage() {
	usageCount++;
}

bool Blacklist::decrementUsage() {
	return ((--usageCount) == 0);
}

bool Blacklist::hasItem(int id) {
	if (active) {
		if (items->find(id) != items->end()) {
			return true;
		} else {
			return false;
		}
	} else {
		if (pred != NULL) {
			return pred->hasItem(id);
		}
		return false;
	}
}

unordered_set<int>* Blacklist::getItems() {
	return items;
}

void Blacklist::addItem(int id) {
	items->insert(id);
	size++;
}

#endif /* BLACKLIST_H_ */
