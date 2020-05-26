// Summer 2020

#pragma once

#include <glm/glm.hpp>
#include <deque>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "maze.hpp"


// Cube struct
struct Cube {
	glm::vec3 *verts;
	unsigned int *indices;

	const size_t numVerts;
	const size_t numIndices;
	const float n;

	Cube(float n);
	~Cube();
};

// Sphere struct
struct Sphere {
	glm::vec3 *verts;
	unsigned int *indices;

	float r;     // radius
	float longs; // Number of longitude lines
	float lats;  // Number of latitude lines

	size_t numVerts;
	size_t numIndices;

	Sphere(float r, float longs = 360.0f, float lats = 180.0f);
	~Sphere();

	void computeVerts();
};

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

private:
	void initGrid();
	void initCube();
	void initFloor();
	void initAvatar();
	
	// Move avatar to (x,y) on the grid, if it's a valid position
	void moveAvatar(const int x, const int y);

	// Dig maze
	void digMaze();
	void solveMaze();

	// Reset maze, camera, avatar, floor, walls, and colours to defaults
	void resetColours();
	void resetMaze();
	void resetMazeSolver();
	void resetView();                      
	void reset();

	// Fields related to the maze object
	Maze maze;
	bool mazeReady;

	// Fields related to the BONUS maze solver
	bool mazeSolved;
	bool mazeSolveActive;
	bool slowDownSolver;
	std::deque<glm::vec2> mazeSol;
	std::deque<glm::vec2>::const_iterator mazeIt;

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni;     // Uniform location for Projection matrix.
	GLint V_uni;     // Uniform location for View matrix.
	GLint M_uni;     // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	// Fields related to the blocks (cube).
	GLuint m_cube_vao; 	 // Vertex Array Object
	GLuint m_cube_vbo; 	 // Vertex Buffer Object
	GLuint m_cube_ibo; 	 // Index Buffer Object
	float blockHeight; 	 // Height of the walls
	float cubeColour[3]; // Colour of the walls

	// Fields related to the floor
	GLuint m_floor_vao;   // Vertex Array Object
	GLuint m_floor_vbo;   // Vertex Buffer Object
	GLuint m_floor_ibo;   // Index Buffer Object
	float floorColour[3]; // Colour of the floor

	// Fields related to the avatar
	GLuint m_avatar_vao;   // Vertex Array Object
	GLuint m_avatar_vbo;   // Vertex Buffer Object
	GLuint m_avatar_ibo;   // Index Buffer Object
	Sphere avatar;         // Avatar sphere object
	glm::vec2 avatarPos;   // Avatar's grid position
	float avatarColour[3]; // Colour of the avatar

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	// Colour picker fields
	float colour[3];
	int current_col; // -1: Unspecified, 0: wall colour, 1: floor colour, 2: avatar colour

	// If true, avatar can move into wall tiles and permanently destroy them
	// - true while shift is pressed, false otherwise
	bool removeWall;

	// Fields related to camera, scale, and rotation
	float scale;
	float rotation;
	float rotationRate;

	// Fields related to mouse
	double xPosMouse; // Previous mouse x-position
	bool dragging;    // True if dragging mouse
	bool persist;     // Continue rotating grid until next mouse button click
};
