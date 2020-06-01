// Summer 2020

#include <algorithm>
#include <cstdio>
#include <deque>
#include <iostream>

#include <glm/glm.hpp>

#include "maze.hpp"

using namespace std;
using namespace glm;

Maze::Maze( size_t D )
	: m_dim( D ), m_initialized(false)
{
	m_values = new int[ D * D ];

	reset();
}

void Maze::reset()
{
	size_t sz = m_dim*m_dim;
	std::fill( m_values, m_values + sz, 0 );
	m_initialized = false;
}

Maze::~Maze()
{
	delete [] m_values;
}

size_t Maze::getDim() const
{
	return m_dim;
}

int Maze::getValue( int x, int y ) const
{
	return m_values[ y * m_dim + x ];
}

void Maze::setValue( int x, int y, int h )
{
	m_values[ y * m_dim + x ] = h;
}

int perm[24][4] = {
	0,1,2,3,
	0,1,3,2,
	0,2,1,3,
	0,2,3,1,
	0,3,1,2,
	0,3,2,1,

	1,0,2,3,
	1,0,3,2,
	1,2,0,3,
	1,2,3,0,
	1,3,0,2,
	1,3,2,0,

	2,1,0,3,
	2,1,3,0,
	2,0,1,3,
	2,0,3,1,
	2,3,1,0,
	2,3,0,1,

	3,1,2,0,
	3,1,0,2,
	3,2,1,0,
	3,2,0,1,
	3,0,1,2,
	3,0,2,1,
};

int Maze::numNeighbors(int r, int c) {
	return getValue(r-1,c) + getValue(r+1,c) + getValue(r,c-1) + getValue(r,c+1);
}

// Print ASCII version of maze for debugging
void Maze::printMaze() {
	int i,j;
	for (i=0; i<m_dim; i++) {
		for (j=0; j<m_dim; j++) { 
			if ( getValue(i,j)==1 ) {
				printf("X");
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}
}


void Maze::recDigMaze(int r, int c) {
	int* p;
	p = perm[random()%24];
	for (int i=0; i<4; i++) {
		switch (p[i]) {
		case 0:
			if ( r > 1 && getValue(r-1,c) && numNeighbors(r-1,c)==3 ) {
				setValue(r-1,c,0);
				recDigMaze(r-1,c);
			}
			break;
		case 1:
			if ( r < m_dim-2 && getValue(r+1,c) && numNeighbors(r+1,c)==3 ) {
				setValue(r+1,c,0);
				recDigMaze(r+1,c);
			}
			break;
		case 2:
			if ( c > 1 && getValue(r,c-1) && numNeighbors(r,c-1)==3 ) {
				setValue(r,c-1,0);
				recDigMaze(r,c-1);
			}
			break;
		case 3:
			if ( c < m_dim-2 && getValue(r,c+1) && numNeighbors(r,c+1)==3 ) {
				setValue(r,c+1,0);
				recDigMaze(r,c+1);
			}
			break;
		}
	} 
}

void Maze::digMaze()
{
	int i,j;
	// set all values to 1
	for (i=0;i<m_dim;i++) {
		for (j=0;j<m_dim;j++) {
			setValue(i,j,1);
		}
	}

	// pick random start location
	int s=random()%(m_dim-2)+1;
	setValue(0,s,0);
	setValue(1,s,0);
	recDigMaze(1,s);
	do {
		s=rand()%(m_dim-2)+1;
		if ( getValue(m_dim-2,s)==0 ) {
			setValue(m_dim-1,s,0);
		}
	} while (getValue(m_dim-1,s)==1);
	setValue(m_dim/2-1,m_dim/2-1,0);
	setValue(m_dim/2-1,m_dim/2,0);
	setValue(m_dim/2,m_dim/2-1,0);
	setValue(m_dim/2+1,m_dim/2+1,0);
	setValue(m_dim/2+1,m_dim/2,0);
	setValue(m_dim/2,m_dim/2+1,0);
	setValue(m_dim/2-1,m_dim/2+1,0);
	setValue(m_dim/2,m_dim/2,1);
	setValue(m_dim/2+1,m_dim/2-1,0);

	m_initialized = true;
}

int Maze::coordToIndex(int x, int y) const
{
	return x * m_dim + y;
}


vec2 Maze::mazeStart() const 
{
	vec2 start(0,0);

	// Find maze start
	for(int c = 0; c < m_dim; ++c){
		if(getValue(0,c) == 0){
			start.y = c;
			break;
		}
	}

	return start;
}


vec2 Maze::mazeEnd() const
{
	vec2 end(0,0);

	// Find maze end
	for(int c = 0; c < m_dim; ++c){
		if(getValue(m_dim-1, c) == 0){
			end.x = m_dim - 1;
			end.y = c;
			break;
		}
	}

	return end;
}

deque<vec2> Maze::solveMaze()
{
	deque<vec2> sol;

	if(!m_initialized)
		return sol;

	size_t sz = m_dim * m_dim;
	vec2 start = mazeStart();
	vec2 end = mazeEnd();

	bool visited[sz];
	std::fill(visited, visited + sz, false);

	vec2 pred[sz];
	vec2 invalid(-1,-1);
	std::fill(pred, pred + sz, invalid);

	deque<vec2> queue; // BFS queue
	queue.push_back(start);
	
	int index = coordToIndex(start.x, start.y);
	visited[index] = true;

	int r, c;

	// Search maze and populate the predicate list
	while(!queue.empty()){
		vec2 point = queue.front();
		queue.pop_front();

		r = point.x;
		c = point.y;

		// Visit northern neighbour, if it exists
		index = coordToIndex(r-1, c);
		if(r != 0 && getValue(r-1, c) == 0 && !visited[index]){
			pred[index] = point;
			queue.push_back(vec2(r-1, c));
			visited[index] = true;
		}

		// Visit eastern neighbour, if it exists
		index = coordToIndex(r,c+1);
		if(c != m_dim-1 && getValue(r,c+1) == 0 && !visited[index]){
			pred[index] = point;
			queue.push_back(vec2(r, c+1));
			visited[index] = true;
		}

		// Visit southern neighbour, if it exists
		index = coordToIndex(r+1,c);
		if(r != m_dim-1 && getValue(r+1,c) == 0 && !visited[index]){
			pred[index] = point;
			queue.push_back(vec2(r+1, c));
			visited[index] = true;
		}

		// Visit western neighbour, if it exists
		index = coordToIndex(r,c-1);
		if(c != 0 && getValue(r,c-1) == 0 && !visited[index]){
			pred[index] = point;
			queue.push_back(vec2(r, c-1));
			visited[index] = true;
		}
	}

	// Construct the deque containing the start -> end path
	vec2 point = end;

	while(point.x != -1 && point.y != -1){
		sol.push_front(point);
		point = pred[coordToIndex(point.x, point.y)];
	}

	return sol;
}