# m2rizkal - A2 Submission

## Compilation
No changes with respect to compilation steps.

Simply go into `cs488/` and run

```
$ premake4 gmake
$ make
```

and then go into `A2/` and run

```
$ premake4 gmake
$ make
```

## Manual

All assignment objectives were completed, relevant screenshots can be found in the [screenshots](screenshots/) folder.

## Model
The x, y, and z axes of the model frame are coloured red, blue, and green respectively.

## World
The x, y, and z axes of the world frame are coloured cyan, magenta, and yellow respectively.

## View
Initial view direction is defined under `generateViewMatrix()` . 
The `lookAt` and `lookFrom` points are `[0, 0, 0, 1]`<sup>`T`</sup> and `[0, 0, 9, 1]`<sup>`T`</sup> respectively (*WCS* co-ordinate system).

## Bonus Features
I implemented an additional user mode, `ShearModel` (`H`-key shortcut), to shear the model with respect to a particular plane. The *left mouse button* shears the model about its x-axis, *middle mouse button* shears the model about its y-axis, and the *right mouse button* shears the model about its z-axis. Since the shear also applies to the gnomon axes, any transformations to the model will be relative to the new, sheared axes.

**Note:** Depending on the extent of the shear, moving the mouse in the opposite direction may not completely reverse the shear, so a *reset* will likely be required.
