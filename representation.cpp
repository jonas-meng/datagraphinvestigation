#include "dependency/rapidjson/document.h"
#include "representation.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

#define DEBUG
#define DEBUG0_1
#define DEBUG0_2
#define DEBUG1_1
#define DEBUG1_2
#define DEBUG2_1
#define DEBUG2_2

void Label::printReadable() {
	short n;
	short precedent = -1;
	std::vector<bool>::iterator it = v.begin();
	while (it != v.end()) {
		n = 0;
		n = (n << 1) + *it; it++;
		n = (n << 1) + *it; it++;
		if ((precedent == 1 && n == 1) ||
				(precedent == 0 && n == 2)) {
			std::cout << ',';
		}
		switch(n) {
			case 0: std::cout << ')'; break;
			case 1: std::cout << '0'; break;
			case 2: std::cout << '('; break;
			default: break;
		}
		precedent = n;
	}
	std::cout << std::endl;
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

SPGRepresentation::SPGRepresentation(rapidjson::Document &d) {
	rpr = NULL;
	initialization(d);
}

void SPGRepresentation::initialization(rapidjson::Document &d) { 
	int a, b, length = d["bond"]["aid1"].Size();
	std::set<int> edge;
	for (int i = 0; i < length; i++) {

		// obtain two end points of one edge
		a = d["bond"]["aid1"][i].GetInt();
		b = d["bond"]["aid2"][i].GetInt();

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

	// initialize label set of node
	for (std::set<int>::iterator it = vertexSet.begin();
			it != vertexSet.end(); it++) {
		nodeLabelSet[*it].push_back(new Label(true));
	}
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

			Label *newEdgeLabel = new Label();
			newEdgeLabel->leftParenthesis();

			for (std::vector<Label*>::iterator
					lit = edgeLabelSet[*it].begin();
					lit != edgeLabelSet[*it].end();
					lit++) {
				newEdgeLabel->insert((*lit)->begin(),
						(*lit)->end());
				delete (*lit);
			}

			newEdgeLabel->rightParenthesis();

			edgeLabelSet[*it].clear();
			edgeLabelSet[*it].push_back(newEdgeLabel);
		}
	}

	return res;
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
		Label *newNodeLabel = new Label(), *t;
		std::set<int> edge;
		edge.insert(a);
		edge.insert(b);

		if (Label::comparison(*nodeLabelSet[b].begin(), 
					*nodeLabelSet[a].begin())) {
			c = a; a = b; b = c;
		}

#ifdef DEBUG1_1
	std::cout << a << ",(" << a << "," << b << "),"<< b << std::endl;
#endif
		newNodeLabel->leftParenthesis();
		t = *nodeLabelSet[a].begin();
		newNodeLabel->insert(t->begin(), t->end());
		delete t;
		nodeLabelSet[a].clear();
		t = *edgeLabelSet[edge].begin();
		newNodeLabel->insert(t->begin(), t->end());
		delete t;
		edgeLabelSet[edge].clear();
		t = *nodeLabelSet[b].begin();
		newNodeLabel->insert(t->begin(), t->end());
		delete t;
		nodeLabelSet[b].clear();
		newNodeLabel->rightParenthesis();

		vertexSet.erase(b);
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
		//std::cout << *it << "," << a << std::endl;
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
		t = *edgeLabelSet[edge].begin();
		newNodeLabel->insert(t->begin(), t->end());
		delete t;
		edgeLabelSet[edge].clear();

		// node
		t = *nodeLabelSet[*it].begin();
		newNodeLabel->insert(t->begin(), t->end());
		delete t;
		nodeLabelSet[*it].clear();

		newNodeLabel->rightParenthesis();

		// maintain consistent state
		nodeLabelSet[a].push_back(newNodeLabel);
		vertexSet.erase(*it);
		adjSet[*it].clear();
	}

	return res;
}


