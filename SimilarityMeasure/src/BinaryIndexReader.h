/*
 * BinaryIndexReader.h
 *
 *  Created on: 09.06.2016
 *      Author: michael
 */

#ifndef BINARYINDEXREADER_H_
#define BINARYINDEXREADER_H_

#include "IndexReader.h"

#include <string>
#include <fstream>
#include <iostream>

class BinaryIndexReader: public PartedIndexReader {
public:
	BinaryIndexReader(string path);
	~BinaryIndexReader();

	bool hasNextItem();
	Item& getItemById(int id);

protected:
};

BinaryIndexReader::BinaryIndexReader(string path) :
		PartedIndexReader(path) {
}

BinaryIndexReader::~BinaryIndexReader() {

}

bool BinaryIndexReader::hasNextItem() {
	if (readIt == false) {
		return true;
	}
	filePointer++;
	while (filePointer < Index::kItemSize) {

		// read position of item
		int pos;
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

Item& BinaryIndexReader::getItemById(int id) {
	// read item from cache
	map<int, CacheLine*>::iterator ii = cache.find(id);
	if (ii != cache.end()) {
		CacheLine* line = ii->second;
		line->usage++;
		cacheUsed++;
		return line->entry;
	}
	// item is not in the cache -> read from file
	file.clear();
//	file.seekg(0, ios_base::beg); // TODO look if this is necessary
	dataFile.clear();
//	dataFile.seekg(0, ios_base::beg);
	dataFile.seekg(0, ios_base::end);

	fileUsed++;

	int pos;
	string line;
	file.seekg(0, ios_base::end);
	file.seekg(id * sizeof(pos), ios_base::beg);
	file.read((char*) &pos, sizeof(pos));
	dataFile.seekg(pos, ios_base::beg);
	if (pos != -1) {
		Item item = parser.parseBinaryItem(id, dataFile);
//		getline(dataFile, line);
//		dataFile.seekg(0, ios_base::end); // TODO check what this does...

//		item.sortStmtGrsBySize();
		pushToCache(item);
		return cache[item.getId()]->entry;
	} else {
		return nullItem;
	}
}



#endif /* BINARYINDEXREADER_H_ */
