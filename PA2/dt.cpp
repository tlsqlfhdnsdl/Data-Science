#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <set>
#include <math.h>
#include <string.h>

using namespace std;



typedef struct Node {
	bool isLeafNode;
	string classOfNode;
	map<string, Node*> bridges;
}Node;

typedef struct Tuple {
	vector<string> attributes;
	string classOfTuble;
	Tuple(vector<string> att, string cla) {
		attributes = att;
		classOfTuble = cla;
	}
}Tuple;

vector<Tuple> tuples;
vector<set<string> > allSetOfAttributeValues;
set<string> setOfClass;
vector<string> attributeSet;
string className;
ofstream outFile;

void initBuffer(int* buffSize, char buffer[]) {
	memset(buffer, 0, *buffSize);
	*buffSize = 0;
}

int getAttributeIndexByString(string att) {
	for (int i = 0; i < attributeSet.size(); i++) {
		if (attributeSet[i] == att) {
			return i;
		}
	}
	return -1;
}

vector<int> getTupleByClass(string classOfTuple, vector<int> indexesForTuples) {
	vector<int> result;
	for (vector<int>::iterator it = indexesForTuples.begin(); it != indexesForTuples.end(); ++it) {
		if (tuples[*it].classOfTuble == classOfTuple) {
			result.push_back(*it);
		}
	}
	return result;
}

vector<int> getTupleByAttributeValue(int indexForAttribute, string attributeValue, vector<int> indexesForTuples) {
	vector<int> result;
	for (vector<int>::iterator it = indexesForTuples.begin(); it != indexesForTuples.end(); ++it) {
		if (tuples[*it].attributes[indexForAttribute] == attributeValue) {
			result.push_back(*it);
		}
	}
	return result;
}

double getEntropy(vector<int> indexesForTuples) {
	double result = 0;
	for (set<string>::iterator className = setOfClass.begin(); className != setOfClass.end(); ++className) {
		int i = 0;
		for (vector<int>::iterator it = indexesForTuples.begin(); it != indexesForTuples.end(); ++it) {
			if (tuples[*it].classOfTuble == *className) { i++; }
		}
		if (i != 0) {
			double p = double(i) / double(indexesForTuples.size());
			result -= p * log2(p);
		}
	}
	return result;
}

double getEntropyToClassify(int indexForAttribute, vector<int> indexesForTuples) {
	double result = 0;
	for (set<string>::iterator attributeValue = allSetOfAttributeValues[indexForAttribute].begin(); attributeValue != allSetOfAttributeValues[indexForAttribute].end(); ++attributeValue) {
		vector<int> tempTuples = getTupleByAttributeValue(indexForAttribute, *attributeValue, indexesForTuples);
		result += getEntropy(tempTuples) * tempTuples.size() / indexesForTuples.size();
	}
	return result;
}

string getMaxClassFromTuple(vector<int> indexesForTuples) {
	string result;
	int max = 0;
	for (set<string>::iterator className = setOfClass.begin(); className != setOfClass.end(); ++className) {
		int i = 0;
		for (vector<int>::iterator it = indexesForTuples.begin(); it != indexesForTuples.end(); ++it) {
			if (tuples[*it].classOfTuble == *className) { i++; }
		}
		if (max < i) { result = *className; }
	}
	return result;
}

void makeDecisionTree(Node* node, vector<int> indexesForTuples, vector<bool> isAttributeUsed) {

	// There are 3 contitions to stop partition.

	// First condition.
	// size of passed tuples is 0.
	if (indexesForTuples.size() == 0) {
		node->isLeafNode = true;
		node->classOfNode = *(setOfClass.begin());
		return;
	}

	// Second condition.
	// All passed tuples are same class.
	vector<int>::iterator it = indexesForTuples.begin();
	string tempClass = tuples[*it].classOfTuble;
	for (++it; it != indexesForTuples.end(); ++it) {
		if (tuples[*it].classOfTuble != tempClass) { break; }
	}
	if (it == indexesForTuples.end()) {
		node->classOfNode = tempClass;
		node->isLeafNode = true;
		return;
	}

	/* 
	 Third condition.
	 All attributes are used to do partition.
	 In this case, majority voting is used to classify leaf.
	 */
	int i;
	for (i = 0; i < isAttributeUsed.size(); i++) {
		if (isAttributeUsed[i] == false) {
			break;
		}
	}
	if (i == isAttributeUsed.size()) {
		node->isLeafNode = true;
		node->classOfNode = getMaxClassFromTuple(indexesForTuples);
		return;
	}

	//if this node is not leaf, do partition
	double minEntropy = log2(setOfClass.size());
	int indexForMinAttribute;
	for (i = 0; i < isAttributeUsed.size(); i++) {
		if (!isAttributeUsed[i]) {
			double tempEntropy = getEntropyToClassify(i, indexesForTuples);
			if (tempEntropy < minEntropy) {
				indexForMinAttribute = i;
				minEntropy = tempEntropy;
			}
		}
	}

	node->isLeafNode = false;
	node->classOfNode = attributeSet[indexForMinAttribute];
	isAttributeUsed[indexForMinAttribute] = true;
	for (set<string>::iterator minAttributeValue = allSetOfAttributeValues[indexForMinAttribute].begin(); minAttributeValue != allSetOfAttributeValues[indexForMinAttribute].end(); ++minAttributeValue) {
		Node *newNode = new Node;
		makeDecisionTree(newNode, getTupleByAttributeValue(indexForMinAttribute, *minAttributeValue, indexesForTuples), isAttributeUsed);
		node->bridges[*minAttributeValue] = newNode;
	}
	isAttributeUsed[indexForMinAttribute] = false;
}

