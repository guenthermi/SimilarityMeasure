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
#include "Tracker.h"

#include <iostream>
#include <unordered_map>
#include <string>
#include <ostream>
#include <fstream>
#include <string.h>

using namespace std;

class Main {
public:
	int main(int argc, const char* argv[]);
	void printTopK(unordered_map<int, TopKEntry> top, ostream& stream);

protected:
	string helpText =
			"usage: similarityMeasure indexFileLocation/IndexFileName [LogFile] [TrackFile]\n\n"
			"More information about similarityMeasure: "
			"https://github.com/guenthermi/SimilarityMeasure";

	ostream* log = NULL;
};

int Main::main(int argc, const char* argv[]) {
	if (!((argc == 2) || (argc == 3) || (argc == 4))){
		cout << helpText << endl;
		return 0;
	}


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
	ostream* stream;
	if (argc >= 3){
		char* csStdout = "stdout";
		if (strcmp(argv[2], csStdout) == 0){
			stream = &cout;
		}else{
			string logFile = string(argv[2]);
			log = new ofstream(logFile);
			stream = log;
		}
	}else{
		stream = &cout;
	}
	Tracker* tracker = NULL;
	if (argc == 4){
		string trackFile = argv[3];
		tracker = new Tracker(trackFile);
		cout << "comma seperated list of items you want to tack (without \"Q\"):" << endl;
		string listeners;
		cin.ignore();
		getline(cin,listeners);
		string buffer;
		for (int i=0; i<listeners.length(); i++){
			if (listeners[i] != ','){
				buffer += listeners[i];
			}else{
				tracker->registerListener(atoi(buffer.c_str()));
				buffer = "";
			}
		}
		tracker->registerListener(atoi(buffer.c_str()));
	}

	string path = string(argv[1]);
	IndexReader reader(path);

	TopKSearch tks(reader, k, acc, stream, tracker);


	unordered_map<int, TopKEntry> top = tks.search(itemId);

	printTopK(top, *stream);

	if (log != NULL){
		delete log;
	}
	if (tracker != NULL){
		tracker->print();
		delete tracker;
	}

	return 0;
}


void Main::printTopK(unordered_map<int, TopKEntry> top, ostream& stream) {
	stream << "TOP K:" << endl;
	for (unordered_map<int, TopKEntry>::iterator it = top.begin();
			it != top.end(); it++) {
		string name = "Q" + std::to_string(it->first);
		stream << "\t" << "(" << it->second.weight << " ; " << it->second.delta
				<< ") \t: " << name << endl;
	}
}

void testIndexReader() {
	cout << "Start Testing - IndexReader" << endl;
	IndexReader reader(
			"/home/michael/workspace/cpp/IndexTransformator/indexFiles/combinedIndexBin");
	Item* item;
	for (int i = 0; i < 1000; i++) {
		item = &reader.getItemById(i);
	}
	cout << item->getDegree() << endl;
	cout << item->getId() << " -- " << item->getStatementGroups()[0].size()
			<< " -- " << item->getStatementGroups()[0].getPropertyId() << " -- "
			<< item->getStatementGroups()[0].getTargets()[0] << endl;

	cout << "Complete IndexReader Test" << endl << endl;
}

int main(int argc, const char* argv[]) {
	Main myMain = Main();
	return myMain.main(argc, argv);
}

void testAStarSearch() {

	// Items for testing
	int testItems[] = { 22101573, 	// show based on Amadeus
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

	TopKSearch tks(reader, 3, 7, &cout, NULL);

	Main myMain = Main();

	unordered_map<int, TopKEntry> top = tks.search(testItems[1]);

	myMain.printTopK(top, cout);

	cout << "Complete AStarSearch Test" << endl << endl;
}

