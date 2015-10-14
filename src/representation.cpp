#include "../dependency/rapidjson/document.h"
#include "representation.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

/*
#define DEBUG
#define DEBUG0_1
#define DEBUG0_2
#define DEBUG1_1
#define DEBUG1_2
#define DEBUG2_1
#define DEBUG2_2
*/

void Label::printReadable() {
	short n;
	short precedent = -1;
	walker = v.begin();
	while ((n = readOneChar()) != -1) {
		if (((n > 0 && n < 4) && 
					(precedent >= 0 && precedent < 4)) ||
				((precedent > 0 && precedent < 4) &&
				 n == 4) ||
				(precedent == 0 && n == 4)) {
			std::cout << ",";
		}
		switch(n) {
			case 0: std::cout << ')'; break;
			case 1: std::cout << "-1"; break;
			case 2: std::cout << '0'; break;
			case 3: std::cout << "+1"; break;
			case 4: std::cout << '('; break;
			default: break;
		}
		precedent = n;
	}
	std::cout << std::endl;
}

void Label::displayAdjSet() {
	std::cout << "--------------------------" << std::endl;
	std::cout << "Vertex set:" << std::endl;
	for (std::map<int, std::set<int> >::iterator it = adjSet.begin(); it != adjSet.end(); it++) {
		std::cout << it->first << ": ";

		for (std::set<int>::iterator vit = it->second.begin();
				vit != it->second.end(); vit++) {
			std::cout << *vit << ",";
		}
		std::cout << std::endl;
	}
}

bool Label::operator<(Label &rhs) {
	// NOTICE: by default left hand side is smaller for a case of equivalence
	std::vector<bool>::iterator it1 = v.begin();
	std::vector<bool>::iterator it2 = rhs.begin();
	for (; it1 != v.end() && it2 != rhs.end(); 
			it1++, it2++) {
		if (*it1 == *it2) continue;
		else return (*it1 < *it2);
	}
	return (it1 == v.end());
}

bool Label::operator==(Label &rhs) {
	return ((*this) < rhs && rhs < (*this));
}

void Label::unpack() {
	parseRepresentation();
	reconstruct();
	delete ps;
}

void Label::reconstruct() {
	nodeCounter = 0;	
	nodeReconstruct(++nodeCounter, ps);
}

void Label::nodeReconstruct(int number, symbol *s) {
	if (s->ruleNO != -1) {
		int t;
		if (s->ruleNO == 1) {
			for (std::vector<symbol*>::iterator it = s->components.begin(); it != s->components.end(); it++) {
				nodeReconstruct(number, *it);	
			}
#ifdef DEBUG
			std::cout << "RULE 0.1 UNPACK" << std::endl;
			displayAdjSet();
#endif
		} else if (s->ruleNO == 11) {
			t = ++nodeCounter;
			std::vector<symbol*>::iterator it = s->components.begin();
			nodeReconstruct(number, *it);
			if ((*(it+1))->orientation == -1) {
				edgeReconstruct(t, number, *(it+1));
			} else {
				edgeReconstruct(number, t, *(it+1));
			}
			nodeReconstruct(t, *(it+2));
#ifdef DEBUG
			std::cout << "RULE 1.1 UNPACK" << std::endl;
			displayAdjSet();
#endif
		} else if (s->ruleNO == 12) {
			t = ++nodeCounter;	

			if ((*s->components.begin())->orientation == -1) {
				edgeReconstruct(t, number, *(s->components.begin()));
			} else {
				edgeReconstruct(number, t, *(s->components.begin()));

			}
			nodeReconstruct(t, *(s->components.rbegin()));	
#ifdef DEBUG
			std::cout << "RULE 1.2 UNPACK" << std::endl;
			displayAdjSet();
#endif
		} else if (s->ruleNO == 13) {
			edgeReconstruct(number, number, *(s->components.begin()));
#ifdef DEBUG
			std::cout << "RULE 1.3 UNPACK" << std::endl;
			displayAdjSet();
#endif
		} else if (s->ruleNO == 22) {
			int t1 = number, t2;
			std::vector<symbol*>::iterator it = s->components.begin();
			for (; (it+2) != s->components.end();) { 
				t2 = ++nodeCounter;
				
				if ((*it)->orientation == -1) {
					edgeReconstruct(t2, t1, *(it++));
				} else {
					edgeReconstruct(t1, t2, *(it++));
				}
				nodeReconstruct(t2, *(it++));
				t1 = t2;
			}
			t2 = number;
			
			if ((*it)->orientation == -1) {
				edgeReconstruct(t2, t1, *(it++));
			} else {
				edgeReconstruct(t1, t2, *(it++));
			}
			nodeReconstruct(t2, *(it++));
#ifdef DEBUG
			std::cout << "RULE 2.2 UNPACK" << std::endl;
			displayAdjSet();
#endif
		} else {
			std::cerr << "WRONG RULE NUMBER FOR NODE:" << s->ruleNO << std::endl;
		}

	}
}

