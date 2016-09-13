/*
 * InMemoryIndexReader.h
 *
 *  Created on: 10.06.2016
 *      Author: michael
 */

#ifndef INMEMORYINDEXREADER_H_
#define INMEMORYINDEXREADER_H_

#include "datamodel/Item.h"
#include "datamodel/StatementGroup.h"
#include "datamodel/BinaryIndex.h"

#include <iostream>
#include <map>
#include <fstream>
#include <limits>
#include <string>

#include <iostream>

using namespace std;

class InMemoryIndexReader{
public:

	int cacheSize = 1000000;
	static const int cacheFreeRate = 100000;

	InMemoryIndexReader(string path);
	~InMemoryIndexReader();

	void jumpToBegin();
	bool hasNextItem();
	bool getNextItem(Item& item, bool caching);
	Item& getItemById(int id);
	void setInUseFlag(int id);
	void unsetInUseFlag(int id);


protected:

	class CacheLine {
	public:
		CacheLine(Item item) :
				entry(item) {
			usage = 0;
		}
		unsigned char usage;
		Item entry;
		bool inUse;
	};

	void pushToCache(Item item);
	void freeCache(int amount);

	map<int, CacheLine*> cache;

	int minCacheUsage;
	Item nullItem;

	int cacheUsed;
	int fileUsed;

	BinaryIndex index;
	int pos;
	bool readIt;
	Item current;
};

InMemoryIndexReader::InMemoryIndexReader(string path){

	cache = map<int, CacheLine*>();
	minCacheUsage = 0;
	cacheUsed = 0;
	fileUsed = 0;
	nullItem = Item(0);

	pos = 0;
	readIt = true;
	current = Item();
	index.init(path + ".map", path + ".data");
	cout << "Index is loaded" << endl;
}

InMemoryIndexReader::~InMemoryIndexReader(){

}

bool InMemoryIndexReader::hasNextItem(){
	if (!readIt){
		return true;
	}
	while(pos < BinaryIndex::kItemSize){
		current = index.getItem(++pos);
		if (current.getId() != 0){
			readIt = false;
			return true;
		}
	}
}

bool InMemoryIndexReader::getNextItem(Item& item, bool caching){
	readIt = true;
	if (caching){
		pushToCache(current);
	}
	item = current;
	return true;
}

Item& InMemoryIndexReader::getItemById(int id){
	map<int, CacheLine*>::iterator ii = cache.find(id);
	if (ii != cache.end()){
		cacheUsed++;
		return cache[id]->entry;
	}
	fileUsed++;
	Item item = index.getItem(id);
	if (item.getId() != 0){
		pushToCache(item);
		return cache[id]->entry;
	}
	return nullItem;

}

void InMemoryIndexReader::jumpToBegin(){
	pos = 0;
	readIt = true;
	current = Item();
}

void InMemoryIndexReader::setInUseFlag(int id){
//	cout << "lookup: " << id << " is at the end: " << (cache.find(id) == cache.end()) << endl;
	cache[id]->inUse = true;
}

void InMemoryIndexReader::unsetInUseFlag(int id){
	cache[id]->inUse = false;
}

void InMemoryIndexReader::pushToCache(Item item) {
	if (cache.size() >= cacheSize) {
		freeCache(cacheFreeRate);
	}
	cache[item.getId()] = new CacheLine(item);
}

void InMemoryIndexReader::freeCache(int amount) {
	if (amount < 0) {
		cache.clear();
	} else {
		int todo = amount;
		int* idArray = new int[amount];
		for (int i = 0; i < amount; i++) {
			idArray[i] = NULL;
		}
		int j = 0;
		int inUseRate = 0;
		while ((todo > 0) && (inUseRate < cacheFreeRate)) {
			map<int, CacheLine*>::iterator ii;
			for (ii = cache.begin(); (ii != cache.end()) && (todo > 0); ++ii) {
				if (ii->second->inUse){
					inUseRate++;
					continue;
				}
				if (ii->second->usage <= minCacheUsage) {
					idArray[j] = ii->first;
					delete ii->second;
					todo--;
					j++;
				}
			}
			if (ii == cache.end()) {
				if (minCacheUsage == 255){
					return;
				}
				minCacheUsage++;
			}
			if (inUseRate >= (cacheFreeRate / 2)){
				cacheSize += cacheFreeRate;
			}
		}
		for (int i = 0; i < j; i++) {
			cache.erase(idArray[i]);
		}
		cout << "cache used: " << cacheUsed << "file used: " << fileUsed
				<< endl;
		cacheUsed = 0;
		fileUsed = 0;
	}
}


#endif /* INMEMORYINDEXREADER_H_ */
