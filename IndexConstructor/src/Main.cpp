/*
 * Main.cpp
 *
 *  Created on: 26.04.2016
 *      Author: michael
 */

#include "datamodel/Item.h"
#include "datamodel/StatementGroup.h"
#include "Parser.h"
#include "datamodel/Index.h"
#include "IndexPrinter.h"
#include "IndexCreator.h"
#include "Merge.h"

#include <iostream>
#include <string>

using namespace std;

void createPartedIncommingIndexBin(string src, string dest){
	IndexCreator icreator(src.c_str());
	icreator.createIncommingEdgesIndex();
	IndexPrinter printer(icreator.getIndex());
	printer.printIndexBinary(dest);
}

void createPartedOutgoingIndexBin(string src, string dest){
	IndexCreator icreator(src.c_str());
	icreator.createOutgoingEdgesIndex();
	IndexPrinter printer(icreator.getIndex());
	printer.printIndexBinary(dest);
}

void mergeIndexes(string src1, string src2, string dest){
	Merge merge(src1, src2, dest);
	merge.createOutput();
}

int main(int argc, const char* argv[]){
	if (argc < 2){
		cout << "Invalid number of arguments" << endl;
	}
	if (string("--outgoing").compare(argv[1]) == 0){
		if (argc != 4){
			cout << "Invalid number of arguments" << endl;
		}
		createPartedOutgoingIndexBin(argv[2], argv[3]);
	}
	if (string("--incomming").compare(argv[1]) == 0){
		if (argc != 4){
			cout << "Invalid number of arguments" << endl;
		}
		createPartedIncommingIndexBin(argv[2], argv[3]);
	}
	if (string("--combined").compare(argv[1]) == 0){
		if (argc != 5){
			cout << "Invalid number of arguments" << endl;
		}
		mergeIndexes(argv[2], argv[3], argv[4]);
	}
	cout << "Done!" << endl;

	return 0;
}

