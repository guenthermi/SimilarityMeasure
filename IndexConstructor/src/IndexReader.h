/*
 * IndexReader.h
 *
 *  Created on: 26.04.2016
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

using namespace std;

class IndexReader {
public:

	IndexReader(const char* indexFile);
	void jumpToBegin();
	bool hasNextItem();
	bool getNextItem(Item& item);

protected:

	ifstream file;
	Parser parser;
	const char* filePath;

};

IndexReader::IndexReader(const char* indexFile) {
	file.open(indexFile);
	parser = Parser();
	filePath = indexFile;


}

void IndexReader::jumpToBegin() {
	file.clear();
	file.seekg(0, ios::beg);
}

bool IndexReader::hasNextItem(){
	return !file.eof();
}

bool IndexReader::getNextItem(Item& item) {
	string line;
	if (file.is_open()) {
		getline(file, line);
		if (line == ""){
			return false;
		}
		item = parser.parseItem(line);
		return true;
	} else {
		cerr << "file is not open" << endl;
		return -1;
	}
}

#endif /* INDEXREADER_H_ */