void Label::edgeReconstruct(int a, int b, symbol *s) {
	if (s->ruleNO != -1) {
		if (s->ruleNO == 2) {
			for (std::vector<symbol*>::iterator it = s->components.begin(); it != s->components.end(); it++) { 
				if ((*it)->orientation == -1) {
					edgeReconstruct(b, a, *it);
				} else {
					edgeReconstruct(a, b, *it);
				}
			}
#ifdef DEBUG
			std::cout << "RULE 0.2 UNPACK" << std::endl;
			displayAdjSet();
#endif
		} else if (s->ruleNO == 21) {
			int t1 = a, t2;
			std::vector<symbol*>::iterator it = s->components.begin();
			for (; (it+1) != s->components.end();) { 
				t2 = ++nodeCounter;
				if ((*it)->orientation == -1) {
					edgeReconstruct(t2, t1, *(it++));
				} else {
					edgeReconstruct(t1, t2, *(it++));
				}
				nodeReconstruct(t2, *(it++));
				t1 = t2;
			}
			t2 = b;
			if ((*it)->orientation == -1) {
				edgeReconstruct(t2, t1, *(it++));
			} else {
				edgeReconstruct(t1, t2, *(it++));
			}
#ifdef DEBUG
			std::cout << "RULE 2.1 UNPACK" << std::endl;
			displayAdjSet();
#endif
		} else {
			std::cerr << "WRONG RULE NUMBER FOR EDGE:" << s->ruleNO << std::endl;
		}
	} else {
		adjSet[a].insert(b);
		adjSet[b].insert(a);
	}
}

void Label::parseRepresentation() {
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
	walker = v.begin();
	readOneChar(); // read first left parenthesis
	ps = recursivePaser();
}

Label::symbol *Label::recursivePaser() {
	int c, n;
	Label::symbol *s = new symbol(), *t;
	std::vector<bool>::iterator beg = walker;
	while (true) { // end of label
		c = readOneChar();
		if (c == 4) { // start of label
			t = recursivePaser();
			s->components.push_back(t);
		} else if (c == 2) { // possible simple node or edge
			c = readOneChar();
			if (c == 0) { // simple node
				s->notation = 0;
				s->ruleNO = -1;
				s->orientation = 0;
			} else if (c == 2) { // simple edge
				s->notation = 1;	
				s->ruleNO = -1;
				s->orientation = 0;
				c = readOneChar(); // read right parenthesis
			} else {
				std::cerr << "UNEXPECTED END OF REPRESENTATION" << std::endl;
			}
			return s;
		} else if (c == 0) {
			break;
		} else if (c >0 && c < 4) {
			// orientation of edge	
			s->orientation = c;	

		} else {
			std::cerr << "UNEXPECTED END OF REPRESENTATION" << std::endl;
		}
	}	
	std::vector<bool>::iterator end = walker;
	for (; beg != end; beg+= 3) {
		c = readOneChar(beg);
		switch(c) {
			case 0: std::cout << ")"; break;
			case 1: std::cout << "-1"; break;
			case 2: std::cout << "0"; break;
			case 3: std::cout << "+1"; break;
			case 4: std::cout << "("; break;
		}
	}
	determineSymbolNotation(s);
	std::cout  << " " << s->ruleNO <<  " " << s->orientation << " " << s->notation << std::endl;

	return s;
}

void Label::determineSymbolNotation(symbol *s) {
	int noe = 0, non = 0, first, last;
	first = (*(s->components).begin())->notation;
	last = (*(s->components).rbegin())->notation;
	for (std::vector<symbol*>::iterator it = s->components.begin(); it != s->components.end(); it++) {
		if ((*it)->notation == 0) non++;
		else noe++;
	}
	if (noe == 0) {
		s->notation = 0;
		s->ruleNO = 1;
	} else if ((non == 0 && noe == 1)) {
		s->notation = 0;
		s->ruleNO = 13;
	} else if ((non == 1 && noe == 1 && first == 1 && last == 0)) {
		s->notation = 0;
		s->ruleNO = 12;
	} else if ((non == noe + 1)) {
		s->notation = 0;
		s->ruleNO = 11;
	} else if ((non == 0)) {
		s->notation = 1;
		s->ruleNO = 2;
	} else if (noe == non + 1) {
		s->notation = 1;
		s->ruleNO = 21;
	} else if (non == noe) {
		s->notation = 0;
		s->ruleNO = 22;
	} else {
		std::cerr << "WRONG COMPOSITION OF NOTATION" << std::endl;
	}
}

SPGRepresentation::SPGRepresentation(rapidjson::Document &d) {
	rpr = NULL;
	initialization(d);
}

SPGRepresentation::SPGRepresentation(std::set<std::set<int> > &edgeSet) {
	int a, b;
	for (std::set<std::set<int> >::iterator it = edgeSet.begin(); it != edgeSet.end(); it++) {
		a = *(*it).begin();
		b = *(*it).rbegin();

		initializeEdgeLabel(a, b);
	}
	initializeNodeLabel();
}

