#pragma once

#include "JointNode.hpp"

#include <vector>
#include <set>

// Snapshot of JointNode
class JointState {
public:
    JointState(JointNode *node);
    void recover() const;

private:
    JointNode *m_node;
    float m_xAngle, m_yAngle;
    glm::mat4 m_trans;
};

class JointStates {
public:
    JointStates(std::set<JointNode *> nodes);
    JointStates(JointStates &&other);

    void recover() const;

private:
    std::vector<JointState> m_states;
};