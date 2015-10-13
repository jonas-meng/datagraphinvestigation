#ifndef FREQUENCYCOUNTING_H 
#define FREQUENCYCOUNTING_H

#include "../dependency/threadpool.h"
#include "../dependency/rapidjson/document.h"
#include "enumeration.h"

#include <string>
#include <fstream>
#include <unistd.h>
#include <time.h>

class FrequencyCounter {
	private:
		static std::map<int, size_t> idxOfGraph;
		static std::map<size_t, int> counter;
		static std::map<int, int> graphSizeCounter;
		static std::map<int, double> avgElapsedTime;
		static int cnt;
		static Mutex output;

	private:
		static void updateAVG(int size, double t) {
			int c = graphSizeCounter[size];
			graphSizeCounter[size] = c + 1;
			avgElapsedTime[size] = (c * 1.0/ (c + 1)) * avgElapsedTime[size] + t / (c + 1);
		}

		static void frequencyOneGraph(void *d) {
			clock_t start = clock();

			SPGEnumerator spge(*(rapidjson::Document*)d);
			spge.start();
			int n = spge.sizeOfGraph();

			clock_t end = clock();

			clock_t elapsed = end - start;

			output.lock();
			
			if (graphSizeCounter.find(n) == graphSizeCounter.end()) {
				graphSizeCounter[n] = 0;
				avgElapsedTime[n] = 0;
			}
			updateAVG(n, (double)elapsed / CLOCKS_PER_SEC);

			output.unlock();

			size_t hvalue;
			for (std::vector<SimplifiedGraph>::iterator it = spge.spgs.begin(); it != spge.spgs.end(); it++) {
				hvalue = it->getHashValue();	
			output.lock();
				if (counter.find(hvalue) ==
						counter.end()) {
					std::cout << hvalue << "\t"; 
					it->printString();
					idxOfGraph[cnt++] = hvalue;
					counter[hvalue] = spge.frequency(hvalue);
				} else {
					counter[hvalue] += spge.frequency(hvalue);
				}
			output.unlock();
			}

			delete (rapidjson::Document*)d;	
		}

		static	void counting(char *fileName, int nOfThread) {
			ThreadPool tp(nOfThread);
			tp.initialize_threadpool();
			std::string smol;
			std::ifstream inputfile(fileName);
			rapidjson::Document *d;
			int cnt = 0;

			while (getline(inputfile, smol)) {
				std::cerr << cnt++ << std::endl;
				d = new rapidjson::Document();
				d->Parse(smol.c_str());

				Task *t = new Task(&frequencyOneGraph, (void*)d);
				tp.add_task(t);
			}


			std::cerr << "start remaining tasks" << std::endl;
			while (tp.task_number() > 0) {
				sleep(10);
				std::cerr << tp.task_number() << std::endl;
			}
			
			std::cerr << "destroy threadpool" << std::endl;
			tp.destroy_threadpool();

			std::string fn = string(fileName) + ".frequency";
			std::ofstream outputfile(fn);
			for(std::map<int, size_t>::iterator it = idxOfGraph.begin(); it != idxOfGraph.end(); it++) {
				outputfile << it->second << "\t" << counter[it->second] << std::endl;
			}
			outputfile.close();

			fn = string(fileName) + ".time";
			outputfile.open(fn);
			for(std::map<int, double>::iterator it = avgElapsedTime.begin(); it != avgElapsedTime.end(); it++) {
				outputfile << it->first << "\t" << it->second << std::endl;
			}
			outputfile.close();


		}

	public:
		FrequencyCounter(char *fileName, int nOfThread) {	
			counting(fileName, nOfThread);
		}
};

std::map<int, size_t> FrequencyCounter::idxOfGraph;
std::map<size_t, int> FrequencyCounter::counter;
std::map<int, int> FrequencyCounter::graphSizeCounter;
std::map<int, double> FrequencyCounter::avgElapsedTime;
int FrequencyCounter::cnt;
Mutex FrequencyCounter::output;

#endif
