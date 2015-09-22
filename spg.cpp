#include "dependency/rapidjson/filereadstream.h"
#include "dependency/rapidjson/document.h"
#include "dependency/threadpool.h"

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
#include <sstream>
#include <unistd.h>

using namespace std;
using namespace rapidjson;

typedef set <int> edgeset;
typedef vector < set <int> > graph;
typedef map <set <int>, set < vector< set<int> > > > termspgset;

Mutex output;
int spgcnt = 0;

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

void spgdetection(void *d) {
	/*
	 * 1. build a map <key, set> where key is index of vertex and set is adjacent vertices
	 * 2. set up 2-queue whoes elements are all vertices with degree two
	 * and set of all vertices
	 * 3. process elements in 2-queue untile the queue becomes empty
	 * 4. remove all pendant vertices and insert elements with degree 2 into 2-queue if available,
	 * if 2-queue is not empty, then go to step 3; otherwise, proceed to step 5
	 * 5. based on szie of vertex set, whether a graph is SPG can be determined
	 * */
	Document &document = *((Document*)d);
	set <int> element;
	set <int> pendantvetices;
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

	queue <int> scqueue; // gather all vertices with degree 2
	for (map<int, set<int> >::iterator itr = dict.begin();
			itr != dict.end(); ++itr) {
		if (itr->second.size() == 2) {
			scqueue.push(itr->first);
			element.erase(itr->first);
		}
	}

	int idx;
	int endPoint[2];
	bool is2q = true;
	while (is2q) {
		// serial & parallel reduction
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
			cout << "current element set : {";
			for (set<int>::iterator sitr = element.begin(); sitr != element.end(); ++sitr) {
				cout << *sitr << ", ";
			}
			cout << "}" << endl;
	#endif
		}
		if (element.size() > 2) {
			// for GSPG, removal of pendant vertex is performed
			pendantvetices.clear();
			// identify current vertices with degree one
			for (set<int>::iterator sitr = element.begin(); sitr != element.end(); ++sitr) {
				if (dict[*sitr].size() == 1) {
					pendantvetices.insert(*sitr);
				}
			}
			// remove those vertices from node set
			for (set<int>::iterator sitr = pendantvetices.begin(); sitr != pendantvetices.end(); ++sitr) {
				element.erase(*sitr);
	#ifdef SPGDETECTIONDEBUG
				cout << "remove pendant vertex " << idx << endl;
	#endif
				a = *(dict[*sitr].begin());
				dict[a].erase(*sitr);
				if (dict[a].size() == 2) {
					scqueue.push(a);
					element.erase(a);
				}
			}
		}
		is2q = scqueue.size() > 0;
	}
	
	if (element.size() <= 2) {
		output.lock();
		cout << document["id"].GetString() << " is GSPG" << endl;
		spgcnt++;
		output.unlock();
	}
	delete (Document*)d;
}

/*
void aprxmaxspg(Docment &document, stringstream &res) {
	map <int, set<int> > terminals;
	int sz = document["bond"]["aid1"].Size(), a, b;
	for (int idx = 0; idx < sz; idx++) {
		a = document["bond"]["aid1"][idx].GetInt();
		b = document["bond"]["aid2"][idx].GetInt();
		terminals[a].insert(b);
		terminals[b].insert(a);
	}
	/ *
	 * way to identify spruce -
	 * intersection of sets of adjacent node of vertices  a and b
	 * if there is intersection and two nodes are adjacent to each other,
	 * then it is a complete spruce (notice that the identified are the maxiaml spruce of those base vertices).
	 * otherwise it is an incomplete spruce.
	 * * /
}
*/

int main(int argc, char **argv) {
	/*
	 * by reading JSON object line by line and parse it, problem solved
	 * */
	ifstream inputfile(argv[1]);
	int nofthreads = 3;
	string smol;
	Document *document;
	ThreadPool tp(nofthreads);
	int ret = tp.initialize_threadpool();
	//cout << "id,sz_max,maximal,sz_vio,violator" << endl;
	int cnt = 0, slptime = 10, pedding = 64;
	while (getline(inputfile, smol)) {
		cnt ++;
		if (cnt % 100000 == 0) {
			cerr << "counting: " << cnt << endl;
			cerr << "sleep time: " << slptime << endl;
			sleep(slptime);
			cerr << "task number: " << tp.task_number() << endl;
		}
		// get single molecule in memory
		document = new Document();
		document->Parse(smol.c_str());
		//Task *t = new Task(&maxspgidentification, (void*)document);
		Task *t = new Task(&spgdetection, (void*)document);
		tp.add_task(t);
	}
	// print information regarding percentage of SPG in database
	//printf("total number = %d, number of spg = %d, percentage = %.2f\n", cnt, cntspg, cntspg * 100.0 / cnt);

	//#pragma omp parallel for
	
	//fclose(fp);
	int tn = tp.task_number();
	while (tn > 0) {
		cerr << "task number: " << tn << endl;
		cerr << "sleep time: " << slptime << endl;
		sleep(10);	
		tn = tp.task_number();
	}

	tp.destroy_threadpool();
	printf("%d, %d\n", cnt, spgcnt);
	
	return 0;
}
