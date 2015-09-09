#include "dependency/rapidjson/filereadstream.h"
#include "dependency/rapidjson/document.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;
using namespace rapidjson;

typedef set <int> edgeset;
typedef vector < set <int> > graph;
typedef map <set <int>, set < vector< set<int> > > > termspgset;

#define EDGE 1
#define NODE 0

//#define SPGDETECTIONDEBUG
#define PROGRESSCHECKING 
//#define MAXIMALSPG

void printgraph(const graph & g) {
	cerr << "edge: ";
	for (set<int>::iterator it = g[EDGE].begin(); it != g[EDGE].end(); it++)
		cerr << *it << " ";
	cerr << " node: ";
	for (set<int>::iterator it = g[NODE].begin(); it != g[NODE].end(); it++)
		cerr << *it << " ";
	cerr << endl;
}

bool spgdetection(Document &document) {
	/*
	 * 1. build a map <key, set> where key is index of vertex and set is adjacent vertices
	 * 2. set up 2-queue and set of all vertices
	 * 3. proceed with elements in 2-queue untile the queue becomes empty
	 * 4. based on szie of vertex set, whether a graph is SPG can be determined
	 * */
	set <int> element;
	map < int, set<int> > dict;
	int sz = document["bond"]["aid1"].Size();
	int a, b;	
	for (int i = 0; i < sz; i++) {
		a = document["bond"]["aid1"][i].GetInt();
		b = document["bond"]["aid2"][i].GetInt();
		dict[a].insert(b);
		dict[b].insert(a);
		element.insert(a);
		element.insert(b);
	}

	queue <int> scqueue; // elements are indices of serial intersection
	for (map<int, set<int> >::iterator itr = dict.begin();
			itr != dict.end(); ++itr) {
		if (itr->second.size() == 2) {
			scqueue.push(itr->first);
			element.erase(itr->first);
		}
	}

	int idx;
	int endPoint[2];
	while (scqueue.size() > 0) {
		idx = scqueue.front();
		scqueue.pop();
		if (dict[idx].size() == 2) {	
			// endPoint[0] and endPoint[1] are two endpoints
			int i = 0;
			for (set<int>::iterator sitr = dict[idx].begin(); 
					sitr != dict[idx].end(); ++sitr) {
				endPoint[i] = *sitr;
				dict[*sitr].erase(idx);
				i++;
			}

			// insert each other into corresponding adjacent sets
			dict[endPoint[0]].insert(endPoint[1]);
			dict[endPoint[1]].insert(endPoint[0]);
			// if any vertex only has two neighbors, then put it into SC-queue
			// Such element should not be in element set, otherwise might cause inifite loop
			if (dict[endPoint[0]].size() == 2 && 
					element.find(endPoint[0]) != element.end()) {
				scqueue.push(endPoint[0]);
				element.erase(endPoint[0]);
			}
			if (dict[endPoint[1]].size() == 2 && 
					element.find(endPoint[1]) != element.end()) {
				scqueue.push(endPoint[1]);
				element.erase(endPoint[1]);
			}
#ifdef SPGDETECTIONDEBUG
			cout << "remove " << idx << ": (" << endPoint[0] << ", " << endPoint[1] << ")" << endl;
#endif
		} else {
			/*
			 * example
			 * three endpoints connecting with each other
			 * after one endpoint being removed, other two endpoints should be added back to element set
			 * otherwise there is no two final endpoints
			 * */
			element.insert(idx);
#ifdef SPGDETECTIONDEBUG
			cout << "insert " << idx << endl;
#endif
		}
#ifdef SPGDETECTIONDEBUG
		cout << "element set : {";
		for (set<int>::iterator sitr = element.begin(); sitr != element.end(); ++sitr) {
			cout << *sitr << ", ";
		}
		cout << "}" << endl;
#endif
	}

	return (element.size() == 2); // SPG should be reduced to one single arc

}

