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

class Main{
public:
	int main();
	void printTopK(unordered_map<int, TopKEntry> top, WebApi* api);

};

int Main::main() {
	cout << "Similarity Measure" << endl;

	int k;
	int maxIteration;
	int itemId;
	int acc;

	cout << "ItemID: Q";
	cin >> itemId;
	cout << "Number of results (k): ";
	cin >> k;
	cout << "Maximal Accurancy: ";
	cin >> acc;

	IndexReader reader("indexFiles/combinedIndexBin");
	TopKSearch tks(reader, k, acc);
	WebApi api;

	unordered_map<int, TopKEntry> top = tks.search(itemId);
	printTopK(top, &api);

	return 0;
}

void Main::printTopK(unordered_map<int, TopKEntry> top, WebApi* api){
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

int main(){
	Main myMain = Main();
	return myMain.main();
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
		39275		// Mouse
	};

	cout << "Start Testing - AStarSearch" << endl;
	IndexReader reader(
				"/home/michael/workspace/cpp/IndexTransformator/indexFiles/combinedIndexBin");

	TopKSearch tks(reader, 3, 7);

	WebApi api;
	Main myMain = Main();

	unordered_map<int, TopKEntry> top = tks.search(testItems[1]);
	myMain.printTopK(top, &api);

	cout << "Complete AStarSearch Test" << endl << endl;
}

