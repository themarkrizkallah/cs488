// Spring 2020

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;

// Number of vertices needed for a 3D cube
const unsigned int c_NumVerts = 8;

typedef std::pair<glm::vec4, glm::vec4> Line;
typedef std::pair<glm::vec2, glm::vec2> Line2D;

// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};


class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

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

	void createShaderProgram();
	void enableVertexAttribIndices();
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void initLineData();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);

	void reset();

	glm::vec4 homogenize(const glm::vec4 &p) const;
	Line homogenizeLine(const Line &line) const;
	glm::vec2 pointToNDC(const glm::vec4 &point) const;

	void drawLineInViewport(const Line &line);

	void drawCube();
	void drawGnomon(bool model = true);
	void drawViewport();

	ShaderProgram m_shader;

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;

	// Model
	std::vector<glm::vec4> c_verts; // Cube vertices
	std::vector<glm::vec4> g_verts; // Gnomon vertices

	std::vector<glm::vec4> generateCubeVerts() const;
	std::vector<glm::vec4> generateGnomonVerts() const;

	std::vector<Line> generateCubeLines(std::vector<glm::vec4> &verts) const;
	std::vector<Line> generateGnomonLines(std::vector<glm::vec4> &verts) const;
	std::vector<Line> generateViewportLines() const;

	// Transformation matrices
	glm::mat4 M; // Model matrix
	glm::mat4 S; // Scale matrix
	glm::mat4 V; // View matrix
	glm::mat4 P; // Projection matrix

	enum Axis {
		X = 0,
		Y,
		Z
	};

	glm::mat4 translationMatrix(const glm::vec3 &p, bool translateTo = false) const;
	glm::mat4 generateModelMatrix() const;
	glm::mat4 generateScaleMatrix(float s = 1.0f) const;
	glm::mat4 generateViewMatrix() const;
	glm::mat4 generateProjectionMatrix() const;

	// Viewport related fields
	float viewX, viewY;
	float viewWidth, viewHeight;

	// View and projection related fields
	float fov;
	float near, far;

	bool clip(Line &line) const;

	enum Mode {
		RotateView,
		TranslateView,
		Perspective,
		RotateModel,
		TranslateModel,
		ScaleModel,
		Viewport,
		EndMode
	};
	Mode mode;

	// Methods to handle transformations
	void rotateView(double xPos, double yPos);
	void translateView(double xPos, double yPos);
	void perspective(double xPos, double yPos);
	void rotateModel(double xPos, double yPos);
	void translateModel(double xPos, double yPos);
	void scaleModel(double xPos, double yPos);
	void viewport(double xPos, double yPos);

	// Mouse related fields
	float xPrev, yPrev;
	bool leftPressed, middlePressed, rightPressed;
	bool getMousePos;
};