bool SPGRepresentation::operator<(SPGRepresentation &rhs) {
	return (*(this->rpr) < *(rhs.rpr));
}

bool SPGRepresentation::operator==(SPGRepresentation &rhs) {
	return (*(this->rpr) == *(rhs.rpr));
}

void SPGRepresentation::initialization(rapidjson::Document &d) { 
	int a, b, length = d["bond"]["aid1"].Size();
	for (int i = 0; i < length; i++) {

		// obtain two end points of one edge
		a = d["bond"]["aid1"][i].GetInt();
		b = d["bond"]["aid2"][i].GetInt();

		initializeEdgeLabel(a, b);
	}

	initializeNodeLabel();
}

void SPGRepresentation::initializeEdgeLabel(int a, int b) {
	std::set<int> edge;
	// insert into adjacent set of each other
	adjSet[a].insert(b);
	adjSet[b].insert(a);

	// insert in set of vertex
	vertexSet.insert(a);
	vertexSet.insert(b);

	// initialize label set of edge
	edge.clear();
	edge.insert(a);
	edge.insert(b);
	edgeLabelSet[edge].push_back(new Label(true, true)); 
}

void SPGRepresentation::initializeNodeLabel() {
	// initialize label set of node
	for (std::set<int>::iterator it = vertexSet.begin();
			it != vertexSet.end(); it++) {
		nodeLabelSet[*it].push_back(new Label(true));
	}
}

void SPGRepresentation::reverseEdge(std::vector<Label*> &v) {
	for (std::vector<Label*>::iterator
			lit = v.begin();
			lit != v.end();
			lit++) {
		(*lit)->reverseOrientation();
	}
}

Label *SPGRepresentation::edgeLabelGenerator(std::vector<Label*> &v) {
	Label *newLabel = new Label();
	newLabel->leftParenthesis();

	for (std::vector<Label*>::iterator
			lit = v.begin();
			lit != v.end();
			lit++) {
		newLabel->insert((*lit)->begin(),
				(*lit)->end());
	}

	newLabel->rightParenthesis();
	return newLabel;
}

Label *SPGRepresentation::labelGeneratorRule0_2(std::vector<Label*> &v) {
	return edgeLabelGenerator(v);
}

bool SPGRepresentation::rule0_1() {
	bool res = false;
	std::vector<int> nodeWithMultiLabel;

	// get indices of all nodes with multiple labels
	for (std::map<int, std::vector<Label*> >::iterator 
			it = nodeLabelSet.begin();
			it != nodeLabelSet.end(); it++) {
		if (it->second.size() > 1) {
			nodeWithMultiLabel.push_back(it->first);	
		}
	}

	if (nodeWithMultiLabel.size() > 0) {
#ifdef DEBUG 
	std::cout << "RULE 0.1 APPLIED" << std::endl;
#endif
		res = true;
#ifdef DEBUG0_1
			std::cout << "node with multiple label: " << std::endl;
#endif
		/*
		 * for each node with multiple labels,
		 * create a new label within which labels are sorted*/
		for (std::vector<int>::iterator
				it = nodeWithMultiLabel.begin();
				it != nodeWithMultiLabel.end();
				it++) {

#ifdef DEBUG0_1
			std::cout << *it << std::endl;
#endif
			std::sort(nodeLabelSet[*it].begin(),
					nodeLabelSet[*it].end(),
					Label::comparison);	

			Label *newNodeLabel = new Label();
			newNodeLabel->leftParenthesis();

			for (std::vector<Label*>::iterator
					lit = nodeLabelSet[*it].begin();
					lit != nodeLabelSet[*it].end();
					lit++) {
				newNodeLabel->insert((*lit)->begin(), (*lit)->end());
				delete (*lit);
			}

			newNodeLabel->rightParenthesis();

#ifdef DEBUG0_1
			newNodeLabel->printReadable();
#endif

			nodeLabelSet[*it].clear();
			nodeLabelSet[*it].push_back(newNodeLabel);
		}
	}

	return res;
}

