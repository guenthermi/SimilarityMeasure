/*
 * IndexReader.h
 *
 *  Created on: 04.05.2016
 *      Author: michael
 */

#ifndef INDEXREADER_H_
#define INDEXREADER_H_

#include "datamodel/Item.h"
#include "datamodel/StatementGroup.h"
#include "Parser.h"

#include <iostream>
#include <map>
#include <fstream>
#include <limits>
#include <string>

using namespace std;

class IndexReader {
public:

	static const int cacheSize = 1000000;
	static const int cacheFreeRate = 100000;

	IndexReader(string indexFile);
	~IndexReader();

	virtual void jumpToBegin();
	virtual bool hasNextItem();
	virtual bool getNextItem(Item& item, bool caching = false);
	virtual Item& getItemById(int id);
	string getPath();

protected:

	class CacheLine {
	public:
		CacheLine(Item item) :
				entry(item) {
			usage = 0;
		}
		unsigned char usage;
		Item entry;
	};
	map<int, CacheLine*> cache;

	void pushToCache(Item item);
	void freeCache(int amount);

	int minCacheUsage;
	ifstream file;
	Parser parser;
	string filePath;
	Item nullItem;

	int cacheUsed;
	int fileUsed;
};

IndexReader::IndexReader(string indexFile) {
	cache = map<int, CacheLine*>();
	minCacheUsage = 0;
	file.open(indexFile.c_str());
	parser = Parser();
	filePath = indexFile;
	cacheUsed = 0;
	fileUsed = 0;
	nullItem = Item(0);

}

IndexReader::~IndexReader(){
	for (map<int, CacheLine*>::iterator it = cache.begin(); it != cache.end(); it++){
		delete it->second;
	}
}

void IndexReader::jumpToBegin() {
	file.clear();
	file.seekg(0, ios::beg);
}

bool IndexReader::hasNextItem() {
	return !file.eof();
}

bool IndexReader::getNextItem(Item& item, bool caching) {
	string line;
	if (file.is_open()) {
		getline(file, line);
		if (line == "") {
			return false;
		}
		item = parser.parseItem(line);
		if (caching) {
			pushToCache(item);
		}
		return true;
	} else {
		cerr << "file is not open" << endl;
		return -1;
	}
}

Item& IndexReader::getItemById(int id) {
	// read item from cache
	map<int, CacheLine*>::iterator ii = cache.find(id);
	if (ii != cache.end()) {
		CacheLine* line = ii->second;
		line->usage++;
		cacheUsed++;
		return line->entry;
	}
	fileUsed++;

	// item is not in the cache -> read from file
	ifstream stream(filePath.c_str());
	long fileSize, upperBoundPos, lowerBoundPos;

	// compute file size
	stream.seekg(0, ios_base::end);
	fileSize = stream.tellg();
	upperBoundPos = fileSize;
	lowerBoundPos = 0;

	while (true) {

		// set stream position in the middle between upper and lower bound
		stream.seekg((upperBoundPos - lowerBoundPos) / 2 + lowerBoundPos,
				ios_base::beg);
		stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		// parse item in current line
		string line;
		getline(stream, line);
		Item item = parser.parseItem(line);
		if (item.getId() == id) {
			pushToCache(item);
			return cache[item.getId()]->entry;
		}
		if ((upperBoundPos - lowerBoundPos) <= 2) {
			return nullItem;
		}
		if (item.getId() > id) {
			lowerBoundPos = (upperBoundPos - lowerBoundPos) / 2 + lowerBoundPos;
		} else {
			upperBoundPos = (upperBoundPos - lowerBoundPos) / 2 + lowerBoundPos;
		}
	}

	throw 20;
}

string IndexReader::getPath(){
	return filePath;
}

void IndexReader::pushToCache(Item item) {
	cache[item.getId()] = new CacheLine(item);
	if (cache.size() > cacheSize) {
		freeCache(cacheFreeRate);
	}
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
		while (todo > 0) {
			map<int, CacheLine*>::iterator ii;
			for (ii = cache.begin(); (ii != cache.end()) && (todo > 0); ++ii) {
				if (ii->second->usage <= minCacheUsage) {
					idArray[j] = ii->first;
					delete ii->second;
					todo--;
					j++;
				}
			}
			if (ii == cache.end()) {
				minCacheUsage++;
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

#endif /* INDEXREADER_H_ */
