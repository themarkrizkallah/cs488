// Spring 2020

#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"

#include "RotateCommand.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <string>
#include <memory>
#include <deque>
#include <math.h>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

// Good default factors and thresholds
static const float TRANSLATE_FACTOR = 0.01f;
static const float ROTATION_FACTOR = 0.2f;
static const float ROTATION_THRESHHOLD = 0.001f;
static const float INTENSITY_FACTOR = 0.01f;

static const vec3 LIGHT_POSITION(10.0f, 10.0f, 10.0f);

static const float AMBIENT_INTENSITY = 0.05f;
static const float MIN_AMBIENT_INTENSITY = 0.0f;
static const float MAX_AMBIENT_INTENSITY = 0.9f;

static const vec3 RGB_INTENSITY(0.9f);
static const float MIN_RGB_INTENSITY = 0.1f;
static const float MAX_RGB_INTENSITY = 2.0f;

const chrono::seconds::rep FEEDBACK_SECONDS = 1.5f;

static const string MODES[] = {
	"Position/Orientation (P)",
	"Joints (J)",
	"Light (L)" // BONUS
};

const vec3 SELECTED_COLOUR(0, 0.3686f, 0.28627f);

//----------------------------------------------------------------------------------------
// Count nodes in scene graph rooted at root
static unsigned int sceneSize(SceneNode *root)
{
	unsigned int count = 0;

	deque<SceneNode *> queue;
	queue.push_back(root);

	while(!queue.empty()){
		auto node = queue.front();
		queue.pop_front();

		for (auto child : node->children)
			queue.push_back(child);

		++count;
	}

	return count;
}


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
{}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{}

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

	m_numNodes = sceneSize(m_rootNode.get());

	mapScene();

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
	m_light.position = LIGHT_POSITION;
	m_light.rgbIntensity = RGB_INTENSITY;
	m_ambientIntensity = AMBIENT_INTENSITY;
	m_light_T = mat4();
}

//----------------------------------------------------------------------------------------
// Translate light w.r.t world coordinates
void A3::translateLight(float xPos, float yPos) {
	const float xDelta = xPos - m_xPrev;
	const float yDelta = yPos - m_yPrev;

	const float xDist = xDelta * TRANSLATE_FACTOR;
	const float yDist = -yDelta * TRANSLATE_FACTOR;

	if(m_leftPressed)
		m_light_T = glm::translate(m_light_T, vec3(xDist, yDist, 0));
	
	if(m_rightPressed)
		m_light_T = glm::translate(m_light_T, vec3(0, 0, yDist));
}