bool setTestSetClassByDT(vector<Tuple> *testSet, Node root) {
	for (vector<Tuple>::iterator tuplePtr = testSet->begin(); tuplePtr != testSet->end(); ++tuplePtr) {
		Node *current = &root;
		while (!current->isLeafNode) {
			int attributeIndex = getAttributeIndexByString(current->classOfNode);
			if (attributeIndex == -1) {
				cout << "Error in setTestSetClassByDT()" << endl;
				return false;
			}
			map<string, Node*>::iterator it;
			if ((it = current->bridges.find(tuplePtr->attributes[attributeIndex])) == current->bridges.end()) {
				cout << "There is no attribute value " << tuplePtr->attributes[attributeIndex] << " in" << current->classOfNode << endl;
				return false;
			}
			current = it->second;
		}
		tuplePtr->classOfTuble = current->classOfNode;
	}
	return true;
}

void printTuples(vector<Tuple> resultTuples) {
	for (int i = 0; i < attributeSet.size(); i++) {
		outFile << attributeSet[i] << "\t";
	}
	outFile << className << endl;
	for (int i = 0; i < resultTuples.size(); i++) {
		for (int j = 0; j < resultTuples[i].attributes.size(); j++) {
			outFile << resultTuples[i].attributes[j] << "\t";
		}
		outFile << resultTuples[i].classOfTuble << endl;
	}
}

int main(int argc, char *argv[]) {
	string line;
	if (argc != 4) {
		cout << "Input form : (1)training file name (2)test file name (3) output file name\n";
		return -1;
	}
	char buffer[256] = {};
	int bufferSize = 0;
	int numAttribute = 0;
	
	ifstream inFile(argv[1]);
	getline(inFile, line);
	for (int i = 0; i < line.length(); i++) {
		if (line.at(i) == '\t') {
			attributeSet.push_back(buffer);
			numAttribute++;
			initBuffer(&bufferSize, buffer);
		}
		else {
			buffer[bufferSize] = line.at(i);
			bufferSize++;
		}
	}
	className = buffer;
	initBuffer(&bufferSize, buffer);
	allSetOfAttributeValues.resize(attributeSet.size());

	while (getline(inFile, line)) {
		vector<string> att;
		string cla;
		for (int i = 0; i < line.length(); i++) {
			if (line.at(i) == '\t') {
				allSetOfAttributeValues[att.size()].insert(buffer);
				att.push_back(buffer);
				initBuffer(&bufferSize, buffer);
			}
			else {
				buffer[bufferSize] = line.at(i);
				bufferSize++;
			}
		}
		cla = buffer;
		setOfClass.insert(buffer);
		initBuffer(&bufferSize, buffer);
		tuples.push_back(Tuple(att, cla));
	}

	vector<bool> isAttributeUsed;
	vector<int> indexesForTuples;
	for (int i = 0; i < attributeSet.size(); i++) {
		isAttributeUsed.push_back(false);
	}
	for (int i = 0; i < tuples.size(); i++) {
		indexesForTuples.push_back(i);
	}
	Node root;
	makeDecisionTree(&root, indexesForTuples, isAttributeUsed);

	inFile.close();
	inFile.open(argv[2]);

	getline(inFile, line);

	vector<string> testAttributeSet;
	for (int i = 0; i < line.length(); i++) {
		if (line.at(i) == '\t') {
			testAttributeSet.push_back(buffer);
			numAttribute++;
			initBuffer(&bufferSize, buffer);
		}
		else {
			buffer[bufferSize] = line.at(i);
			bufferSize++;
		}
	}
	testAttributeSet.push_back(buffer);
	initBuffer(&bufferSize, buffer);
	
	if (testAttributeSet != attributeSet) {
		cout << "Test attributes should be same with train!" << endl;
		return -1;
	}
	

	vector<Tuple> testTuples;

	while (getline(inFile, line)) {
		vector<string> att;
		for (int i = 0; i < line.length(); i++) {
			if (line.at(i) == '\t') {
				att.push_back(buffer);
				initBuffer(&bufferSize, buffer);
			}
			else {
				buffer[bufferSize] = line.at(i);
				bufferSize++;
			}
		}
		att.push_back(buffer);
		setOfClass.insert(buffer);
		initBuffer(&bufferSize, buffer);
		testTuples.push_back(Tuple(att, ""));
	}
	inFile.close();
	setTestSetClassByDT(&testTuples, root);
	outFile.open(argv[3]);
	printTuples(testTuples);
	outFile.close();

	return 0;
}