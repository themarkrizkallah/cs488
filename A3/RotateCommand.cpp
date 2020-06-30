#include "RotateCommand.hpp"

#include <glm/common.hpp>


//----------------------------------------------------------------------------------------
// Constructor
RotateCommand::RotateCommand(JointNode *node, char axis, float angle)
    : rotAngle(angle),
      jointAngle(axis == 'x' ? node->m_joint_x.cur : node->m_joint_y.cur),
      prevJointAngle(axis == 'x' ? node->m_joint_x.prev : node->m_joint_y.prev),
      minAngle(axis == 'x' ? (float)node->m_joint_x.min : (float)node->m_joint_y.min),
      maxAngle(axis == 'x' ? (float)node->m_joint_x.max : (float)node->m_joint_y.max),
      snapshot(jointAngle),
      prevSnapshot(prevJointAngle),
      executed(false)
{}

//----------------------------------------------------------------------------------------
// Copy Constructor
RotateCommand::RotateCommand(const RotateCommand &other)
    : rotAngle(other.rotAngle),
      jointAngle(other.jointAngle),
      prevJointAngle(other.prevJointAngle),
      minAngle(other.minAngle),
      maxAngle(other.maxAngle),
      snapshot(other.snapshot),
      prevSnapshot(other.prevSnapshot),
      executed(other.executed)
{}

//----------------------------------------------------------------------------------------
// Destructor
RotateCommand::~RotateCommand()
{}

//----------------------------------------------------------------------------------------
// Execute Rotation
void RotateCommand::execute()
{
    if(executed)
        return;

    // Apply rotation and appropriately clamp new angle
    jointAngle = glm::clamp(jointAngle + rotAngle, minAngle, maxAngle); 
    prevJointAngle = jointAngle;

    executed = true;
}

//----------------------------------------------------------------------------------------
// Undo Rotation
void RotateCommand::undo()
{
    if(!executed)
        return;

    // Restore joint angle and previous angle from snapshots
    jointAngle = snapshot;
    prevJointAngle = prevSnapshot;

    executed = false;
}