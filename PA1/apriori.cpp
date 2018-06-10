#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <map>
#include <cmath>
#include <cstring>

using namespace std;


int minimumSupport;

ofstream outFile;
vector<set<int> > transactionSet;
vector<map<set<int>, int> > frequentItemSet;

double Rounding(double x, int digit)
{
	return (floor((x)* pow(float(10), digit) + 0.5f) / pow(float(10), digit));
}

bool isLeftInRight(set<int> l, set<int> r) {
	for (set<int>::iterator left = l.begin(); left != l.end(); ++left) {
		bool isIncluded = false;
		for (set<int>::iterator right = r.begin(); right != r.end(); ++right) {
			if (*left == *right) {
				isIncluded = true;
				break;
			}
		}
		if (isIncluded == false) {
			return false;
		}
	}
	return true;
}

map<set<int>, int> getFrequentItem(vector<set<int> > candidateItem) {
	map<set<int>, int> frequentResult;
	map<set<int>, int> mapForCount;
	for (int i = 0; i < candidateItem.size(); i++) {
		mapForCount.insert(pair<set<int>, int>(candidateItem[i], 0));
		for (int j = 0; j < transactionSet.size(); j++) {
			if (isLeftInRight(candidateItem[i], transactionSet[j])) {
				mapForCount[candidateItem[i]] ++;
			}
		}
	}

	for (map<set<int>, int>::iterator i = mapForCount.begin(); i != mapForCount.end(); ++i) {
		if ((*i).second >= minimumSupport) {
			frequentResult.insert((*i));
		}
	}

	return frequentResult;
}

void initFrequentItem() {
	vector<set<int> > tempFrequent;
	for (int i = 0; i < transactionSet.size(); i++) {
		for (set<int>::iterator it = transactionSet[i].begin(); it != transactionSet[i].end(); ++it) {
			set<int> temp;
			temp.insert(*it);
			if (find(tempFrequent.begin(), tempFrequent.end(), temp) == tempFrequent.end()) {
				tempFrequent.push_back(temp);
			}
		}
	}
	frequentItemSet.push_back(getFrequentItem(tempFrequent));
}

vector<set<int> > selfjoinAndPruning(map<set<int>, int> frequentItem) {
	set<set<int> > selfjoinResult;
	for (map<set<int>, int>::iterator it = frequentItem.begin(); next(it) != frequentItem.end(); ++it) {
		for (map<set<int>, int>::iterator itt = next(it); itt != frequentItem.end(); itt++) {
			vector<int> difference(itt->first.size());
			vector<int>::iterator ittt;
			ittt = set_difference(itt->first.begin(), itt->first.end(), it->first.begin(), it->first.end(), difference.begin());
			difference.resize(ittt - difference.begin());
			for (int k = 0; k < difference.size(); k++) {
				set<int> tempSet = it->first;
				tempSet.insert(difference[k]);
				selfjoinResult.insert(tempSet);
			}
		}
	}

	vector<set<int> > pruningResult;
	for (set<set<int> >::iterator it = selfjoinResult.begin(); it != selfjoinResult.end(); ++it) {
		set<int> temp = (*it);
		bool isAllSubsetExist = true;
		for (set<int>::iterator itt = (*it).begin(); itt != (*it).end(); ++itt) {
			temp.erase(*itt);
			bool isExist = false;
			if (frequentItem.find(temp) != frequentItem.end()) {
				isExist = true;
			}
			if (isExist == false) {
				isAllSubsetExist = false;
				break;
			}
			temp.insert(*itt);
		}
		if (isAllSubsetExist == true) {
			pruningResult.push_back((*it));
		}
	}
	return pruningResult;
}

