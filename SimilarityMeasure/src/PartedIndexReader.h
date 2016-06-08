/*
 * PartedIndexReader.h
 *
 *  Created on: 15.05.2016
 *      Author: michael
 */

#ifndef PARTEDINDEXREADER_H_
#define PARTEDINDEXREADER_H_

#include "IndexReader.h"

#include <string>
#include <fstream>
#include <iostream>

class PartedIndexReader: public IndexReader {
public:
	PartedIndexReader(string path);
	~PartedIndexReader();

	bool hasNextItem();
	bool getNextItem(Item& item, bool caching);
	Item& getItemById(int id);
	void jumpToBegin();
	string getDataFilePath();

protected:
	ifstream dataFile;
	int filePointer;
	bool readIt;
	Item nextItem;
	string dataFilePath;
};

PartedIndexReader::PartedIndexReader(string path) :
		IndexReader((path + ".map").c_str()) {
	filePointer = 0;
	readIt = true;
	dataFilePath = path + ".data";
	dataFile.open(dataFilePath.c_str());
}

PartedIndexReader::~PartedIndexReader() {
}

void PartedIndexReader::jumpToBegin() {
	filePointer = 0;
	hasNextItem();
}

bool PartedIndexReader::hasNextItem() {
	if (readIt == false) {
		return true;
	}
	filePointer++;
	while (filePointer < Index::kItemSize) {

		// read position of item
		long pos;
		file.seekg(filePointer * sizeof(pos), ios_base::beg);
		file.read((char*) &pos, sizeof(pos));

		// read item
		dataFile.seekg(pos, ios_base::beg);
		string line;
		getline(dataFile, line);
		nextItem = parser.parsePureItem(filePointer, line);
		filePointer++;
	}
	return false;
}

bool PartedIndexReader::getNextItem(Item& item, bool caching) {
	if (readIt) {
		hasNextItem();
	}

	item = nextItem;
//	item.sortStmtGrsBySize();

	if (item.getId() == 0) {
		return false;
	}

	if (caching) {
		pushToCache(item);
	}

	readIt = true;

	return true;
}

Item& PartedIndexReader::getItemById(int id) {
	file.clear();
	file.seekg(0, ios_base::beg);
	dataFile.clear();
	dataFile.seekg(0, ios_base::beg);
	dataFile.seekg(0, ios_base::end);
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
	long pos;
	string line;
	file.seekg(0, ios_base::end);
	file.seekg(id * sizeof(pos), ios_base::beg);
	file.read((char*) &pos, sizeof(pos));
	dataFile.seekg(pos, ios_base::beg);
	if (pos != -1) {
		getline(dataFile, line);
		dataFile.seekg(0, ios_base::end);
		Item item = parser.parsePureItem(id, line);
//		item.sortStmtGrsBySize();
		pushToCache(item);
		return cache[item.getId()]->entry;
	} else {
		return nullItem;
	}
}

string PartedIndexReader::getDataFilePath() {
	return dataFilePath;
}

#endif /* PARTEDINDEXREADER_H_ */