bool SPGRepresentation::rule0_2() {
	bool res = false;

	std::vector<std::set<int> > edgeWithMultipleLabels;

	for (std::map<std::set<int>, std::vector<Label*> >::
			iterator it = edgeLabelSet.begin();
			it != edgeLabelSet.end();
			it++) {
		if (it->second.size() > 1) {
			edgeWithMultipleLabels.push_back(it->first);
		}	
	}

	if (edgeWithMultipleLabels.size() > 0) {
#ifdef DEBUG 
	std::cout << "RULE 0.2 APPLIED" << std::endl;
#endif
		res = true;

#ifdef DEBUG0_2
			std::cout << "edge with multiple label: " << std::endl;
#endif
		for (std::vector<std::set<int> >::iterator
				it = edgeWithMultipleLabels.begin();
				it != edgeWithMultipleLabels.end();
				it++) {

#ifdef DEBUG0_2
			std::cout << "(" << *(it->begin()) << ", " 
				<< *(it->rbegin()) << ")" << std::endl;
#endif
			std::sort(edgeLabelSet[*it].begin(),
					edgeLabelSet[*it].end(),
					Label::comparison);
			Label *newEdgeLabel1 = labelGeneratorRule0_2(edgeLabelSet[*it]);

			reverseEdge(edgeLabelSet[*it]);

			std::sort(edgeLabelSet[*it].begin(),
					edgeLabelSet[*it].end(),
					Label::comparison);
			Label *newEdgeLabel2 = labelGeneratorRule0_2(edgeLabelSet[*it]);

			Label *newEdgeLabel = new Label();
			newEdgeLabel->leftParenthesis();
			if (Label::comparison(newEdgeLabel1, newEdgeLabel2)) {
				newEdgeLabel->insert(newEdgeLabel1->begin(), newEdgeLabel1->end());
				if (Label::comparison(newEdgeLabel2, newEdgeLabel1)) {
					newEdgeLabel->zero();
				} else {
					newEdgeLabel->positiveOne();
				}
			} else {
				newEdgeLabel->insert(newEdgeLabel2->begin(),
						newEdgeLabel2->end());
				newEdgeLabel->negativeOne();
			}
			newEdgeLabel->rightParenthesis();

			delete newEdgeLabel1;
			delete newEdgeLabel2;

			for (std::vector<Label*>::iterator
					lit = edgeLabelSet[*it].begin();
					lit != edgeLabelSet[*it].end();
					lit++) {
				delete (*lit);
			}
#ifdef DEBUG0_2
			newEdgeLabel->printReadable();
#endif

			edgeLabelSet[*it].clear();
			edgeLabelSet[*it].push_back(newEdgeLabel);
		}
	}

	return res;
}

Label *SPGRepresentation::labelGeneratorRule1_1(int a, std::set<int> &edge, int b) {
	Label *newNodeLabel = new Label(), *t;
	newNodeLabel->leftParenthesis();

	t = *nodeLabelSet[a].begin();
	newNodeLabel->insert(t->begin(), t->end());

	t = *edgeLabelSet[edge].begin();
	newNodeLabel->insert(t->begin(), t->end());

	t = *nodeLabelSet[b].begin();
	newNodeLabel->insert(t->begin(), t->end());

	newNodeLabel->rightParenthesis();

	return newNodeLabel;
}

bool SPGRepresentation::rule1_1() {
	bool res = false;
	// a connected graph with only two vertices is a dipole
	if (vertexSet.size() == 2) {
#ifdef DEBUG 
	std::cout << "RULE 1.1 APPLIED" << std::endl;
#endif
		res = true;
		int a = *vertexSet.begin(), b = *vertexSet.rbegin(), c;
		Label *newNodeLabel, *t;
		std::set<int> edge;
		edge.insert(a);
		edge.insert(b);

		if (a > b) {
			c = a; a = b; b = c;
		}

		Label *newLabel1, *newLabel2;
		newLabel1 = labelGeneratorRule1_1(a, edge, b);	
		(*edgeLabelSet[edge].begin())->reverseOrientation();
		newLabel2 = labelGeneratorRule1_1(b, edge, a);	

#ifdef DEBUG1_1
	std::cout << a << ",(" << a << "," << b << "),"<< b << std::endl;
#endif
		if (Label::comparison(newLabel1, newLabel2)) {
			newNodeLabel = newLabel1;
			delete newLabel2;
		} else {
			newNodeLabel = newLabel2;
			delete newLabel1;
		}

		clearNode(a);
		clearNode(b);
		clearEdge(edge);
#ifdef DEBUG1_1
		newNodeLabel->printReadable();
#endif

		vertexSet.insert(a);
		nodeLabelSet[a].push_back(newNodeLabel);
	}	

	return res;
}

bool SPGRepresentation::rule1_2() {
	bool res = false;
	std::set<int> pendantVertex;
	std::set<int> edge;

  	for (std::set<int>::iterator it = vertexSet.begin();
		   it != vertexSet.end();
		   it++) {
		if (adjSet[*it].size() == 1) {
			pendantVertex.insert(*it);
		}
	}	

	Label *newNodeLabel, *t;
	int a;

	if (pendantVertex.size() > 0) {
#ifdef DEBUG 
	std::cout << "RULE 1.2 APPLIED" << std::endl;
#endif
		res = true;
	}

	for (std::set<int>::iterator it = pendantVertex.begin();
			it != pendantVertex.end();
			it++) {

		a = *adjSet[*it].begin();
		adjSet[a].erase(*it);

		edge.clear();
		edge.insert(a);
		edge.insert(*it);

#ifdef DEBUG1_2
		std::cout << *it << ", (" <<
			*it << ", " << a << ")" << std::endl;
#endif

		newNodeLabel = new Label();	

		newNodeLabel->leftParenthesis();
		// edge
		if (a < *it) {
			(*edgeLabelSet[edge].begin())->reverseOrientation();
		}

		t = *edgeLabelSet[edge].begin();
		newNodeLabel->insert(t->begin(), t->end());
		// node
		t = *nodeLabelSet[*it].begin();
		newNodeLabel->insert(t->begin(), t->end());

		newNodeLabel->rightParenthesis();

		// maintain consistent state
		clearNode(*it);
		clearEdge(edge);
#ifdef DEBUG1_2
		newNodeLabel->printReadable();
#endif

		nodeLabelSet[a].push_back(newNodeLabel);
	}

	return res;
}


