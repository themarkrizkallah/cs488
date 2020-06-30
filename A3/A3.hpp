// Spring 2020

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"
#include "JointNode.hpp"
#include "GeometryNode.hpp"
#include "RotateCommand.hpp"

#include <glm/glm.hpp>

#include <chrono>
#include <memory>
#include <stack>
#include <list>
#include <set>


struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};


class A3 : public CS488Window {
public:
	A3(const std::string & luaSceneFile);
	virtual ~A3();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	//-- One time initialization methods:
	void processLuaSceneFile(const std::string & filename);
	void createShaderProgram();
	void enableVertexShaderInputSlots();
	void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
	void mapVboDataToVertexShaderInputLocations();
	void updateShaderUniforms(const GeometryNode &node, const glm::mat4 & viewMatrix, const glm::mat4 & modelMatrix);
	void initViewMatrix();
	void initLightSources();

	void initPerspectiveMatrix();
	void uploadCommonSceneUniforms();
	void renderSceneGraph(const SceneNode &node);
	void renderSceneNode(const SceneNode &node, const glm::mat4 &accumMatrix);
	void renderArcCircle();

	glm::mat4 m_perpsective;
	glm::mat4 m_view;

	/* Light */
	LightSource m_light;
	glm::mat4 m_light_T;
	float m_ambientIntensity;
	
	void translateLight(float xPos, float yPos);                   // BONUS
	void changeLightIntensity(float offset, bool ambient = false); // BONUS

	//-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	ShaderProgram m_shader;

	//-- GL resources for trackball circle geometry:
	GLuint m_vbo_arcCircle;
	GLuint m_vao_arcCircle;
	GLint m_arc_positionAttribLocation;
	ShaderProgram m_shader_arcCircle;

	// BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
	// object. Each BatchInfo object contains an index offset and the number of indices
	// required to render the mesh with identifier MeshId.
	BatchInfoMap m_batchInfoMap;

	std::string m_luaSceneFile;

	std::shared_ptr<SceneNode> m_rootNode;
	unsigned int m_numNodes;

	/* NodeID to SceneNode* and parent mappings */
	typedef unsigned int NodeID;

	std::vector<SceneNode *> m_nodeMap; // m_nodeId -> SceneNode*
	std::vector<SceneNode *> m_predMap; // m_nodeId -> Parent SceneNode*
	void mapScene();

	/* Scene graph Transformations */
	glm::mat4 m_position; // Position/Translation matrix
	glm::mat4 m_orientation; // Orientation, matrix

	// Mouse related fields
	float m_xPrev, m_yPrev;
	bool m_leftPressed, m_middlePressed, m_rightPressed;

	/* Trackball and Controls */
	glm::vec3 m_trackball;
	glm::vec3 getTrackballPos(float xPos, float yPos);

	void trackballPan(float xPos, float yPos);
	void trackballZoom(float xPos, float yPos);
	void trackballRotate(const glm::vec3 &v);

	/* UI Modes and Options */
	enum Mode {
		PositionMode,
		JointsMode,
		LightMode, // BONUS
		EndMode
	};
	Mode m_mode;

 	// Rotate joint (wrt y-axis) using relative x-movement
	bool m_yNaturalMode;

	/* Picking and joint selection */
	bool m_currentlyPicking;
	std::set<JointNode *> m_selectedJoints;

	void selectJoint(NodeID id);
	void unselectAllJoints();
	void pickJoint();

	/* State and RotationCommands */
	typedef std::vector<RotateCommand> Commands;

	std::shared_ptr<Commands> m_commands;
	bool m_dirty;

	void generateRotations(float xPos, float yPos);

	void executeCommands();
	void undoCommands();
	void clearCommands();

	std::stack<Commands> m_undoStack;
	std::stack<Commands> m_redoStack;

	void saveState();
	bool undo(bool buttonPress = false);
	bool redo(bool buttonPress = false);

	/* Undo/Redo UI feedback */
	std::chrono::steady_clock::time_point m_feedbackStart;
	std::string m_feedback;
	bool m_feedbackTriggered;

	// Options Menu Items
	bool m_drawTrackball;
	bool m_enableZbuffer, m_enableBackfaceCull, m_enableFrontfaceCull;

	// Reset methods
	void resetPosition();
	void resetOrientation();
	void resetJoints();
	void resetAll();

	// Resets everything to default
	void resetDefaults();
};
