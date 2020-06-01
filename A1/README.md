# m2rizkal - A1 Submission

## Compilation
No changes with respect to compilation steps. 

Simply go into `cs488/` and run

```
$ premake4 gmake
$ make
```

and then go into `A1/` and run

```
$ premake4 gmake
$ make
```

## Manual

All assignment objectives were completed, relevant screenshots can be found [here](screenshot.png), [here](screenshot1.png), [here](screenshot2.png), and [here](screenshot3.png).

## Avatar
Avatar is a UV-Sphere as required.

With regards to the avatar's starting position, I assumed `(0,0)` to be the top left corner of the grid. This aligns well with how `Maze::printMaze()` treats `(0,0)`. Thus, with respect to the Maze's location on the grid, the avatar is diagonal to the top-left corner of the maze.

**Note:** `(0,0)` is in `(row, col)` standard.

## Shift key functionality
With regards to the shift key functionality, the right or left shift keys can be used. Either one is sufficient to trigger Hulk mode!

## Dig and Reset buttons
The `Dig` button digs a new maze and moves the avatar to the start of the maze. The `Reset` button clears the maze and all walls as well as moves the avatar to `(0,0)`.

### Growing/Shrinking bars
Space and Backspace keys increase block height by 1 unit as expected. With regards to *how* the walls grow/shrink, I kept track of the required height in a `blockHeight` field and rendered **multiple** layers of blocks on top of each other. This functionality could have been also implemented by incrementing the `y` values of the vertices used in rendering the "top face". That being said, updating the `y` values would have required constant rewriting of the buffer object containing the cube. Since I was unsure of which one was the better approach, I simply chose to render multiple layers of blocks if applicable.

Furthermore, there was ambiguity regarding `blockHeight` being set to 0 and what that means for the avatar's movement restrictions. The following is a description of the current behaviour that I believe to be reasonable:

If a maze is dug and the block height was set to 0, the avatar is able to move wherever it wants. If the avatar moves on a tile that has a wall with block height is set to 0 and then the block height is increased, the avatar will appear to "clip" through the block. This is expected because they're both rendered on the same time. That being said, the avatar can still be moved to a valid surrounding tile (if it exists). If somehow the avatar is in a block and is surrounded by other blocks, it can be moved in one of the following 2 ways:
1. Hold shift to destroy surrounding blocks
2. Set block height to 0 again and then move the avatar to safety

**Note**: You can always click dig or reset to get out of this sticky situation!

`MAX_BLOCK_HEIGHT` is set to `10.0f` by default and can be changed. The reason it's a `float` is because I used `glm::clamp` to ensure that `blockHeight` is always in the interval `[0.0f, MAX_BLOCK_HEIGHT]`. If you wish to change `MAX_BLOCK_HEIGHT`, ensure that it is a non-negative integer value.

### Rotation
Rotation and persistence work as expected. I made the assumption that while the mouse is dragging and has moved from the moment the dragging begun, the grid is to continuously rotate at the appropriate rate. This means that if you stop moving the mouse while still dragging (holding the left mouse button), the grid will still rotate. If the mouse is not moving while the left mouse button is released, rotation is stopped. If the left mouse button is released while the mouse is moving, `persistence` is set to `true` and the grid rotates until the next left mouse button click or until the user resets the maze.


## Bonus Features
I implemented a shortest-path maze solver using BFS. This required **additions** to the `Maze` class. No functionality was taken away or modified.

The user can click the `Solve Maze` button **after** the maze is dug. The avatar will move to the maze's entrance and then move 1 tile per frame. Since the movement speed is tied to the framerate, I added a checkbox `Slow Maze Solver` that sleeps for `100ms` between frames in order for the user to more easily see the avatar's movement. Obviously, this can tank the framerate which is only apparent if the user wants to rotate or zoom in/out while the solver is active. If `Slow Maze Solver` is unchecked, `appLogic` no longer sleeps and the framerate shoudn't be affected. Once the maze solver is finished, `Slow Maze Solver` is automatically unchecked.

At any time, the user can use the arrow keys to take over movement of the avatar and wander around the maze to their heart's content. In other words, arrow keys interrupt the maze solver and unchecks `Slow Maze Solver`.

**Note:** Framerate is **only** slowed if the maze solver is *actively* running **and** `Slow Maze Solver` is checked. Even if `Slow Maze Solver` is checked and the maze solver has finished running or hasn't been triggered, framerate is unaffected.