void printRule(set<int> lhs, set<int> rhs, int supportNum, int transactionNum, int lhsCount, int rhsCount) {
	string s = "{";
	for (set<int>::iterator it = lhs.begin(); it != lhs.end(); ++it) {
		if (it != lhs.begin()) {
			s += ",";
		}
		s += to_string(*it);
	}
	s += ("}\t{");
	for (set<int>::iterator it = rhs.begin(); it != rhs.end(); ++it) {
		if (it != rhs.begin()) {
			s += ",";
		}
		s += to_string(*it);
	}
	s += "}\t";
	outFile << s << Rounding(double(supportNum) / double(transactionNum) * 100, 2) << "\t" << Rounding(double(rhsCount) / double(lhsCount) * 100, 2) << endl;
}

void findRulesByDFS(vector<pair<int, bool> > frequentItemsWithFlag, int n, int supportNum) {
	if (n == frequentItemsWithFlag.size()) {
		set<int> lhs;
		set<int> rhs;
		for (int i = 0; i < frequentItemsWithFlag.size(); i++) {
			if (frequentItemsWithFlag[i].second == true) {
				lhs.insert(frequentItemsWithFlag[i].first);
			}
			else {
				rhs.insert(frequentItemsWithFlag[i].first);
			}
		}
		if (lhs.size() == 0 || rhs.size() == 0) {
			return;

		}
		int lhsCount = 0, rhsCount = 0;
		for (int j = 0; j < transactionSet.size(); j++) {
			if (isLeftInRight(lhs, transactionSet[j])) {
				lhsCount++;
				if (isLeftInRight(rhs, transactionSet[j])) {
					rhsCount++;
				}
			}
		}
		printRule(lhs, rhs, supportNum, transactionSet.size(), lhsCount, rhsCount);
		return;
	}

	findRulesByDFS(frequentItemsWithFlag, n + 1, supportNum);
	frequentItemsWithFlag[n].second = true;
	findRulesByDFS(frequentItemsWithFlag, n + 1, supportNum);
}

void findAllRules() {
	for (int i = 1; i < frequentItemSet.size(); i++) {
		for (map<set<int>, int>::iterator it = frequentItemSet[i].begin(); it != frequentItemSet[i].end(); ++it) {
			vector<pair<int, bool> > frequentItemsWithFlag;
			for (set<int>::iterator itt = it->first.begin(); itt != it->first.end(); ++itt) {
				frequentItemsWithFlag.push_back(pair<int, bool>(*itt, false));
			}
			findRulesByDFS(frequentItemsWithFlag, 0, it->second);
		}
	}
}

int main(int argc, char *argv[]) {
	string line;
	if (argc != 4) {
		cout << "Input form : (1)Minimun support (2)input file name (3) output file name\n";
		return -1;
	}

	int minimumSupportRatio = 0;
	for (int i = 0; i < strlen(argv[1]); i++) {
		minimumSupportRatio = minimumSupportRatio * 10 + (argv[1][i] - '0');
	}
	if (minimumSupportRatio > 100 || minimumSupportRatio < 0) {
		printf("%d", minimumSupportRatio);
		cout << "(1)Minimum support should be less than 100 or more than 0\n";
		return -3;
	}

	ifstream inFile(argv[2]);


	while (getline(inFile, line)) {
		int num = 0;
		set<int> transaction;
		for (int i = 0; i < line.length(); i++) {
			if (line.at(i) == '\t') {
				transaction.insert(num);
				num = 0;
			}
			else {
				num = num * 10 + (line.at(i) - '0');
			}
		}
		transaction.insert(num);
		transactionSet.push_back(transaction);
	}

	inFile.close();
	minimumSupport = transactionSet.size() * minimumSupportRatio / 100;
	initFrequentItem();
	for (int i = 0; frequentItemSet[i].size() != 0; i++) {
		vector<set<int> > candidate = selfjoinAndPruning(frequentItemSet[i]);
		frequentItemSet.push_back(getFrequentItem(candidate));
	}
	outFile.open(argv[3]);
	findAllRules();
	outFile.close();
	return 0;
}
