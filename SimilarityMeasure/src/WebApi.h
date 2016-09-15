/*
 * WebApi.h
 *
 *  Created on: 20.05.2016
 *      Author: michael
 */

#ifndef WEBAPI_H_
#define WEBAPI_H_

#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <curl/curl.h>

using namespace std;

class WebApi {
public:

	string getLabelById(int id);
	int getIdByLabel(string label);

protected:

	static const string baseUrl;

	string sendRequest(string url);
	static size_t write_data(char *ptr, size_t size, size_t nmemb,
			string *userdata);
	string parseLabel(string* json);
	int parseId(string* json);
};

const string WebApi::baseUrl = "https://www.wikidata.org/w/api.php";

string WebApi::getLabelById(int id) {
	stringstream url;
	url << baseUrl << "?action=wbgetentities&format=json&props=labels&ids=Q"
			<< id << "&languages=en";
	string result = sendRequest(url.str());

	string label = parseLabel(&result);

	if (label == "") {
		cerr << "no label available for Q" << id << endl;
	}
	return label;
}

int WebApi::getIdByLabel(string label) {
	stringstream url;
	url << baseUrl << "?action=wbgetentities&format=json&props=&titles="
			<< label << "&sites=enwiki&languages=en";
	string result = sendRequest(url.str());
	return parseId(&result);
}

size_t WebApi::write_data(char *ptr, size_t size, size_t nmemb,
		string *userdata) {
//	cout << baseUrl;
	(*userdata) += ptr;
	return size * nmemb;
}

string WebApi::parseLabel(string* json) {
	if (json->find("\"labels\":{}}}") != -1) {
		return "";
	}
	size_t posStart = json->find("value");
	size_t posEnd = json->find("}}}}");
	return json->substr(posStart + 8, posEnd - posStart - 9);
}

int WebApi::parseId(string* json) {
	size_t posStart = json->find("id\":");
	size_t posEnd = json->find(",\"type");
	return atoi(json->substr(posStart + 6, posEnd - posStart - 7).c_str());
}

string WebApi::sendRequest(string url) {
	CURL *curl;
	CURLcode res;
	string result;
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res)
					<< endl;
			return "";
		}
		curl_easy_cleanup(curl);
		return result;
	}
	return "";
}

#endif /* WEBAPI_H_ */
