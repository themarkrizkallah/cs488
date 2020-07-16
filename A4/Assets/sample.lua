-- #############################################
-- Saddle your horses because tonight, WE RIDE!
-- #############################################


function centerCube(node)
    node:translate(-0.5, -0.5, -0.5)
end

------------------- Draw Puppet -------------------
function drawPuppet()
    -- Head --
    drawHead()

    -- Torso --
    drawTorso()

    -- Waist --
    drawWaist()
end

------------------- Draw Head -------------------
function drawHead()
    ------------------- Neck -------------------

    ---- Neck Node ---- 
    neckNode = gr.node('neckNode')
    neckNode:translate(0, 0.5, 0)
    torsoNode:add_child(neckNode)

    ---- Neck Joint ---- 
    neckJoint = gr.joint('neckJoint', {-8, 0, 8}, {0, 0, 0})
    neckNode:add_child(neckJoint)

    ---- Neck Geometry ----
    neck = gr.sphere('neck')
    neck:set_material(brown)
    neck:translate(0.0, 0.6, 0.0)
    neck:scale(0.07, 0.16, 0.07)
    neckJoint:add_child(neck)

    ------------------- Head -------------------

    ---- Head Node ----
    headNode = gr.node('headNode')
    headNode:translate(0, 0.5, 0)
    neckJoint:add_child(headNode)

    ---- Head Joint ----
    headJoint = gr.joint('headJoint', {0, 0, 0}, {-30, 0, 30})
    headNode:add_child(headJoint)

    ---- Head Geometry ----
    head = gr.mesh('suzanne', 'suzanne.obj')
    head:set_material(skintone)
    head:translate(0, -0.75, 0)
    head:scale(0.28, 0.28, 0.28)
    headJoint:add_child(head)
end

------------------- Draw Torso -------------------
function drawTorso()
    ---- Torso Geometry ----
    torso = gr.cube('torso')
    centerCube(torso)
    torso:set_material(brown)
    torso:scale(0.85, 1, 0.4)
    torsoNode:add_child(torso)

    -- ---- Torso Decorations ----
    drawTorsoDecorations("left")
    drawTorsoDecorations("right")

    -- ---- Arms ----
    drawArm("left")
    drawArm("right")
end

------------------- Draw Torso Decorations -------------------
--  * side is one of {"left", "right"}
function drawTorsoDecorations(side)
    x = 1
    if side == "left" then
        x = -1
    end

    ------------------- Pec -------------------
    pecNode = gr.node(side..'PecNode')
    pecNode:translate(x*0.1538, 0.289, 0.16)
    torsoNode:add_child(pecNode)

    ---- Pec Joint ----
    pecJoint = gr.joint(side .. 'PecJoint', {-8, 0, 8}, {0, 0, 0})
    pecNode:add_child(pecJoint)

    ---- Pec Geoemtry ----
    pec = gr.sphere(side .. 'Pec')
    pec:set_material(brown)
    pec:scale(0.27, 0.17, 0.2)
    pecJoint:add_child(pec)

    ------------------- Abs -------------------
    abY = 0.1
    dY = -0.05

    for i = 1, 3 do
        abNode = gr.node(side..'AbNode' ..i)
        abNode:translate(x*0.12, dY, 0.16)
        torsoNode:add_child(abNode)

        ---- Ab Joint ----
        abJoint = gr.joint(side.. 'PecJoint' ..i, {0, 0, 0}, {-5, 0, 5})
        abNode:add_child(abJoint)
    
        ---- Ab Geoemtry ----
        ab = gr.sphere(side .. 'Ab'..i)
        ab:set_material(brown)
        ab:scale(0.1588, 0.1, 0.1)
        abJoint:add_child(ab)

        dY = dY - abY
    end
end

