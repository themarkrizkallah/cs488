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
static const vec3 CYAN = vec3(0.0f, 255.0f, 255.0f);
static const vec3 MAGENTA = vec3(255.0f, 0.0f, 255.0f);
static const vec3 YELLOW = vec3(255.0f, 255.0f, 0.0f);
static const vec3 CUBE_COLOUR = vec3(1.0f, 0.7f, 0.8f);
static const vec3 VIEWPORT_COLOUR = BLACK;

// Min, Max, and Bound values
static const float ZERO_BOUND = 0.00001f;
static const float MIN_PLANE = 0.1f;
static const float MAX_PLANE = 1000.0f;
static const float MIN_FOV = 5.0f;
static const float MAX_FOV = 160.0f;
static const float MIN_SCALE = ZERO_BOUND;
static const float MAX_SCALE = 10.0f;
static const float MIN_DIM = 1.0f;

// Various scaling factors
static const float FOV_FACTOR = 100.0f;
static const float NEAR_FAR_FACTOR = 25.0f;
static const float TRANSLATE_FACTOR = 10.0f;
static const float SCALE_FACTOR = 1.03f;
static const float ROTATION_FACTOR = 2.0f;

// Default values
static const float MARGIN = 0.05f;
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


//---------------------------------------------------------------------------------------
static void printLine(const Line &line)
{
	vec4 p = line.first, q = line.second;

	cout << "p: (" << p.x << "," << p.y << "," << p.z << ", " << p.w << ")" << endl;
	cout << "q: (" << q.x << "," << q.y << "," << q.z << ", " << q.w << ")" << endl;
}


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
// Reset viewport, near + far planes, all matrices, and user mode
void A2::reset()
{
	viewX = MARGIN * m_framebufferWidth;
	viewY = MARGIN * m_framebufferHeight;

	viewWidth  = m_framebufferWidth  * (1.0f - 2.0f * MARGIN);
	viewHeight = m_framebufferHeight * (1.0f - 2.0f * MARGIN);

	near = NEAR;
	far = FAR;
	fov = FOV;

	M = generateModelMatrix();
	S = generateScaleMatrix();
	V = generateViewMatrix();
	P = generateProjectionMatrix();

	mode = RotateModel;

	getMousePos = false;
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

	// Front face
	lines.push_back(make_pair(verts[FBL], verts[FBR])); // bottom line
	lines.push_back(make_pair(verts[FBR], verts[FTR])); // right line
	lines.push_back(make_pair(verts[FTR], verts[FTL])); // top line
	lines.push_back(make_pair(verts[FTL], verts[FBL])); // left line

	// Right face
	lines.push_back(make_pair(verts[FBR], verts[BBR])); // bottom line
	lines.push_back(make_pair(verts[FTR], verts[BTR])); // top line

	// Back face
	lines.push_back(make_pair(verts[BBL], verts[BBR])); // bottom line
	lines.push_back(make_pair(verts[BBR], verts[BTR])); // right line
	lines.push_back(make_pair(verts[BTR], verts[BTL])); // top line
	lines.push_back(make_pair(verts[BTL], verts[BBL])); // left line

	// Left face
	lines.push_back(make_pair(verts[FBL], verts[BBL])); // bottom line
	lines.push_back(make_pair(verts[FTL], verts[BTL])); // top line

	return lines;
}