//----------------------------------------------------------------------------------------
// Change light intesity
void A3::changeLightIntensity(float offset, bool ambient) {
	const float delta = offset > 0 ? INTENSITY_FACTOR : -INTENSITY_FACTOR;

	// Increment ambient intensity
	if(ambient){
		m_ambientIntensity = glm::clamp(
			m_ambientIntensity + delta, 
			MIN_AMBIENT_INTENSITY, 
			MAX_AMBIENT_INTENSITY
		);

	// Increment RGB intensity
	} else {
		m_light.rgbIntensity = glm::clamp(
			m_light.rgbIntensity + delta, 
			MIN_RGB_INTENSITY, 
			MAX_RGB_INTENSITY
		);
	}
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("picking");
		glUniform1i( location, m_currentlyPicking ? 1 : 0 );

		//-- Set LightSource uniform for the scene:
		if (!m_currentlyPicking){
			{
				// Translate light
				vec4 lightPoint(m_light.position.x, m_light.position.y, m_light.position.z, 1.0f);
				lightPoint = m_light_T * lightPoint;

				const vec3 lightPos(lightPoint.x, lightPoint.y, lightPoint.z);

				location = m_shader.getUniformLocation("light.position");
				glUniform3fv(location, 1, value_ptr(lightPos));
				location = m_shader.getUniformLocation("light.rgbIntensity");
				glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
				CHECK_GL_ERRORS;
			}

			//-- Set background light ambient intensity
			{
				location = m_shader.getUniformLocation("ambientIntensity");
				vec3 ambientIntensity(m_ambientIntensity);
				glUniform3fv(location, 1, value_ptr(ambientIntensity));
				CHECK_GL_ERRORS;
			}
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
	if(!show_gui)
		return;

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
			// Application Menu
			if (ImGui::BeginMenu("Application")) {
				if (ImGui::MenuItem("Reset Position", "I"))
					resetPosition();

				if (ImGui::MenuItem("Reset Orientation", "O"))
					resetOrientation();

				if (ImGui::MenuItem("Reset Joints", "S"))
					resetJoints();

				if (ImGui::MenuItem("Reset All", "A"))
					resetAll();

				if(ImGui::MenuItem( "Reset Defaults", "D"))
					resetDefaults();
				
				ImGui::Separator();

				if (ImGui::MenuItem("Quit", "Q"))
					glfwSetWindowShouldClose(m_window, GL_TRUE);

				ImGui::EndMenu();
			}

			// Edit Menu
			if (ImGui::BeginMenu("Edit")){
				if (ImGui::MenuItem("Undo", "U", false, !m_undoStack.empty()))
					undo(true);

				if (ImGui::MenuItem("Redo", "R", false, !m_redoStack.empty()))
					redo(true);

				ImGui::EndMenu();
			}

			// Options Menu
			if (ImGui::BeginMenu("Options")){
				if (ImGui::Checkbox("Circle (C)", &m_drawTrackball)){}

				ImGui::Separator();

				if (ImGui::Checkbox("Z-buffer (Z)", &m_enableZbuffer)) {}
				if (ImGui::Checkbox("Backface culling (B)", &m_enableBackfaceCull)) {}
				if (ImGui::Checkbox("Frontface culling (F)", &m_enableFrontfaceCull)) {}

				ImGui::Separator();

				if (ImGui::Checkbox("Natural Joint Control (N)", &m_yNaturalMode)){}
				
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		// Radio Buttons
		for (int mode = PositionMode; mode != EndMode; ++mode){
			ImGui::PushID(mode);
			if(ImGui::RadioButton(MODES[mode].c_str(), (int*) &m_mode, mode)) {}
			ImGui::PopID();
		}

		// BONUS
		if(m_mode == LightMode){
			ImGui::SliderFloat(
				"", 
				&m_ambientIntensity, 
				MIN_AMBIENT_INTENSITY,
				MAX_AMBIENT_INTENSITY,
				"Ambient Intensity = %.3f"
			);

			if(ImGui::Button("Reset Light"))
				initLightSources();
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

		// Show/Hide Undo/Redo related feedback
		if(m_feedbackTriggered){
			auto now = chrono::steady_clock::now();
			auto elapsed = chrono::duration_cast<chrono::seconds>(now - m_feedbackStart).count();
		
			if(elapsed <= FEEDBACK_SECONDS)
				ImGui::Text("Error: %s", m_feedback.c_str());
			else
				m_feedbackTriggered = false;
		}

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms
void A3::updateShaderUniforms(
		const GeometryNode & node,
		const glm::mat4 & viewMatrix,
		const glm::mat4 & modelMatrix
) {

	m_shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = m_shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * modelMatrix;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		if(m_currentlyPicking){
			const NodeID id = node.m_nodeId;

			// Construct false colurs from id
			float r = float(id&0xff) / 255.0f;
			float g = float((id>>8)&0xff) / 255.0f;
			float b = float((id>>16)&0xff) / 255.0f;

			location = m_shader.getUniformLocation("material.kd");
			glUniform3f(location, r, g, b);

			CHECK_GL_ERRORS;

		} else {
			//-- Set NormMatrix:
			location = m_shader.getUniformLocation("NormalMatrix");
			mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
			glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
			CHECK_GL_ERRORS;

			//-- Set Material values:
			location = m_shader.getUniformLocation("material.kd");
			vec3 kd = node.isSelected ? SELECTED_COLOUR : node.material.kd;
			glUniform3fv(location, 1, value_ptr(kd));

			location = m_shader.getUniformLocation("material.ks");
			vec3 ks = node.isSelected ? SELECTED_COLOUR : node.material.ks;
			glUniform3fv(location, 1, value_ptr(ks));
	
			CHECK_GL_ERRORS;
		}
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {
	if(m_enableZbuffer)
		glEnable(GL_DEPTH_TEST);

	if(m_enableBackfaceCull || m_enableFrontfaceCull)
		glEnable(GL_CULL_FACE);

	if(m_enableBackfaceCull && m_enableFrontfaceCull)
		glCullFace(GL_FRONT_AND_BACK);

	else if(m_enableBackfaceCull) 
		glCullFace(GL_BACK);

	else if(m_enableFrontfaceCull)
		glCullFace(GL_FRONT);

	renderSceneGraph(*m_rootNode);

	if(m_enableZbuffer)
		glDisable(GL_DEPTH_TEST);

	if(m_enableBackfaceCull || m_enableFrontfaceCull)
		glDisable(GL_CULL_FACE);

	if(m_mode == PositionMode && m_drawTrackball)
		renderArcCircle();
}

//----------------------------------------------------------------------------------------
// Select joint using NodeID
void A3::selectJoint(NodeID id)
{
	if(id >= m_nodeMap.size())
		return;
	
	SceneNode *clicked = m_nodeMap[id];
	SceneNode *parent = m_predMap[id];

	const bool isParentAJoint = parent && parent->m_nodeType == NodeType::JointNode;

	if(!isParentAJoint || (parent->isSelected && !clicked->isSelected))
		return;

	parent->isSelected = !parent->isSelected;

	// Select all direct GeometryNode descendants
	if(parent->isSelected){
		m_selectedJoints.insert(static_cast<JointNode *>(parent));
		for(auto child : parent->children){
			if(child->m_nodeType == NodeType::GeometryNode)
				child->isSelected = true;
		}
	}
	// Unselect all direct GeometryNode descendants
	else{
		m_selectedJoints.erase(static_cast<JointNode *>(parent));
		for(auto child : parent->children){
			if(child->m_nodeType == NodeType::GeometryNode)
				child->isSelected = false;
		}
	}
}

//----------------------------------------------------------------------------------------
// Unselect all joints
void A3::unselectAllJoints()
{
	m_selectedJoints.clear();

	for(auto node : m_nodeMap)
		node->isSelected = false;
}

//----------------------------------------------------------------------------------------
// Pick joint based on mouse click
void A3::pickJoint()
{
	double xPos, yPos;
	glfwGetCursorPos(m_window, &xPos, &yPos);

	m_currentlyPicking = true;

	uploadCommonSceneUniforms();
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.85, 0.85, 0.85, 1.0);

	// Draw graph in false colours
	draw();

	CHECK_GL_ERRORS;

	// Ugly -- FB coordinates might be different than Window coordinates
	// (e.g., on a retina display).  Must compensate.
	xPos *= double(m_framebufferWidth) / double(m_windowWidth);
	// WTF, don't know why I have to measure y relative to the bottom of
	// the window in this case.
	yPos = m_windowHeight - yPos;
	yPos *= double(m_framebufferHeight) / double(m_windowHeight);

	GLubyte buffer[ 4 ] = { 0, 0, 0, 0 };
	// A bit ugly -- don't want to swap the just-drawn false colours
	// to the screen, so read from the back buffer.
	glReadBuffer( GL_BACK );
	// Actually read the pixel at the mouse location.
	glReadPixels( int(xPos), int(yPos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
	CHECK_GL_ERRORS;

	// Reassemble the object ID.
	NodeID objectID = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);
	selectJoint(objectID);

	m_currentlyPicking = false;

	CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
// Map node IDs to their SceneNode object and direct predecessor
void A3::mapScene()
{
	if(!m_rootNode)
		return;

	m_nodeMap.resize(m_numNodes);
	m_predMap.resize(m_numNodes);

	deque<SceneNode *> queue;
	queue.push_back(m_rootNode.get());

	m_predMap[m_rootNode->m_nodeId] = nullptr;

	while(!queue.empty()){
		auto node = queue.front();
		queue.pop_front();

		m_nodeMap[node->m_nodeId] = node;

		for (auto child : node->children){
			m_predMap[child->m_nodeId] = node;
			queue.push_back(child);
		}
	}
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

	// Apply global position and orientation w.r.t the screen not model coordinates
	const mat4 rootMatrix = root.get_transform();
	renderSceneNode(root, m_position * rootMatrix * m_orientation * glm::inverse(rootMatrix));

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
// Recursively traverse and render the scene graph
void A3::renderSceneNode(const SceneNode &node, const glm::mat4 &accumMatrix)
{
	mat4 modelMatrix = accumMatrix * node.get_transform();

	switch(node.m_nodeType){
		// JointNode, generate rotation matrices
		case NodeType::JointNode: {
			const JointNode &jointNode = static_cast<const JointNode &>(node);
			mat4 xRot = glm::rotate(degreesToRadians(jointNode.m_joint_x.cur), vec3(1,0,0));
			mat4 yRot = glm::rotate(degreesToRadians(jointNode.m_joint_y.cur), vec3(0,1,0));
			modelMatrix *= yRot * xRot;
			break;
		}
		
		// GeometryNode, draw it!
		case NodeType::GeometryNode: {
			const GeometryNode &geometryNode = static_cast<const GeometryNode &>(node);
			updateShaderUniforms(geometryNode, m_view, modelMatrix);
			// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
			BatchInfo batchInfo = m_batchInfoMap[geometryNode.meshId];
			//-- Now render the mesh:
			m_shader.enable();
				glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
			m_shader.disable();
			break;
		}

		default:
			break;
	}

	// Render subtree and accumulate transformations
	for (const SceneNode *child : node.children)
		renderSceneNode(*child, modelMatrix);
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();

		GLint m_location = m_shader_arcCircle.getUniformLocation("M");
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		mat4 M;

		if(aspect > 1.0f)
			M = glm::scale( glm::mat4(), glm::vec3(0.5/aspect, 0.5, 1.0));
		else
			M = glm::scale( glm::mat4(), glm::vec3(0.5, 0.5*aspect, 1.0));

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

	m_position = glm::translate(m_position, vec3(xDelta * TRANSLATE_FACTOR, -yDelta * TRANSLATE_FACTOR, 0.0f));
}

//----------------------------------------------------------------------------------------
// Zoom trackball
void A3::trackballZoom(float xPos, float yPos) 
{
	const float yDelta = yPos - m_yPrev;

	m_position = glm::translate(m_position, vec3(0.0f, 0.0f, yDelta * TRANSLATE_FACTOR));
}


//----------------------------------------------------------------------------------------
// Rotate trackball
void A3::trackballRotate(const vec3 &v) {
	const vec3 n = glm::cross(v, m_trackball);
	const float projection = glm::dot(v, m_trackball);
	const float theta = glm::acos(projection);

	if(abs(theta) <= 1 && n != vec3(0.0f)){
		mat4 R = glm::rotate(mat4(), -theta, n);
		m_orientation = R * m_orientation;
	}
}

//----------------------------------------------------------------------------------------
// Generate RotationCommands for selected joints
void A3::generateRotations(float xPos, float yPos)
{
	const float xDelta = xPos - m_xPrev;
	const float yDelta = yPos - m_yPrev;

	const float xAxisAngle = yDelta * ROTATION_FACTOR;
	const float yAxisAngle = (m_yNaturalMode ? xDelta : yDelta) * ROTATION_FACTOR;

	const bool xSignificant = std::abs(xAxisAngle) >= ROTATION_THRESHHOLD;
	const bool ySignificant = std::abs(yAxisAngle) >= ROTATION_THRESHHOLD;

	// Check if we need to process rotations
	if(m_selectedJoints.empty() || !(xSignificant || ySignificant))
		return;

	// Rotate along x-axis, set dirty bit to true
	if(m_middlePressed && xSignificant){
		for(auto node : m_selectedJoints)
			m_commands->emplace_back(node, 'x', xAxisAngle);

		m_dirty = true;
	}
	
	// Rotate along y-axis, set dirty bit to true
	if(m_rightPressed && ySignificant){
		for(auto node : m_selectedJoints)
			m_commands->emplace_back(node, 'y', yAxisAngle);

		m_dirty = true;
	}

	if(m_dirty)
		executeCommands();
}

//----------------------------------------------------------------------------------------
// Execute all current RotationCommands
void A3::executeCommands()
{
	if(m_commands){
		for(auto &cmd : *m_commands)
			cmd.execute();
	}
}

//----------------------------------------------------------------------------------------
// Undo all current RotationCommands
void A3::undoCommands()
{
	if(m_commands){
   		for (auto cmd =  m_commands->rbegin(); cmd != m_commands->rend(); ++cmd)
			cmd->undo();
	}
}

//----------------------------------------------------------------------------------------
// Clear all RotationCommands and reset dirty bit
void A3::clearCommands()
{
	m_commands = std::make_shared<Commands>();
	m_dirty = false;
}

//----------------------------------------------------------------------------------------
// Save m_commands on m_undoStack and clear m_redoStack
void A3::saveState()
{
	m_undoStack.emplace(*m_commands);
	m_redoStack = std::stack<Commands>();
	clearCommands();
}

//----------------------------------------------------------------------------------------
// Undo last joint movement
bool A3::undo(bool buttonPress)
{
	if(m_undoStack.empty()) {
		if(buttonPress){
			m_feedback = "Cannot undo!";
			m_feedbackStart = chrono::steady_clock::now();
			m_feedbackTriggered = true;
		}

		return false;
	}
	else if(m_dirty)
		return false;

	// Get current state and undo
	m_commands = std::make_shared<Commands>(m_undoStack.top());
	undoCommands();

	// Push to redo stack and clear state
	m_redoStack.emplace(*m_commands);
	clearCommands();

	// Pop undo stack
	m_undoStack.pop();

	return true;
}

//----------------------------------------------------------------------------------------
// Redo last joint movement
bool A3::redo(bool buttonPress)
{
	if(m_redoStack.empty()) {
		if(buttonPress){
			m_feedback = "Cannot redo!";
			m_feedbackStart = chrono::steady_clock::now();
			m_feedbackTriggered = true;
		}
		return false;

	} else if(m_dirty) {
		return false;
	}

	// Get new state and redo
	m_commands = std::make_shared<Commands>(m_redoStack.top());
	executeCommands();

	// Push to undo stack and clear state
	m_undoStack.emplace(*m_commands);
	clearCommands();

	// Pop redo stack
	m_redoStack.pop();

	return true;
}

//----------------------------------------------------------------------------------------
// Reset the origin of the puppet to its initial position
void A3::resetPosition()
{
	m_position = mat4();
}

//----------------------------------------------------------------------------------------
// Reset the puppet to its initial orientation
void A3::resetOrientation()
{
	m_orientation = mat4();
}

//----------------------------------------------------------------------------------------
// Reset all joint angles, and clear the undo/redo stack.
void A3::resetJoints()
{
	bool notDone = true;

	// Clear undo stack
	while(notDone)
		notDone = undo();
	
	m_redoStack = std::stack<Commands>();
	clearCommands();

	m_feedbackTriggered = false;
}

//----------------------------------------------------------------------------------------
//  Reset the position, orientation, and joint angles of the puppet, and clear the undo/redo stack
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
	m_currentlyPicking = false;
	m_drawTrackball = true;
	m_enableZbuffer = true;
	m_enableBackfaceCull = false;
	m_enableFrontfaceCull = false;
	m_mode = PositionMode;
	m_yNaturalMode = false;

	// Reset Light
	initLightSources();

	unselectAllJoints();
	resetAll();
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{}

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
		// Trackball
		case PositionMode:
			if(m_leftPressed)
				trackballPan(xPos, yPos);

			if(m_middlePressed)
				trackballZoom(xPos, yPos);

			if(m_rightPressed)
				trackballRotate(trackball);
			break;
		
		// Joint Manipulation
		case JointsMode:
			if(!m_leftPressed)
				generateRotations(xPos, yPos);
			break;

		// Light
		case LightMode:
			translateLight(xPos, yPos);
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
	bool eventHandled(true);

	// Fill in with event handling code...
	if(actions == GLFW_PRESS){
		switch(button){
			case GLFW_MOUSE_BUTTON_LEFT:
				if(m_mode == JointsMode) pickJoint();
				m_leftPressed = true;
				break;

			case GLFW_MOUSE_BUTTON_MIDDLE:
				m_middlePressed = true;

				// Initiating new rotations, save state if applicable
				if(m_mode == JointsMode && m_dirty)
					saveState();
				break;

			case GLFW_MOUSE_BUTTON_RIGHT:
				m_rightPressed = true;

				// Initiating new rotations, save state if applicable
				if(m_mode == JointsMode && m_dirty)
					saveState();
				break;

			default:
				eventHandled = false;
				break;
		}
	}

	if(actions == GLFW_RELEASE){
		eventHandled = true;

		switch(button){
			case GLFW_MOUSE_BUTTON_LEFT:
				m_leftPressed = false;
				break;

			case GLFW_MOUSE_BUTTON_MIDDLE:
				m_middlePressed = false;

				// Finished rotating joints, save state
				if(m_mode == JointsMode && m_dirty)
					saveState();
				break;

			case GLFW_MOUSE_BUTTON_RIGHT:
				m_rightPressed = false;

				// Finished rotating joints, save state
				if(m_mode == JointsMode && m_dirty)
					saveState();
				break;

			default:
				eventHandled = false;
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
	switch(m_mode){
		case LightMode:
			if(xOffSet != 0.0f)
				changeLightIntensity(xOffSet, true);

			if(yOffSet != 0.0f)
				changeLightIntensity(yOffSet);
	
			eventHandled = true;
			break;
		
		default:
			break;
	}

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
	bool eventHandled = true;

	switch(action){
		case GLFW_PRESS:
			switch(key){
				case GLFW_KEY_M:
					show_gui = !show_gui;
					break;
				case GLFW_KEY_N:
					m_yNaturalMode = !m_yNaturalMode;
					break;
				case GLFW_KEY_P:
					m_mode = PositionMode;
					break;
				case GLFW_KEY_J:
					m_mode = JointsMode;
					break;
				case GLFW_KEY_L: // BONUS
					m_mode = LightMode;
					break;
				
				// Application Menu Shortcuts
				case GLFW_KEY_I:
					resetPosition();
					break;
				case GLFW_KEY_O:
					resetOrientation();
					break;
				case GLFW_KEY_S:
					resetJoints();
					break;
				case GLFW_KEY_A:
					resetAll();
					break;
				case GLFW_KEY_D:
					resetDefaults();
					break;
				case GLFW_KEY_Q:
					glfwSetWindowShouldClose(m_window, GL_TRUE);
					break;

				// Edit Menu Shortucts
				case GLFW_KEY_U:
					undo(true);
					break;
				case GLFW_KEY_R:
					redo(true);
					break;

				// Option Menu Shortucts
				case GLFW_KEY_C:
					m_drawTrackball = !m_drawTrackball;
					eventHandled = true;
					break;
				case GLFW_KEY_Z:
					m_enableZbuffer = !m_enableZbuffer;
					break;
				case GLFW_KEY_B:
					m_enableBackfaceCull = !m_enableBackfaceCull;
					break;
				case GLFW_KEY_F:
					m_enableFrontfaceCull = !m_enableFrontfaceCull;
					break;

				default:
					eventHandled = false;
					break;
			}
			break;

		default:
			eventHandled = false;
			break;
	}
	// Fill in with event handling code...

	return eventHandled;
}
