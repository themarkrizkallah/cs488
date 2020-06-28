// Spring 2020

#include "JointNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;

//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
	: SceneNode(name), m_xAngle(0), m_yAngle(0)
{
	m_nodeType = NodeType::JointNode;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}

 //---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
	m_joint_x.min = min;
	m_joint_x.init = init;
	m_joint_x.max = max;

	rotate('x', (float)init);
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
	m_joint_y.min = min;
	m_joint_y.init = init;
	m_joint_y.max = max;

	rotate('y', (float)init);
}

//---------------------------------------------------------------------------------------
float JointNode::netAngle(char axis, float angle) const
{
	switch (axis) {
		case 'x':
			angle = glm::clamp(angle, (float)m_joint_x.min - m_xAngle, (float)m_joint_x.max - m_xAngle);
			break;
		case 'y':
			angle = glm::clamp(angle, (float)m_joint_y.min - m_yAngle, (float)m_joint_y.max - m_yAngle);
	        break;
		default:
			break;
	}

	return angle;
}

//---------------------------------------------------------------------------------------
void JointNode::rotate(char axis, float angle) {
	vec3 rot_axis;

	angle = netAngle(axis, angle);

	switch (axis) {
		case 'x':
			rot_axis = vec3(1,0,0);
			m_xAngle += angle;
			break;
		case 'y':
			rot_axis = vec3(0,1,0);
			m_yAngle += angle;
	        break;
		case 'z':
			rot_axis = vec3(0,0,1);
			break;
		default:
			break;
	}

	mat4 rot_matrix = glm::rotate(degreesToRadians(angle), rot_axis);
	trans = rot_matrix * trans;
}
