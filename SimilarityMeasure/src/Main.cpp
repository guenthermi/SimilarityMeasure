/*
 * Main.cpp
 *
 *  Created on: 04.05.2016
 *      Author: michael
 */

#include "datamodel/Item.h"
#include "datamodel/TopKEntry.h"
#include "IndexReader.h"
#include "TopKSearch.h"
#include "IndexReader.h"
#include "WebApi.h"

#include <iostream>
#include <unordered_map>
#include <string>

using namespace std;

void printTopK(unordered_map<int, TopKEntry> top, WebApi* api){
	cout << "TOP K:" << endl;
	for (unordered_map<int, TopKEntry>::iterator it = top.begin(); it != top.end(); it++){
		string name = "Q" + std::to_string(it->first);
		if (api != NULL){
			string label = api->getLabelById(it->first);
			if (label != ""){
				name = label + "(Q" + std::to_string(it->first) + ")";
			}
		}
		cout << "\t" << "(" << it->second.weight << " ; " << it->second.delta << ") \t: " << name  << endl;
	}
}

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
			<< item->getStatementGroups()[0].size() << " -- "
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
		111, 		// Mars
		39275
	};

	cout << "Start Testing - AStarSearch" << endl;
	IndexReader reader(
				"/home/michael/workspace/cpp/IndexTransformator/indexFiles/combinedIndexBin");

	TopKSearch tks(reader, 3);

	WebApi api;

	unordered_map<int, TopKEntry> top = tks.search(testItems[5]);
	printTopK(top, &api);

	cout << "Complete AStarSearch Test" << endl << endl;
}

int main() {
	cout << "Start Testing" << endl << endl;
	testAStarSearch();
	cout << "End" << endl;
	return 0;
}

