#include "Node.h"

Node::Node(float x, float y) :
	x(x), y(y)
{
	velx = 0;
	vely = 0;
	accx = 0;
	accy = -1;
	k = 0.1;
	mounted = 0;
}
Node::Node() {
	x = 0;
	y = 0;
	velx = 0;
	vely = 0;
	accx = 0;
	accy = -1;
	k = 0.1;
	mounted = 0;
}

void Node::apply_forces() {
	float dir, dist;
	for (int i = 0; i < connections.size(); i++) {
		float dy = y - connections[i]->y;
		float dx = x - connections[i]->x;
		dir = atan2(dy, dx);
		dist = sqrt(dy * dy + dx * dx);
		accx += k * (rest_lengths[i] - dist) * cos(dir);
		accy += k * (rest_lengths[i] - dist) * sin(dir);
	}
}
void Node::update() {
	velx += accx;
	vely += accy;
	x += velx;
	y += vely;

	accx = 0;
	accy = -1;

	velx *= 0.99;
	vely *= 0.99;

}

void Node::operator=(Node other) {
	x = other.x;
	y = other.y;
}