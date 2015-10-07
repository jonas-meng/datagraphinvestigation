#ifndef REPRESENTATION_H
#define REPRESENTATION_H

#include "../dependency/rapidjson/document.h"

#include <vector>
#include <iostream>
#include <map>
#include <set>
#include <functional>

class Label {
	/*
	 * Rule number is neglected since we are not interested in rebuilding original graph based on final representation
	 *
	 * Adopted encoding schema:
	 * |character|encoding|
	 * |:-------:|:------:|
	 * |)|00|
	 * |0|01|
	 * |(|10|
	 * */
	private:
		std::vector<bool> v;
		std::hash<std::vector<bool> > hasher;

	public:
		static bool comparison(Label* lhs, Label* rhs) { return *lhs < *rhs; }
		static bool equivalence(Label* lhs, Label *rhs) { return (*lhs < *rhs) && (*rhs < *lhs); }

		Label() {}
		Label(bool) { initialNodeLabel(); }
		Label(bool, bool) { initialEdgeLabel(); }

		void rightParenthesis() { v.push_back(0); v.push_back(0); } // encoding for ')' is 00
		void zero() { v.push_back(0); v.push_back(1); } // encoding for '0' is 01
		void leftParenthesis() { v.push_back(1); v.push_back(0); } // encoding for '(' is 10

		void insert(std::vector<bool>::iterator first,
				std::vector<bool>::iterator last) { 
			v.insert(v.end(), first, last); 
		}

		std::vector<bool>::iterator begin() { return v.begin(); }
		std::vector<bool>::iterator end() { return v.end(); }

		void initialNodeLabel() {
			leftParenthesis();
			zero();
			rightParenthesis();
		}

		void initialEdgeLabel() {
			leftParenthesis();
			zero(); zero();
			rightParenthesis();
		}

		void printReadable();
		size_t hashValue() {
			return hasher(v);
		}
		
		bool operator<(Label &rhs);
		bool operator==(Label &rhs);

		// from the perspective of practice, hash<vector<bool>> may be a better option than directly use bit vector; however, consequence of such hash function is remaining to be disclosed.
};

class SPGRepresentation {
	private:
		std::map<int, std::vector<Label*> > nodeLabelSet;
		std::map<std::set<int>, std::vector<Label*> > edgeLabelSet;
		std::set<int> vertexSet;
		std::map<int, std::set<int> > adjSet;

		Label *rpr;

	public:	
		static bool comparisonOfTwoVector(
				std::vector<Label*> *, 
				std::vector<Label*> *);
		static bool equivalenceOfTwoVector(
				std::vector<Label*> *, 
				std::vector<Label*> *);
		
		SPGRepresentation(rapidjson::Document &);
		SPGRepresentation(std::set<std::set<int> > &);
		~SPGRepresentation() { if (rpr) delete rpr; }
		void initialization(rapidjson::Document &);
		void initializeNodeLabel();
		void initializeEdgeLabel(int, int);
		bool rule0_1();
		bool rule0_2();
		bool rule1_1();
		bool rule1_2();
		bool rule1_3();
		bool rule2_1();
		bool rule2_2(std::set<int> &);
		void computation();
		void display(); // display current state including nodelabelset, edgelabelset, and vertexset
		void printRepresentation();
		size_t hashValue() {
			return rpr->hashValue();			
		}

		bool operator==(SPGRepresentation &rhs);
		bool operator<(SPGRepresentation &rhs);

	private:
		std::vector<Label*> *loopLabelGenerator(int, int);
		int approachEndpoint(int, int, std::set<int> &, std::vector<Label*> *);
		void serialComposition(std::set<int> &, int);
		std::vector<Label*>* findMinInLoop(std::vector<Label*>*,int n);
		void clearNode(int n);
		void clearEdge(std::set<int> &e);

};

#endif
