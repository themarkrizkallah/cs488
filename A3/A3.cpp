// Winter 2020

#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;
const float AMBIENT_INTENSITY = 0.25f;

// Constants affecting transformations
static const float TRANSLATE_FACTOR = 0.01;

static const string MODES[] = {
	"Position/Orientation (P)",
	"Joints (J)"
};

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.85, 0.85, 0.85, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();

	resetDefaults();
	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could Not Open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float pts[2 * CIRCLE_PTS];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(10.0f, 10.0f, 10.0f);
	m_light.rgbIntensity = vec3(0.6f); // light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;


		//-- Set LightSource uniform for the scene:
		{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
		}

		//-- Set background light ambient intensity
		{
			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(AMBIENT_INTENSITY);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
		}
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...

		// Main Menu Bar
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Application")) {
				if (ImGui::MenuItem("Reset Position", "I"))
					resetPosition();

				if (ImGui::MenuItem("Reset Orientation", "O"))
					resetOrientation();

				if (ImGui::MenuItem("Reset Joints", "S"))
					resetJoints();

				if (ImGui::MenuItem("Reset All", "A"))
					resetAll();
				
				ImGui::Separator();

				if (ImGui::MenuItem("Quit", "Q"))
					glfwSetWindowShouldClose(m_window, GL_TRUE);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit")){
				if (ImGui::MenuItem("Undo", "U"))
					undo();

				if (ImGui::MenuItem("Redo", "R"))
					redo();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options")){
				if (ImGui::Checkbox("Circle (C)", &m_drawTrackball)){}

				ImGui::Separator();

				if (ImGui::Checkbox("Z-buffer (Z)", &m_enableZbuffer)) {}
				if (ImGui::Checkbox("Backface culling (B)", &m_enableBackfaceCull)) {}
				if (ImGui::Checkbox("Frontface culling (F)", &m_enableFrontfaceCull)) {}
				
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		// Radio Buttons
		for (int mode = Mode::Position; mode != Mode::EndMode; ++mode){
			ImGui::PushID(mode);

			if(ImGui::RadioButton(MODES[mode].c_str(), (int*) &m_mode, mode)) {}

			ImGui::PopID();
		}

		if(ImGui::Button( "Reset Defaults"))
			resetDefaults();

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix,
		const glm::mat4 & modelMatrix
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * modelMatrix;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		//-- Set NormMatrix:
		location = shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;


		//-- Set Material values:
		location = shader.getUniformLocation("material.kd");
		vec3 kd = node.material.kd;
		glUniform3fv(location, 1, value_ptr(kd));
		CHECK_GL_ERRORS;
	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {
	if(m_enableZbuffer)
		glEnable( GL_DEPTH_TEST );

	if(m_enableBackfaceCull || m_enableFrontfaceCull)
		glEnable( GL_CULL_FACE );

	if(m_enableBackfaceCull && m_enableFrontfaceCull)
		glCullFace(GL_FRONT_AND_BACK);
	else if(m_enableBackfaceCull)
		glCullFace(GL_BACK);
	else if(m_enableFrontfaceCull)
		glCullFace(GL_FRONT);


	renderSceneGraph(*m_rootNode);

	if(m_enableZbuffer)
		glDisable( GL_DEPTH_TEST );

	if(m_enableBackfaceCull || m_enableFrontfaceCull)
		glDisable( GL_CULL_FACE );

	if(m_mode == Mode::Position && m_drawTrackball)
		renderArcCircle();
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.

	const mat4 rootMatrix = root.get_transform();
	renderSceneNode(root, m_T * rootMatrix * m_R * glm::inverse(rootMatrix));

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
// Recursively traverse and render the scene graph
void A3::renderSceneNode(const SceneNode &node, const glm::mat4 &accumMatrix)
{
	const mat4 modelMatrix = accumMatrix * node.get_transform();

	// Render node if it is a GeometryNode
	if(node.m_nodeType == NodeType::GeometryNode){
		const GeometryNode &geometryNode = static_cast<const GeometryNode &>(node);

		updateShaderUniforms(m_shader, geometryNode, m_view, modelMatrix);

		// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
		BatchInfo batchInfo = m_batchInfoMap[geometryNode.meshId];

		//-- Now render the mesh:
		m_shader.enable();
			glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
		m_shader.disable();
	}

	// Recursively run on subtree
	for (const SceneNode *child : node.children)
		renderSceneNode(*child, modelMatrix);
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Record trackball position
vec3 A3::getTrackballPos(float xPos, float yPos) {
	// Trackball center coordinates and diameter
	const float iCenterX = m_framebufferWidth * 0.5f;
	const float iCenterY = m_framebufferHeight * 0.5f;
	const float radius = std::min(m_framebufferWidth, m_framebufferHeight) / 4.0f;

	// New coordinates in trackball space
	xPos -= iCenterX;
	yPos -= iCenterY;

   /* Vector pointing from center of virtual trackball to
    * new mouse position
    */
	float newVecX = xPos / radius;
	float newVecY = yPos / radius;
    float newVecZ = (1.0 - newVecX * newVecX - newVecY * newVecY);

   /* If the Z component is less than 0, the mouse point
    * falls outside of the trackball which is interpreted
    * as rotation about the Z axis.
    */
	if (newVecZ < 0.0) {
		float len = sqrt(1.0 - newVecZ);
		newVecZ  = 0.0;
		newVecX /= len;
		newVecY /= len;
	} else {
		newVecZ = sqrt(newVecZ);
	}

	return vec3(newVecX, -newVecY, newVecZ);
}


//----------------------------------------------------------------------------------------
// Pan trackball
void A3::trackballPan(float xPos, float yPos) 
{
	const float xDelta = xPos - m_xPrev;
	const float yDelta = yPos - m_yPrev;

	m_T = glm::translate(m_T, vec3(xDelta * TRANSLATE_FACTOR, -yDelta * TRANSLATE_FACTOR, 0.0f));
}

//----------------------------------------------------------------------------------------
// Zoom trackball
void A3::trackballZoom(float xPos, float yPos) 
{
	const float yDelta = yPos - m_yPrev;

	m_T = glm::translate(m_T, vec3(0.0f, 0.0f, yDelta * TRANSLATE_FACTOR));
}


//----------------------------------------------------------------------------------------
// Rotate trackball
void A3::trackballRotate(const vec3 &v) {
	const vec3 n = glm::cross(v, m_trackball); // Axis of rotation
	const float projection = glm::dot(v, m_trackball);
	const float theta = glm::acos(projection);

	if(abs(theta) <= 1 && n != vec3(0.0f)){
		mat4 R = glm::rotate(mat4(), -theta, n);
		m_R = R * m_R;
	}
}

//----------------------------------------------------------------------------------------
// Reset the origin of the puppet to its initial position
void A3::resetPosition()
{
	m_T = mat4();
}

//----------------------------------------------------------------------------------------
// Reset the puppet to its initial orientation
void A3::resetOrientation()
{
	m_R = mat4();
}

//----------------------------------------------------------------------------------------
// (TODO) Reset all joint angles, and clear the undo/redo stack.
void A3::resetJoints(){}

//----------------------------------------------------------------------------------------
// (TODO) Reset the position, orientation, and joint angles of the puppet, and clear the undo/redo stack
void A3::resetAll()
{
	resetPosition();
	resetOrientation();
	resetJoints();
}

//----------------------------------------------------------------------------------------
// Reset everything to default settings
void A3::resetDefaults()
{
	m_drawTrackball = true;
	m_enableZbuffer = true;
	m_enableBackfaceCull = false;
	m_enableFrontfaceCull = false;

	resetAll();
}

//----------------------------------------------------------------------------------------
// (TODO) Undo last joint movement
void A3::undo(){}

//----------------------------------------------------------------------------------------
// (TODO) Redo last joint movement
void A3::redo(){}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
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
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	// Fill in with event handling code...
	vec3 trackball = getTrackballPos(xPos, yPos);

	switch(m_mode){
		case Mode::Position:
			if(m_leftPressed)   trackballPan(xPos, yPos);
			if(m_middlePressed) trackballZoom(xPos, yPos);
			if(m_rightPressed)  trackballRotate(trackball);
			break;
		case Mode::Joints:
			break;
		default:
			break;
	}
		
	m_xPrev = xPos;
	m_yPrev = yPos;
	m_trackball = trackball;

	return true;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if(actions == GLFW_PRESS){
		switch(button){
			case GLFW_MOUSE_BUTTON_LEFT:
				m_leftPressed = true;
				eventHandled = true;
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				m_middlePressed = true;
				eventHandled = true;
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				m_rightPressed = true;
				eventHandled = true;
				break;
		}
	}

	if(actions == GLFW_RELEASE){
		switch(button){
			case GLFW_MOUSE_BUTTON_LEFT:
				m_leftPressed = false;
				eventHandled = true;
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				m_middlePressed = false;
				eventHandled = true;
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				m_rightPressed = false;
				eventHandled = true;
				break;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	switch(action){
		case GLFW_PRESS:
			switch(key){
				case GLFW_KEY_M:
					show_gui = !show_gui;
					eventHandled = true;
					break;
				
				// Application Menu Shortcuts
				case GLFW_KEY_I:
					resetPosition();
					eventHandled = true;
					break;
				case GLFW_KEY_O:
					resetOrientation();
					eventHandled = true;
					break;
				case GLFW_KEY_S:
					resetJoints();
					eventHandled = true;
					break;
				case GLFW_KEY_A:
					resetAll();
					eventHandled = true;
					break;

				// Edit Menu Shortucts
				case GLFW_KEY_U:
					undo();
					eventHandled = true;
					break;
				case GLFW_KEY_R:
					redo();
					eventHandled = true;
					break;

				// Option Menu Shortucts
				case GLFW_KEY_C:
					m_drawTrackball = !m_drawTrackball;
					eventHandled = true;
					break;
				case GLFW_KEY_Z:
					m_enableZbuffer = !m_enableZbuffer;
					eventHandled = true;
					break;
				case GLFW_KEY_B:
					m_enableBackfaceCull = !m_enableBackfaceCull;
					eventHandled = true;
					break;
				case GLFW_KEY_F:
					m_enableFrontfaceCull = !m_enableFrontfaceCull;
					eventHandled = true;
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}
	// Fill in with event handling code...

	return eventHandled;
}
