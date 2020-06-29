#pragma once

#include "JointNode.hpp"

class RotateCommand {
public:
    RotateCommand(JointNode *node, char axis, float angle);
    RotateCommand(const RotateCommand &other);
    ~RotateCommand();

    void execute();
    void undo();

private:
    const float rotAngle;

    float &jointAngle;
    float &prevJointAngle;

    const float minAngle, maxAngle;
    const float snapshot, prevSnapshot;

    bool executed;
};