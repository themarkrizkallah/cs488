// Spring 2020

#include "Ray.hpp"
#include "SceneNode.hpp"
#include "GeometryNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <iostream>
#include <sstream>
using namespace std;

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;


// Static class variable
unsigned int SceneNode::nodeInstanceCount = 0;


//---------------------------------------------------------------------------------------
SceneNode::SceneNode(const std::string& name)
  : m_name(name),
	m_nodeType(NodeType::SceneNode),
	trans(mat4()),
	invtrans(mat4()),
	m_nodeId(nodeInstanceCount++)
{

}

//---------------------------------------------------------------------------------------
// Deep copy
SceneNode::SceneNode(const SceneNode & other)
	: m_nodeType(other.m_nodeType),
	  m_name(other.m_name),
	  trans(other.trans),
	  invtrans(other.invtrans)
{
	for(SceneNode * child : other.children) {
		this->children.push_front(new SceneNode(*child));
	}
}

//---------------------------------------------------------------------------------------
SceneNode::~SceneNode() {
	for(SceneNode * child : children) {
		delete child;
	}
}

//---------------------------------------------------------------------------------------
void SceneNode::set_transform(const glm::mat4& m) {
	trans = m;
	invtrans = glm::inverse(m);
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_transform() const {
	return trans;
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_inverse() const {
	return invtrans;
}

//---------------------------------------------------------------------------------------
void SceneNode::add_child(SceneNode* child) {
	children.push_back(child);
}

//---------------------------------------------------------------------------------------
void SceneNode::remove_child(SceneNode* child) {
	children.remove(child);
}

//---------------------------------------------------------------------------------------
void SceneNode::rotate(char axis, float angle) {
	vec3 rot_axis;

	switch (axis) {
		case 'x':
			rot_axis = vec3(1,0,0);
			break;
		case 'y':
			rot_axis = vec3(0,1,0);
	        break;
		case 'z':
			rot_axis = vec3(0,0,1);
	        break;
		default:
			break;
	}
	mat4 rot_matrix = glm::rotate(degreesToRadians(angle), rot_axis);
	set_transform( rot_matrix * trans );
}

//---------------------------------------------------------------------------------------
void SceneNode::scale(const glm::vec3 & amount) {
	set_transform( glm::scale(amount) * trans );
}

//---------------------------------------------------------------------------------------
void SceneNode::translate(const glm::vec3& amount) {
	set_transform( glm::translate(amount) * trans );
}


//---------------------------------------------------------------------------------------
int SceneNode::totalSceneNodes() const {
	return nodeInstanceCount;
}

//---------------------------------------------------------------------------------------
std::ostream & operator << (std::ostream & os, const SceneNode & node) {

	//os << "SceneNode:[NodeType: ___, name: ____, id: ____, isSelected: ____, transform: ____"
	switch (node.m_nodeType) {
		case NodeType::SceneNode:
			os << "SceneNode";
			break;
		case NodeType::GeometryNode:
			os << "GeometryNode";
			break;
		case NodeType::JointNode:
			os << "JointNode";
			break;
	}
	os << ":[";

	os << "name:" << node.m_name << ", ";
	os << "id:" << node.m_nodeId;

	os << "]\n";
	return os;
}

/*
In hierarchical ray tracing, on "the way down" you should transform the point and vector 
forming the ray with a node's inverse transform. On "the way back up" you should transform the 
intersection point by the original transformation, and (assuming you represent the normal as a column vector) 
you should transform the normal with the transpose of the upper-3×3 part of the inverse transform.
*/

//---------------------------------------------------------------------------------------
HitRecord SceneNode::hit(const Ray &r, const glm::mat4 &worldToModel) const 
{
	HitRecord rec;

	// On "the way down", transform ray by node's inverse transform
	const mat4 accumMat = invtrans * worldToModel;
	const Ray r_model = invtrans * r;

	// GeometryNode, intersect primitive
	if(m_nodeType == NodeType::GeometryNode){
		const GeometryNode *geometryNode = static_cast<const GeometryNode *>(this);
		rec = geometryNode->m_primitive->hit(r_model);

		// Node is hit, store material in hit record
		if(rec.hit)
			rec.mat = geometryNode->m_material;
	}

	// Check if children are hit. If child is hit, use the closest intersection
	for(auto child : children){
		HitRecord childRec = child->hit(r_model, accumMat);
		if(childRec.hit){
			if(rec.hit == false)
				rec = childRec;
			else
				rec = std::min(rec, childRec);
		}
	}

	// On "the way up", transform normal with transpose of inverse transform
	if(rec.hit){
		rec.n = mat4(glm::transpose(mat3(invtrans))) * rec.n;
		rec.point = trans * rec.point;
	}

	return rec;
}