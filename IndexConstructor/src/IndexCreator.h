/*
 * IndexCreator.h
 *
 *  Created on: 30.04.2016
 *      Author: michael
 */

#ifndef INDEXCREATOR_H_
#define INDEXCREATOR_H_

#include "datamodel/Index.h"
#include "datamodel/Item.h"
#include "IndexReader.h"

#include <iostream>

using namespace std;

class IndexCreator {
public:
	IndexCreator(const char* indexPath);
	Index* getIndex();
	void createIncommingEdgesIndex();
	void createOutgoingEdgesIndex();
protected:
	Index* index;
	const char* indexPath;
	IndexReader reader;
};

IndexCreator::IndexCreator(const char* indexPath) :
		reader(indexPath) {
	index = new Index();
	this->indexPath = indexPath;
}

Index* IndexCreator::getIndex() {
	return index;
}

void IndexCreator::createIncommingEdgesIndex() {
	int statementCount = 0;
	int itemCount = 0;
	int y = 0;
	Item item;
	while (reader.hasNextItem()) {
		bool valid = reader.getNextItem(item);
		if (!valid) {
			break;
		}
		itemCount++;
		if (y > 1000000) {
			y = 0;
			cout << "StatementCount: " << statementCount << " ItemCount: "
					<< itemCount << endl;
		}
		for (size_t i = 0; i < item.getStatementGroups().size(); i++) {
			for (size_t j = 0;
					j < item.getStatementGroups()[i].getTargets().size(); j++) {
				index->pushStatement(
						item.getStatementGroups()[i].getTargets()[j],
						item.getStatementGroups()[i].getPropertyId(),
						item.getId());
				y++;
				statementCount++;
			}
		}
	}
}

void IndexCreator::createOutgoingEdgesIndex() {
	int statementCount = 0;
	int itemCount = 0;
	int y = 0;
	Item item;
	while (reader.hasNextItem()) {
		bool valid = reader.getNextItem(item);
		if (!valid) {
			break;
		}
		itemCount++;
		if (y > 1000000) {
			y = 0;
			cout << "StatementCount: " << statementCount << " ItemCount: "
					<< itemCount << endl;
		}
		for (size_t i = 0; i < item.getStatementGroups().size(); i++) {
			for (size_t j = 0;
					j < item.getStatementGroups()[i].getTargets().size(); j++) {
				index->pushStatement(item.getId(),
						item.getStatementGroups()[i].getPropertyId(),
						item.getStatementGroups()[i].getTargets()[j]);
				y++;
				statementCount++;
			}
		}
	}
}

#endif /* INDEXCREATOR_H_ */
