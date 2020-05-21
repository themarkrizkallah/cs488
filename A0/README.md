# m2rizkal - A0 Submission

## Compilation
No changes with respect to compilation steps. 

Simply go into `cs488/` and run

```
$ premake4 gmake
$ make
```

and then go into `A0/` and run

```
$ premake4 gmake
$ make
```

## Manual
As per the assignment spec, I *only* made changes to the `A0.cpp` file. I would have liked to make some changes to `A0.h`, mainly consisting of adding a method to the `A0` class that handles resetting the triangle back to its original state. The relevant member variables are protected so I couldn't simply add a static function to A0.cpp to change those values.

With regards to the scale factor, I believe `0.5` was a reasonable factor. It can be easily changed by modifying `DEFAULT_SCALE` at the top of [`A0.cpp`](A0.cpp).

All assignment objectives were completed, relevant screenshot can be found here [here](screenshot.png).


