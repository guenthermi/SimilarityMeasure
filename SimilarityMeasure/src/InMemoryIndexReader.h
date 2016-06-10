/*
 * InMemoryIndexReader.h
 *
 *  Created on: 10.06.2016
 *      Author: michael
 */

#ifndef INMEMORYINDEXREADER_H_
#define INMEMORYINDEXREADER_H_

#include "datamodel/BinaryIndex.h"
#include "IndexReader.h"
#include "datamodel/Index.h"
#include "datamodel/Item.h"

#include <iostream>

using namespace std;

class InMemoryIndexReader : public IndexReader {
public:
	InMemoryIndexReader(string path);
	~InMemoryIndexReader();

	bool hasNextItem();
	bool getNextItem(Item& item, bool caching);
	Item& getItemById(int id);
	void jumpToBegin();

protected:
	BinaryIndex index;
	int pos;
	bool readIt;
	Item current;
};

InMemoryIndexReader::InMemoryIndexReader(string path) : IndexReader(path.c_str()){
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
	while(pos < Index::kItemSize){
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
		return cache[id]->entry;
	}
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


#endif /* INMEMORYINDEXREADER_H_ */