bool SPGRepresentation::rule1_3(Label *edge, int n) {
	Label *newNodeLabel = new Label();
	newNodeLabel->leftParenthesis();
	newNodeLabel->insert(edge->begin(), edge->end());
	newNodeLabel->rightParenthesis();

	nodeLabelSet[n].push_back(newNodeLabel);
	delete edge;
	return true;
}

Label *SPGRepresentation::labelGeneratorRule2_1(std::vector<Label*> &v) {
	return edgeLabelGenerator(v);
}

bool SPGRepresentation::rule2_1() {
	bool res = false;
	
	std::set<int> vertexWithDegree2;
	for (std::set<int>::iterator it = vertexSet.begin();
			it != vertexSet.end(); it++) {
		if (adjSet[*it].size() == 2) {
			vertexWithDegree2.insert(*it);
		}
	}

	// check whether remaining graph is a connected two-regular graph or not
	if (vertexWithDegree2.size() == vertexSet.size()) {
		// all remaining vertices are of degree two, indicating rule 2.2 is applicable	
		res = rule2_2(vertexWithDegree2);
	} else if (vertexWithDegree2.size() > 0) {
#ifdef DEBUG 
	std::cout << "RULE 2.1 APPLIED" << std::endl;
#endif
		int a;
		res = true;

		while (!vertexWithDegree2.empty()) {
			a = *vertexWithDegree2.begin();
			serialComposition(vertexWithDegree2, a);
		}
	}

	return res;
}

void SPGRepresentation::serialComposition(std::set<int> &vertexWithDegree2, int a) {
#ifdef DEBUG2_1
	std::cout << "---" << std::endl;
#endif
	// approaching one endpoint
	int b = *adjSet[a].begin(), e1, e2;
	std::vector<Label*> *t1, *t2;
	t1 = new std::vector<Label*>();
	e1 = approachEndpoint(a, b, vertexWithDegree2, t1);

	for (std::vector<Label*>::iterator it = t1->begin()+1;
			(it+1) != t1->end(); it += 2) {
		(*it)->reverseOrientation();	
	}
	(*t1->rbegin())->reverseOrientation();

	// approaching another endpoint
	b = *adjSet[a].rbegin();
	t2 = new std::vector<Label*>(t1->rbegin(), t1->rend());
	t2->erase(t2->end()-1);
	e2 = approachEndpoint(a, b, vertexWithDegree2, t2);	

	Label *newLabel1, *newLabel2;
	if (e1 < e2) {
		newLabel1 = labelGeneratorRule2_1(*t2);
	} else {
		newLabel2 = labelGeneratorRule2_1(*t2);
	}
	delete t1;

	t1 = new std::vector<Label*>(t2->rbegin(), t2->rend());
	
	// reverse direction of edges
	for (std::vector<Label*>::iterator it = t1->begin();
			(it+1) != t1->end(); it += 2) {
		(*it)->reverseOrientation();	
	}
	(*t1->rbegin())->reverseOrientation();

	if (e1 < e2) {
		newLabel2 = labelGeneratorRule2_1(*t1);
	} else {
		newLabel1 = labelGeneratorRule2_1(*t1);
	}

	delete t2;

	// create new label
	Label *newEdgeLabel = new Label();
	newEdgeLabel->leftParenthesis();
	if (Label::comparison(newLabel1, newLabel2)) {
		newEdgeLabel->insert(newLabel1->begin(),
				newLabel1->end());	
		if (Label::comparison(newLabel2, newLabel1)) {
			newEdgeLabel->zero();
		} else {
			newEdgeLabel->positiveOne();
		}
	} else {
		newEdgeLabel->insert(newLabel2->begin(),
				newLabel2->end());
		newEdgeLabel->negativeOne();
	}
	newEdgeLabel->rightParenthesis();
#ifdef DEBUG2_1
		newEdgeLabel->printReadable();
#endif

	// release contained labels
	for (std::vector<Label*>::iterator it = t1->begin();
			it != t1->end(); it++) {
		delete (*it);
	}
	delete t1;

	delete newLabel1;
	delete newLabel2;

	if (e1 == e2) {
		rule1_3(newEdgeLabel, e1);
	} else {
		std::set<int> edge;
		edge.insert(e1);
		edge.insert(e2);
		edgeLabelSet[edge].push_back(newEdgeLabel);
		adjSet[e1].insert(e2);
		adjSet[e2].insert(e1);
	}
}

