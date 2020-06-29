------------------- Materials -------------------
red = gr.material({0.8, 0.2, 0.2}, {0.1, 0.1, 0.1}, 10.0)
green = gr.material({0.2, 0.8, 0.2}, {0.1, 0.1, 0.1}, 10.0)
blue = gr.material({0.2, 0.2, 0.8}, {0.1, 0.1, 0.1}, 10.0)
brown = gr.material({0.5, 0.3, 0}, {0.1, 0.1, 0.1}, 10.0)
white = gr.material({1, 1, 1}, {0.1, 0.1, 0.1}, 10.0)

------------------- Root -------------------
rootNode = gr.node('root')
rootNode:translate(0, 0, -5)


---- Draw the puppet's torso
----  * side is one of {"left", "right"}
function drawTorsoDecorations(side)
    x = 1
    if side == "left" then
        x = -1
    end

    ------------------- Pec -------------------
    pecNode = gr.node(side..'PecNode')
    pecNode:translate(x*0.1538, 0.289, 0.16)
    rootNode:add_child(pecNode)

    ---- Pec Joint ----
    pecJoint = nil
    pecJoint = gr.joint(side .. 'PecJoint', {-8, 0, 8}, {0, 0, 0})
    pecNode:add_child(pecJoint)

    ---- Pec Geoemtry ----
    pec = gr.mesh('sphere', side .. 'Pec')
    pec:set_material(brown)
    pec:scale(0.27, 0.17, 0.2)
    pecJoint:add_child(pec)

    ------------------- Abs -------------------
    abY = -0.1
    dY = abY

    for i = 1, 3 do
        abNode = gr.node(side..'AbNode' ..i)
        abNode:translate(x*0.12, dY, 0.16)
        rootNode:add_child(abNode)

        ---- Ab Joint ----
        abJoint = gr.joint(side.. 'PecJoint' ..i, {0, 0, 0}, {-5, 0, 5})
        abNode:add_child(abJoint)
    
        ---- Ab Geoemtry ----
        ab = gr.mesh('sphere', side .. 'Ab'..i)
        ab:set_material(brown)
        ab:scale(0.1588, 0.1, 0.1)
        abJoint:add_child(ab)

        dY = dY + abY
    end
end


---- Draw the puppet's arm
----  * side is one of {"left", "right"}
function drawArm(side)
    x = 1
    if side == "left" then
        x = -1
    end

    ------------------- Shoulder -------------------
    shoulderNode = gr.node(side .. "ShoulderNode")
    shoulderNode:rotate('z', x*20)
    shoulderNode:translate(x*0.5, 0.45, 0.0)
    rootNode:add_child(shoulderNode)

    ---- Shoulder Joint ----
    shoulderJoint = nil
    shoulderJoint = gr.joint(side .. 'ShoulderJoint', {-100, 0, 0}, {0, 0, 0})
    -- shoulderJoint:rotate('z', x*20)
    shoulderNode:add_child(shoulderJoint)
    
    ---- Shoulder Geometry (Representing Joint) ----
    shoulder = gr.mesh('sphere', side .. 'Shoulder')
    shoulder:set_material(brown)
    shoulder:scale(0.15, 0.15, 0.15)
    shoulderNode:add_child(shoulder)

    ---- Upper Arm Geometry ----
    upperArm = gr.mesh('sphere', side .. 'UpperArm')
    upperArm:set_material(brown)
    upperArm:translate(0, -1.2, 0.0)
    upperArm:scale(0.1, 0.2, 0.1)
    shoulderJoint:add_child(upperArm)

    ------------------- Elbow -------------------
    elbowNode = gr.node(side .. 'ElbowNode')
    elbowNode:translate(0, -0.48, 0.0)
    shoulderJoint:add_child(elbowNode)

    ---- Elbow Joint ----
    elbowJoint = nil
    if side == "left" then
        elbowJoint = gr.joint(side .. 'ElbowJoint', {-115, -45, 0}, {0, 0, 0})
    else 
        elbowJoint = gr.joint(side .. 'ElbowJoint', {-115, -45, 0}, {0, 0, 0})
    end
    elbowNode:add_child(elbowJoint)

    ---- Elbow Geometry (Representing Joint) ----
    elbow = gr.mesh('sphere', side .. 'Elbow')
    elbow:set_material(brown)
    elbow:scale(0.08, 0.08, 0.08)
    elbowNode:add_child(elbow)

    ------------------- Forearm -------------------
    forearm = gr.mesh('sphere', side .. 'Forearm')
    forearm:set_material(brown)
    forearm:translate(0, -1, 0.0)
    forearm:scale(0.07, 0.2, 0.07)
    elbowJoint:add_child(forearm)

    ------------------- Wrist -------------------
    wristNode = gr.node(side .. 'wristNode')
    wristNode:translate(0, -0.4, 0)
    elbowJoint:add_child(wristNode)

    ---- Wrist Joint ----
    wristJoint = nil
    if side == "left" then
        wristJoint = gr.joint(side .. 'WristJoint', {-70, 0, 70}, {0, 0, 0})
    else 
        wristJoint = gr.joint(side .. 'WristJoint', {-70, 0, 70}, {0, 0, 0})
    end
    wristNode:add_child(wristJoint)

    ------------------- Hand -------------------
    hand = gr.mesh('sphere', side .. 'Hand')
    hand:set_material(brown)
    hand:translate(0, -0.7, 0.0)
    hand:scale(0.06, 0.1, 0.06)
    wristJoint:add_child(hand)
end

------------------- Torso -------------------
---- Torso Geometry ----
torso = gr.mesh('cube', 'torso')
torso:set_material(brown)
torso:scale(0.85, 1, 0.4)
rootNode:add_child(torso)

---- Torso Decorations ----
drawTorsoDecorations("left")
drawTorsoDecorations("right")

------------------- Neck -------------------
---- Neck Node ----
neckNode = gr.node('neckNode')
neckNode:translate(0, 0.5, 0)
rootNode:add_child(neckNode)

---- Neck joint ---- 
neckJoint = gr.joint('neckJoint', {-10, 0, 10}, {0, 0, 0})
neckNode:add_child(neckJoint)

---- Neck geometry ----
neck = gr.mesh('sphere', 'neck')
neck:set_material(brown)
neck:translate(0.0, 0.9, 0.0)
neck:scale(0.07, 0.16, 0.07)
neckJoint:add_child(neck)

------------------- Head -------------------
---- Head Node ----
headNode = gr.node('headNode')
headNode:translate(0, 0.5, 0)
neckJoint:add_child(headNode)

---- Head joint ----
headJoint = gr.joint('headJoint', {0, 0, 0}, {-50, 0, 50})
headNode:add_child(headJoint)

---- Head Geometry ----
head = gr.mesh('suzanne', 'head')
head:set_material(brown)
head:translate(0, -0.5, 0)
head:scale(0.3, 0.3, 0.3)
headJoint:add_child(head)

------------------- Arms -------------------
drawArm("left")
drawArm("right")

------------------- Hip -------------------
-- torso = gr.mesh('cube', 'torso')
-- torso:set_material(brown)
-- torso:scale(0.75, 1, 0.75)
-- rootNode:add_child(torso)

-- Return the root with all of it's childern.  The SceneNode A3::m_rootNode will be set
-- equal to the return value from this Lua script.
return rootNode
