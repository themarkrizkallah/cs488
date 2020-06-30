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
    // Angle to rotate by
    const float rotAngle;

    // Reference to joint's current and previous angles
    float &jointAngle;
    float &prevJointAngle;

    // Lower and Upper Bounds on the axis' angle
    const float minAngle, maxAngle;

    // Snapshot of joint's pre-rotation angle and previous angle
    const float snapshot, prevSnapshot;

    // Idempotency bit, commands are single execute only
    bool executed;
};