// Spring 2020

#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <string>
#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;


// Constants
static const vec3 BLACK = vec3(0.0f, 0.0f, 0.0f);
static const vec3 RED = vec3(255.0f, 0.0f, 0.0f);
static const vec3 GREEN = vec3(0.0f, 255.0f, 0.0f);
static const vec3 BLUE = vec3(0.0f, 0.0f, 255.0f);
static const vec3 CUBE_COLOUR = vec3(1.0f, 0.7f, 0.8f);

static const float ZERO_BOUND = 0.00001f;
static const float MAX_PLANE = 1000.0f;
static const float MIN_FOV = 5.0f;
static const float MAX_FOV = 160.0f;
static const float MIN_SCALE = 0.01F;
static const float MAX_SCALE = 10.0f;

// Factors
static const float FOV_FACTOR = 50.0f;
static const float NEAR_FAR_FACTOR = 25.0f;
static const float TRANSLATE_FACTOR = 100.0f;
static const float SCALE_FACTOR = 1.05f;

// Default values
static const float FOV = 30.0f;
static const float NEAR = 0.1f;
static const float FAR = 30.0f;

static const string MODES[] = {
	"Rotate View",
	"Translate View",
	"Perspective",
	"Rotate Model",
	"Translate Model",
	"Scale Model",
	"Viewport"
};

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f)),
	  c_verts(generateCubeVerts()), 
	  g_verts(generateGnomonVerts())
{

}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
// Resets everything
void A2::reset()
{
	viewXL = 0.05f * m_framebufferWidth;
	viewXR = 0.95f * m_framebufferWidth;
	viewYB = 0.05f * m_framebufferHeight;
	viewYT = 0.95f * m_framebufferHeight;

	aspect = m_framebufferWidth/m_framebufferWidth;
	near = NEAR;
	far = FAR;
	fov = FOV;

	M = generateModelMatrix();
	S = generateScaleMatrix();
	V = generateViewMatrix();
	P = generateProjectionMatrix();

	mode = RotateModel;
}

//----------------------------------------------------------------------------------------
// Homogenizes a point or vector
glm::vec4 A2::homogenize(const glm::vec4 &v) const 
{
	// Check if v.w is above our zero threshhold
	if (v.w < ZERO_BOUND){
		vec4 rounded(v);
		rounded.w = 1.0f;

		return rounded;
	}

	vec4 homogenized(
		v.x / v.w,
		v.y / v.w,
		v.z / v.w,
		1.0f
	);

	return homogenized;
}

//----------------------------------------------------------------------------------------
// Maps a point to NDC
vec2 A2::pointToNDC(const vec4 &point) const
{
	// Window boundaries
	const float x_wl = -1.0f, x_wr = 1.0f;
	const float y_wb = -1.0f, y_wt = 1.0f;

	float L_w = x_wr - x_wl;
	float H_w = y_wt - y_wb;

	// Viewport boundaries
	float x_vl = 2.0f * (viewXL + 0.5f) / m_framebufferWidth - 1.0f;
	float x_vr = 2.0f * (viewXR + 0.5f) / m_framebufferWidth - 1.0f;
	float y_vb = 2.0f * (viewYB + 0.5f) / m_framebufferHeight - 1.0f;
	float y_vt = 2.0f * (viewYT + 0.5f) / m_framebufferHeight - 1.0f;
	
	float L_v = x_vr - x_vl;
	float H_v = y_vt - y_vb;

	// Window to Viewport mapping
	vec2 result(
		(L_v / L_w) * (point.x - x_wl) + x_vl,
		(H_v / H_w) * (point.y - y_wb) + y_vb
	);

	return result;
}

void A2::drawLineInViewport(const Line &line) 
{
	drawLine(pointToNDC(line.first), pointToNDC(line.second));
}

