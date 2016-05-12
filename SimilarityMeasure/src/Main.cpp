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

void testAStarSearch() {
	cout << "Start Testing - AStarSearch" << endl;
	IndexReader outReader(
			"/home/michael/workspace/cpp/IndexTransformator/indexFiles/edgeIndex.txt");
	IndexReader inReader(
			"/home/michael/workspace/cpp/IndexTransformator/indexFiles/incommingEdgeIndex.txt");

//	Item& item = inReader.getItemById(13442814);

	cout << "Complete AStarSearch Test" << endl << endl;
}

int main() {
	cout << "Start Testing" << endl << endl;
	testIndexReader();
	testAStarSearch();
	cout << "Ende" << endl;
	return 0;
}

