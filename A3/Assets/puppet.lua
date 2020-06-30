------------------- Materials -------------------
red = gr.material({0.8, 0.2, 0.2}, {0.1, 0.1, 0.1}, 10.0)
green = gr.material({0.2, 0.8, 0.2}, {0.1, 0.1, 0.1}, 10.0)
blue = gr.material({0.2, 0.2, 0.8}, {0.1, 0.1, 0.1}, 10.0)
brown = gr.material({0.5, 0.3, 0}, {0.1, 0.1, 0.1}, 10.0)
skintone = gr.material({0.7725, 0.549, 0.5216}, {0.1, 0.1, 0.1}, 10.0)
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
    pecJoint = gr.joint(side .. 'PecJoint', {-8, 0, 8}, {0, 0, 0})
    pecNode:add_child(pecJoint)

    ---- Pec Geoemtry ----
    pec = gr.mesh('sphere', side .. 'Pec')
    pec:set_material(brown)
    pec:scale(0.27, 0.17, 0.2)
    pecJoint:add_child(pec)

    ------------------- Abs -------------------
    abY = 0.1
    dY = -0.05

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

        dY = dY - abY
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
    shoulderJoint = gr.joint(side .. 'ShoulderJoint', {-100, 0, 0}, {0, 0, 0})
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
    elbowJoint = gr.joint(side .. 'ElbowJoint', {-115, -45, 0}, {0, 0, 0})
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
    wristJoint = gr.joint(side .. 'WristJoint', {-70, 0, 70}, {0, 0, 0})
    wristNode:add_child(wristJoint)

    ------------------- Hand -------------------
    hand = gr.mesh('sphere', side .. 'Hand')
    hand:set_material(skintone)
    hand:translate(0, -0.7, 0.0)
    hand:scale(0.06, 0.1, 0.06)
    wristJoint:add_child(hand)
end


function drawLegs(side)
    x = 1
    if side == "left" then
        x = -1
    end

    ------------------- Hip -------------------
    hipNode = gr.node(side .. "HipNode")
    hipNode:translate(x*0.23, -0.45, 0)
    hipNode:rotate('y', x*10)
    waistJoint:add_child(hipNode)

    ---- Hip Joint ----
    hipJoint = gr.joint(side .. 'HipJoint', {-50, -10, 50}, {0, 0, 0})
    hipNode:add_child(hipJoint)
    
    ---- Hip Geometry (Representing Joint) ----
    hip = gr.mesh('sphere', side .. 'Hip')
    hip:set_material(brown)
    hip:scale(0.16, 0.16, 0.16)
    hipJoint:add_child(hip)

    ------------------- Femur -------------------
    femur = gr.mesh('sphere', side .. "Femur")
    femur:set_material(brown)
    femur:translate(0, -1, 0)
    femur:scale(0.1, 0.26, 0.1)
    hipJoint:add_child(femur)

    ------------------- Knee -------------------
    kneeNode = gr.node(side .. "KneeNode")
    kneeNode:translate(0, -0.52, 0)
    hipJoint:add_child(kneeNode)

    ---- Knee Joint ----
    kneeJoint = gr.joint(side .. 'KneeJoint', {0, 30, 130}, {0, 0, 0})
    kneeNode:add_child(kneeJoint)
    
    ---- Knee Geometry (Representing Joint) ----
    knee = gr.mesh('sphere', side .. 'Hip')
    knee:set_material(brown)
    knee:scale(0.09, 0.09, 0.09)
    kneeJoint:add_child(knee)

    ------------------- Tibia -------------------
    tibia = gr.mesh('sphere', side .. 'Tibia')
    tibia:set_material(brown)
    tibia:translate(0, -1, 0.0)
    tibia:scale(0.07, 0.2, 0.07)
    kneeJoint:add_child(tibia)

    ------------------- Ankle -------------------
    ankleNode = gr.node(side .. "AnkleNode")
    ankleNode:translate(0, -0.4, 0)
    kneeJoint:add_child(ankleNode)

    ---- Ankle Joint ----
    ankleJoint = gr.joint(side .. 'AnkleJoint', {-20, -18, 60}, {0, 0, 0})
    ankleNode:add_child(ankleJoint)

    ------------------- Foot -------------------
    foot = gr.mesh('sphere', side .. 'Foot')
    foot:set_material(skintone)
    foot:translate(0, -0.3, 0.7)
    foot:scale(0.075, 0.03, 0.17)
    ankleJoint:add_child(foot)

    ------------------- Heel -------------------
    heel = gr.mesh('sphere', side .. 'Foot')
    heel:set_material(skintone)
    heel:translate(0, -0.3, -0.9)
    heel:scale(0.06, 0.04, 0.06)
    ankleJoint:add_child(heel)

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
head:set_material(skintone)
head:translate(0, -0.5, 0)
head:scale(0.3, 0.3, 0.3)
headJoint:add_child(head)

------------------- Arms -------------------
drawArm("left")
drawArm("right")

------------------- Waist -------------------
waistNode = gr.node('waistNode')
waistNode:translate(0, -0.25, 0)
rootNode:add_child(waistNode)

---- Wait joint ----
waistJoint = gr.joint('waistJoint', {-20, 0, 5}, {0, 0, 0})
waistNode:add_child(waistJoint)

---- Waist Geometry ----
waist = gr.mesh('cube', 'waist')
waist:set_material(brown)
waist:translate(0, -1, 0)
waist:scale(0.9, 0.3, 0.45)
waistJoint:add_child(waist)

------------------- Legs -------------------
drawLegs("left")
drawLegs("right")

-- Return the root with all of it's childern.  The SceneNode A3::m_rootNode will be set
-- equal to the return value from this Lua script.
return rootNode
