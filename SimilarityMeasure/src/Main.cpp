/*
 * Main.cpp
 *
 *  Created on: 04.05.2016
 *      Author: michael
 */

#include "datamodel/Item.h"
#include "IndexReader.h"
#include "TopKSearch.h"
#include "IndexReader.h"
#include "WebApi.h"

#include <iostream>

using namespace std;

void testIndexReader(){
	cout << "Start Testing - IndexReader" << endl;
	IndexReader reader(
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

	cout << "Complete IndexReader Test" << endl << endl;
}

void testAStarSearch() {

	// Items for testing
	int testItems[] = {
		22101573, 	// show based on Amadeus
		22101603, 	// russian encyclopedic article
		421, 		// UFO
		2001293, 	// Tine Reymer (singer, actor) -> sehr langsam?!
		567, 		// Angela Merkel
		183, 		// Germany
		8337, 		// Harry Potter (novel series)
		111 		// Mars
	};

	cout << "Start Testing - AStarSearch" << endl;
	IndexReader reader(
				"/home/michael/workspace/cpp/IndexTransformator/indexFiles/combinedIndexBin");

	TopKSearch tks(reader);

	tks.search(testItems[1]);

	cout << "Complete AStarSearch Test" << endl << endl;
}

int main() {
	cout << "Start Testing" << endl << endl;
	testAStarSearch();
	cout << "End" << endl;
	return 0;
}

