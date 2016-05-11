/*
 * PropertyStatistics.h
 *
 *  Created on: 04.05.2016
 *      Author: michael
 */

#ifndef PROPERTYSTATISTICS_H_
#define PROPERTYSTATISTICS_H_

#include "datamodel/Index.h"
#include "Parser.h"

#include <fstream>

using namespace std;

class PropertyStatistics {
public:

	PropertyStatistics(char* path);
	~PropertyStatistics();

	int* getPropertyCounts();

protected:
	char* path;
	int* propertyCounts;
};

PropertyStatistics::PropertyStatistics(char* path) {
	this->path = path;
	this->propertyCounts = NULL;
}

PropertyStatistics::~PropertyStatistics(){
	delete propertyCounts;
}

int* PropertyStatistics::getPropertyCounts() {
	if (propertyCounts != NULL) {
		return propertyCounts;
	} else {
		propertyCounts = new int[Index::kNumberOfProperties];
		ifstream file(path);
		string line;
		Parser parser;
		if (file.is_open()) {
			while (!file.eof()) {
				getline(file, line);
				int* count = parser.parsePropertyCount(line);
				propertyCounts[count[0]] = count[1];
			}
		}
		return propertyCounts;
	}
}

#endif /* PROPERTYSTATISTICS_H_ */
