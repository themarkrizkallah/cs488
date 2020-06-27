// Spring 2020

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <list>

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
	void initViewMatrix();
	void initLightSources();

	void initPerspectiveMatrix();
	void uploadCommonSceneUniforms();
	void renderSceneGraph(const SceneNode &node);
	void renderArcCircle();

	glm::mat4 m_perpsective;
	glm::mat4 m_view;

	LightSource m_light;

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

	// Recursively render the scene graph
	void renderSceneNode(const SceneNode &node, const glm::mat4 &accumMatrix);

	// Transformations to entire scene graph
	glm::mat4 m_T; // Position/Translation matrix
	glm::mat4 m_R; // Orientatiom, matrix

	void undo();
	void redo();

	// Mouse related fields
	float m_xPrev, m_yPrev;
	bool m_leftPressed, m_middlePressed, m_rightPressed;

	// Trackball controls and fields
	glm::vec3 m_trackball;

	glm::vec3 getTrackballPos(float xPos, float yPos);
	void trackballPan(float xPos, float yPos);
	void trackballZoom(float xPos, float yPos);
	void trackballRotate(const glm::vec3 &v);

	// UI Modes and Options
	enum Mode {
		Position,
		Joints,
		EndMode
	};
	Mode m_mode;

	bool m_drawTrackball;
	bool m_enableZbuffer, m_enableBackfaceCull, m_enableFrontfaceCull;

	// Reset methods for puppet
	void resetPosition();
	void resetOrientation();
	void resetJoints();
	void resetAll();

	// Resets EVERYTHING
	void resetDefaults();
};