//----------------------------------------------------------------------------------------
// Generate vertices for a cube in MCS (right-handed coordinate system)
vector<vec4> A2::generateCubeVerts() const
{
	// (x,y,z,1), right-handed coordinate system, y up, x right, z "from" the screen
	vector<vec4> verts = {
		// Front face
		vec4(-1.0f, -1.0f, 1.0f, 1.0f),  // bottom left,  0
		vec4(1.0f, -1.0f, 1.0f, 1.0f),   // bottom right, 1
		vec4(-1.0f, 1.0f, 1.0f, 1.0f),   // top left,     2
		vec4(1.0f, 1.0f, 1.0f, 1.0f),    // top right,    3

		// Back face
		vec4(-1.0f, -1.0f, -1.0f, 1.0f), // bottom left,  4
		vec4(1.0f, -1.0f, -1.0f, 1.0f),  // bottom right, 5
		vec4(-1.0f, 1.0f, -1.0f, 1.0f),  // top left,     6
		vec4(1.0f, 1.0f, -1.0f, 1.0f),   // top right,    7
	};

	return verts;
}

//----------------------------------------------------------------------------------------
// Generate lines for a cube from m_c_verts
vector<Line> A2::generateCubeLines(vector<vec4> &verts) const
{
	// Cube vertex indices
	// FBL = front bottom left vertex, BTR = back top right vertex, etc.
	enum Vertex {
		FBL,
		FBR,
		FTL,
		FTR,
		BBL,
		BBR,
		BTL,
		BTR
	};

	vector<Line> lines;
	lines.reserve(12); // 12 lines needed to represent cube

	// Front face, in counterclockwise order for readability
	lines.push_back(make_pair(verts[FBL], verts[FBR])); // bottom line
	lines.push_back(make_pair(verts[FBR], verts[FTR])); // right line
	lines.push_back(make_pair(verts[FTR], verts[FTL])); // top line
	lines.push_back(make_pair(verts[FTL], verts[FBL])); // left line

	// Right face, in counterclockwise order for readability
	lines.push_back(make_pair(verts[FBR], verts[BBR])); // bottom line
	lines.push_back(make_pair(verts[FTR], verts[BTR])); // top line

	// Back face, in counterclockwise order for readability
	lines.push_back(make_pair(verts[BBL], verts[BBR])); // bottom line
	lines.push_back(make_pair(verts[BBR], verts[BTR])); // right line
	lines.push_back(make_pair(verts[BTR], verts[BTL])); // top line
	lines.push_back(make_pair(verts[BTL], verts[BBL])); // left line

	// Left face, in counterclockwise order for readability
	lines.push_back(make_pair(verts[FBL], verts[BBL])); // bottom line
	lines.push_back(make_pair(verts[FTL], verts[BTL])); // top line

	return lines;
}

//----------------------------------------------------------------------------------------
// Generate vertices for a gnomon in MCS (right-handed coordinate system)
vector<vec4> A2::generateGnomonVerts() const 
{
	// (x,y,z,1), right-handed coordinate system, y up, x right, z "from" the screen
	vector<vec4> verts = {
		vec4(0, 0, 0, 1),  // Origin
		vec4(1, 0, 0, 1),  // x
		vec4(0, 1, 0, 1),  // y
		vec4(0, 0, 1, 1)   // z
	};

	return verts;
}

//----------------------------------------------------------------------------------------
// Generate lines for the gnomon from g_verts
vector<Line> A2::generateGnomonLines(std::vector<glm::vec4> &verts) const 
{
	enum basis {
		Origin,
		X,
		Y,
		Z
	};

	vector<Line> lines;
	lines.reserve(3); // 3 lines required to draw gnomon

	lines.push_back(make_pair(verts[Origin], verts[X])); // x-axis
	lines.push_back(make_pair(verts[Origin], verts[Y])); // y-axis
	lines.push_back(make_pair(verts[Origin], verts[Z])); // z-axis

	return lines;
}

//----------------------------------------------------------------------------------------
// Generate lines for the gnomon from g_verts
vector<Line> A2::generateViewportLines() const
{
	vec4 bottomLeft(-1.0f, -1.0f, 0.0f, 1.0f);
	vec4 bottomRight(1.0f, -1.0f, 0.0f, 1.0f);
	vec4 topLeft(-1.0f, 1.0f, 0.0f, 1.0f);
	vec4 topRight(1.0f, 1.0f, 0.0f, 1.0f);

	vector<Line> lines;
	lines.reserve(4);

	lines.push_back(make_pair(bottomLeft, bottomRight)); // Bottom border
	lines.push_back(make_pair(bottomRight, topRight));   // Right border
	lines.push_back(make_pair(topRight, topLeft));       // Top border
	lines.push_back(make_pair(topLeft, bottomLeft));    // Left border

	return lines;
}

