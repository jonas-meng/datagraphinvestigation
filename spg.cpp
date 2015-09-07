#include "dependency/rapidjson/filereadstream.h"
#include "dependency/rapidjson/document.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <queue>
#include <cstdio>

using namespace std;
using namespace rapidjson;

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
			for (set<int>::iterator sitr = dict[idx].begin(); sitr != dict[idx].end(); ++sitr) {
				endPoint[i] = *sitr;
				dict[*sitr].erase(idx);
				i++;
			}

			// insert each other into corresponding adjacent sets
			dict[endPoint[0]].insert(endPoint[1]);
			dict[endPoint[1]].insert(endPoint[0]);
			// if any vertex only has two neighbors, then put it into SC-queue
			// Such element should not be in element set, otherwise might cause inifite loop
			if (dict[endPoint[0]].size() == 2 && element.find(endPoint[0]) != element.end()) {
				scqueue.push(endPoint[0]);
				element.erase(endPoint[0]);
			}
			if (dict[endPoint[1]].size() == 2 && element.find(endPoint[1]) != element.end()) {
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
		if (cnt % 10000 == 0) {			
			printf("\r%.2f", (cnt*100.0/2000000));
		}
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
