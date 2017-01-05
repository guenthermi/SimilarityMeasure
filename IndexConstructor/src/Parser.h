/*
 * Parser.h
 *
 *  Created on: 26.04.2016
 *      Author: michael
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "datamodel/Item.h"
#include "datamodel/StatementGroup.h"

#include <vector>
#include <string>
#include "stdlib.h"
#include <iostream>

using namespace std;

class Parser {
public:
	Parser();
	Item parseItem(string line);
protected:
	static vector<string> split(string text, char dlm) {
		size_t pos;
		vector<string> result;
		while ((pos = text.find(dlm)) != string::npos) {
			result.push_back(text.substr(0, pos));
			text.erase(0, pos + 1);
		}
		if (text.length()){
			result.push_back(text);
		}
		return result;
	}
	static vector<int> splitInts(string text, char dlm) {
		vector<string> splits = split(text, dlm);
		vector<int> result;
		for (int i = 0; i < splits.size(); i++) {
			result.push_back(atoi(splits[i].c_str()));
		}
		return result;
	}
};

Parser::Parser() {

}

Item Parser::parseItem(string line) {
	vector<string> splits = split(line, ';');
	int itemId = atoi(splits[0].c_str());
	Item item = Item(itemId);
	for (int i = 1; i < splits.size(); i++) {
		vector<int> stmtgr = splitInts(splits[i], ',');
		StatementGroup statementGroup(stmtgr[0]);
		for (int j = 1; j < stmtgr.size(); j++) {
			statementGroup.pushTarget(stmtgr[j]);
		}
		item.pushStmtGr(statementGroup);
	}

	return item;
}
#endif /* PARSER_H_ */