//----------------------------------------------------------------------------------------
// Generate a scale matrix
mat4 A2::generateScaleMatrix(const float s) const 
{
	mat4 scale(
		vec4(s, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, s, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, s, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	return scale;
}

//----------------------------------------------------------------------------------------
// Generate a translation matrix
glm::mat4 A2::translationMatrix(const vec3 &p, bool translateTo) const
{
	float factor = translateTo ? 1 : -1;

	mat4 translate(
		vec4(1.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 1.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 1.0f, 0.0f),
		vec4(factor * p.x, factor * p.y, factor * p.z, 1.0f)
	);

	return translate;
}

//----------------------------------------------------------------------------------------
// Generate a model matrix
mat4 A2::generateModelMatrix() const 
{
	mat4 model(1.0f); // Identity matrix

	return model;
}

//----------------------------------------------------------------------------------------
// Generate a typical view matrix (Course Notes 6.10)
mat4 A2::generateViewMatrix() const 
{
	const vec3 lookFrom(0.0f, 0.0f, 9.0f);
	const vec3 lookAt(0.0f, 0.0f, -1.0f);
	const vec3 up(lookFrom.x, 1.0f, lookFrom.z);

	vec3 v_z = glm::normalize(lookAt - lookFrom);
	vec3 v_x = glm::normalize(glm::cross(v_z, up));
	vec3 v_y = glm::cross(v_x, v_z);

	// Translation matrix in column-major order
	mat4 T = translationMatrix(lookFrom, false);

	// Rotation matrix in column-major order
	mat4 R(
		vec4(v_x, 0.0f),
		vec4(v_y, 0.0f),
		vec4(v_z, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	return glm::transpose(R) * T;
}

//----------------------------------------------------------------------------------------
// Generate a typical projection matrix (Course Notes P.63)
mat4 A2::generateProjectionMatrix() const 
{
	const float aspect = m_framebufferWidth / m_framebufferHeight;
	const float theta = glm::radians<float>(fov);
	const float cot = glm::cos(theta/2.0f) / glm::sin(theta/2.0f);

	// Column major layout
	mat4 proj(
		vec4(cot/aspect, 0.0f, 0.0f,                        0.0f),
		vec4(0.0f,       cot,  0.0f,                        0.0f),
		vec4(0.0f,       0.0f, (far+near)/(far-near),       1.0f),
		vec4(0.0f,       0.0f, (-2.0f*far*near)/(far-near), 0.0f)
	);

	return proj;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();

	reset();
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//---------------------------------------------------------------------------------------- Winter 2020
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & V0,   // Line Start (NDC coordinate)
		const glm::vec2 & V1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = V0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = V1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

//---------------------------------------------------------------------------------------
void A2::drawCube()
{	
	setLineColour(CUBE_COLOUR);

	vector<vec4> cubeVerts;
	cubeVerts.reserve(c_verts.size());

	// Transform the points
	for(const vec4 &v : c_verts)
		cubeVerts.push_back(homogenize(P * V * M * S * v));

	vector<Line> cubeLines = generateCubeLines(cubeVerts);
	for(const Line &line : cubeLines)
		drawLineInViewport(line);
}

//---------------------------------------------------------------------------------------
void A2::drawGnomon(bool model) 
{
	const vec3 colours[] = {
		RED,   // Colour of x-axis
		GREEN, // Colour of y-axis
		BLUE   // Colour of z-axis
	};

	vector<vec4> gnomonVerts;
	gnomonVerts.reserve(3);

	// Transform the points
	for(const vec4 &v : g_verts) {
		if (model) 
			gnomonVerts.push_back(homogenize(P * V * M * v));
			
		else
			gnomonVerts.push_back(homogenize(P * V * v));
	}

	int i = X;
	vector<Line> gnomonLines = generateGnomonLines(gnomonVerts);
	for(const auto &line : gnomonLines) {
		setLineColour(colours[i++]);
		drawLineInViewport(line);
	}
}

//---------------------------------------------------------------------------------------
void A2::drawViewport() 
{
	setLineColour(BLACK);

	vector<Line> viewportLines = generateViewportLines();
	for(const auto &line : viewportLines)
		drawLineInViewport(line);
}

//---------------------------------------------------------------------------------------
static void printLine(const Line &line)
{
	vec4 p = line.first, q = line.second;

	cout << "p: (" << p.x << "," << p.y << "," << p.z << ")" << endl;
	cout << "q: (" << q.x << "," << q.y << "," << q.z << ")" << endl;
}

// Todo
void A2::rotateView(double xPos, double yPos)
{
	const float xDelta = -1 * (xPos - xPrev);
	const float theta = glm::radians(xDelta);
	const float cos = glm::cos(theta);
	const float sin = glm::sin(theta);

	// Rotate about the eye's x-axis (horizontal)
	if (leftPressed) {
		mat4 R(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, cos, sin, 0.0f),
			vec4(0.0f, -sin, cos, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		V = R * V;
	}

	// Rotate about the eye's y-axis (vertical)
	if (middlePressed) {
		mat4 R(
			vec4(cos, 0.0f, -sin, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(sin, 0.0f, cos, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		V = R * V;
	}

	// Rotate about the eye's z-axis
	if (rightPressed) {
		mat4 R(
			vec4(cos, sin, 0.0f, 0.0f),
			vec4(-sin, cos, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		V = R * V;
	}
}

// Todo
void A2::translateView(double xPos, double yPos)
{
	const float xDelta = (xPos - xPrev) / TRANSLATE_FACTOR;

	// Translates the eye position along the eye's x-axis (horizontal)
	if (leftPressed) {
		mat4 T(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(xDelta, 0.0f, 0.0f, 1.0f)
		);

		V = T * V;
	}

	// Translates the eye position along the eye's y-axis (vertical)
	if (middlePressed) {
		mat4 T(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, xDelta, 0.0f, 1.0f)
		);

		V = T * V;
	}

	// Translates the eye position along the eye's z-axis
	if (rightPressed) {
		mat4 T(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, 0.0f, xDelta, 1.0f)
		);

		V = T * V;
	}
}

// Todo
void A2::perspective(double xPos, double yPos) 
{
	float factor;
	const float xDelta = xPos - xPrev;

	// Change field of view of the projection over a range from 5 to 160 degrees
	if (leftPressed) {
		factor = xDelta / FOV_FACTOR;
		fov = glm::clamp(fov + factor, MIN_FOV, MAX_FOV);
		P = generateProjectionMatrix();
	}

	// Translates projection's near plane along the view direction
	if (middlePressed) {
		factor = xDelta / NEAR_FAR_FACTOR;
		near = near + factor > far ? far : near + factor;
		near = glm::clamp(near, -MAX_PLANE, MAX_PLANE);
		P = generateProjectionMatrix();
	}

	// Translates projection's far plane along the view direction
	if (rightPressed) {
		factor = xDelta / NEAR_FAR_FACTOR;
		far = far + factor < near ? near : far + factor;
		far = glm::clamp(far, -MAX_PLANE, MAX_PLANE);
		P = generateProjectionMatrix();
	}
}

// Todo
// Rotate clockwise
void A2::rotateModel(double xPos, double yPos) 
{
	const float xDelta = xPos - xPrev;
	const float theta = glm::radians(xDelta);
	const float cos = glm::cos(theta);
	const float sin = glm::sin(theta);

	// Rotate the cube about its local x-axis
	if (leftPressed) {
		mat4 R(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, cos, sin, 0.0f),
			vec4(0.0f, -sin, cos, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		M = M * R;
	}

	// Rotate the cube about its local y-axis
	if (middlePressed) {
		mat4 R(
			vec4(cos, 0.0f, -sin, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(sin, 0.0f, cos, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		M = M * R;
	}

	// Rotate the cube about its local z-axis
	if (rightPressed) {
		mat4 R(
			vec4(cos, sin, 0.0f, 0.0f),
			vec4(-sin, cos, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		M = M * R;
	}
}

// Translates the model cube and gnomon
// Course Notes Section 10.2
void A2::translateModel(double xPos, double yPos) 
{
	const float xDelta = (xPos - xPrev) / TRANSLATE_FACTOR;

	// Translate the cube about its local x-axis
	if (leftPressed) {
		mat4 T(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(xDelta, 0.0f, 0.0f, 1.0f)
		);

		M = T * M;
	}

	// Translate the cube about its local y-axis
	if (middlePressed) {
		mat4 T(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, xDelta, 0.0f, 1.0f)
		);

		M = T * M;
	}

	// Translate the cube about its local z-axis
	if (rightPressed) {
		mat4 T(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, 0.0f, xDelta, 1.0f)
		);

		M = T * M;
	}
}

// Todo
// Course Notes Section 6.8
void A2::scaleModel(double xPos, double yPos) 
{
	const float s = xPos > xPrev ? SCALE_FACTOR : 1/SCALE_FACTOR;

	// Scale the cube in its local x direction
	if (leftPressed) {
		mat4 S_prime(
			vec4(s, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		S = S * S_prime;
	}

	// Scale the cube in its local y direction
	if (middlePressed) {
		mat4 S_prime(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, s, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		S = S * S_prime;
	}

	// Scale the cube in its local z direction
	if (rightPressed) {
		mat4 S_prime(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, s, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		S = S * S_prime;
	}
}

// Todo
void A2::viewport(double xPos, double yPos) 
{
	// Rotate about the eye's x-axis (horizontal)
	if (leftPressed) {}

	// Rotate about the eye's y-axis (vertical)
	if (middlePressed) {}

	// Rotate about the eye's z-axis
	if (rightPressed) {}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();

	// Clamp scale to min parameters
	for(int i = 0; i < 4; ++i){
		S[i][i] = glm::clamp(S[i][i], MIN_SCALE, MAX_SCALE);
	}

	// Draw cube
	drawCube();

	// Draw model gnomon
	drawGnomon();

	// Draw view gnomon
	drawGnomon(false);

	// Draw Viewport
	drawViewport();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		// Add more gui elements here here ...

		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if( ImGui::Button( "Reset" ) ) {
			reset();
		}

		for (int m = RotateView; m != EndMode; m++) {
			ImGui::PushID(m);

			if(ImGui::RadioButton(MODES[m].c_str(), (int*) &mode, m)) {

			}

			ImGui::PopID();
		}

		ImGui::Text( "FOV: %.1f", fov); 
		ImGui::Text( "Near: %.1f", near); ImGui::SameLine();
		ImGui::Text( "Far: %.1f", far); 

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
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
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		switch(mode) {
			case RotateView:
				rotateView(xPos, yPos);
				eventHandled = true;
				break;
			case TranslateView:
				translateView(xPos, yPos);
				eventHandled = true;
				break;
			case Perspective:
				perspective(xPos, yPos);
				eventHandled = true;
				break;
			case RotateModel:
				rotateModel(xPos, yPos);
				eventHandled = true;
				break;
			case TranslateModel:
				translateModel(xPos, yPos);
				eventHandled = true;
				break;
			case ScaleModel:
				scaleModel(xPos, yPos);
				eventHandled = true;
				break;
			default:
				break;
		}
	}

	  xPrev = xPos;
  	  yPrev = yPos;

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if(actions == GLFW_PRESS){
		switch(button){
			case GLFW_MOUSE_BUTTON_LEFT:
				leftPressed = true;
				eventHandled = true;
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				middlePressed = true;
				eventHandled = true;
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				rightPressed = true;
				eventHandled = true;
				break;
		}
	}

	if(actions == GLFW_RELEASE){
		switch(button){
			case GLFW_MOUSE_BUTTON_LEFT:
				leftPressed = false;
				eventHandled = true;
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				middlePressed = false;
				eventHandled = true;
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				rightPressed = false;
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
bool A2::mouseScrollEvent (
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
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}
