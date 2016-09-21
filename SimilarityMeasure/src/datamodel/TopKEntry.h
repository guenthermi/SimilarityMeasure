/*
 * TopKEntry.h
 *
 *  Created on: 13.06.2016
 *      Author: michael
 */

#ifndef TOPKENTRY_H_
#define TOPKENTRY_H_

class TopKEntry{
public:
	TopKEntry();
	double weight;
	double delta;
};

TopKEntry::TopKEntry(){
	weight = 0;
	delta = 0;
}

#endif /* TOPKENTRY_H_ */
