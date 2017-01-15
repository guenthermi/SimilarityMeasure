/*
 * IndexPrinter.h
 *
 *  Created on: 29.04.2016
 *      Author: michael
 */

#ifndef INDEXPRINTER_H_
#define INDEXPRINTER_H_

#include "datamodel/Index.h"

#include <fstream>
#include <string>
#include <iostream>

class IndexPrinter {
public:

	IndexPrinter(Index* index);

	void printIndexBinary(string outputPath);
	void printPropertyCounts(char* outputPath);

protected:

	Index* index;

	long writeItemBinary(ostream& stream, vector<StatementGroup>& statements);
};

IndexPrinter::IndexPrinter(Index* index) {
	this->index = index;
}

void IndexPrinter::printIndexBinary(string outputPath) {
	ofstream mapStream((outputPath + ".map").c_str(), ios::out | ios::binary);
	ofstream dataStream((outputPath + ".data").c_str(), ios::out | ios::binary);
	if (mapStream.is_open() && dataStream.is_open()) {
		vector<StatementGroup>** data = index->getData();
		for (int i = 0; i < index->getSize(); i++) {
			int pos;

			// write item in data file
			if (data[i]) {
				pos = writeItemBinary(dataStream, (*data[i]));
			} else {
				pos = -1;
			}

			// write map entry
			mapStream.write((char*) &pos, sizeof(pos));
		}
	}
	mapStream.close();
	dataStream.close();
}

void IndexPrinter::printPropertyCounts(char* outputPath) {
	ofstream stream(outputPath);
	if (stream.is_open()) {
		int* properties = index->getPropertyCounts();
		for (int i = 0; i < (index->kNumberOfProperties); i++) {
			if (properties[i] != 0) {
				stream << i << ":" << properties[i] << endl;
			}
		}
	}
	stream.close();
}

long IndexPrinter::writeItemBinary(ostream& stream, vector<StatementGroup>& statements){
	long pos = stream.tellp();
	int size = statements.size();
	stream.write((char*) &size, sizeof(int));
	for (size_t i = 0; i < statements.size(); i++){
		int pId = statements[i].getPropertyId();
		stream.write((char*) &pId, sizeof(pId));
		vector<int>& targets = statements[i].getTargets();
		size = targets.size();
		stream.write((char*) &size, sizeof(int));
		for (size_t j = 0; j < targets.size(); j++){
			stream.write((char*) &targets[j], sizeof(targets[j]));
		}
	}
	return pos;
}

#endif /* INDEXPRINTER_H_ */
