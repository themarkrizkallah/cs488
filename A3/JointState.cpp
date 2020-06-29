#include "JointState.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <set>

using namespace std;

//----------------------------------------------------------------------------------------
// Constructor
JointState::JointState(JointNode *node)
    : m_node(node),
      m_xAngle(node ? node->m_xAngle : 0),
      m_yAngle(node ? node->m_yAngle : 0),
      m_trans(node ? node->trans : glm::mat4())
{}

//----------------------------------------------------------------------------------------
// Set m_node's state according to JointState
void JointState::recover() const
{
    m_node->m_xAngle = m_xAngle;
    m_node->m_yAngle = m_yAngle;
    m_node->trans = m_trans;
}

//----------------------------------------------------------------------------------------
// Constructor
JointStates::JointStates(std::set<JointNode *> nodes)
{
    m_states.reserve(nodes.size());

    for(auto node : nodes)
        m_states.emplace_back(node);
}

//----------------------------------------------------------------------------------------
// Move Constructor
JointStates::JointStates(JointStates &&other)
    : m_states(std::move(other.m_states))
{}

//----------------------------------------------------------------------------------------
// Set m_node's state according to JointState
void JointStates::recover() const
{
    for(const auto &state : m_states)
        state.recover();
}
