#include "enumeration.h"
#include "../dependency/rapidjson/document.h"
#include "representation.h"
#include "identification.h"

#include <set>
#include <iostream>
#include <string>

/*
#define DEBUG
*/

void SPGEnumerator::initialization(rapidjson::Document &d) {
	spgi.initialization(d);
	isSPG = spgi.identify();
	int a, b, length = d["bond"]["aid1"].Size();
	g.setCapacity(length);
	std::set<int> edge;
	for (int i = 0; i < length; i++) {

		// obtain two end points of one edge
		a = d["bond"]["aid1"][i].GetInt();
		b = d["bond"]["aid2"][i].GetInt();

		// insert into adjacent set of each other
		adjSet[a].insert(b);
		adjSet[b].insert(a);


		// initialize label set of edge
		edge.clear();
		edge.insert(a);
		edge.insert(b);
		edgeSet.insert(i);
		n2e[i] = edge;
		e2n[edge] = i;
	}
}

void SPGEnumerator::start() {
	std::map<int, std::set<int> > curAdjSet;
	for (std::set<int>::iterator it = edgeSet.begin(); it != edgeSet.end(); it++) {
		g.addNewEdge(n2e[*it], *it);
		graphVisited.insert(g.hashEdgeSet());
#ifdef DEBUG
		std::cout << "SUB-GRAPH:"<< std::endl;
		g.printGraph();
#endif
		counting();
		enumeration();
		g.removeEdge(n2e[*it], *it);
	}
}

void SPGEnumerator::enumeration() {
	std::map<int, std::set<int> > curAdjSet;
	constructAdjSet(curAdjSet);
	std::set<int> edge;
	int a, b;

	for (std::map<int, std::set<int> >::iterator it = curAdjSet.begin(); it != curAdjSet.end(); it++) {
		a = it->first;
		for (std::set<int>::iterator sit = (it->second).begin();
				sit != (it->second).end(); sit++) {
			b = *sit;
			edge.clear();
			edge.insert(a);
			edge.insert(b);

			g.addNewEdge(edge, e2n[edge]);
			if (graphVisited.find(g.hashEdgeSet()) == graphVisited.end()) {

				graphVisited.insert(g.hashEdgeSet());
#ifdef DEBUG
				std::cout << "SUB-GRAPH:"<< std::endl;
				g.printGraph();
#endif
				enumeration();
				if (!isSPG) {
					spgi.initialization(g.realEdgeSet);
					if (spgi.identify()) {
						counting();
						enumeration();
					}
				} else {
					counting();
					enumeration();
				}
			}
			g.removeEdge(edge, e2n[edge]);
		}
	}

}

void SPGEnumerator::constructAdjSet(std::map<int, std::set<int> > &curAdjSet) {
	int a,b;
	for (std::set<int>::iterator it = g.nodeSet.begin();
			it != g.nodeSet.end(); it++) {
		curAdjSet[*it] = adjSet[*it];
	}
	for (std::set<std::set<int> >::iterator it = g.realEdgeSet.begin();
			it != g.realEdgeSet.end(); it++) {
		a = *(*it).begin();
		b = *(*it).rbegin();
		curAdjSet[a].erase(b);
		curAdjSet[b].erase(a);
	}
}


void SPGEnumerator::displayFrequency() {
}

void SPGEnumerator::counting() {
	g.computation();
	int size = g.sizeOfGraph();
	if (GSPGSizeCounter.find(size) == GSPGSizeCounter.end()) {
		GSPGSizeCounter[size] = 0;
	}
	if (counter.find(g.hashValue) == counter.end()) {
		counter[g.hashValue] = 0;
	}
	counter[g.hashValue] = counter[g.hashValue]+1;
	GSPGSizeCounter[size] = GSPGSizeCounter[size] + 1;
}

void testCase() {
	std::string json[] = {
		"{\"bond\": {"
		"\"aid2\":[1,2,1],"
		"\"aid1\":[2,3,3] "
		"}}", "{\"bond\": {"
		"\"aid2\":[1,2,3,3],"
		"\"aid1\":[2,3,1,4] "
		"}}", "{\"bond\": {"
		"\"aid2\":[1,1,1],"
		"\"aid1\":[2,3,4] "
		"}}", "{\"bond\": {"
		"\"aid2\":[1,2,2,4,4],"
		"\"aid1\":[2,3,4,5,6] "
		"}}", "{\"bond\": {"
		"\"aid2\":[1],"
		"\"aid1\":[2] "
		"}}", "{\"bond\": {"
		"\"aid2\":[1,2,2,3,4],"
		"\"aid1\":[2,3,4,4,5] "
		"}}","{\"bond\": {"
		"\"aid2\": [3, 8, 4, 5, 6, 9, 7, 8, 8, 10, 11, 12, 13, 14, 15], "
		"\"aid1\": [1, 2, 3, 3, 4, 4, 5, 6, 7, 5, 6, 7, 9, 9, 9]"
		"}}", "{\"bond\": {\"aid2\": [3, 4, 8, 7, 9, 20, 10, 18, 20, 19, 17, 15, 16, 15, 11, 12, 13, 19, 13, 16, 14, 14, 18, 21, 22, 23, 24, 25, 26, 27, 28], \"aid1\": [1, 1, 2, 2, 3, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 8, 9, 10, 10, 11, 11, 12, 17, 1, 2, 3, 8, 9, 12, 13, 14]}}"};
	/*
	for (int i = 0; i < 6; i++) {
		std::cout<<"-------"<<std::endl;
		*/
	rapidjson::Document d;
	d.Parse(json[7].c_str());
	SPGEnumerator spge(d);
	spge.start();
	spge.displayFrequency();
	/*
	}
	*/
}

/*
int main(int argc, char **argv) {
	testCase();
	return 0;
}
*/
