#pragma once
#include <vector>

class Node {
public:
	double x, y;
	float velx, vely;
	float accx, accy;
	float k;
	bool mounted;
	std::vector<float> rest_lengths;
	std::vector<Node*> connections;

	Node(float x, float y);
	Node();
	void apply_forces();
	void update();
	void operator=(Node other);
};