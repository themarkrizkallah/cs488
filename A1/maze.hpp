// Summer 2020

#pragma once

#include <deque>

#include <glm/glm.hpp>

class Maze
{
public:
	Maze( size_t dim );
	~Maze();

	void reset();

	size_t getDim() const;

	int getValue( int x, int y ) const;

	void setValue( int x, int y, int h );

	void digMaze();
	void printMaze(); // for debugging

	glm::vec2 mazeStart() const;
	glm::vec2 mazeEnd() const;
	std::deque<glm::vec2> solveMaze();

private:
	size_t m_dim;
	int *m_values;
	bool m_initialized;

	void recDigMaze(int r, int c);
	int numNeighbors(int r, int c);
	int coordToIndex(int x, int y) const;
};
