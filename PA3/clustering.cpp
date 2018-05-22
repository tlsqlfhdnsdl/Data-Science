#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <string>

std::ofstream outFile;

int n, eps, minPts;
std::string outputFormat;

typedef struct object {
	int id;
	int type; // -1 = outlier, 0 = not defined, other = num of cluster
	double x_coor;
	double y_coor;
}object;

std::vector<object> objects;

bool areNeighbor(int i, int j) {
	double dist = pow(objects[i].x_coor - objects[j].x_coor, 2.0) + pow(objects[i].y_coor - objects[j].y_coor, 2.0);
	if (dist <= pow(eps, 2.0)) {
		return true;
	}
	return false;
}

std::vector<int> getNeighbos(int pts) {
	std::vector<int> neighbors;
	for (int i = 0; i < objects.size(); i++) {
		if (i == pts) {
			continue;
		}
		if (areNeighbor(i, pts)) {
			neighbors.push_back(i);
		}
	}
	return neighbors;
}

void unionNeighbors(std::vector<int>* originalNeighbor, std::vector<int> newNeighbor) {
	for (std::vector<int>::iterator it = newNeighbor.begin(); it != newNeighbor.end(); ++it) {
		if (std::find(originalNeighbor->begin(), originalNeighbor->end(), *it) == originalNeighbor->end()) {
			originalNeighbor->push_back(*it);
		}
	}
}

void dbScan() {
	int clusterNumber = 0;
	for (int i = 0; i < objects.size(); i++) {
		if (objects[i].type != 0) {
			continue;
		}
		std::vector<int> neighbors = getNeighbos(i);
		if (neighbors.size() + 1 < minPts) { // add self
			objects[i].type = -1;
			continue;
		}

		outFile.open(outputFormat + std::to_string(clusterNumber) + ".txt");
		clusterNumber++;
		objects[i].type = clusterNumber;
		int it = -1;
		while (it + 1 < neighbors.size()) {
			it++;
			if (objects[neighbors[it]].type == -1) {
				objects[neighbors[it]].type = clusterNumber;
			}
			if (objects[neighbors[it]].type != 0) {
				continue;
			}
			outFile << neighbors[it] << std::endl;
			objects[neighbors[it]].type = clusterNumber;
			std::vector<int> newNeighbors = getNeighbos(neighbors[it]);
			if (newNeighbors.size() + 1 >= minPts) { // add self
				unionNeighbors(&neighbors, newNeighbors);
			}
		}
		outFile.close();
	}
}

int main(int argc, char *argv[]) {
	
	if (argc != 5) {
		std::cout << "Input form : (1)input file name (2) number of clusters (3) maximum radious (4) minimum number of points\n";
		return -1;
	}
	std::ifstream inFile(argv[1]);
	outputFormat = "input";
	outputFormat.push_back(argv[1][5]);
	outputFormat += "_cluster_";
	n = atoi(argv[2]);
	eps = atoi(argv[3]);
	minPts = atoi(argv[4]);

	int beforeId = -1;
	while (!inFile.eof()) {
		object temp;
		inFile >> temp.id >> temp.x_coor >> temp.y_coor;
		if (beforeId == temp.id) {
			break;
		}
		beforeId = temp.id;
		temp.type = 0;
		objects.push_back(temp);
	}

	inFile.close();
	dbScan();
	objects;
	return 0;
}