------------------- Draw Arm -------------------
--  * side is one of {"left", "right"}
function drawArm(side)
    x = 1
    if side == "left" then
        x = -1
    end

    ------------------- Shoulder -------------------
    ---- Shoulder Node ----
    shoulderNode = gr.node(side .. "ShoulderNode")
    shoulderNode:rotate('z', x*20)
    shoulderNode:translate(x*0.5, 0.45, 0.0)
    torsoNode:add_child(shoulderNode)

    ---- Shoulder Joint ----
    shoulderJoint = gr.joint(side .. 'ShoulderJoint', {-90, 0, 30}, {0, 0, 0})
    shoulderNode:add_child(shoulderJoint)
    
    ---- Shoulder Geometry (Representing Joint) ----
    shoulder = gr.sphere(side .. 'Shoulder')
    shoulder:set_material(brown)
    shoulder:scale(0.15, 0.15, 0.15)
    shoulderNode:add_child(shoulder)

    ------------------- Upper Arm -------------------

    ---- Upper Arm Geometry ----
    upperArm = gr.sphere(side .. 'UpperArm')
    upperArm:set_material(brown)
    upperArm:translate(0, -1.2, 0.0)
    upperArm:scale(0.1, 0.2, 0.1)
    shoulderJoint:add_child(upperArm)

    ------------------- Elbow -------------------

    ---- Elbow Node ----
    elbowNode = gr.node(side .. 'ElbowNode')
    elbowNode:translate(0, -0.48, 0.0)
    shoulderJoint:add_child(elbowNode)

    ---- Elbow Joint ----
    elbowJoint = gr.joint(side .. 'ElbowJoint', {-115, -45, 0}, {0, 0, 0})
    elbowNode:add_child(elbowJoint)

    ---- Elbow Geometry (Representing Joint) ----
    elbow = gr.sphere(side .. 'Elbow')
    elbow:set_material(brown)
    elbow:scale(0.08, 0.08, 0.08)
    elbowNode:add_child(elbow)

    ------------------- Forearm -------------------

    ---- Forearm Geometry ----
    forearm = gr.sphere(side .. 'Forearm')
    forearm:set_material(brown)
    forearm:translate(0, -1, 0.0)
    forearm:scale(0.07, 0.2, 0.07)
    elbowJoint:add_child(forearm)

    ------------------- Wrist -------------------

    ---- Wrist Node ----
    wristNode = gr.node(side .. 'wristNode')
    wristNode:translate(0, -0.4, 0)
    elbowJoint:add_child(wristNode)

    ---- Wrist Joint ----
    wristJoint = gr.joint(side .. 'WristJoint', {-60, 0, 60}, {0, 0, 0})
    wristNode:add_child(wristJoint)

    ------------------- Hand -------------------

    ---- Hand Geometry ----
    hand = gr.sphere(side .. 'Hand')
    hand:set_material(skintone)
    hand:translate(0, -0.7, 0.0)
    hand:scale(0.06, 0.1, 0.06)
    wristJoint:add_child(hand)
end

