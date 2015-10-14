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
	 * |) |000|0
	 * |-1|001|1
	 * |0 |010|2
	 * |+1|011|3
	 * |( |100|4
	 * */
	private:
		std::vector<bool> v;
		std::hash<std::vector<bool> > hasher;

		struct symbol {
			bool notation; // 0 denotes node while 1 denotes edge
			int ruleNO;
			int orientation;
			std::vector<symbol*> components;

			~symbol() {
				for (std::vector<symbol*>::iterator it = components.begin(); it!= components.end(); it++) {
					delete (*it);
				}
			}
		};

		std::vector<bool>::iterator walker;
		std::map<int, std::set<int> > adjSet;
		int nodeCounter;
		symbol *ps;

	public:
		static bool comparison(Label* lhs, Label* rhs) { return *lhs < *rhs; }
		static bool equivalence(Label* lhs, Label *rhs) { return (*lhs < *rhs) && (*rhs < *lhs); }

		Label() {}
		Label(bool) { initialNodeLabel(); }
		Label(bool, bool) { initialEdgeLabel(); }

		int readOneChar() {
			int res = 0;
			for (int i = 0; i < 3; i ++) {
				if (walker != v.end()) {
					res = res * 2 + *(walker++);
				} else {
					res = -1;
					break;
				}	
			}
			return res;
		}

		void backOneChar() { walker = walker - 3; }

		int readOneChar(std::vector<bool>::iterator it) {
			int res = 0;
			for (int i = 0; i < 3; i ++) {
				if (it != v.end()) {
					res = res * 2 + *(it++);
				} else {
					res = -1;
					break;
				}	
			}
			return res;
		}

		/*
		 * NOTICE: this is a function of semantic level
		 * which reverse edge's orientation and 
		 * should only be used when label corresponds to edge
		 * */
		void reverseOrientation() {
			std::vector<bool>::iterator it = v.end() - 6;
			int orientation = readOneChar(it);
			if (orientation != 2) {
				v.erase(it, v.end());
				if (orientation == 1) {
					positiveOne();	
				} else {
					negativeOne();
				}
				rightParenthesis();
			}
		}

		void rightParenthesis() { 
			v.push_back(0); 
			v.push_back(0); 
			v.push_back(0); 
		} // encoding for ')' is 000
		void negativeOne() { 
			v.push_back(0); 
			v.push_back(0); 
			v.push_back(1); 
		} // encoding for ')' is 001
		void zero() { 
			v.push_back(0); 
			v.push_back(1); 
			v.push_back(0); 
		} // encoding for '0' is 010
		void positiveOne() { 
			v.push_back(0); 
			v.push_back(1); 
			v.push_back(1); 
		} // encoding for '0' is 011
		void leftParenthesis() { 
			v.push_back(1); 
			v.push_back(0); 
			v.push_back(0); 
		} // encoding for '(' is 100

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
		void displayAdjSet();
		size_t hashValue() {
			return hasher(v);
		}
		std::map<int, std::set<int> > &getAdjSet() { return adjSet; }

		void parseRepresentation();
		symbol *recursivePaser();
		void determineSymbolNotation(symbol *);
		void reconstruct();
		void nodeReconstruct(int, symbol*);
		void edgeReconstruct(int, int, symbol*);
		void unpack();
		
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
		~SPGRepresentation() { if(rpr) { delete rpr;} }
		void initialization(rapidjson::Document &);
		void initializeNodeLabel();
		void initializeEdgeLabel(int, int);
		void computation();

		void display(); // display current state including nodelabelset, edgelabelset, and vertexset
		void displayAdjSet();
		void displayLabelOfNode();
		void displayLabelOfEdge();
		void printRepresentation();

		size_t hashValue() {
			if (rpr) {
				return rpr->hashValue();			
			} else {
				return 0;
			}
		}
		void unpack() { if(rpr) rpr->unpack(); adjSet = rpr->getAdjSet(); }

		bool operator==(SPGRepresentation &rhs);
		bool operator<(SPGRepresentation &rhs);

	private:
		std::vector<Label*> *loopLabelGenerator(int, int);
		int approachEndpoint(int, int, std::set<int> &, std::vector<Label*> *);
		void serialComposition(std::set<int> &, int);
		std::vector<Label*>* findMinInLoop(std::vector<Label*>*,int n);
		void clearNode(int n);
		void clearEdge(std::set<int> &e);
		void reverseEdge(std::vector<Label*> &);
		Label *edgeLabelGenerator(std::vector<Label*> &);
		Label *labelGeneratorRule0_2(std::vector<Label*> &);
		Label *labelGeneratorRule1_1(int, std::set<int> &,int);
		Label *labelGeneratorRule2_1(std::vector<Label*> &);
		Label *labelGeneratorRule2_2(std::vector<Label*> &);

		bool rule0_1();
		bool rule0_2();
		bool rule1_1();
		bool rule1_2();
		bool rule1_3(Label*, int);
		bool rule2_1();
		bool rule2_2(std::set<int> &);
	
};

#endif
