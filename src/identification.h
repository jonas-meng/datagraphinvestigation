#include "../dependency/rapidjson/document.h"

#include <set>
#include <map>
#include <queue>

class SPGIdentifier {
	private: 
		std::set<int> nodeSet;
		std::map<int, std::set<int> > adjSet;
		std::queue<int> queueofv2;
		std::set<int> pendentVertex;
		rapidjson::Document document;

	public:
		SPGIdentifier(){}

		void initialization(const char *);
		void searchv2();
		void searchv1();
		void serialReduction(int);
		void removalOfPendantVertex();
		void removalv1(int);
		bool identify(const char *);
};