------------------- Draw Waist -------------------
function drawWaist()
    ------------------- Waist -------------------

    ---- Waist Node ----
    waistNode = gr.node('waistNode')
    waistNode:translate(0, -0.25, 0)
    torsoNode:add_child(waistNode)

    ---- Waist joint ----
    waistJoint = gr.joint('waistJoint', {-35, 0, 5}, {0, 0, 0})
    waistNode:add_child(waistJoint)

    ---- Waist Geometry ----
    waist = gr.cube('waist')
    centerCube(waist)
    waist:set_material(brown)
    waist:translate(0, -1, 0)
    waist:scale(0.78, 0.3, 0.4)
    waistJoint:add_child(waist)

    ------------------- Tail -------------------
    shrinkFactor = 0.7
    
    ---- Base-Tail Node ----
    baseTailNode = gr.node('baseTail')
    baseTailNode:rotate('z', 180);
    baseTailNode:translate(0, -0.25, -0.225)
    waistJoint:add_child(baseTailNode)

    ---- Base-Tail joint ----
    baseTailJoint = gr.joint('baseTailJoint', {-30, -20, 30}, {0, 0, 0})
    baseTailNode:add_child(baseTailJoint)

    ---- Base-Tail Geometry ----
    baseTail = gr.sphere('baseTail')
    baseTail:set_material(brown)
    baseTail:scale(0.05, 0.05, 0.4)
    baseTailJoint:add_child(baseTail)

    ---- Mid-Tail Node ----
    midTailNode = gr.node('midTail')
    midTailNode:translate(0, 0, -0.4)
    baseTailJoint:add_child(midTailNode)

    ---- Mid-Tail joint ----
    midTailJoint = gr.joint('midTailJoint', {-50, -40, 40}, {0, 0, 0})
    midTailNode:add_child(midTailJoint)

    ---- Mid-Tail Geometry (For Joint) ----
    midTailBall = gr.sphere('midTailBall')
    midTailBall:set_material(brown)
    midTailBall:scale(0.03, 0.03, 0.03)
    midTailNode:add_child(midTailBall)

    ---- Mid-Tail Geometry ----
    midTail = gr.sphere('midTail')
    midTail:set_material(brown)
    midTail:scale(0.05*shrinkFactor, 0.05*shrinkFactor, 0.4*shrinkFactor)
    midTail:translate(0, 0, -0.28)
    midTailJoint:add_child(midTail)

    shrinkFactor = shrinkFactor * shrinkFactor

    ---- Tail Node ----
    tailNode = gr.node('tail')
    tailNode:translate(0, 0, -0.55)
    midTailJoint:add_child(tailNode)

    -----Tail joint ----
    tailJoint = gr.joint('tailJoint', {-50, -40, 30}, {0, 0, 0})
    tailNode:add_child(tailJoint)

    ---- Tail Geometry (For Joint) ----
    tailBall = gr.sphere('tailBall')
    tailBall:set_material(brown)
    tailBall:scale(0.03, 0.03, 0.03)
    tailNode:add_child(tailBall)

    ---- Tail Geometry ----
    tail = gr.sphere('tail')
    tail:set_material(brown)
    tail:scale(0.05*shrinkFactor, 0.05*shrinkFactor, 0.4*shrinkFactor)
    tail:translate(0, 0, -0.2)
    tailJoint:add_child(tail)

    ---- Legs ----
    drawLeg("left")
    drawLeg("right")
end

------------------- Legs -------------------
--  * side is one of {"left", "right"}
function drawLeg(side)
    x = 1
    if side == "left" then
        x = -1
    end

    ------------------- Hip -------------------
    ---- Hip Node ----
    hipNode = gr.node(side .. "HipNode")
    hipNode:translate(x*0.23, -0.45, 0)
    hipNode:rotate('z', x*20)
    hipNode:rotate('x', x*-10);
    waistJoint:add_child(hipNode)

    ---- Hip Joint ----
    hipJoint = gr.joint(side .. 'HipJoint', {-60, -60, 60}, {0, 0, 0})
    hipNode:add_child(hipJoint)
    
    ---- Hip Geometry (Representing Joint) ----
    hip = gr.sphere(side .. 'Hip')
    hip:set_material(brown)
    hip:scale(0.16, 0.16, 0.16)
    hipNode:add_child(hip)

    ------------------- Femur -------------------

    ---- Femur Geometry ----
    femur = gr.sphere(side .. "Femur")
    femur:set_material(brown)
    femur:translate(0, -1, 0)
    femur:scale(0.1, 0.26, 0.1)
    hipJoint:add_child(femur)

    ------------------- Knee -------------------

    ---- Knee Node ----
    kneeNode = gr.node(side .. "KneeNode")
    kneeNode:translate(0, -0.52, 0)
    hipJoint:add_child(kneeNode)

    ---- Knee Joint ----
    kneeJoint = gr.joint(side .. 'KneeJoint', {0, 30, 130}, {0, 0, 0})
    kneeNode:add_child(kneeJoint)
    
    ---- Knee Geometry (Representing Joint) ----
    knee = gr.sphere(side .. 'Hip')
    knee:set_material(brown)
    knee:scale(0.09, 0.09, 0.09)
    kneeNode:add_child(knee)

    ------------------- Tibia -------------------

    ---- Tibia Geometry ----
    tibia = gr.sphere(side .. 'Tibia')
    tibia:set_material(brown)
    tibia:translate(0, -1, 0.0)
    tibia:scale(0.07, 0.2, 0.07)
    kneeJoint:add_child(tibia)

    ------------------- Ankle -------------------

    ---- Ankle Node ----   
    ankleNode = gr.node(side .. "AnkleNode")
    ankleNode:translate(0, -0.4, 0)
    kneeJoint:add_child(ankleNode)

    ---- Ankle Joint ----
    ankleJoint = gr.joint(side .. 'AnkleJoint', {-20, -18, 60}, {0, 0, 0})
    ankleNode:add_child(ankleJoint)

    ------------------- Foot -------------------

    ---- Foot Geometry ----
    foot = gr.sphere(side .. 'Foot')
    foot:set_material(skintone)
    foot:translate(0, -0.3, 0.7)
    foot:scale(0.075, 0.03, 0.17)
    ankleJoint:add_child(foot)

    ---- Heel Geometry ----
    heel = gr.sphere(side .. 'Foot')
    heel:set_material(skintone)
    heel:translate(0, -0.3, -0.9)
    heel:scale(0.06, 0.04, 0.06)
    ankleJoint:add_child(heel)
