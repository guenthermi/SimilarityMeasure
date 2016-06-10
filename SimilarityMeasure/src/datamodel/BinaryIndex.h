/*
 * BinaryIndex.h
 *
 *  Created on: 09.06.2016
 *      Author: michael
 */

#ifndef BINARYINDEX_H_
#define BINARYINDEX_H_

#include "Item.h"
#include "../Parser.h"

#include <string>
#include <fstream>


using namespace std;

class BinaryIndex {
public:
	BinaryIndex();
	~BinaryIndex();
	void init(string mapPath, string dataPath);
	Item getItem(int id);

protected:
	int* map;
	int* data;
	int dataSize;
	Parser parser;
};

BinaryIndex::BinaryIndex(){

}

BinaryIndex::~BinaryIndex(){
	delete[] map;
	delete[] data;
}

void BinaryIndex::init(string mapPath, string dataPath){
	ifstream mapFile(mapPath);
	ifstream dataFile(dataPath);
	dataFile.seekg(0, ios_base::end);
	mapFile.seekg(0, ios_base::end);
	dataSize = dataFile.tellg();
	int mapSize = mapFile.tellg();
	map = new int[mapSize / sizeof(int)];
	data = new int[dataSize / sizeof(int)];
	mapFile.seekg(0, ios_base::beg);
	dataFile.seekg(0, ios_base::beg);
	mapFile.read((char*) map, mapSize);
	dataFile.read((char*) data, dataSize);
	mapFile.close();
	dataFile.close();

}

Item BinaryIndex::getItem(int id){
	int pos = map[id];
	if (pos != -1){
		pos /= sizeof(int);
		return parser.parseInMemoryItem(id, pos, data);
	}
	return Item();
}

#endif /* BINARYINDEX_H_ */