void serialcomb(termspgset &oldset, termspgset &newset,
		termspgset &intermidateset, map <int, set <int> > &terminals) {
	int p[2]; // two terminals
	for (termspgset::iterator it = newset.begin(); it != newset.end(); it++) {
		// obtain two terminals
		p[0] = *((it->first).begin());	
		p[1] = *((it->first).rbegin());	
		for (int i = 0; i < 2; i ++) {
			// check whether there are other nodes connecting to one of those terminals
			if (terminals.find(p[i]) == terminals.end())
				continue;
			
			for (set<int>::iterator itr = terminals[p[i]].begin();
					itr != terminals[p[i]].end(); itr++) {
				if (*itr == p[(i+1)%2]) 
					continue;
				set<int> key;
				key.insert(p[i]);
				key.insert(*itr);
				set<graph > *oldspg = &oldset[key];
				key.erase(p[i]);
				key.insert(p[(i+1)%2]);
				graph tmp;
				vector<int> stmp;
				// enumerating all possible combination of SPGs from new and old SPG sets
				for (set<graph>::iterator oldit = oldspg->begin();
						oldit != oldspg->end(); oldit++) {
					for (set<graph>::iterator newit = (it->second).begin();
							newit != (it->second).end(); newit++) {
						// std::back_inserter is necessary because no memory is allocated to
						// vector tmp right now
						stmp.clear();
						set_intersection((*newit)[NODE].begin(), (*newit)[NODE].end(),
								(*oldit)[NODE].begin(), (*oldit)[NODE].end(),
								back_inserter(stmp));
						// if two SPGs contain no same arcs, then perform serial combination
						if (stmp.size() <= 1) { // serial combination should only has one single shared terminal
							stmp.clear();
							tmp.clear();
							set_union((*newit)[NODE].begin(), (*newit)[NODE].end(),
									(*oldit)[NODE].begin(), (*oldit)[NODE].end(),
									back_inserter(stmp));
							tmp.push_back(set<int>(stmp.begin(), stmp.end())); // push node set into graph representation
							stmp.clear();
							set_union((*newit)[EDGE].begin(), (*newit)[EDGE].end(),
									(*oldit)[EDGE].begin(), (*oldit)[EDGE].end(),
									back_inserter(stmp));
							tmp.push_back(set<int>(stmp.begin(), stmp.end())); // push edge set into graph representation
#ifdef MAXIMALSPG
							cerr << "-------------serial----------------" << endl;
							printgraph(*newit);
							printgraph(*oldit);
							cerr << "->" << endl;
							printgraph(tmp);
#endif
							if (oldset[key].find(tmp) == oldset[key].end() &&
									newset[key].find(tmp) == newset[key].end()) {
								intermidateset[key].insert(tmp);	
								terminals[p[i]].insert(p[(i+1)%2]);
								terminals[p[(i+1)%2]].insert(p[i]);
							}
						}
					}
				}
			}		

		}
	}
}

void parallelcomb(termspgset &oldset, termspgset &newset,
	   	termspgset &intermidateset) {
	for (termspgset::iterator it = newset.begin(); 
			it != newset.end(); it++) {
		set<int> key = it->first;
		if (oldset.find(key) == oldset.end())
			continue;
		set<graph>& newspg = it->second;
		graph tmp;
		vector<int> stmp;
		// try to combine all SPGs with same terminals
		for (set<graph>::iterator oldit = oldset[key].begin();
				oldit != oldset[key].end(); oldit++) {
			for (set<graph>::iterator newit = newspg.begin();
					newit != newspg.end(); newit++) {
				stmp.clear();
				set_intersection((*newit)[NODE].begin(), (*newit)[NODE].end(),
						(*oldit)[NODE].begin(), (*oldit)[NODE].end(), 
						back_inserter(stmp));
				// if two SPGs contain no same arcs, then perform serial combination
				if (stmp.size() <= 2) { // for parallel combination, there are at most 2
					stmp.clear();
					tmp.clear();
					set_union((*newit)[NODE].begin(), (*newit)[NODE].end(),
							(*oldit)[NODE].begin(), (*oldit)[NODE].end(),
							back_inserter(stmp));
					tmp.push_back(set<int>(stmp.begin(), stmp.end())); // push node set into graph represetnation
					stmp.clear();
					// because size of result of set union definitely larger than
					// size of result of set intersection, which leads to overwrite of last result
					// therefore set clear operation is ignored
					// c++ union operation only add same element once
					set_union((*newit)[EDGE].begin(), (*newit)[EDGE].end(),
							(*oldit)[EDGE].begin(), (*oldit)[EDGE].end(),
							back_inserter(stmp));
					tmp.push_back(set<int>(stmp.begin(), stmp.end())); // push edge set into graph represetnation
#ifdef MAXIMALSPG
					cerr << "--------------parallel--------------------" << endl;
					printgraph(*newit);
					printgraph(*oldit);
					cerr << "->" << endl;
					printgraph(tmp);
#endif
					if (oldset[key].find(tmp) == oldset[key].end() &&
							newset[key].find(tmp) == newset[key].end())
						intermidateset[key].insert(tmp);
				}
			}
		}	
	}
}

bool iteration(termspgset &oldset, termspgset &newset,
		termspgset &intermidateset, map <int, set <int> > &terminals) {
	serialcomb(oldset, newset, intermidateset, terminals);
	parallelcomb(oldset, newset, intermidateset);	
	// merge old and new set
	for (termspgset::iterator newit = newset.begin();
			newit != newset.end(); ++newit) {
		// set difference makes sure that all SPGs in vector are not included in old set
		vector<graph> tmp; 
		const set<int> &key = newit->first;
		// following iterator providing const values which are read-only
		// therefore no way to compare and copy
		// possible solution is to write a three set operation manually
		tmp.clear();
		set_difference(newset[key].begin(), newset[key].end(),
				oldset[key].begin(), oldset[key].end(),
				back_inserter(tmp));
		oldset[key].insert(tmp.begin(), tmp.end());
	}
	bool isLoop = false;
	if (intermidateset.size() > 0) {
		// copy intermidate set to new set
		newset = intermidateset;
		// clear intermidate set for next iteration
		intermidateset.clear();
		isLoop = true;
	}
	
	// if new set is non-empty, then return true to continue iteration
	// otherwise return false to stop
	return isLoop;
}