end

------------------- Materials -------------------
brown = gr.material({0.5, 0.3, 0}, {0.1, 0.1, 0.1}, 1.0)
skintone = gr.material({0.7725, 0.549, 0.5216}, {0.1, 0.1, 0.1}, 1.0)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20)
stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0)
-- moon

eye = {0, 0, 8}
view = {0, 0, -30}
up = {0, 1, 0}
fov = 50

------------------- Lights -------------------
mainLight = gr.light({0, 202, 430}, {1, 1, 1}, {1, 0, 0})

------------------- Scene -------------------
scene = gr.node('scene')
scene:translate(0, 0, -2)
scene:rotate('X', 5)
scene:rotate('Y', 40)

------------------- Arches -------------------

inst = gr.node('inst')

arc = gr.node('arc')
inst:add_child(arc)
arc:translate(0, -1.7, -10)

p1 = gr.nh_box('p1', {0, 0, 0}, 1)
arc:add_child(p1)
p1:set_material(stone)
p1:scale(0.8, 4, 0.8)
p1:translate(-2.4, 0, -0.4)

p2 = gr.nh_box('p2', {0, 0, 0}, 1)
arc:add_child(p2)
p2:set_material(stone)
p2:scale(0.8, 4, 0.8)
p2:translate(1.6, 0, -0.4)

s = gr.nh_sphere('s', {0, 0, 0}, 1)
arc:add_child(s)
s:set_material(stone)
s:scale(4, 0.6, 0.6)
s:translate(0, 4, 0)

for i = 1, 6 do
    an_arc = gr.node('arc' .. tostring(i))
    an_arc:rotate('Y', (i-1) * 60)
    scene:add_child(an_arc)
    an_arc:add_child(arc)
 end

------------------- Puppet -------------------
torsoNode = gr.node('puppet')
torsoNode:translate(0, 0.7, 0)
scene:add_child(torsoNode)
drawPuppet()

------------------- Trusty Steed -------------------
cow_node = gr.node('cowNode')
cow_node:translate(0, 1, -2)
scene:add_child(cow_node)

factor = 2.0/(2.76+3.637)
cow_poly = gr.mesh('cow', 'cow.obj')
cow_poly:set_material(hide)
cow_poly:scale(factor, factor, factor)
cow_poly:translate(2.2, -1.53, 0.0)
cow_poly:rotate('Y', -90)
cow_node:add_child(cow_poly)

------------------- Ground -------------------
plane = gr.mesh('plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(grass)
plane:translate(0, -0.0545, 0)
plane:scale(30, 30, 30)

gr.render(scene,
	  'sample.png', 512, 512,
	  eye, view, up, fov,
	  {0.4, 0.4, 0.4}, {mainLight})