int SPGRepresentation::approachEndpoint(int a, int b, std::set<int> &vertexWithDegree2, std::vector<Label*> * t) {
	std::set<int> visited;
	std::set<int> edge;
	int c;

	while (vertexWithDegree2.find(b) != 
			vertexWithDegree2.end()) {
		visited.insert(a);
#ifdef DEBUG2_1
		std::cout << a << ",";
#endif
		t->push_back(*nodeLabelSet[a].begin());
		nodeLabelSet[a].clear();

		if (vertexWithDegree2.find(a) != vertexWithDegree2.end()) {
			vertexWithDegree2.erase(a);
			vertexSet.erase(a);
		}
		edge.clear();
		edge.insert(a);
		edge.insert(b);
		if (a > b) {
			(*edgeLabelSet[edge].begin())->reverseOrientation();
		}
		t->push_back(*edgeLabelSet[edge].begin());
		edgeLabelSet[edge].clear();

		c = b;
		b = (*adjSet[b].begin() != a ? *adjSet[b].begin() : *adjSet[b].rbegin());
		a = c;

	}

	t->push_back(*nodeLabelSet[a].begin());
	nodeLabelSet[a].clear();
	if (vertexWithDegree2.find(a) != vertexWithDegree2.end()) {
		vertexWithDegree2.erase(a);
		vertexSet.erase(a);
	}

	edge.clear();
	edge.insert(a);
	edge.insert(b);
#ifdef DEBUG2_1
		std::cout << a << "," << b << std::endl;
#endif
	adjSet[b].erase(a);
	if (a > b) {
		(*edgeLabelSet[edge].begin())->reverseOrientation();
	}
	t->push_back(*edgeLabelSet[edge].begin());
	edgeLabelSet[edge].clear();

	return b;
	
}

Label *SPGRepresentation::labelGeneratorRule2_2(std::vector<Label*> &v) {
	return edgeLabelGenerator(v);
}

bool SPGRepresentation::rule2_2(std::set<int> &vertices) {
#ifdef DEBUG 
	std::cout << "RULE 2.2 APPLIED" << std::endl;
#endif
	int a = *vertices.begin(), b;
	std::vector<Label*> *t, *minv1, *minv2;
	Label *minl1, *minl2;

	// two different direction
	b = *adjSet[a].begin();
	t = loopLabelGenerator(a, b);
	minv1 = findMinInLoop(t, vertices.size());	
	minl1 = labelGeneratorRule2_2(*minv1);
	delete t;

	b = *adjSet[a].rbegin();
	t = loopLabelGenerator(a, b);
	minv2 = findMinInLoop(t, vertices.size());
	minl2 = labelGeneratorRule2_2(*minv2);
	delete t;

	if (!Label::comparison(minl1, minl2)) {
		delete minv1;
		delete minl1;
		minv1 = minv2;
		minl1 = minl2;
	} else {
		delete minv2;
		delete minl2;
		minv2 = NULL;
		minl2 = NULL;
	}

	Label *newNodeLabel = minl1;

	for (std::vector<Label*>::iterator it = minv1->begin();
			it != minv1->end(); it++) {
		delete (*it);
	}
	delete minv1;

#ifdef DEBUG2_2
		newNodeLabel->printReadable();
#endif
	vertexSet.clear();
	vertexSet.insert(a);
	adjSet[a].clear();
	nodeLabelSet[a].clear();
	nodeLabelSet[a].push_back(newNodeLabel);
	return true;
}

std::vector<Label*>* SPGRepresentation::findMinInLoop(std::vector<Label*> *t, int n) {
	int cnt = 0;
	std::vector<Label*> *minv = new std::vector<Label*>(t->begin(), t->end());
	Label *l1=NULL, *l2=NULL;
	while (cnt < n-1) {
		l1 = *(t->begin()), l2 = *(t->begin()+1);
		for (std::vector<Label*>::iterator it = t->begin();
				it+2 != t->end(); it += 2) {
			*it = *(it+2);
			*(it+1) = *(it+3);
		}
		*(t->rbegin()+1) = l1;
		*(t->rbegin()) = l2;

		if (!SPGRepresentation::comparisonOfTwoVector(minv, t)) {
			delete minv;
			minv = new std::vector<Label*>(t->begin(), t->end());
		}
		cnt++;
	}
	return minv;
}

std::vector<Label*> *SPGRepresentation::loopLabelGenerator(int a, int b) {
	std::vector<Label*> *t = new std::vector<Label*>();
	std::set<int> edge;
	std::set<int> visited;
	visited.insert(a);
	int initiala = a, c;

	while (visited.find(b) == visited.end()) {
		edge.clear();
		edge.insert(a);
		edge.insert(b);
#ifdef DEBUG2_2
		std::cout << "(" << a << "," << b << "),"
			<< a << std::endl;
#endif
		if (a > b) {
			(*edgeLabelSet[edge].begin())->reverseOrientation();
		}
		t->push_back(*edgeLabelSet[edge].begin());
		t->push_back(*nodeLabelSet[a].begin());
		edgeLabelSet[edge].clear();
		nodeLabelSet[a].clear();

		visited.insert(b);
		c = b;
		b = (*adjSet[b].begin() != a ?
			*adjSet[b].begin() : *adjSet[b].rbegin());
		a = c;
	}

	b = initiala;
	// edge between end points
	edge.clear();
	edge.insert(a);
	edge.insert(b);
#ifdef DEBUG2_2
		std::cout << "(" << a << "," << b << "),"
			<< a << std::endl;
#endif
	if (a > b) {
		(*edgeLabelSet[edge].begin())->reverseOrientation();
	}

	t->push_back(*edgeLabelSet[edge].begin());
	t->push_back(*nodeLabelSet[a].begin());
	edgeLabelSet[edge].clear();
	nodeLabelSet[a].clear();

	return t;
}

