/*
 * Merge.h
 *
 *  Created on: 22.06.2016
 *      Author: michael
 */

#ifndef MERGE_H_
#define MERGE_H_

#include "datamodel/Index.h"

#include <string>
#include <fstream>
#include <vector>
#include <iostream>

using namespace std;

class Merge{
public:
	Merge(string outgoingPath, string incommingPath, string outputPath);
	void createOutput();
	void close();
protected:
	ifstream outgoingMap;
	ifstream outgoingData;

	ifstream incommingMap;
	ifstream incommingData;

	ofstream outputMap;
	ofstream outputData;
};

Merge::Merge(string outgoingPath, string incommingPath, string outputPath){
	outgoingMap.open((outgoingPath + ".map").c_str());
	outgoingData.open((outgoingPath + ".data").c_str());

	incommingMap.open((incommingPath + ".map").c_str());
	incommingData.open((incommingPath + ".data").c_str());

	outputMap.open((outputPath + ".map").c_str());
	outputData.open((outputPath + ".data").c_str());
}

void Merge::createOutput(){
	outgoingMap.clear();
	outgoingData.clear();
	incommingMap.clear();
	incommingData.clear();
	for (int i=0; i < Index::kItemSize; i++){
		int posOut, posIn;
		outgoingMap.seekg(i* sizeof(int), ios_base::beg);
		outgoingMap.read((char*) &posOut, sizeof(posOut));
		incommingMap.seekg(i*sizeof(int), ios_base::beg);
		incommingMap.read((char*) &posIn, sizeof(posIn));
		if ((posIn == -1) && (posOut == -1)){
			int value = -1;
			outputMap.write((char*) &value, sizeof(value));
			continue;
		}
		int size = 0;
		vector<int> data;
		vector<int> dataIn;
		vector<int> dataOut;
		if (posIn != -1){
			incommingData.seekg(posIn, ios_base::beg);
			int sizeIn;
			incommingData.read((char*) &sizeIn, sizeof(sizeIn));
			size += sizeIn;
			for (int j=0; j < sizeIn; j++){
				int pId;
				int stmtGrSize;
				incommingData.read((char*) &pId, sizeof(pId));
				incommingData.read((char*) &stmtGrSize, sizeof(stmtGrSize));
				dataIn.push_back(-pId);
				dataIn.push_back(stmtGrSize);
				for (int k=0; k < stmtGrSize; k++){
					int id;
					incommingData.read((char*) &id, sizeof(id));
					dataIn.push_back(id);
				}
			}
		}
		if (posOut != -1){
			outgoingData.seekg(posOut, ios_base::beg);
			int sizeOut;
			outgoingData.read((char*) &sizeOut, sizeof(sizeOut));
			size += sizeOut;
			for (int j=0; j < sizeOut; j++){
				int pId;
				int stmtGrSize;
				outgoingData.read((char*) &pId, sizeof(pId));
				outgoingData.read((char*) &stmtGrSize, sizeof(stmtGrSize));
				dataOut.push_back(pId);
				dataOut.push_back(stmtGrSize);
				for (int k=0; k < stmtGrSize; k++){
					int id;
					outgoingData.read((char*) &id, sizeof(id));
					dataOut.push_back(id);
				}
			}
		}
		data.push_back(size);
		data.insert(data.end(), dataIn.begin(), dataIn.end());
		data.insert(data.end(), dataOut.begin(), dataOut.end());
		int pos = outputData.tellp();
		outputMap.write((char*) &pos, sizeof(pos));
		for (int j=0; j< data.size(); j++){
			outputData.write((char*) &data[j], sizeof(int));
		}
		if (i % 10000 == 0){
			cout << i << endl;
		}
	}
}

void Merge::close(){
	outgoingMap.close();
	outgoingData.close();

	incommingMap.close();
	incommingData.close();

	outputMap.close();
	outputData.close();
}

#endif /* MERGE_H_ */
