/*
 * DebugHelpers.h
 *
 *  Created on: 21.09.2016
 *      Author: michael
 */

#ifndef DEBUGHELPERS_H_
#define DEBUGHELPERS_H_

#include <vector>
#include <iostream>

using namespace std;

class DebugHelpers{
public:
	// debug
	static void printTrail(vector<int>& trail) {
		cout << "[ ";
		for (size_t i = 0; i < trail.size(); i++) {
			cout << trail[i] << " ";
		}
		cout << "]";
	}

};



#endif /* DEBUGHELPERS_H_ */
