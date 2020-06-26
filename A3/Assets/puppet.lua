-- Unpack table t
function unpack (t, i)
    i = i or 1
    if t[i] ~= nil then
        return t[i], unpack(t, i + 1)
    end
end

-- Reverses scale s
function reverseScale(s)
    s[1] = 1/s[1]
    s[2] = 1/s[2]
    s[3] = 1/s[3]

    return s
end

---- Materials ----
red = gr.material({0.8, 0.2, 0.2}, {0.8, 0.8, 0.8}, 10.0)
green = gr.material({0.2, 0.8, 0.2}, {0.8, 0.8, 0.8}, 10.0)
blue = gr.material({0.2, 0.2, 0.8}, {0.8, 0.8, 0.8}, 10.0)

---- Top level scale ----
s = {0.75, 0.75, 0.75}

---- Torso ----
torso = gr.mesh('cube', 'torso')
torso:set_material(red)
torso:translate(0.0, 0, -5.0)
torso:scale(unpack(s))

---- Neck ----

-- Neck joint
neckJoint = gr.joint('neckJoint', {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0})
s = reverseScale(s) --  Revert parent's scale
neckJoint:scale(unpack(s))
neckJoint:translate(0.0, 0.375, 0.0)
torso:add_child(neckJoint)

-- Neck geometry
neck = gr.mesh('sphere', 'neck')
neck:set_material(green)
s = {0.07, 0.13, 0.07}
neck:scale(unpack(s))
neck:translate(0.0, 0.2, 0.0)
neckJoint:add_child(neck)

---- Head ----

-- Head joint
headJoint = gr.joint('headJoint', {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0})
s = reverseScale(s) --  Revert parent's scale
headJoint:scale(unpack(s))
headJoint:translate(0.0, 0.1, 0.0)
neck:add_child(headJoint)

-- Head
head = gr.mesh('sphere', 'head')
s = {0.2, 0.2, 0.2}
head:scale(0.3, 0.3, 0.3)
head:translate(0, 0.4, 0)
head:set_material(blue)
headJoint:add_child(head)

-- Return the root with all of it's childern.  The SceneNode A3::m_rootNode will be set
-- equal to the return value from this Lua script.
return torso
