/*
 * IndexReader.h
 *
 *  Created on: 10.06.2016
 *      Author: michael
 */

#ifndef INDEXREADER_H_
#define INDEXREADER_H_

#include "datamodel/Item.h"
#include "datamodel/StatementGroup.h"
#include "datamodel/BinaryIndex.h"

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <limits>
#include <string>

#include <iostream>

using namespace std;

class IndexReader{
public:

	int cacheSize = 4000000;
	static const int cacheFreeRate = 1000000;

	IndexReader(string path);
	~IndexReader();

	void jumpToBegin();
	bool hasNextItem();
	bool getNextItem(Item& item, bool caching);
	Item& getItemById(int id);
	Item& getItemById(int id, bool*& inUse);
	void setInUseFlag(int id);
	void unsetInUseFlag(int id);


protected:

	class CacheLine {
	public:
		CacheLine(Item item) :
				entry(item) {
			usage = 0;
			inUse = false;
		}
		~CacheLine(){
			entry.clear();
		}
		unsigned char usage;
		Item entry;
		bool inUse;
	};

	void freeCache(int amount);

	CacheLine* pushToCache(Item item){
		if (cache.size() >= cacheSize) {
			freeCache(cacheFreeRate);
		}
		CacheLine* line = new CacheLine(item);
		cache[item.getId()] = line;
		return line;
	};


	unordered_map<int, CacheLine*> cache;

	int minCacheUsage;
	Item nullItem;

	int cacheUsed;
	int fileUsed;

	BinaryIndex index;
	int pos;
	bool readIt;
	Item current;
	int inUse = 0;
};

IndexReader::IndexReader(string path){

	cache = unordered_map<int, CacheLine*>();
	minCacheUsage = 0;
	cacheUsed = 0;
	fileUsed = 0;
	nullItem = Item();

	pos = 0;
	readIt = true;
	current = Item();
	index.init(path + ".map", path + ".data");
	cout << "Index is loaded" << endl;
}

IndexReader::~IndexReader(){

}

bool IndexReader::hasNextItem(){
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

bool IndexReader::getNextItem(Item& item, bool caching){
	readIt = true;
	if (caching){
		pushToCache(current);
	}
	item = current;
	return true;
}

Item& IndexReader::getItemById(int id){
	unordered_map<int, CacheLine*>::iterator ii = cache.find(id);
	if (ii != cache.end()){
		cacheUsed++;
		return ii->second->entry;
	}
	fileUsed++;
	Item item = index.getItem(id);
	if (item.getId() != 0){
		pushToCache(item);
		return cache[id]->entry;
	}
	return nullItem;

}

Item& IndexReader::getItemById(int id, bool*& inUse){
	unordered_map<int, CacheLine*>::iterator ii = cache.find(id);
	if (ii != cache.end()){
		cacheUsed++;
		inUse = &ii->second->inUse;
		return ii->second->entry;
	}
	fileUsed++;
	Item item = index.getItem(id);
	if (item.getId() != 0){
		CacheLine* line = pushToCache(item);
		inUse = &line->inUse;
		return line->entry;
	}
	return nullItem;

}

void IndexReader::jumpToBegin(){
	pos = 0;
	readIt = true;
	current = Item();
}

void IndexReader::setInUseFlag(int id){
//	cout << "lookup: " << id << " is at the end: " << (cache.find(id) == cache.end()) << endl;
	cache[id]->inUse = true;
	inUse++;
}

void IndexReader::unsetInUseFlag(int id){
	cache[id]->inUse = false;
	inUse--;
}

void IndexReader::freeCache(int amount) {
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
			unordered_map<int, CacheLine*>::iterator ii;
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
		cout << "cache used: " << cacheUsed << "file used: " << fileUsed << " In Use: " << inUse
				<< endl;
		cacheUsed = 0;
		fileUsed = 0;
	}
}


#endif /* INDEXREADER_H_ */
