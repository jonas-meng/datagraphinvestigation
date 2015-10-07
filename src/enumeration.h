#include "../dependency/rapidjson/document.h"
#include "representation.h"

#include <set>
#include <vector>
#include <map>

class SimplfiedGraph {
	private:
		std::set<std::set<int> > edgeSet;
		size_t hashValue;
	public:
		SimplfiedGraph(std::set<std::set<int> > &pEdgeSet, size_t hashValue): edgeSet(pEdgeSet.begin(), pEdgeSet.end()), hashValue(hashValue) {} 

		size_t getHashValue() { return hashValue; }

		void display(int frequency) {
			std::cout << "Hash value: " << hashValue 
				<< " Frequency: " << frequency << std::endl;
			std::cout << "Example Graph:" << std::endl;
			printGraph();
		}

		void printGraph() {
			int cnt = 0;
			for (std::set<std::set<int> >::iterator it = edgeSet.begin(); it != edgeSet.end(); it++) {
				if (cnt > 4) {
					cnt = 0;
					std::cout << std::endl;
				}
				cnt++;
				std::cout << " (" << *(*it).begin() 
					<< ", " << *(*it).rbegin() << ") ";
			}
			std::cout << std::endl;
		}
};

class Graph {
	public:
		std::set<int> edgeSet;
		std::set<int> nodeSet;
		std::set<std::set<int> > realEdgeSet;
		SPGRepresentation *spgr;
		size_t hashValue;
	
		void addNewEdge(std::set<int> &edge, int edgeNO) {
			nodeSet.insert(*edge.begin());
			nodeSet.insert(*edge.rbegin());
			edgeSet.insert(edgeNO);
			realEdgeSet.insert(edge);
		}
	
		void removeEdge(std::set<int> &edge, int edgeNO) {
			nodeSet.erase(*edge.begin());
			nodeSet.erase(*edge.rbegin());
			edgeSet.erase(edgeNO);
			realEdgeSet.erase(edge);
		}

		void computation() {
			spgr = new SPGRepresentation(realEdgeSet);
			spgr->computation();
			spgr->printRepresentation();
			hashValue = spgr->hashValue();
			delete spgr;
		}
	
		void printGraph() {
			int cnt = 0;
			for (std::set<std::set<int> >::iterator it = realEdgeSet.begin(); it != realEdgeSet.end(); it++) {
				if (cnt > 4) {
					cnt = 0;
					std::cout << std::endl;
				}
				cnt++;
				std::cout << " (" << *(*it).begin() 
					<< ", " << *(*it).rbegin() << ") ";
			}
			std::cout << std::endl;
		}
};

class SPGEnumerator {
	private:
		std::map<int, std::set<int> > adjSet;
		std::set<int> edgeSet;
		std::map<int, std::set<int> > edgeNumbering;
		std::map<std::set<int>, int > e2n;
		std::set<std::set<int> > graphVisited;
		std::map<size_t, int> counter;
		std::vector<SimplfiedGraph> spgs;
		Graph g;

	public:
		SPGEnumerator(rapidjson::Document &d) {
			initialization(d);
		}

		void initialization(rapidjson::Document &d);
		void start();
		void enumeration();
		void constructAdjSet(std::map<int, std::set<int> >&);
		void displayFrequency();
		void counting();
};
