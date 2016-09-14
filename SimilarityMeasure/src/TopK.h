/*
 * TopK.h
 *
 *  Created on: 14.09.2016
 *      Author: michael
 */

#ifndef TOPK_H_
#define TOPK_H_

#include "datamodel/TopKEntry.h"
#include "Blacklist.h"

#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>

using namespace std;

class TopK {
public:
	TopK(int k);
	void updateTopK(map<int, double>* candidates, double gReduction,
			double cReduction, Blacklist& bl);
	void reduceDeltas(double gReduction, Blacklist* bl);
	void printTopK();
	bool hasConverged();
	unordered_set<int>& getTopK();
	double getGlobalDelta();
	size_t getContentsSize();

protected:
	unordered_map<int, TopKEntry> contents;
	unordered_set<int> topElements;
	int k;
	double globalDelta;
	int minTopId;
	double minTopValue;

	void determineMin();
};

TopK::TopK(int k) {
	this->k = k;
	this->globalDelta = 1;
}

void TopK::updateTopK(map<int, double>* candidates, double gReduction,
		double cReduction, Blacklist& bl) {
	cout << "cReduce: " << cReduction << " gReduce: " << gReduction
			<< " candidates size: " << candidates->size() << endl;
	for (map<int, double>::iterator it = candidates->begin();
			it != candidates->end(); it++) {
		TopKEntry& value = contents[it->first];
		value.weight += it->second;
		if (value.delta == 0) {
			value.delta = globalDelta;
		}
		value.delta -= cReduction;

		// update topElements
		if (topElements.size() < k) {
			topElements.insert(it->first);
			determineMin();
		} else {
			if (value.weight > minTopValue) {
				if (topElements.find(it->first) == topElements.end()) { // not in topElements
					int toRemove = it->first;
					double toRemoveValue = value.weight;
					for (unordered_set<int>::iterator it = topElements.begin();
							it != topElements.end(); it++) {
						double secondWeight = contents[*it].weight;
						if (toRemoveValue < secondWeight) {
							toRemoveValue = secondWeight;
							toRemove = *it;
						}
					}
					if (toRemove != it->first) {
						topElements.erase(toRemove);
						topElements.insert(it->first);
						determineMin();
					}
				} else {
					if (it->first == minTopId) {
						determineMin();
					}
				}
			}
		}
	}

	vector<int> toErase;

	for (unordered_map<int, TopKEntry>::iterator it = contents.begin();
			it != contents.end(); it++) {
		TopKEntry& value = it->second;
		if (!bl.hasItem(it->first)) {
			value.delta -= gReduction;
		}
		if (value.delta < 0) {
			cout << "################ FEHLER #####################" << endl;
		}
		if (value.delta < (minTopValue - value.weight)) {
			if (minTopId == it->first) {
				cout << "####Problem:" << endl;
			}
			toErase.push_back(it->first);
		}
	}
	for (size_t i = 0; i < toErase.size(); i++) {
		contents.erase(toErase[i]);
	}
	globalDelta -= gReduction;
}

void TopK::reduceDeltas(double gReduction, Blacklist* bl){
	vector<int> toErase;

	for (unordered_map<int, TopKEntry>::iterator it = contents.begin();
			it != contents.end(); it++) {
		TopKEntry& value = it->second;
		if (bl){
			if (!bl->hasItem(it->first)) {
				value.delta -= gReduction;
			}
		}
		if (value.delta < 0) {
			cout << "################ FEHLER #####################" << endl;
		}
		if (value.delta < (minTopValue - value.weight)) {
			toErase.push_back(it->first);
		}
	}
	for (size_t i = 0; i < toErase.size(); i++) {
		contents.erase(toErase[i]);
	}
	globalDelta -= gReduction;
	cout << "globalDelta: " << globalDelta << endl;
}

void TopK::printTopK(){
	cout << "TOP K:" << endl;
	for (unordered_set<int>::iterator it = topElements.begin(); it != topElements.end(); it++){
		cout << "\t" << *it << ": (" << contents[*it].weight << " ; " << contents[*it].delta << ")" << endl;
	}
}

bool TopK::hasConverged(){
	return ( (globalDelta < minTopValue) && (topElements.size() == contents.size()) );
}

unordered_set<int>& TopK::getTopK() {
	return topElements;
}

double TopK::getGlobalDelta(){
	return globalDelta;
}

size_t TopK::getContentsSize(){
	return contents.size();
}

void TopK::determineMin() {
	double minValue = 1;
	int minId;
	for (unordered_set<int>::iterator it = topElements.begin();
			it != topElements.end(); it++) {
		double newValue = contents[*it].weight;
		if (newValue < minValue) {
			minId = *it;
			minValue = newValue;
		}
	}
	minTopId = minId;
	minTopValue = minValue;
}

#endif /* TOPK_H_ */
