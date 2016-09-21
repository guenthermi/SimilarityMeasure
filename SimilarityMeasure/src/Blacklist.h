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

	Blacklist();
	Blacklist(Blacklist* bl);

	bool hasItem(int id);
	unordered_set<int>& getItems();
	void addItem(int id);

protected:
	unordered_set<int> items;
	bool end;

};

Blacklist::Blacklist() {
}


Blacklist::Blacklist(Blacklist* bl) {
	if (bl != NULL) {
		items.insert(bl->getItems().begin(), bl->getItems().end());
	}
}

bool Blacklist::hasItem(int id) {
	if (items.find(id) != items.end()) {
		return true;
	} else {
		return false;
	}
}

unordered_set<int>& Blacklist::getItems() {
	return items;
}

void Blacklist::addItem(int id) {
	items.insert(id);
}

#endif /* BLACKLIST_H_ */
