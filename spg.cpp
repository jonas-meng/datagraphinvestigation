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

using namespace std;
using namespace rapidjson;

typedef set <int> edgeset;
typedef map <set <int>, set < set <int> > > termspgset;

//#define SPGDETECTIONDEBUG
//#define PROGRESSCHECKING 

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
				set< set <int> > *oldspg = &oldset[key];
				key.erase(p[i]);
				key.insert(p[(i+1)%2]);
				vector<int> tmp;
				// enumerating all possible combination of SPGs from new and old SPG sets
				for (set< set <int> >::iterator oldit = oldspg->begin();
						oldit != oldspg->end(); oldit++) {
					for (set< set <int> >::iterator newit = (it->second).begin();
							newit != (it->second).end(); newit++) {
						set_intersection(newit->begin(), newit->end(),
								oldit->begin(), oldit->end(), tmp.begin());
						// if two SPGs contain no same arcs, then perform serial combination
						if (tmp.size() == 0) {
							set_union(newit->begin(), newit->end(),
									oldit->begin(), oldit->end(), tmp.begin());
							intermidateset[key].insert(set<int>(tmp.begin(), tmp.end()));		
							terminals[*itr].insert(p[(i+1)%2]);
							terminals[p[(i+1)%2]].insert(*itr);
						}
						tmp.clear();
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
		set< set <int> >& newspg = it->second;
		vector<int> tmp;
		// try to combine all SPGs with same terminals
		for (set< set<int> >::iterator oldit = oldset[key].begin();
				oldit != oldset[key].end(); oldit++) {
			for (set< set <int> >::iterator newit = newspg.begin();
					newit != newspg.end(); newit++) {
				set_intersection(newit->begin(), newit->end(),
						oldit->begin(), oldit->end(), tmp.begin());
				// if two SPGs contain no same arcs, then perform serial combination
				if (tmp.size() == 0) {
					// because size of result of set union definitely larger than
					// size of result of set intersection, which leads to overwrite of last result
					// therefore set clear operation is ignored
					set_union(newit->begin(), newit->end(),
							oldit->begin(), oldit->end(), tmp.begin());
					intermidateset[key].insert(set<int>(tmp.begin(), tmp.end()));		
				}
				tmp.clear();
			}
		}	
	}
}

bool iteration(termspgset &oldset, termspgset &newset,
		termspgset &intermidateset, map <int, set <int> > &terminals) {
	//serialcomb(oldset, newset, intermidateset, terminals);
	parallelcomb(oldset, newset, intermidateset);	
	// merge old and new set
	for (termspgset::iterator newit = newset.begin();
			newit != newset.end(); ++newit) {
		// set difference makes sure that all SPGs in vector are not included in old set
		vector< set<int> > tmp; 
		const set<int> &key = newit->first;
		// following iterator providing const values which are read-only
		// therefore no way to compare and copy
		// possible solution is to write a three set operation manually
		set_difference(newset[key].begin(), 
				newset[key].end(),
				oldset[key].begin(),
				oldset[key].end(),
				tmp.begin());
		oldset[key].insert(tmp.begin(), tmp.end());
		tmp.clear();
	}
	// copy intermidate set to new set
	newset = intermidateset;
	// clear intermidate set for next iteration
	intermidateset.clear();
	
	// if new set is non-empty, then return true to continue iteration
	// otherwise return false to stop
	return (newset.size() > 0);
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
	termspgset oldset, newset, intermidateset;
	int sz = document["bond"]["aid1"].Size(), a, b;
	for (int idx = 0; idx < sz; idx++) {
		a = document["bond"]["aid1"][idx].GetInt();
		b = document["bond"]["aid2"][idx].GetInt();
		terminals[a].insert(b);
		terminals[b].insert(a);
		edgelabel[idx].insert(a);
		edgelabel[idx].insert(b);
		set <int> tmp;
		tmp.insert(idx);
		oldset[edgelabel[idx]].insert(tmp);
		newset[edgelabel[idx]].insert(tmp);
	}
	
	while (iteration(oldset, newset, intermidateset, terminals));
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
		/*
		if (cnt % 2000000 == 0) {			
			fprintf(stderr, "\r%.2f", (cnt*100.0/2000000));
		}
		*/
#ifdef PROGRESSCHECKING
		printf("%d | %s\n", cnt, mol);
#endif
		// get single molecule in memory
		document.Parse(mol);
		if(spgdetection(document)) {
			cntspg++;
#ifdef PROGRESSCHECKING
			printf("%d is SPG\n", cnt);
#endif
		} else {
			enumeratingspg(document);
#ifdef PROGRESSCHECKING
			printf("%d is not SPG\n", cnt);
#endif
		}
	}
	printf("total number = %d, number of spg = %d, percentage = %.2f\n", cnt, cntspg, cntspg * 100.0 / cnt);

	//#pragma omp parallel for
	
	//fclose(fp);
	
	return 0;
}