void enumeratingspg(Document &document) {
	/*
	 * 1. label edge - a map of edge-terminal
	 * 2. construct three terminal-spg mapping for sequential steps
	 * 3. serial and parallel combination
	 * 4. iteration procedure
	 * */
	map <int, set <int> > edgelabel;
	map <int, set <int> > terminals;
	set <int> edges;
	termspgset oldset, newset, intermidateset;
	int sz = document["bond"]["aid1"].Size(), a, b;
	for (int idx = 0; idx < sz; idx++) {
		edges.insert(idx);
		a = document["bond"]["aid1"][idx].GetInt();
		b = document["bond"]["aid2"][idx].GetInt();
		terminals[a].insert(b);
		terminals[b].insert(a);
		edgelabel[idx].insert(a);
		edgelabel[idx].insert(b);
		// create a spg with both edge and node set
		set <int> edge;
		edge.insert(idx);
		set <int> node;
		node.insert(a);
		node.insert(b);
		vector <set <int> > tmp;
		tmp.push_back(node);
		tmp.push_back(edge);
		// insert spg into old and new set
		oldset[edgelabel[idx]].insert(tmp);
		newset[edgelabel[idx]].insert(tmp);
	}
	
	while (iteration(oldset, newset, intermidateset, terminals))
		;

	// identify maximal sub-SPG and corresponding violator set
	int sz_max = 0;
	set<graph>::iterator maxitr;
	for (termspgset::iterator it = oldset.begin();
			it != oldset.end(); it++) {
		for (set<graph>::iterator ssit = (it->second).begin();
				ssit != (it->second).end(); ++ssit) {
			if ((*ssit)[EDGE].size() > sz_max) {
				sz_max = (*ssit)[EDGE].size();
				maxitr = ssit;
			}
		}
	}
	
	vector<int> violator;
	set<int>::iterator it = (*maxitr)[EDGE].begin();
	cout << "," << sz_max <<",\"" << *it;
	for (it++; it != (*maxitr)[EDGE].end(); it++)
		cout << "," << *it;
	int sz_vio = document["bond"]["aid1"].Size() - sz_max;
	set_difference(edges.begin(), edges.end(),
			(*maxitr)[EDGE].begin(), (*maxitr)[EDGE].end(),
			back_inserter(violator));
	if (violator.size() > 0) {
		vector<int>::iterator vioit = violator.begin();
		cout << "\"," << sz_vio << ",\"" << *vioit;
		for (vioit++; vioit != violator.end(); vioit++)
			cout << "," << *vioit;
		cout << "\"" << endl;
	} else {
		cout << "\",0,\"\"" << endl;
	}
}

int main(int argc, char **argv) {
	/*
	 * by reading JSON object line by line and parse it, problem solved
	 * */
	ifstream inputfile(argv[1]);
	int size = 65536;
	char mol[65536];
	Document document;
	int cnt = 0, cntspg = 0;
	while (inputfile.getline(mol, size)) {
		cnt++;
		if (cnt % 20000 == 0) {			
			fprintf(stderr, "\r%.2f", (cnt*100.0/2000000));
		}
#ifdef PROGRESSCHECKING
		fprintf(stderr, "%d | %s\n", cnt, mol);
#endif
		// get single molecule in memory
		document.Parse(mol);
		cout << "id,sz_max,maximal,sz_vio,violator" << endl;
		cout << document["id"].GetString();
		// omit graphs whose size is less than 0 or larger than 60
		if(document["bond"]["aid1"].Size() <= 0 ||
				document["bond"]["aid1"].Size() > 60) {
			cout << ",0,\"\",0,\"\"" << endl;
			continue;
		}
		if(spgdetection(document)) {
			cntspg++;
#ifdef PROGRESSCHECKING
			fprintf(stderr, "%d is SPG\n", cnt);
#endif
			cout << "," << document["bond"]["aid1"].Size() << ",\"0";
			for (int i = 1; i < document["bond"]["aid1"].Size(); i++)
				cout << "," << i;
			cout << "\",0,\"\"" << endl;
		} else {
#ifdef PROGRESSCHECKING
			fprintf(stderr, "%d is not SPG\n", cnt);
#endif
			enumeratingspg(document);
		}
	}
	// print information regarding percentage of SPG in database
	//printf("total number = %d, number of spg = %d, percentage = %.2f\n", cnt, cntspg, cntspg * 100.0 / cnt);

	//#pragma omp parallel for
	
	//fclose(fp);
	
	return 0;
}