bool SPGRepresentation::comparisonOfTwoVector(
		std::vector<Label*> *lhs,
		std::vector<Label*> *rhs) {
	std::vector<Label*>::iterator lit = lhs->begin(), 
	rit = rhs->begin();
	for (; lit != lhs->end() && rit != rhs->end();
			lit++, rit++) {
		if (Label::equivalence(*lit, *rit)) continue;
		else return Label::comparison(*lit, *rit);
	}
	return (lit == lhs->end());
}

bool SPGRepresentation::equivalenceOfTwoVector(
		std::vector<Label*> *lhs,
		std::vector<Label*> *rhs) {
	return SPGRepresentation::comparisonOfTwoVector(lhs, rhs) && SPGRepresentation::comparisonOfTwoVector(rhs, lhs);
}

void SPGRepresentation::computation() {
#ifdef DEBUG
	std::cout << "start computation" << std::endl;
#endif
	while (vertexSet.size() > 1 ||
			nodeLabelSet[*vertexSet.begin()].size() > 1) 
		if (rule0_1() || rule0_2() ||
				rule1_1() || rule1_2() || rule2_1()) {
		}

	this->rpr = *nodeLabelSet[*vertexSet.begin()].begin();
}

void SPGRepresentation::clearNode(int n) {
	vertexSet.erase(n);
	for (std::set<int>::iterator it = adjSet[n].begin();
			it != adjSet[n].end(); it++) {
		adjSet[*it].erase(n);
	}
	adjSet[n].clear();
	for (std::vector<Label*>::iterator it = nodeLabelSet[n].begin(); it != nodeLabelSet[n].end(); it++) {
		delete *it;
	}
	nodeLabelSet[n].clear();
}

void SPGRepresentation::clearEdge(std::set<int> &e) {
	for (std::vector<Label*>::iterator it = edgeLabelSet[e].begin(); it != edgeLabelSet[e].end(); it++) {
		delete *it;
	}
	edgeLabelSet[e].clear();
}

/*
 * NOTICE: following part is used for testing and debugging
 * which is not a part of the implementation
 * */

void labelCaseTest() {
	Label node(true);
	Label edge(true, true);
	std::cout<<"node representation"<<std::endl;
	node.printReadable();

	std::cout<<"edge representation"<<std::endl;
	edge.printReadable();

	Label comp;
	comp.leftParenthesis();
	comp.insert(node.begin(), node.end());
	comp.insert(edge.begin(), edge.end());
	comp.rightParenthesis();

	std::cout<<"complex representation"<<std::endl;
	comp.printReadable();

	// test less than operation
	std::cout<<"by default, node is smaller than node: "<<(node<node)<<std::endl;
	std::cout<<"node is smaller than edge: "<<(node<edge)<<std::endl;
	std::cout<<"node is smaller than compplex: "<<(node<comp)<<std::endl;

	// test sorting functionality
	std::vector<Label*> vl;
	vl.push_back(&comp);
	vl.push_back(&edge);
	vl.push_back(&node);

	std::cout<<"before sorting"<<std::endl;
	for (std::vector<Label*>::iterator it = vl.begin();
			it != vl.end(); it++) {
		(*it)->printReadable();
	}
	std::cout<<"after sorting"<<std::endl;
	std::sort(vl.begin(), vl.end(), Label::comparison);
	for (std::vector<Label*>::iterator it = vl.begin();
			it != vl.end(); it++) {
		(*it)->printReadable();
	}
}

void SPGRepresentation::displayAdjSet() {
	std::cout << "--------------------------" << std::endl;
	std::cout << "Vertex set:" << std::endl;
	for (std::map<int, std::set<int> >::iterator it = adjSet.begin(); it != adjSet.end(); it++) {
		std::cout << it->first << ": ";

		for (std::set<int>::iterator vit = it->second.begin();
				vit != it->second.end(); vit++) {
			std::cout << *vit << ",";
		}
		std::cout << std::endl;
	}
}

void SPGRepresentation::displayLabelOfNode() {
	std::cout << "--------------------------" << std::endl;
	
	std::cout << "labels of node:" << std::endl;
	for (std::set<int>::iterator it = vertexSet.begin();
			it != vertexSet.end(); it++) {
		std::cout << *it << ": {" << std::endl;
		for (std::vector<Label*>::iterator vit = nodeLabelSet[*it].begin(); vit != nodeLabelSet[*it].end(); vit++) {
			(*vit)->printReadable();
		}
		std::cout << "}" << std::endl;
	}
}