//----------------------------------------------------------------------------------------
// Generate vertices for a gnomon in WCS
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
// Generate lines for the gnomon
vector<Line> A2::generateGnomonLines(vector<vec4> &verts) const 
{
	enum basis {
		Origin = 0,
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
// Generate lines for thew viewport boundaries
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
	lines.push_back(make_pair(topLeft, bottomLeft));     // Left border

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
	float factor = translateTo ? 1.0f : -1;

	mat4 translate(
		vec4(1.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 1.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 1.0f, 0.0f),
		vec4(factor * p.x, factor * p.y, factor * p.z, 1.0f)
	);

	return translate;
}

//----------------------------------------------------------------------------------------
// Generate a model matrix (identity matrix)
mat4 A2::generateModelMatrix() const 
{
	mat4 model(1.0f); // Identity matrix

	return model;
}

//----------------------------------------------------------------------------------------
// Generate a typical view matrix (Course Notes 6.10 + SI)
mat4 A2::generateViewMatrix() const 
{
	// Coordinates in WCS
	const vec3 lookAt(0.0f);
	const vec3 lookFrom(0.0f, 0.0f, 9.0f);
	const vec3 up(lookFrom.x, 1.0f, lookFrom.z);

	vec3 v_z = glm::normalize(lookAt - lookFrom);
	vec3 v_x = glm::normalize(glm::cross(v_z, up));
	vec3 v_y = glm::cross(v_x, v_z);

	// Translation matrix
	mat4 T = translationMatrix(lookFrom, false);

	// Rotation matrix
	mat4 R(
		vec4(v_x, 0.0f),
		vec4(v_y, 0.0f),
		vec4(v_z, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	return R * T;
}

//----------------------------------------------------------------------------------------
// Generate a typical projection matrix (Course Notes 9.1)
mat4 A2::generateProjectionMatrix() const 
{
	const float aspect = m_framebufferWidth / m_framebufferHeight;
	const float theta = glm::radians<float>(fov);
	const float cot = glm::cos(theta/2.0f) / glm::sin(theta/2.0f);

	mat4 proj(
		vec4(cot/aspect, 0.0f, 0.0f,                        0.0f),
		vec4(0.0f,       cot,  0.0f,                        0.0f),
		vec4(0.0f,       0.0f, (far+near)/(far-near),       1.0f),
		vec4(0.0f,       0.0f, (-2.0f*far*near)/(far-near), 0.0f)
	);

	return proj;
}

//----------------------------------------------------------------------------------------
// Clip line in-place (Course Notes 9.5)
bool A2::clip(Line &line) const
{
	using FUNC_TYPE = float(glm::vec4 &);

	// Boundary coordinate generating lambda functions
	const auto BL = [](vec4& p) { return p.w + p.x; }; // Left plane
	const auto BR = [](vec4& p) { return p.w - p.x; }; // Right plane
	const auto BB = [](vec4& p) { return p.w + p.y; }; // Bottom plane
	const auto BT = [](vec4& p) { return p.w - p.y; }; // Top plane
	const auto BN = [](vec4& p) { return p.w + p.z; }; // Near plane
	const auto BF = [](vec4& p) { return p.w - p.z; }; // Far plane

	const vector<FUNC_TYPE *> boundaryTests = {
		BL,
		BR,
		BB,
		BT,
		BN,
		BF
	};

	vec4 &A = line.first;
	vec4 &B = line.second;

	// Course Notes 8.1 (pg. 58)
	for(auto boundaryTest : boundaryTests) {
		const float wecA = boundaryTest(A);
		const float wecB = boundaryTest(B);

		// Reject
		if(wecA < ZERO_BOUND && wecB < ZERO_BOUND)
			return false;

		// Line entirely within boundary
		else if(wecA >= ZERO_BOUND && wecB >= ZERO_BOUND)
			continue;

		const float t = wecA / (wecA - wecB);

		if(wecA < 0)
			A = A + t * (B - A);
		else
			B = A + t * (B - A);
	}

	return true;
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

//----------------------------------------------------------------------------------------
// Homogenize a point p, ensuring no division by zero happens
glm::vec4 A2::homogenize(const glm::vec4 &p) const 
{
	// Check if v.w is above the zero threshhold
	if (p.w < ZERO_BOUND){
		vec4 rounded(p);
		rounded.w = 1.0f;

		return rounded;
	}

	vec4 homogenized(
		p.x / p.w,
		p.y / p.w,
		p.z / p.w,
		1.0f
	);

	return homogenized;
}

//----------------------------------------------------------------------------------------
// Homogenize line
Line A2::homogenizeLine(const Line &line) const 
{
	Line homogenized = make_pair(
		homogenize(line.first),
		homogenize(line.second)
	);

	return homogenized;
}

//----------------------------------------------------------------------------------------
// Map a point from WCS to NDC
vec2 A2::pointToNDC(const vec4 &p) const
{
	using std::min;
	using std::max;

	// Window boundaries, length, and height
	const float x_wl = -1.0f, x_wr = 1.0f;
	const float y_wb = -1.0f, y_wt = 1.0f;

	const float L_w = x_wr - x_wl;
	const float H_w = y_wt - y_wb;

	// Find the viewport edges and clamp them to appropriate values
	const float viewLeft = glm::clamp(min(viewX, viewX + viewWidth), 1.0f, (float) m_framebufferWidth);
	const float viewRight = glm::clamp(max(viewX, viewX + viewWidth), 1.0f, (float) m_framebufferWidth);
	const float viewBot = glm::clamp(min(viewY, viewY + viewHeight), 1.0f, (float) m_framebufferHeight);
	const float viewTop = glm::clamp(max(viewY, viewY + viewHeight), 1.0f, (float) m_framebufferHeight);

	// Viewport boundaries, length, and height
	float x_vl = 2.0f * (viewLeft / m_framebufferWidth) - 1.0f;
	float x_vr = 2.0f * (viewRight / m_framebufferWidth) - 1.0f;
	float y_vb = 2.0f * (viewBot / m_framebufferHeight) - 1.0f;
	float y_vt = 2.0f * (viewTop / m_framebufferHeight) - 1.0f;

	float L_v = x_vr - x_vl;
	float H_v = y_vt - y_vb;

	// Window to Viewport mapping
	vec2 ndcPoint(
		(L_v / L_w) * (p.x - x_wl) + x_vl,
		(H_v / H_w) * (p.y - y_wb) + y_vb
	);

	return ndcPoint;
}

// Draw a line in the viewport
void A2::drawLineInViewport(const Line &line) 
{
	drawLine(pointToNDC(line.first), pointToNDC(line.second));
}


//----------------------------------------------------------------------------------------
// Generate, transform, clip, and draw the cube
void A2::drawCube()
{	
	setLineColour(CUBE_COLOUR);

	vector<vec4> cubeVerts;
	cubeVerts.reserve(c_verts.size());

	// Transform the points
	for(const vec4 &v : c_verts)
		cubeVerts.push_back(P * V * M * S * v);

	// Clip, homogenize, and draw lines
	vector<Line> cubeLines = generateCubeLines(cubeVerts);
	for(auto &line : cubeLines){
		bool accepted = clip(line);
		if(accepted)
			drawLineInViewport(homogenizeLine(line));
	}
}

//----------------------------------------------------------------------------------------
// Generate, transform, clip, and draw the gnomon
void A2::drawGnomon(bool model) 
{
	const vec3 colours[] = {
		RED,     // Colour of x-axis (model)
		GREEN,   // Colour of y-axis (model)
		BLUE,    // Colour of z-axis (model)
		CYAN,    // Colour of x-axis (view)  
		MAGENTA, // Colour of y-axis (view)          
		YELLOW   // Colour of z-axis (view)    
	};

	vector<vec4> gnomonVerts;
	gnomonVerts.reserve(3);

	// Transform the points
	for(const vec4 &v : g_verts) {
		// Apply modelling transformations if applicable
		if (model) 
			gnomonVerts.push_back(homogenize(P * V * M * v));
		else
			gnomonVerts.push_back(homogenize(P * V * v));
	}

	// Clip, homogenize, and draw lines
	int i = model ? X : 3;
	vector<Line> gnomonLines = generateGnomonLines(gnomonVerts);
	for(auto &line : gnomonLines) {
		setLineColour(colours[i++]);
		bool accepted = clip(line);
		if(accepted)
			drawLineInViewport(homogenizeLine(line));
	}
}

//----------------------------------------------------------------------------------------
// Draw the 4 lines representing the viewport
void A2::drawViewport() 
{
	setLineColour(VIEWPORT_COLOUR);

	vector<Line> viewportLines = generateViewportLines();
	for(const auto &line : viewportLines)
		drawLineInViewport(line);
}

//----------------------------------------------------------------------------------------
// Rotate the view (Course Notes 6.9 and 10.2)
void A2::rotateView(double xPos, double yPos)
{
	const float xDelta = xPos - xPrev;
	const float theta = glm::radians(xDelta / ROTATION_FACTOR);
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

		V = glm::inverse(R) * V;
	}

	// Rotate about the eye's y-axis (vertical)
	if (middlePressed) {
		mat4 R(
			vec4(cos, 0.0f, -sin, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(sin, 0.0f, cos, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		V = glm::inverse(R) * V;
	}

	// Rotate about the eye's z-axis
	if (rightPressed) {
		mat4 R(
			vec4(cos, sin, 0.0f, 0.0f),
			vec4(-sin, cos, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

		V = glm::inverse(R) * V;
	}
}

//----------------------------------------------------------------------------------------
// Translate the view (Course Notes 6.9 and 10.2)
void A2::translateView(double xPos, double yPos)
{
	const float xDelta = (xPos - xPrev) / m_framebufferWidth;
	float factor = xDelta * TRANSLATE_FACTOR;

	// Translates the eye position along the eye's x-axis (horizontal)
	if (leftPressed) {
		mat4 T(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(factor, 0.0f, 0.0f, 1.0f)
		);

		V = glm::inverse(T) * V;
	}

	// Translates the eye position along the eye's y-axis (vertical)
	if (middlePressed) {
		mat4 T(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, factor, 0.0f, 1.0f)
		);

		V = glm::inverse(T) * V;
	}

	// Translates the eye position along the eye's z-axis
	if (rightPressed) {
		mat4 T(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, 0.0f, factor, 1.0f)
		);

		V = glm::inverse(T) * V;
	}
}

//----------------------------------------------------------------------------------------
// Change the perspective matrix
void A2::perspective(double xPos, double yPos) 
{
	const float xDelta = xPos - xPrev;
	float factor;

	// Change field of view of the projection over a range from 5 to 160 degrees
	if (leftPressed) {
		factor = FOV_FACTOR * xDelta / m_framebufferWidth;;
		fov = glm::clamp(fov + factor, MIN_FOV, MAX_FOV);
		P = generateProjectionMatrix();
	}

	// Translate projection's near plane along the view direction
	if (middlePressed) {
		factor = NEAR_FAR_FACTOR * xDelta / m_framebufferWidth;
		near = near + factor > far ? far : near + factor;
		near = glm::clamp(near, MIN_PLANE, MAX_PLANE);
		P = generateProjectionMatrix();
	}

	// Translate projection's far plane along the view direction
	if (rightPressed) {
		factor = NEAR_FAR_FACTOR * xDelta / m_framebufferWidth;
		far = far + factor < near ? near : far + factor;
		far = glm::clamp(far, MIN_PLANE, MAX_PLANE);
		P = generateProjectionMatrix();
	}
}

//----------------------------------------------------------------------------------------
// Rotate the model counterclocwise (Course Notes 6.9)
void A2::rotateModel(double xPos, double yPos) 
{
	const float xDelta = xPos - xPrev;
	const float theta = glm::radians(xDelta / ROTATION_FACTOR);
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

//----------------------------------------------------------------------------------------
// Translate the model (Course Notes 6.9)
void A2::translateModel(double xPos, double yPos) 
{
	const float xDelta = (xPos - xPrev) / m_framebufferWidth;
	float factor = xDelta * TRANSLATE_FACTOR;

	// Translate the cube about its local x-axis
	if (leftPressed) {
		mat4 T(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(factor, 0.0f, 0.0f, 1.0f)
		);

		M = M * T;
	}

	// Translate the cube about its local y-axis
	if (middlePressed) {
		mat4 T(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, factor, 0.0f, 1.0f)
		);

		M = M * T;
	}

	// Translate the cube about its local z-axis
	if (rightPressed) {
		mat4 T(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, 0.0f, factor, 1.0f)
		);

		M = M * T;
	}
}

//----------------------------------------------------------------------------------------
// Scale the model (Course Notes 6.9)
void A2::scaleModel(double xPos, double yPos) 
{
	const float xDelta = xPos - xPrev;
	const float s = xDelta > 0 ? SCALE_FACTOR : 1/SCALE_FACTOR;

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

//----------------------------------------------------------------------------------------
// Redraw the viewport
void A2::viewport(double xPos, double yPos) 
{
	// Top left of screen is (0,0)
	float x = xPos;
	float y = m_framebufferHeight - yPos;

	// Redraw the viewport
	if (leftPressed) {
		// First click, set viewX, viewY and a minimum width and height
		if(getMousePos){
			viewX = glm::clamp(x, 1.0f, (float) m_framebufferWidth);
			viewY = glm::clamp(y, 1.0f, (float) m_framebufferHeight);
			viewWidth = MIN_DIM;
			viewHeight = MIN_DIM;

			getMousePos = false;
		
		// Set width relative to viewX, viewY, and current mouse position
		} else {
			viewWidth = glm::clamp(x, 0.0f, (float) m_framebufferWidth) - viewX;
			viewHeight = glm::clamp(y, 0.0f, (float) m_framebufferHeight) - viewY;
		}
	}
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

	// Clamp scale
	for(int i = 0; i < 4; ++i){
		S[i][i] = glm::clamp(S[i][i], MIN_SCALE, MAX_SCALE);
	}

	// Draw view gnomon
	drawGnomon(false);

	// Draw model gnomon
	drawGnomon();

	// Draw cube
	drawCube();

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

			if(ImGui::RadioButton(MODES[m].c_str(), (int*) &mode, m)) {}

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
			case Viewport:
				viewport(xPos, yPos);
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

				if(mode == Viewport)
					getMousePos = true;

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
	switch(key){
		case GLFW_KEY_O:
			mode = RotateView;
			eventHandled = true;
			break;
		case GLFW_KEY_E:
			mode = TranslateView;
			eventHandled = true;
			break;
		case GLFW_KEY_P:
			mode = Perspective;
			eventHandled = true;
			break;
		case GLFW_KEY_R:
			mode = RotateModel;
			eventHandled = true;
			break;
		case GLFW_KEY_T:
			mode = TranslateModel;
			eventHandled = true;
			break;
		case GLFW_KEY_S:
			mode = ScaleModel;
			eventHandled = true;
			break;
		case GLFW_KEY_V:
			mode = Viewport;
			eventHandled = true;
			break;
		default:
			break;
	}

	return eventHandled;
}
	float aspect;