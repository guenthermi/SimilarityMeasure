/*
 * Tracker.h
 *
 *  Created on: 19.10.2016
 *      Author: michael
 */

#ifndef TRACKER_H_
#define TRACKER_H_

#include "DebugHelpers.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <algorithm>

using namespace std;

class Tracker {
public:

	static const bool SORT_OUTPUT = true;

	Tracker(string filename);
	void registerListener(int itemId);
	void notify(int itemId, double weight, vector<int>& itemTrail,
			vector<int>& propertyTrail);
	void clearListeners();
	void print();
protected:

	class Aspect {
	public:
		Aspect(double weight, vector<int>& itemTrail,
				vector<int>& propertyTrail) :
				itemTrail(itemTrail), propertyTrail(propertyTrail) {
			this->weight = weight;
		}

		double weight;
		vector<int> itemTrail;
		vector<int> propertyTrail;
	};

	static bool comparator(Aspect& a1, Aspect a2){
		return (a1.weight > a2.weight);
	}

	string filename;
	unordered_map<int, vector<Aspect>> listeners;

};

Tracker::Tracker(string filename) {
	this->filename = filename;
}

void Tracker::registerListener(int itemId) {
	listeners[itemId] = vector<Aspect>();

}

void Tracker::notify(int itemId, double weight, vector<int>& itemTrail,
		vector<int>& propertyTrail) {
	if (listeners.find(itemId) != listeners.end()) {
		listeners[itemId].push_back(Aspect(weight, itemTrail, propertyTrail));
	}
}

void Tracker::clearListeners(){
	for (unordered_map<int, vector<Aspect>>::iterator it = listeners.begin(); it != listeners.end(); it++){
		it->second.clear();
	}
}

void Tracker::print() {
	ofstream file;
	file.open(filename);
	for (unordered_map<int, vector<Aspect>>::iterator it = listeners.begin();
			it != listeners.end(); it++) {
		if (SORT_OUTPUT){
			sort(it->second.begin(), it->second.end(), comparator);
		}
		file << endl << "Tracking for Q" << it->first << ":" << endl;
		for (int i = 0; i < it->second.size(); i++) {
			int j;
			for (j = 0; j < it->second[i].propertyTrail.size(); j++) {
				file << "Q" << it->second[i].itemTrail[j] << " -P"
						<< it->second[i].propertyTrail[j] << "> ";
			}
			file << "Q" << it->second[i].itemTrail[j];
			file << " W: " << it->second[i].weight << endl;

		}
	}
	file.close();
}

#endif /* TRACKER_H_ */
