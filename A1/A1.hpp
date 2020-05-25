// Summer 2020

#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "maze.hpp"

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
	
	void moveAvatar(const int x, const int y); // Moves avatar to (x,y) on the grid
	void digMaze(); // Digs maze
	void reset(); // Resets maze, camera, avatar, floor, walls, and colours to defaults

	// Fields related to the maze object
	Maze maze;
	bool mazeReady;

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
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

	// fields related to the avatar
	GLuint m_avatar_vao;   // Vertex Array Object
	GLuint m_avatar_vbo;   // Vertex Buffer Object
	GLuint m_avatar_ibo;   // Index Buffer Object
	int avatarPos[2];      // Avatar's grid position
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
};
