/*
 * Main.cpp
 *
 *  Created on: 04.05.2016
 *      Author: michael
 */

#include "datamodel/Item.h"
#include "datamodel/StatementGroup.h"
#include "datamodel/Index.h"
#include "IndexReader.h"
#include "AStarSearch.h"
#include "InMemoryIndexReader.h"
#include "WebApi.h"
#include "Blacklist.h"

#include <iostream>

using namespace std;

void testIndexReader() {
	cout << "Start Testing - IndexReader" << endl;
	IndexReader outReader(
			"/home/michael/workspace/cpp/IndexTransformator/indexFiles/edgeIndex.txt");
	Item& item = outReader.getItemById(22101602);
	cout << item.getId() << " -- " << item.getStatementGroups().size() << "--"
			<< item.getStatementGroups()[0].getPropertyId() << "--"
			<< item.getStatementGroups()[0].getTargets()[0] << endl;

	IndexReader inReader(
			"/home/michael/workspace/cpp/IndexTransformator/indexFiles/incommingEdgeIndex.txt");

	item = inReader.getItemById(5);
	cout << item.getId() << " -- "
			<< item.getStatementGroups()[0].getTargets().size() << "--"
			<< item.getStatementGroups()[0].getPropertyId() << "--"
			<< item.getStatementGroups()[0].getTargets()[0] << endl;

	cout << "Complete IndexReader Test" << endl << endl;
}

void testCombinedIndexReader(){
	cout << "Start Testing - CombinedIndexReader" << endl;
	InMemoryIndexReader reader(
			"/home/michael/workspace/cpp/IndexTransformator/indexFiles/combinedIndexBin");
	Item* item;
	for (int i=0; i<1000; i++){
		item = &reader.getItemById(i);
	}
	cout << item->getDegree() << endl;
	cout << item->getId() << " -- "
			<< item->getStatementGroups()[0].getTargets().size() << " -- "
			<< item->getStatementGroups()[0].getPropertyId() << " -- "
			<< item->getStatementGroups()[0].getTargets()[0] << endl;

	cout << "Complete CombinedIndexReader Test" << endl << endl;
}

void testAStarSearch() {
	cout << "Start Testing - AStarSearch" << endl;
	InMemoryIndexReader reader(
				"/home/michael/workspace/cpp/IndexTransformator/indexFiles/combinedIndexBin");

	AStarSearch ass(reader);
//	ass.search(22101573);

//	ass.search(22101603);

//	ass.search(251657);

//	ass.search(421);

//	ass.search(2001293); // -> sehr langsam?!

//	ass.search(567);

	ass.search(183);

//	ass.search(8337);

//	Item& item = inReader.getItemById(13442814);

	cout << "Complete AStarSearch Test" << endl << endl;
}

int main() {
	cout << "Start Testing" << endl << endl;
//	testIndexReader();
	testAStarSearch();
//	testCombinedIndexReader();
	cout << "Ende" << endl;
	return 0;
}

