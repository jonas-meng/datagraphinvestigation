#include "identification.h"
#include "../dependency/rapidjson/document.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <queue>

#define DEBUG

void SPGIdentifier::initialization(const char *smol) {
	document.Parse(smol);
	int length = document["bond"]["aid1"].Size();
	int a, b;
	for (int i = 0; i < length; i++) {
		a = document["bond"]["aid1"][i].GetInt();
		b = document["bond"]["aid2"][i].GetInt();
		nodeSet.insert(a);
		nodeSet.insert(b);
		adjSet[a].insert(b);
		adjSet[b].insert(a);
	}	
}

bool SPGIdentifier::identify(const char *smol) {
	initialization(smol);
	searchv2();	
	int mid;
	bool isEnd = false;
	while (!isEnd) {
		// apply serial & parallel reduction till not applicable
		while (!queueofv2.empty()) {
			mid = queueofv2.front();
			queueofv2.pop();
			if (adjSet[mid].size() == 2) {
				serialReduction(mid);
			} else {
				nodeSet.insert(mid);
#ifdef DEBUG
				std::cout << "put node " << mid << " back" << std::endl;
#endif
			}
		}
		if (nodeSet.size() > 2) {
			removalOfPendantVertex();
		}
		isEnd = queueofv2.empty();
	}
	return (nodeSet.size() <= 2);	
}

void SPGIdentifier::searchv2() {
	for (std::map<int, std::set<int> >::iterator
			it = adjSet.begin();
			it != adjSet.end(); ++it) {
		if (it->second.size() == 2) {
			queueofv2.push(it->first);
			nodeSet.erase(it->first);
		}
	}
}

void SPGIdentifier::serialReduction(int mid) {
	int a = *adjSet[mid].begin();
	int b = *adjSet[mid].rbegin();

#ifdef DEBUG
	std::cout << "remove node " << mid 
		<< ", create new edge (" << a << ", " << b << ")" << std::endl;
#endif

	adjSet[a].erase(mid);
	adjSet[b].erase(mid);

	adjSet[a].insert(b);
	adjSet[b].insert(a);

	if (adjSet[a].size() == 2 &&
			nodeSet.find(a) != nodeSet.end()) {
		queueofv2.push(a);
		nodeSet.erase(a);
	}
	if (adjSet[b].size() == 2 &&
			nodeSet.find(b) != nodeSet.end()) {
		queueofv2.push(b);
		nodeSet.erase(b);
	}
}

void SPGIdentifier::removalOfPendantVertex() {
	pendentVertex.clear();
	searchv1();
	for (std::set<int>::iterator it = pendentVertex.begin();
			it != pendentVertex.end(); it++) {
#ifdef DEBUG
		std::cout << "remove pendant vertex " << *it << std::endl;
#endif
		removalv1(*it);
	}
}

void SPGIdentifier::searchv1() {
	for (std::set<int>::iterator it = nodeSet.begin();
			it != nodeSet.end(); it++) {
		if (adjSet[*it].size() == 1) {
			pendentVertex.insert(*it);
		}
	}
}

void SPGIdentifier::removalv1(int a) {
	nodeSet.erase(a);
	int b = *adjSet[a].begin();
	adjSet[b].erase(a);
	if (adjSet[b].size() == 2) {
		queueofv2.push(b);
		nodeSet.erase(b);
	}
}

void testCase() {
	std::ifstream inputfile(argv[1]);
	std::string smol;

	while (getline(inputfile, smol)) {
		SPGIdentifier spgid;	
		std::cout << smol << std::endl;
		std::cout << spgid.identify(smol.c_str()) << std::endl;
	}
}

int main(int argc, char** argv) {
	testCase();
}
