-- function drawArm(side)
--     x = 1
--     if side == "left" then
--         x = -1
--     end

--     ---- Shoudler ----
--     -- Shoulder Node --
--     shoulderNode = gr.node(side .. "ShoulderNode")
--     shoulderNode:translate(x*0.5, 0.45, 0.0)
--     rootNode:add_child(shoulderNode)

--     -- Shoulder Joint --
--     shoulderJoint = nil
--     if side == "left" then
--         shoulderJoint = gr.joint(side .. 'ShoulderJoint', {-4, 0, 170}, {0, 0, 0})
--     else
--         shoulderJoint = gr.joint(side .. 'ShoulderJoint', {-170, 0, 4}, {0, 0, 0})
--     end
   
--     -- shoulderJoint:rotate('z', x*-45)
--     shoulderNode:add_child(shoulderJoint)
    
--     -- Shoulder Geometry (For Joint) --
--     shoulder = gr.mesh('sphere', side .. 'Shoulder')
--     shoulder:set_material(brown)
--     shoulder:scale(0.15, 0.15, 0.15)
--     shoulderNode:add_child(shoulder)

--     -- Upper Arm Geometry --
--     upperArm = gr.mesh('sphere', side .. 'UpperArm')
--     upperArm:set_material(brown)
--     upperArm:translate(x*1.2, 0.0, 0.0)
--     upperArm:scale(0.2, 0.1, 0.1)
--     shoulderJoint:add_child(upperArm)

--     ---- Elbow ----
--     -- Elbow Node --
--     elbowNode = gr.node(side .. 'elbowNode')
--     elbowNode:translate(x*0.48, 0.0, 0.0)
--     shoulderJoint:add_child(elbowNode)

--     -- Elbow Joint --
--     elbowJoint = nil
--     if side == "left" then
--         elbowJoint = gr.joint(side .. 'ElbowJoint', {0.0, 0.0, 0.0}, {0, 45, 115})
--     else 
--         elbowJoint = gr.joint(side .. 'ElbowJoint', {0.0, 0.0, 0.0}, {-115, -45, 0})
--     end
--     elbowNode:add_child(elbowJoint)

--     -- Elbow Geometry (For Joint) --
--     elbow = gr.mesh('sphere', side .. 'Elbow')
--     elbow:set_material(brown)
--     elbow:scale(0.08, 0.08, 0.08)
--     elbowNode:add_child(elbow)

--     -- Forearm Geometry--
--     forearm = gr.mesh('sphere', side .. 'Forearm')
--     forearm:set_material(brown)
--     forearm:translate(x*1, 0.0, 0.0)
--     forearm:scale(0.2, 0.07, 0.07)
--     elbowJoint:add_child(forearm)
-- end

function drawArm(side)
    x = 1
    if side == "left" then
        x = -1
    end

    ---- Shoudler ----
    -- Shoulder Node --
    shoulderNode = gr.node(side .. "ShoulderNode")
    shoulderNode:translate(x*0.5, 0.45, 0.0)
    rootNode:add_child(shoulderNode)

    -- Shoulder Joint --
    shoulderJoint = nil
    if side == "left" then
        shoulderJoint = gr.joint(side .. 'ShoulderJoint', {-100, 0, 0}, {0, 0, 0})
        shoulderJoint:rotate('z', x*20)
    else
        shoulderJoint = gr.joint(side .. 'ShoulderJoint', {-100, 0, 0}, {0, 0, 0})
        shoulderJoint:rotate('z', x*20)
    end
    shoulderNode:add_child(shoulderJoint)
    
    -- Shoulder Geometry (For Joint) --
    shoulder = gr.mesh('sphere', side .. 'Shoulder')
    shoulder:set_material(brown)
    shoulder:scale(0.15, 0.15, 0.15)
    shoulderNode:add_child(shoulder)

    -- Upper Arm Geometry --
    upperArm = gr.mesh('sphere', side .. 'UpperArm')
    upperArm:set_material(brown)
    upperArm:translate(0, -1.2, 0.0)
    upperArm:scale(0.1, 0.2, 0.1)
    shoulderJoint:add_child(upperArm)

    ---- Elbow ----
    -- Elbow Node --
    elbowNode = gr.node(side .. 'elbowNode')
    elbowNode:translate(0, -0.48, 0.0)
    shoulderJoint:add_child(elbowNode)

    -- Elbow Joint --
    elbowJoint = nil
    if side == "left" then
        elbowJoint = gr.joint(side .. 'ElbowJoint', {-115, -45, 0}, {0, 0, 0})
    else 
        elbowJoint = gr.joint(side .. 'ElbowJoint', {-115, -45, 0}, {0, 0, 0})
    end
    elbowNode:add_child(elbowJoint)

    -- Elbow Geometry (For Joint) --
    elbow = gr.mesh('sphere', side .. 'Elbow')
    elbow:set_material(brown)
    elbow:scale(0.08, 0.08, 0.08)
    elbowNode:add_child(elbow)

    -- Forearm Geometry--
    forearm = gr.mesh('sphere', side .. 'Forearm')
    forearm:set_material(brown)
    forearm:translate(0, -1, 0.0)
    forearm:scale(0.07, 0.2, 0.07)
    elbowJoint:add_child(forearm)
end

---- Materials ----
red = gr.material({0.8, 0.2, 0.2}, {0.1, 0.1, 0.1}, 10.0)
green = gr.material({0.2, 0.8, 0.2}, {0.1, 0.1, 0.1}, 10.0)
blue = gr.material({0.2, 0.2, 0.8}, {0.1, 0.1, 0.1}, 10.0)
brown = gr.material({0.5, 0.3, 0}, {0.1, 0.1, 0.1}, 10.0)
white = gr.material({1, 1, 1}, {0.1, 0.1, 0.1}, 10.0)

--- Root Node --
rootNode = gr.node('root')
rootNode:translate(0.0, 0.0, -5)

---- Torso ----
torso = gr.mesh('cube', 'torso')
torso:set_material(brown)
torso:scale(0.75, 1, 0.75)
rootNode:add_child(torso)

---- Neck ----
-- Neck Node --
neckNode = gr.node('neckNode')
neckNode:translate(0.0, 0.5, 0.0)
rootNode:add_child(neckNode)

-- Neck joint -- 
neckJoint = gr.joint('neckJoint', {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0})
neckNode:add_child(neckJoint)

-- Neck geometry --
neck = gr.mesh('sphere', 'neck')
neck:set_material(brown)
neck:translate(0.0, 0.9, 0.0)
neck:scale(0.07, 0.16, 0.07)
neckJoint:add_child(neck)

---- Head ----
-- Head Node --
headNode = gr.node('headNode')
headNode:translate(0.0, 0.5, 0.0)
neckJoint:add_child(headNode)

-- Head joint --
headJoint = gr.joint('headJoint', {-10, 0.0, 10}, {-50, 0.0, 50})
headNode:add_child(headJoint)

-- Head --
head = gr.mesh('suzanne', 'head')
head:set_material(brown)
head:translate(0, -0.5, 0)
head:scale(0.3, 0.3, 0.3)
headJoint:add_child(head)

-- Right Arm --
drawArm("right")

-- Left Arm --
drawArm("left")

-- Return the root with all of it's childern.  The SceneNode A3::m_rootNode will be set
-- equal to the return value from this Lua script.
return rootNode
