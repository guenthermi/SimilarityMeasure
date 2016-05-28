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
#include "NodeFinder.h"
#include "PropertyStatistics.h"
#include "AStarSearch.h"
#include "PartedIndexReader.h"
#include "WebApi.h"
#include "Blacklist.h";

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

void testPartedIndexReader() {
	cout << "Start Testing - PartedIndexReader" << endl;

	PartedIndexReader inReader(
			"/home/michael/workspace/cpp/IndexTransformator/indexFiles/incommingEdgesIndex");
	Item* item;
	for (int i=0; i<100; i++){
		item = &inReader.getItemById(17329259);
	}
	cout << item->getDegree() << endl;
	cout << item->getId() << " -- "
			<< item->getStatementGroups()[0].getTargets().size() << "--"
			<< item->getStatementGroups()[0].getPropertyId() << "--"
			<< item->getStatementGroups()[0].getTargets()[0] << endl;

	cout << "Complete PartedIndexReader Test" << endl << endl;
}

void testAStarSearch() {
	cout << "Start Testing - AStarSearch" << endl;
	PartedIndexReader outReader(
			"/home/michael/workspace/cpp/IndexTransformator/indexFiles/outgoingEdgesIndex");
	PartedIndexReader inReader(
			"/home/michael/workspace/cpp/IndexTransformator/indexFiles/incommingEdgesIndex");
	AStarSearch ass(inReader, outReader);
//	ass.search(22101573);

//	ass.search(22101603);

//	ass.search(251657);

//	ass.search(421);

//	ass.search(2001293); // -> sehr langsam?!

//	ass.search(567);

	ass.search(43361);

//	Item& item = inReader.getItemById(13442814);

	cout << "Complete AStarSearch Test" << endl << endl;
}

int main() {
	cout << "Start Testing" << endl << endl;
//	testIndexReader();
	testAStarSearch();
//	testPartedIndexReader();
	cout << "Ende" << endl;
	return 0;
}