bool SPGRepresentation::rule1_3() {
	return false;
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
	int b = *adjSet[a].begin(), e1, e2;
	std::vector<Label*> *t1, *t2;
	t1 = new std::vector<Label*>();
	e1 = approachEndpoint(a, b, vertexWithDegree2, t1);

	b = *adjSet[a].rbegin();
	t2 = new std::vector<Label*>(t1->rbegin(), t1->rend());
	t2->erase(t2->end()-1);
	e2 = approachEndpoint(a, b, vertexWithDegree2, t2);	

	delete t1;
	t1 = new std::vector<Label*>(t2->rbegin(), t2->rend());

	if (!SPGRepresentation::comparisonOfTwoVector(t1, t2)) {
		delete t1; t1 = t2;
	} else {
		delete t2; t2 = NULL;
	}

	Label *newEdgeLabel = new Label();
	newEdgeLabel->leftParenthesis();
	for (std::vector<Label*>::iterator it = t1->begin();
			it != t1->end(); it++) {
		newEdgeLabel->insert((*it)->begin(), (*it)->end());
		delete (*it);
	}
	newEdgeLabel->rightParenthesis();

	delete t1;

	if (e1 == e2) {
		nodeLabelSet[e1].push_back(newEdgeLabel);
	} else {
		std::set<int> edge;
		edge.insert(e1);
		edge.insert(e2);
		edgeLabelSet[edge].push_back(newEdgeLabel);
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
	t->push_back(*edgeLabelSet[edge].begin());
	edgeLabelSet[edge].clear();

	return b;
	
}

bool SPGRepresentation::rule2_2(std::set<int> &vertices) {
#ifdef DEBUG 
	std::cout << "RULE 2.2 APPLIED" << std::endl;
#endif
	int a = *vertices.begin(), b, cnt = 0;
	std::vector<Label*> *t, *minv;

	// two different direction
	b = *adjSet[a].begin();
	t = loopLabelGenerator(a, b);
	minv = new std::vector<Label*>(t->begin(), t->end());
	Label *l1, *l2;
	while (cnt+1 < vertices.size()) {
		// rotate edge and node label
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
	delete t;

	b = *adjSet[a].rbegin();
	t = loopLabelGenerator(a, b);
	cnt = 0;
	while (cnt < vertices.size()) {
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
	delete t;

	Label *newNodeLabel = new Label();
	newNodeLabel->leftParenthesis();
	for (std::vector<Label*>::iterator it = minv->begin();
			it != minv->end(); it++) {
		newNodeLabel->insert((*it)->begin(), (*it)->end());
		delete *it;
	}
	newNodeLabel->rightParenthesis();

	delete minv;
	vertexSet.clear();
	vertexSet.insert(a);
	adjSet[a].clear();
	nodeLabelSet[a].clear();
	nodeLabelSet[a].push_back(newNodeLabel);
	return true;
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
		t->push_back(*edgeLabelSet[edge].begin());
		t->push_back(*nodeLabelSet[a].begin());

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

	t->push_back(*edgeLabelSet[edge].begin());
	t->push_back(*nodeLabelSet[a].begin());

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

void SPGRepresentation::iteration() {
	while (vertexSet.size() > 1 ||
			nodeLabelSet[*vertexSet.begin()].size() > 1) 
		if (rule0_1() || rule0_2() ||
				rule1_1() || rule1_2() || rule2_1()) {;}

	this->rpr = *nodeLabelSet[*vertexSet.begin()].begin();
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

void SPGRepresentation::display() {
	std::cout << "Vertex set:";
	for (std::set<int>::iterator it = vertexSet.begin();
			it != vertexSet.end(); it++) {
		std::cout << *it << ": ";

		for (std::set<int>::iterator vit = adjSet[*it].begin();
				vit != adjSet[*it].end(); vit++) {
			std::cout << *vit << ",";
		}
		std::cout << std::endl;
	}

	std::cout << "--------------------------" << std::endl;
	
	std::cout << "labels of node:" << std::endl;
	for (std::map<int, std::vector<Label*> >::iterator
			it = nodeLabelSet.begin(); it != nodeLabelSet.end();
			it++) {
		std::cout << it->first << ": {" << std::endl;
		for (std::vector<Label*>::iterator vit = it->second.begin(); vit != it->second.end(); vit++) {
			(*vit)->printReadable();
		}
		std::cout << "}" << std::endl;
	}

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

void testRule(SPGRepresentation &rpr, int ruleNO, std::string srNO) {
	std::cout << "BEFORE RULE " << srNO << std::endl;
	rpr.display();
	switch(ruleNO) {
		case 0: rpr.rule0_1(); break;
		case 1: rpr.rule0_2(); break;
		case 2: rpr.rule1_1(); break;
		case 3: rpr.rule1_2(); break;
		case 4: rpr.rule1_3(); break;
		case 5: rpr.rule2_1(); break;
		default: break;
	}
	std::cout << "AFTER RULE " << srNO<< std::endl;
	rpr.display();
}

void rprCaseTest() {
	rapidjson::Document d;
	// 0: rule 0.2
	// 1: rule 1.1
	// 2: rule 2.2
	// 3: rule 1.2
	// 4: rule 1.2, 0.1, 2.2 
	// 5: rule 2.1, 0.2, 1.1
	std::string json[] = {"{\"bond\": {"
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
		"\"aid2\":[1,2,2,3,4,5,5,6,7,8,9,9,10,11,12,12,13,13,14],"
		"\"aid1\":[13,13,14,14,15,6,15,7,15,12,10,12,11,12,13,15,14,15,15]"
		"}}"};
	d.Parse(json[7].c_str());

	SPGRepresentation frpr = SPGRepresentation(d);
	frpr.iteration();
	frpr.printRepresentation();
}

int main(int argc, char **argv) {
	//labelCaseTest();
	rprCaseTest();
}
