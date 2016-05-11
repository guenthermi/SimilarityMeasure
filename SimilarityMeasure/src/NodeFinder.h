/*
 * NodeFinder.h
 *
 *  Created on: 04.05.2016
 *      Author: michael
 */

#ifndef NODEFINDER_H_
#define NODEFINDER_H_

#include "Direction.h"

#include <vector>

using namespace std;

class NodeFinder {
public:

	NodeFinder(IndexReader& inReader, IndexReader& outReader);
	vector<int>& getIncommingNeighbours(int itemId, int property);
	vector<int>& getOutgoingNeighbours(int itemId, int property);

protected:

	vector<int>& getNeighbours(int itemId, int property, Direction direct);

	IndexReader& iReader;
	IndexReader& oReader;
};

NodeFinder::NodeFinder(IndexReader& inReader, IndexReader& outReader) :
		iReader(inReader), oReader(outReader) {
}

vector<int>& NodeFinder::getNeighbours(int itemId, int property,
		Direction direct) {
	IndexReader& reader = (direct == incomming) ? iReader : oReader;
	Item& item = reader.getItemById(itemId);
	vector<StatementGroup>& stmtGrps = item.getStatementGroups();
	for (int i = 0; i < stmtGrps.size(); i++) {
		if (stmtGrps[i].getPropertyId() == property) {
			return stmtGrps[i].getTargets();
		}
	}
	throw 20;
}

vector<int>& NodeFinder::getIncommingNeighbours(int itemId, int property) {
	return getNeighbours(itemId, property, incomming);
}

vector<int>& NodeFinder::getOutgoingNeighbours(int itemId, int property) {
	return getNeighbours(itemId, property, outgoing);
}

#endif /* NODEFINDER_H_ */
