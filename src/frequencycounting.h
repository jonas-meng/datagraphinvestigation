#ifndef FREQUENCYCOUNTING_H 
#define FREQUENCYCOUNTING_H

#include "../dependency/threadpool.h"
#include "../dependency/rapidjson/document.h"
#include "enumeration.h"

#include <string>
#include <fstream>
#include <unistd.h>
#include <time.h>

#define DEBUG

class FrequencyCounter {
	private:
		static std::map<size_t, int> counter;
		static std::map<int, int> GSPGSizeCounter;
		static std::map<int, int> graphSizeCounter;
		static std::map<int, double> avgElapsedTime;
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
			
			updateAVG(n, (double)elapsed / CLOCKS_PER_SEC);

			output.unlock();

			size_t hvalue;
			for (std::map<size_t, int>::iterator it = spge.counter.begin(); it != spge.counter.end(); it++) {
				hvalue = it->first;	
			output.lock();
					counter[hvalue] = counter[hvalue] + it->second;
			output.unlock();
			}

			for (std::map<int, int>::iterator it = spge.GSPGSizeCounter.begin(); it != spge.GSPGSizeCounter.end(); it++) {
				GSPGSizeCounter[it->first] = GSPGSizeCounter[it->first] + it->second;
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
				std::cerr << "index of graph : " << cnt++ << std::endl;
				d = new rapidjson::Document();
				d->Parse(smol.c_str());

				Task *t = new Task(&frequencyOneGraph, (void*)d);
				tp.add_task(t);
				//frequencyOneGraph((void*)d);
			}
			inputfile.close();

			std::cerr << "start remaining tasks" << std::endl;
			while (tp.task_number() > 0) {
				sleep(30);
				std::cerr << tp.task_number() << std::endl;
			}
			
			std::cerr << "destroy threadpool" << std::endl;
			tp.destroy_threadpool();

			std::string fn = string(fileName) + ".gspg.frequency";
			std::ofstream outputfile(fn);
			outputfile << "GSPGHASH\tFREQUENCY" << std::endl;
			for(std::map<size_t, int>::iterator it = counter.begin(); it != counter.end(); it++) {
				outputfile << it->first << "\t" << it->second << std::endl;
			}
			outputfile.close();

			fn = string(fileName) + ".size.frequency";
			outputfile.open(fn);
			outputfile << "GSPGSIZE\tFREQUENCY" << std::endl;
			for(std::map<int, int>::iterator it = GSPGSizeCounter.begin(); it != GSPGSizeCounter.end(); it++) {
				outputfile << it->first << "\t" << it->second << std::endl;
			}
			outputfile.close();

			fn = string(fileName) + ".size.time";
			outputfile.open(fn);
			outputfile << "GSIZE\tTIME" << std::endl;
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

std::map<size_t, int> FrequencyCounter::counter;
std::map<int, int> FrequencyCounter::GSPGSizeCounter;
std::map<int, int> FrequencyCounter::graphSizeCounter;
std::map<int, double> FrequencyCounter::avgElapsedTime;
Mutex FrequencyCounter::output;

#endif
