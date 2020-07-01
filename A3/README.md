# m2rizkal - A3 Submission

## Compilation
Added [`RotateCommand.hpp`](RotateCommand.hpp) and [`RotateCommand.cpp`](RotateCommand.cpp).

No changes with respect to compilation steps.

Simply go into `cs488/` and run

```
$ premake4 gmake
$ make
```

and then go into `A3/` and run

```
$ premake4 gmake
$ make
```

## Manual

All assignment objectives were completed, relevant screenshots can be found in the [screenshots](screenshots/) folder.

Required screenshot can be found [here](screenshots/screenshot.png).

### Puppet Hierarchy
The puppet's hierarchy (top-down) is described as follows:

```
rootNode (SceneNode)
    **** Neck *****
    neckNode (SceneNode)
        neckJoint (JointNode)
            neck (GeometryNode)

            **** Head ****
            headNode (SceneNode)
                headJoint (JointNode)
                    head (GeometryNode)

    **** Torso *****
    torso (GeometryNode)

    **** Torso Decorations *****
    ** For side in {left, "right} **
    sidePecNode (SceneNode)
        sidePecJoint (JointNode)
            sidePec (GeometryNode)

    ** For X in {1, 2, 3} **
    sideAbNodeX (SceneNode)
        sideAbJointX (JointNode)
            sideAbX (GeometryNode)

    **** Shoulders ****
    ** For side in {left, "right} **
    sideShoulderNode (SceneNode)
        sideShoulder (GeometryNode)
        sideShoulderJoint (JointNode)
            sideUpperArm (GeometryNode)

            **** Elbow ****
            sideElbowNode (SceneNode)
                sideElbow (GeometryNode)
                sideElbowJoint (JointNode)
                    sideForearm (GeometryNode)

                    **** Wrist ****
                    sideWristNode (SceneNode)
                        sideWristJoint (JointNode)
                            sideHand (GeometryNode)

    **** Waist ****
    waistNode (SceneNode)
        waistJoint (JointNode)
            waistGeometry (GeometryNode)

            **** Tail ****
            baseTailNode (SceneNode)
                baseTailJoint (JointNode)
                    baseTail (GeometryNode)
                    midTailNode (SceneNode)
                        midTailBall (GeometryNode)
                        midTailJoint (JointNode)
                                midTail (GeometryNode)
                                tailNode (SceneNode)
                                    tailBall (GeometryNode) 
                                    tailJoint (JointNode)   
                                        tail (GeometryNode)
            **** Hips ****
            ** For side in {left, "right} **
            sideHipNode (SceneNode)
                sideHip (GeometryNode)
                hipJoint (JointNode)
                    sideFemur (GeometryNode)

                    **** Knee ****
                    sideKneeNode (SceneNode)
                        sideKnee (GeometryNode)
                        sideKneeJoint (JointNode)
                            sideTibia (GeometryNode)

                            **** Ankle ****
                            sideAnkleNode (SceneNode)
                                sideAnkleJoint (JointNode)
                                    sideFoot (GeometryNode)
                                    sideHeel (GeometryNode)
    
```

**Note**: Indentations describe hierachy

`GeometryNodes` are always leaf nodes, this ensures `scale` operations never interfere with propagating transformations down the scene graph.

Furthermore, the puppet has more than 15 degrees of freedom as required. Individual abs and pecs can be manipulated!

## Picking and Selection
`JointNodes` are selected by clicking on any of their direct `GeometryNode` children. 

Selecting a `JointNode` highlights *all* of its direct `GeometryNode` children (exmample is the ankle joint). Similarly, deselecting a `JointNode` "unhighlights" *all* direct `GeometryNode` children.

`SceneNodes` are *never* selected. In the included puppet, `GeometryNodes` are used to visualize some `JointNodes`. This means that if a `GeometryNode` is a direct child of a `SceneNode`, clicking on it does *not* select it nor highlight it (cannot select the joint that way either). This way is aesthetically pleasing and consistent with the assignment description.

Selection records are stored in an `std::set` and in the appropriate `isSelected` field of the respective `SceneNode` object.

**Note**: Picking still works with `Z-buffer` turned off, but it might be a little harder to use.

## Culling
Culling works as expected. If both `Backface culling` and `Frontface culling` are enabled, the puppet is no longer on the screen; thus, picking is impossible. That being said, joints that are selected prior to both culling modes being enabled are still selected and can be manipulated.

## Data Structure Modifications
2 `float` fields are added to `JointNode::JointRange`, namely `cur` and `prev`.

- `cur` contains the *current* angle of the `JointRange`
- `prev` contains the immediate *previous* angle of the `JointRange`
 
This allows for simplifications in the `undo` and `redo` methods.

`SceneNode` and `GeometryNode` are untouched.

## Shader Modifications
The [vertex](Assets/VertexShader.vs) and [fragment](Assets/FragmentShader.fs) shaders were updated to use Phong shading (a-la the provided sample shaders). In addition, the [fragment](Assets/FragmentShader.fs) was modified to accept a `picking` boolean uniform that is used while picking.

## Position Mode
Trackball is implemented and functions exactly like the given [`Trackball Example`](../Trackball_Example/).

## Joints Mode
Joint rotations are represented as instances of `RotateCommand`, defined and implemented in [`RotateCommand.hpp`](RotateCommand.hpp) and [`RotateCommand.cpp`](RotateCommand.cpp) respectively.

**Note**: A `RotateCommand` cannot `execute` twice in a row without a call to `undo` in between. This protects against accidental repeated calls to `execute`.

Rotation angles around the x-axis (middle mouse button) and y-axis (right mouse button) are w.r.t. the relative y-motion of the mouse while the respective button is pressed. That being said, a `Natural Joint Control` toggle is available in the `Options` menu (`N`-key shortcut). While this mode is active, rotation angles around the y-axis are calculated w.r.t. the relative x-motion of the mouse while the right mouse button is pressed.

**Note**: At any point, clicking on `Reset Defaults` in the `Application` menu (`D`-key shortcut) resets the *entire* app to its default state.

## Undo/Redo
If a user tries to `undo` or `redo` past the end of the respective `RotateCommand` stack, a message appears in the UI, appropriately notifiying the user. The messages last for 1.5 seconds (can be modified in [`A3.cpp`](A3.cpp)). In addition, the `Edit` menu disables the `undo` and `redo` functionalities when the respective stacks are empty.

**Note**: User can continue to `undo` and `redo` outside of `Joints` mode since all joint angles persist between modes.

## Light Mode (Bonus Feature)
I implemented an additional user mode, `Light` (`L`-key shortcut). In this mode, the user can reposition the light by clicking the left mouse button and dragging up, down, left, and right. Just like moving the puppet in `Position` mode, those movements translate the light across the x-axis and y-axis as you would expect. In addition, the user can move the light across the z-axis by holding the right mouse button and moving the mouse up and down (up moves the light towards the -z axis, down is the opposite).

The RGB intensity of the light can be increased or decreased by scrolling up or down on the scroll wheel. Ambient intensity can also be increased or decreased by using the appropriate slider (appears when in `Light` mode) or by scrolling right or left on the *horizontal* scroll wheel (if the user's mouse supports this functionality).

Appropriate bounds for the RGB and Ambient intensities are defined at the top of [`A3.cpp`](A3.cpp).

**Note**: Any changes to the light can be reset by clicking on `Reset Light` (appears when in `Light` mode) or by the `Reset Defaults` functionality.