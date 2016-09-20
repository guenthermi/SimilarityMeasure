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

class IndexReader {
public:

	int cacheSize = 2000000;
	static const int cacheFreeRate = 500000;

	IndexReader(string path);
	~IndexReader();

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
		~CacheLine() {
			entry.clear();
		}
		unsigned char usage;
		Item entry;
		bool inUse;
	};

	void freeCache(int amount);

	CacheLine* pushToCache(Item item) {
		if (cacheUsage >= cacheSize) {
			freeCache(cacheFreeRate);
		}
		CacheLine* line = new CacheLine(item);
		cache[item.getId()] = line;
		cacheUsage++;
		return line;
	}
	;

//	unordered_map<int, CacheLine*> cache;
	CacheLine** cache;
	int cacheUsage;

	int minCacheUsage;
	Item nullItem;

	int cacheUsed;
	int fileUsed;

	BinaryIndex index;
	int inUseCount = 0;
};

IndexReader::IndexReader(string path) {

//	cache = unordered_map<int, CacheLine*>();
	cache = new CacheLine*[25000000];
	for (int i = 0; i < 25000000; i++) {
		cache[i] = NULL;
	}
	cacheUsage = 0;
	minCacheUsage = 0;
	cacheUsed = 0;
	fileUsed = 0;
	nullItem = Item();

	index.init(path + ".map", path + ".data");
	cout << "Index is loaded" << endl;
}

IndexReader::~IndexReader() {

}

Item& IndexReader::getItemById(int id) {
	CacheLine* ii = cache[id];
	if (ii != NULL) {
		cacheUsed++;
		if (ii->usage < 255) {
			ii->usage++;
		}
		return ii->entry;
	}
	fileUsed++;
	Item item = index.getItem(id);
	if (item.getId() != 0) {
		pushToCache(item);
		return cache[id]->entry;
	}
	return nullItem;

}

Item& IndexReader::getItemById(int id, bool*& inUse) {
	CacheLine* ii = cache[id];
	if (ii != NULL) {
		cacheUsed++;
		if (ii->usage < 255) {
			ii->usage++;
		}
		inUse = &ii->inUse;
		return ii->entry;
	}
	fileUsed++;
	Item item = index.getItem(id);
	if (item.getId() != 0) {
		CacheLine* line = pushToCache(item);
		inUse = &line->inUse;
		return line->entry;
	}
	return nullItem;
}

void IndexReader::setInUseFlag(int id) {
//	cout << "lookup: " << id << " is at the end: " << (cache.find(id) == cache.end()) << endl;
	cache[id]->inUse = true;
	inUseCount++;
}

void IndexReader::unsetInUseFlag(int id) {
	cache[id]->inUse = false;
	inUseCount--;
}

void IndexReader::freeCache(int amount) {
	cout << "Call free cache (" << amount << ")" << endl;
	if (amount < 0) {
		for (int i=0; i<25000000; i++){
			cache[i] = NULL;
		}
	} else {
		int todo = amount;
		int j = 0;
		int inUseRate = 0;
		while ((todo > 0) && (inUseRate < cacheFreeRate)) {
			int ii = 0;
			for (ii = 0; (ii < 25000000) && (todo > 0); ++ii) {
				if (cache[ii] !=NULL){
					if (cache[ii]->inUse) {
						inUseRate++;
						continue;
					}
					if (cache[ii]->usage <= minCacheUsage) {
						delete cache[ii];
						cache[ii] = NULL;
						todo--;
						j++;
						cacheUsage--;
					}
				}
			}
			if (ii == 25000000) {
				if (minCacheUsage == 255) {
					return;
				}
				minCacheUsage++;
			}
			if (inUseRate >= (cacheSize / 2)) {
				cacheSize += cacheFreeRate;
				break;
			}
		}
		cout << "cache used: " << cacheUsed << "file used: " << fileUsed
				<< " In Use: " << inUseRate << endl;
		cacheUsed = 0;
		fileUsed = 0;
	}
}

#endif /* INDEXREADER_H_ */
