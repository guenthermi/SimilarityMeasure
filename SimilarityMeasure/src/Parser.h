/*
 * Parser.h
 *
 *  Created on: 26.04.2016
 *      Author: michael
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "datamodel/Index.h"
#include "datamodel/Item.h"
#include "datamodel/StatementGroup.h"

#include <vector>
#include <string>
#include "stdlib.h"
#include <iostream>
#include <algorithm>

using namespace std;

class Parser {
public:
	Parser();
	Item parseItem(string line);
	int* parsePropertyCount(string line);
protected:
	static void split(string text, char dlm, vector<string>& result) {
		string::const_iterator start = text.begin();
		string::const_iterator end = text.end();
		string::const_iterator next = find( start, end, dlm );
		while ( next != end ) {
			result.push_back( string( start, next ) );
			start = next + 1;
			next = find( start, end, dlm );
		}
		if (start < end){
			result.push_back( string( start, next ) );
		}
	}
	static void splitInts(string text, char dlm, vector<int>& result) {
		vector<string> splits;
		split(text, dlm, splits);
//		cout << "1.1" << endl;
		for (size_t i = 0; i < splits.size(); i++) {
			result.push_back(atoi(splits[i].c_str()));
		}
	}
};

Parser::Parser() {

}

Item Parser::parseItem(string line) {
	vector<string> splits;
	split(line, ';', splits);
	int itemId = atoi(splits[0].c_str());
	Item item = Item(itemId);
	for (size_t i = 1; i < splits.size(); i++) {
		vector<int> stmtgr;
		splitInts(splits[i], ',', stmtgr);
		StatementGroup statementGroup(stmtgr[0]);
		size_t j;
		for (j = 1; j < stmtgr.size(); j++) {
			statementGroup.pushTarget(stmtgr[j]);
		}
		item.pushStmtGr(statementGroup);
	}

	return item;
}

int* Parser::parsePropertyCount(string line){
	vector<int> splits;
	splitInts(line, ':', splits);
	int array[2];

	if (splits.size() != 2){
		return NULL;
	}

	array[0] = splits[0];
	array[1] = splits[1];
	return array;

}

#endif /* PARSER_H_ */
