// Summer 2020

#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <sys/types.h>
#include <unistd.h>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

// Constants
static const size_t DIM = 16;
static const float MAX_BLOCK_HEIGHT = 10.0f;

// Constants related to the colour picker
static const int UNSPECIFIED = -1;
static const int WALL = 0;
static const int FLOOR = 1;
static const int AVATAR = 2;
static const float BLACK[] = {0, 0, 0};
static const float WHITE[] = {1.0f, 1.0f, 1.0f};
static const float GREEN[] = {0.20f, 0.32f, 0.06f};

// Helper to change src's colour to dest's colour
static void changeColour(const float src[3], float dest[3]) 
{
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
}

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col(UNSPECIFIED), maze(DIM), mazeReady(false), blockHeight(1.0f), removeWall(false)
{
	
	changeColour(BLACK, colour); // Set colour picker to black
	changeColour(WHITE, cubeColour); // Set walls to white
	changeColour(GREEN, floorColour); // Set floor to green
	changeColour(BLACK, avatarColour); // Set avatar to black

	moveAvatar(0,0); // Set avatar's position to (0,0)
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Initialize random number generator
	int rseed=getpid();
	srandom(rseed);
	// Print random number seed in case we want to rerun with
	// same random numbers
	cout << "Random number seed = " << rseed << endl;
	
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	initGrid();
	initCube();
	initFloor();
	initAvatar();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, 2.*float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective( 
		glm::radians( 30.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

void A1::initCube()
{
	size_t sz = 8;

	// (x,y,z) for all 8 unique cube vertices
	vec3 cubeVerts[] = {
		vec3(0,0,0), // 0
		vec3(0,0,1), // 1
		vec3(0,1,0), // 2
		vec3(0,1,1), // 3
		vec3(1,0,0), // 4
		vec3(1,0,1), // 5
		vec3(1,1,0), // 6
		vec3(1,1,1)  // 7
	};

	// 3 indices per triangle, 2 triangles per face, 8 faces
	size_t numIndices = 3 * 2 * 8;

	// Indices for all 6 faces (2 triangles, counter-clockwise)
	unsigned int cubeIndices[] = {
		// Front face
		0, 4, 6,
		6, 2, 0,

		// Right face
		4, 5, 7,
		7, 6, 4,

		// Back face
		5, 1, 3,
		3, 7, 5,

		// Left face
		1, 0, 2,
		2, 3, 1,

		// Top face
		2, 6, 7,
		7, 3, 2,

		// Bottom face
		0, 4, 5,
		5, 1, 0
	};

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays(1, &m_cube_vao);
	glBindVertexArray(m_cube_vao);

	// Generate a vertex buffer object to hold the cube's vertex data
	glGenBuffers(1, &m_cube_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
	glBufferData(GL_ARRAY_BUFFER, sz * sizeof(vec3), cubeVerts, GL_STATIC_DRAW);

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Generate an index buffer object to map the vertices to the triangles
	glGenBuffers(1, &m_cube_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cube_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), cubeIndices, GL_STATIC_DRAW);

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	CHECK_GL_ERRORS;
}

void A1::initFloor()
{
	size_t sz = 4;

	// (x,y,z) for all 4 unique square vertices
	vec3 squareVerts[] = {
		vec3(0,0,0), // 0
		vec3(1,0,0), // 1
		vec3(1,0,1), // 2
		vec3(0,0,1), // 3
	};

	// 3 indices per triangle, 2 triangles per face, 8 faces
	size_t numIndices = 3 * 2;

	// Indices for the square (2 triangles, counter-clockwise)
	unsigned int squareIndices[] = {
		0, 1, 2,
		2, 3, 0
	};

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays(1, &m_floor_vao);
	glBindVertexArray(m_floor_vao);

	// Generate a vertex buffer object to hold the cube's vertex data
	glGenBuffers(1, &m_floor_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_floor_vbo);
	glBufferData(GL_ARRAY_BUFFER, sz * sizeof(vec3), squareVerts, GL_STATIC_DRAW);

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Generate an index buffer object to map the vertices to the triangles
	glGenBuffers(1, &m_floor_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_floor_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), squareIndices, GL_STATIC_DRAW);

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	CHECK_GL_ERRORS;
}

// Initially identical to initCube
void A1::initAvatar()
{
	size_t sz = 8;

	// (x,y,z) for all 8 unique cube vertices
	vec3 cubeVerts[] = {
		vec3(0,0,0), // 0
		vec3(0,0,1), // 1
		vec3(0,1,0), // 2
		vec3(0,1,1), // 3
		vec3(1,0,0), // 4
		vec3(1,0,1), // 5
		vec3(1,1,0), // 6
		vec3(1,1,1)  // 7
	};

	// 3 indices per triangle, 2 triangles per face, 8 faces
	size_t numIndices = 3 * 2 * 8;

	// Indices for all 6 faces (2 triangles, counter-clockwise)
	unsigned int cubeIndices[] = {
		// Front face
		0, 4, 6,
		6, 2, 0,

		// Right face
		4, 5, 7,
		7, 6, 4,

		// Back face
		5, 1, 3,
		3, 7, 5,

		// Left face
		1, 0, 2,
		2, 3, 1,

		// Top face
		2, 6, 7,
		7, 3, 2,

		// Bottom face
		0, 4, 5,
		5, 1, 0
	};

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays(1, &m_avatar_vao);
	glBindVertexArray(m_avatar_vao);

	// Generate a vertex buffer object to hold the cube's vertex data
	glGenBuffers(1, &m_avatar_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_avatar_vbo);
	glBufferData(GL_ARRAY_BUFFER, sz * sizeof(vec3), cubeVerts, GL_STATIC_DRAW);

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Generate an index buffer object to map the vertices to the triangles
	glGenBuffers(1, &m_avatar_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_avatar_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), cubeIndices, GL_STATIC_DRAW);

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	CHECK_GL_ERRORS;
}

void A1::moveAvatar(const int x, const int y)
{
	// Verify that the new location is valid
	if(x >= 0 && x <= DIM+1 && y >= 0 && y <= DIM+1){
		// Make sure you don't collide with a wall
		if(blockHeight > 0.0f && mazeReady && x > 0 && y > 0 && x <= DIM && y <= DIM 
			&& maze.getValue(x-1,y-1) == 1)
			return;

		avatarPos[0] = x;
		avatarPos[1] = y;
	} else {
		cout << "Out of bounds" << endl;
	}
}

void A1::digMaze() 
{
	maze.digMaze();
	maze.printMaze();
	mazeReady = true;

	moveAvatar(0,0);
}

void A1::reset()
{
	current_col = -1;
	changeColour(BLACK, colour);
	changeColour(WHITE, cubeColour);
	changeColour(GREEN, floorColour);
	changeColour(BLACK, avatarColour);

	blockHeight = 1.0f;

	moveAvatar(0, 0);

	maze.reset();
	mazeReady = false;
}

//----------------------------------------------------------------------------------------
/*c
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...

	// Handle colour radio buttons
	switch(current_col) 
	{
		case WALL:
			changeColour(colour, cubeColour);
			break;
		case FLOOR:
			changeColour(colour, floorColour);
			break;
		case AVATAR:
			changeColour(colour, avatarColour);
			break;
		default:
			break;
	}

	blockHeight = glm::clamp(blockHeight, 0.0f, MAX_BLOCK_HEIGHT);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if(ImGui::Button("Quit Application")) {
			cout << "Quitting..." << endl;
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if( ImGui::Button("Dig")) {
			cout << "Digging..." << endl;
			digMaze();
		} ImGui::SameLine();

		if(ImGui::Button("Reset")) {
			cout << "Resetting..." << endl;
			reset();
		}

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		ImGui::PushID(0);
		ImGui::ColorEdit3("##Colour", colour);
		
		if( ImGui::RadioButton("Wall", &current_col, WALL)){
			changeColour(colour, cubeColour);

		} ImGui::SameLine();

		if( ImGui::RadioButton("Floor", &current_col, FLOOR)){
			changeColour(colour, floorColour);

		} ImGui::SameLine();

		if( ImGui::RadioButton("Avatar", &current_col, AVATAR)) {
			changeColour(colour, avatarColour);
		}

		ImGui::PopID();

/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

		ImGui::End();

		if( showTestWindow ) {
			ImGui::ShowTestWindow( &showTestWindow );
		}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;
	W = glm::translate(W, vec3(-float(DIM)/2.0f, 0, -float(DIM)/2.0f));

	mat4 modelOrigin = W; // A copy of the original model transformation matrix
	mat4 aM = W; // Avatar's model transform, starts at (0,0)
	aM = glm::translate(aM, vec3(-1, 0, -1) );

	m_shader.enable();
		glEnable(GL_DEPTH_TEST);

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr(proj) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr(view) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr(W) );

		// Just draw the grid for now.
		glBindVertexArray(m_grid_vao);
		glUniform3f(col_uni, WHITE[0], WHITE[1], WHITE[2]);
		glDrawArrays(GL_LINES, 0, (3+DIM)*4);

		const int numIndices = 3 * 2 * 8;

		// Draw the walls and floor
		for (int i = 0; i < DIM+2; ++i){
			for (int j = 0; j < DIM+2; ++j){
				W = glm::translate(W, vec3(j-1, 0, i-1));
				glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(W));

				// Draw floor
				if (blockHeight == 0 || i == 0 || j == 0 || i > DIM || j > DIM 
					|| maze.getValue(i-1,j-1) == 0) {
					glBindVertexArray(m_floor_vao);
					glUniform3f(col_uni, floorColour[0], floorColour[1], floorColour[2]);
					glDrawElements(GL_TRIANGLES, 3*2, GL_UNSIGNED_INT, nullptr);
					
				// Draw wall
				} else if (mazeReady) {
					glBindVertexArray(m_cube_vao);
					glUniform3f(col_uni, cubeColour[0], cubeColour[1], cubeColour[2]);

					// Draw as many layers of blocks as determined by block height
					for(int m = 0; m < blockHeight; ++m){
						if (m > 0) W = glm::translate(W, vec3(0, 1, 0));
						glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
						glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
					}
				}

				// Reset model transformation matrix
				W = modelOrigin;
			}
		}

		// Draw the avatar
		glBindVertexArray(m_avatar_vao);
		aM = glm::translate(aM, vec3(avatarPos[1], 0, avatarPos[0]));

		glUniform3f(col_uni, avatarColour[0], avatarColour[1], avatarColour[2]);
		glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(aM));
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);

		// Highlight the active square.
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler. Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler. Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled = false;
	int x, y;

	// Fill in with event handling code...
	if(action == GLFW_PRESS) {
		// Quit
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		}

		// Dig Maze
		if (key == GLFW_KEY_D) {
			digMaze();
			eventHandled = true;
		}

		// Reset Maze
		if (key == GLFW_KEY_R) {
			reset();
			eventHandled = true;
		}

		// Increase block height
		if (key == GLFW_KEY_SPACE) {
			if(mazeReady) ++blockHeight;
			else cout << "Initialize maze first before changing height." << endl;
			eventHandled = true;
		}

		// Decrease block height
		if (key == GLFW_KEY_BACKSPACE) {
			if(mazeReady) --blockHeight;
			else cout << "Initialize maze first before changing height." << endl;
			eventHandled = true;
		}

		if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
			cout << "HULK SMASH!" << endl;
			removeWall = true;
			eventHandled = true;
		}

		// Move avatar up
		if (key == GLFW_KEY_UP) {
			x = avatarPos[0]-1;
			y = avatarPos[1];

			if(removeWall && x > 0 && x <= DIM) maze.setValue(x-1, y-1, 0);

			moveAvatar(x, y);
			eventHandled = true;
		}

		// Move avatar right
		if (key == GLFW_KEY_RIGHT) {
			x = avatarPos[0];
			y = avatarPos[1]+1;

			if(removeWall && y > 0 && y <= DIM) maze.setValue(x-1, y-1, 0);

			moveAvatar(x, y);
			eventHandled = true;
		}

		// Move avatar down
		if (key == GLFW_KEY_DOWN) {
			x = avatarPos[0]+1;
			y = avatarPos[1];

			if(removeWall && x > 0 && x <= DIM) maze.setValue(x-1, y-1, 0);

			moveAvatar(x, y);
			eventHandled = true;
		}

		// Move avatar left
		if (key == GLFW_KEY_LEFT) {
			x = avatarPos[0];
			y = avatarPos[1]-1;

			if(removeWall && y > 0 && y <= DIM) maze.setValue(x-1, y-1, 0);

			moveAvatar(x, y);
			eventHandled = true;
		}
	}
	
	// Check if any of the shift keys is released
	if(action == GLFW_RELEASE && (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)) {
		removeWall = false;
		eventHandled = true;
	}

	return eventHandled;
}
