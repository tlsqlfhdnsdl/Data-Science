#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <string>

int **user_item_arr;
double **pcc_arr;
bool **neighbor_arr;
double *average_arr;
int max_user_id = -1;
int max_item_id = -1;
double minPCC = 0.13;

std::vector<int> getItemsBothRate(int user_a, int user_b) {
	std::vector<int> result;
	for (int i=0; i<=max_item_id; i++) {
		if (user_item_arr[user_a][i] != 0 && user_item_arr[user_b][i] != 0) {
			result.push_back(i);
		}
	}
	return result;
}

double getAverage(int user) {
	int sum=0;
	int numOfItems=0;
	if (average_arr[user] == 0) {
		for (int i=0; i<=max_item_id; i++) {
			if (user_item_arr[user][i] != 0) {
				sum += user_item_arr[user][i];
				numOfItems++;
			}
		}
	  average_arr[user] = (double)sum / (double)numOfItems;
	}
	return average_arr[user];
}

double getPCC(int user_a, int user_b) {
	if (pcc_arr[user_a][user_b] == 0) {
		double child = 0, parent = 0;
		double parent_part1 = 0, parent_part2 = 0;
		double user_a_average = getAverage(user_a);
		double user_b_average = getAverage(user_b);
		std::vector<int> itemsBothRate = getItemsBothRate(user_a, user_b);
		if (itemsBothRate.size() < 2) {
			return 0;
		}
		for (std::vector<int>::iterator it = itemsBothRate.begin(); it != itemsBothRate.end(); ++it) {
			double temp1 = ((double)user_item_arr[user_a][*it] - user_a_average);
			double temp2 = ((double)user_item_arr[user_b][*it] - user_b_average);
			child += temp1 * temp2;
			parent_part1 += temp1 * temp1;
			parent_part2 += temp2 * temp2;
		}
		parent = sqrt(parent_part1 * parent_part2);
		if (parent != 0) { 
			pcc_arr[user_a][user_b] = child / parent; 
			pcc_arr[user_b][user_a] = child / parent;
		}
	}
	return pcc_arr[user_a][user_b];
}

void setAllNeighbor() {
	for (int i=0; i<=max_user_id - 1; i++) {
		for (int j=i+1; j<=max_user_id; j++) {
			double tempPCC = getPCC(i, j);
			if (tempPCC > minPCC) {
				neighbor_arr[i][j] = true;
				neighbor_arr[j][i] = true;
			}
		}
	}
}

std::vector<int> getNeighbor(int user) {
	std::vector<int> neighbor;
	for (int i=0; i<=max_user_id; i++) {
		if (neighbor_arr[user][i]) {
			neighbor.push_back(i);
		}
	}
	return neighbor;
}

int getAggregation(int user, int item) {
	std::vector<int> neighbor = getNeighbor(user);
	double user_average = getAverage(user);
	double result = 0;
	double k = 0;
	if (item > max_item_id) { return user_average; }
	for (std::vector<int>::iterator it = neighbor.begin(); it != neighbor.end(); ++it) {
		double neighbor_average = getAverage(*it);
		if (user_item_arr[*it][item] == 0) {
			continue;
		}
		double temp = getPCC(user, *it);
		k += temp;
		result += temp * (user_item_arr[*it][item] - neighbor_average);
	}
	//std::cout << user_average << " " << result / k << std::endl;
	if (k == 0) {
		return user_average;
	}
	result = (result / k) + user_average;
	if (result < 1) {
		return 1;
	}
	else if (result > 5) {
		return 5;
	}
	return (int) (result+0.5);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		std::cout << "Input form : (1) training data name, (2) : test data name\n";
		return 0;
	}

	//initialize array size
	std::ifstream inFile(argv[1]);
	while(!inFile.eof()) {
		int user_id, item_id, rank, timestamp;
		inFile >> user_id >> item_id >> rank >> timestamp;
		if (user_id > max_user_id) {
			max_user_id = user_id;
		}
		if (item_id > max_item_id) {
			max_item_id = item_id;
		}
	}
	inFile.close();
	user_item_arr = new int*[max_user_id + 1];
	neighbor_arr = new bool*[max_user_id + 1];
	pcc_arr = new double*[max_user_id + 1];
	average_arr = new double[max_user_id + 1]();
	for (int i=0; i<=max_user_id; i++) {
		user_item_arr[i] = new int[max_item_id + 1]();
		neighbor_arr[i] = new bool[max_user_id + 1]();
		pcc_arr[i] = new double[max_user_id + 1]();
	}

	//initialize array
	inFile.open(argv[1]);
	int before_user_id = -1, before_item_id = -1;
	while (!inFile.eof()) {
		int user_id, item_id, rank, timestamp;
		inFile >> user_id >> item_id >> rank >> timestamp;
		if (before_user_id == user_id && before_item_id == item_id) {
			break;
		}
		user_item_arr[user_id][item_id] = rank;
		before_user_id = user_id;
		before_item_id = item_id;
	}
	inFile.close();

	setAllNeighbor();
	inFile.open(argv[2]);
	std::string outputFormat = "u";
	outputFormat.push_back(argv[1][1]);
	outputFormat += ".base_prediction.txt";
	std::ofstream outFile(outputFormat);
	double sum = 0;
	int num = 0;
	while (!inFile.eof()) {
		int user_id, item_id, rank, timestamp;
		inFile >> user_id >> item_id >> rank >> timestamp;
		if (before_user_id == user_id && before_item_id == item_id) {
			break;
		}
		int temp = getAggregation(user_id, item_id);
		outFile << user_id << '\t' << item_id << '\t' << temp << std::endl;
		sum += (rank - temp) * (rank - temp);
		num++;
	}

	return 0;
}
