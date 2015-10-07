#include "enumeration.h"
#include "../dependency/rapidjson/document.h"
#include "representation.h"

#include <set>
#include <iostream>
#include <string>

#define DEBUG

void SPGEnumerator::initialization(rapidjson::Document &d) {
	int a, b, length = d["bond"]["aid1"].Size();
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
		edgeNumbering[i] = edge;
		e2n[edge] = i;
	}
}

void SPGEnumerator::start() {
	std::map<int, std::set<int> > curAdjSet;
	for (std::set<int>::iterator it = edgeSet.begin(); it != edgeSet.end(); it++) {
		g.addNewEdge(edgeNumbering[*it], *it);
		graphVisited.insert(g.edgeSet);
#ifdef DEBUG
		std::cout << "SUB-GRAPH:"<< std::endl;
		g.printGraph();
#endif
		counting();
		enumeration();
		g.removeEdge(edgeNumbering[*it], *it);
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
			if (graphVisited.find(g.edgeSet) == graphVisited.end()) {
				graphVisited.insert(g.edgeSet);
#ifdef DEBUG
				std::cout << "SUB-GRAPH:"<< std::endl;
				g.printGraph();
#endif
				counting();
				enumeration();
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
	for (std::set<int>::iterator it = g.edgeSet.begin();
			it != g.edgeSet.end(); it++) {
		a = *edgeNumbering[*it].begin();
		b = *edgeNumbering[*it].rbegin();
		curAdjSet[a].erase(b);
		curAdjSet[b].erase(a);
	}
}


void SPGEnumerator::displayFrequency() {
	for (std::vector<SimplfiedGraph>::iterator it = spgs.begin(); it != spgs.end(); it++) {
		it->display(counter[it->getHashValue()]);
	}
}

void SPGEnumerator::counting() {
	g.computation();
	if (counter.find(g.hashValue) == counter.end()) {
		counter[g.hashValue] = 0;
		spgs.push_back(SimplfiedGraph(g.realEdgeSet, g.hashValue));
	}
	counter[g.hashValue] = counter[g.hashValue]+1;
}

void testCase() {
	std::string json[] = {"{\"bond\": {"
		"\"aid2\":[1,1,2,3],"
		"\"aid1\":[2,3,3,4] "
		"}}"};
	rapidjson::Document d;
	d.Parse(json[0].c_str());
	SPGEnumerator spge(d);
	spge.start();
	spge.displayFrequency();
}

int main(int argc, char **argv) {
	testCase();
	return 0;
}