void SPGRepresentation::displayLabelOfEdge() {
	std::cout << "--------------------------" << std::endl;

	std::cout << "labels of edge:" << std::endl;
	for (std::map<std::set<int>, std::vector<Label*> >::iterator it = edgeLabelSet.begin(); it != edgeLabelSet.end(); it++) {
		std::cout << "(" << *(it->first).begin() << ", " 
			<< *(it->first).rbegin() << ") : {" << std::endl;
		for (std::vector<Label*>::iterator vit = it->second.begin(); vit != it->second.end(); vit++) {
			(*vit)->printReadable();
		}
		std::cout << "}" << std::endl;
	}
}

void SPGRepresentation::display() {
	displayAdjSet();
	displayLabelOfNode();
	displayLabelOfEdge();
}

void SPGRepresentation::printRepresentation() {
	if(rpr)  {
		std::cout << "FINAL REPRESENTATION:" << std::endl;
		rpr->printReadable(); 
	} else {
		std::cout << "NO REPRESENTATION AVAILABLE" << std::endl;
	}
}

void rprCaseTest(int n) {
	rapidjson::Document d;
	// 0: rule 0.2, 1.1
	// 1: rule 1.1
	// 2: rule 2.2
	// 3: rule 1.2
	// 4: rule 1.2, 0.1, 2.2 
	// 5: rule 2.1, 0.2, 1.1
	// 6: rule 2.1, 1.3, 0.1
	// 7: isomorphic to 6
	// 8: present in paper
	// 9: slight different with 8
	// 10: isomorphic to 8
	// 11
	std::string json[] = {
		"{\"bond\": {"
		"\"aid2\":[1,1],"
		"\"aid1\":[2,2] "
		"}}","{\"bond\": {"
		"\"aid2\":[1],"
		"\"aid1\":[2] "
		"}}","{\"bond\": {"
		"\"aid2\":[1,1,2],"
		"\"aid1\":[2,3,3] "
		"}}","{\"bond\": {"
		"\"aid2\":[1,2],"
		"\"aid1\":[2,3] "
		"}}","{\"bond\": {"
		"\"aid2\":[1,2,2,3,4],"
		"\"aid1\":[2,3,4,4,5] "
		"}}","{\"bond\": {"
		"\"aid2\":[1,1,1,2,3,4],"
		"\"aid1\":[2,4,5,3,4,5] "
		"}}","{\"bond\": {"
		"\"aid2\":[1,1,1,1,2,4],"
		"\"aid1\":[2,3,4,5,3,5] "
		"}}","{\"bond\": {"
		"\"aid2\":[2,2,2,2,1,4],"
		"\"aid1\":[1,3,4,5,3,5] "
		"}}","{\"bond\": {"
		"\"aid2\":[1,2,2,3,4,5,5,6,7,8,9,9,10,11,12,12,13,13,14],"
		"\"aid1\":[13,13,14,14,15,6,15,7,15,12,10,12,11,12,13,15,14,15,15]"
		"}}","{\"bond\": {"
		"\"aid2\":[1,1,1,1,2,2,2,2,2,2,4,4,7,7,7,9,10,13,14],"
		"\"aid1\":[2,3,4,7,4,6,7,12,13,15,5,6,8,9,11,10,11,14,15]"
		"}}","{\"bond\": {"
		"\"aid2\":[1,2,2,3,4,5,5,6,7,8,9,9,10,11,12,12,12,13,14],"
		"\"aid1\":[13,13,14,14,15,6,15,7,15,12,10,12,11,12,13,14,15,14,15]"
		"}}", "{\"bond\":{\"aid2\": [1,1,1,2,2,2,3,3,4,4,5,5,5,6,6,6,7,7,8,8,9,9,10,10,11,11,13,17],\"aid1\": [3,4,21,7,8,22,9,23,10,20,18,19,20,15,16,17,11,15,12,24,13,25,13,19,14,16,27,18]}}"};
	std::cout << json[n] << std::endl;
	d.Parse(json[n].c_str());

	SPGRepresentation rpr = SPGRepresentation(d);
	rpr.computation();
	rpr.printRepresentation();
	std::cout << "Hash Value: " << rpr.hashValue() << std::endl;

	/*
	d.Parse(json[9].c_str());
	SPGRepresentation frpr = SPGRepresentation(d);
	frpr.computation();

	if (frpr.hashValue() == rpr.hashValue()) {
		std::cout << "SAME REPRESENTATION" << std::endl;
	} else {
		std::cout << "WRONG CONSTRUCTION" << std::endl;
	}
	*/
	//rpr.unpack();
	//rpr.displayAdjSet();
}

/*
int main(int argc, char **argv) {
	//labelCaseTest();
	int a = 11;
	std::cout << "Graph Number: ";
	while (std::cin>>a) {
		rprCaseTest(a);
		std::cout << "Graph Number: ";
	}
}
*/
