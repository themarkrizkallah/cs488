# m2rizkal - A4 Submission

## Compilation
No changes with respect to compilation steps.

Simply go into `cs488/` and run

```
$ premake4 gmake
$ make
```

and then go into `A4/` and run

```
$ premake4 gmake
$ make
```

## Manual

All assignment objectives were completed, required screenshots can be found in the [Assets/](Assets/) folder.

## Bounding Volumes
I implemented both *Bounding Spheres* and *Bounding Boxes*. *Bounding Boxes* is enabled by default, but you can change it by modifying the `BOUNDING_VOLUME` preprocessor variable according to the enum `BoundingVolume` in [Options.hpp](Options.hpp).

To illustrate both, I rendered the appropriately named [nonhier-bs.png](Assets/nonhier-bs.png) and [machow-cows-bb.png](Assets/machow-cows-bb.png) to illustrate bounding spheres and bounding boxes respectively. 

*Rendering* bounding volumes can be enabled in [Options.hpp](Options.hpp) by uncommenting `#define RENDER_BOUNDING_VOLUMES`.

**Note**: Bounding Volume acceleration can be disabled entirely in [Options.hpp](Options.hpp) by commenting `#define ENABLE_BOUNDING_VOLUMES`. Performance will suffer as a result.

## Supersampling (*Selected* Additional Feature)
For the required additional feature, I implemented *supersampling*. Relevant flag is in [Options.hpp](Options.hpp). It is disabled by default, but can be enabled by uncommenting `#define ENABLE_SUPERSAMPLING`. The supersampling
factor is defined in the same options file.

**Note**: The novel scene, [sample.png](Assets/sample.png), is rendered with *supersampling* **on**

## Novel Scene
My novel scene in [sample.png](Assets/sample.png) is my `A3` puppet riding a horse, err, cow by the stonehenge. The image has a resolution of `512x512`. By popular demand, `1080p`, `1440p`, and `4k` *supersample*d wallpapers of this scene are available in the [Images/](Images/) folder.

**Note**: I rendered a non-supersampled version of [sample.lua](Assets/sample.lua) in the same directory, [sample-no-ss.png](Assets/sample-no-ss.png), for comparison.

## Bonus Features

### Reflections
*Reflections* can be enabled in [Options.hpp](Options.hpp) by uncommenting `#define ENABLE_REFLECTIONS`. To visualize it, I rendered [nonhier.lua](Assets/nonhier.lua) and placed various PNGs in the [Image/](Image/) folder. The files are as follows:
- [original.png](Images/original.png) is a normal `512x512` rendering of [nonhier.lua](Assets/nonhier.lua)
- [reflections.png](Images/reflections.png) is a rendering of [nonhier.lua](Assets/nonhier.lua) with *reflections* enabled
- [reflections-ss.png](Images/reflections-ss.png) is a rendering of [nonhier.lua](Assets/nonhier.lua) with *reflections* and *supersampling* enabled
- [supersampling.png](Images/supersampling.png) is a rendering of [nonhier.lua](Assets/nonhier.lua) with just *supersampling* enabled

I rendered [macho-cows-ss-reflections.png](Assets/macho-cows-ss-reflections.png) as well as [mucho-macho-cows.lua](Assets/mucho-macho-cows.lua) with both *reflections* and *supersampling* enabled. Light attenuation is also implemented.

### Multithreading and progress Indicator
I implemented multithreading in order to speed up rendering times. This option is enabled by default and can be disabled in [Options.hpp](Options.hpp) by commenting `#define ENABLE_MULTITHREADING`. 

Furthermore, I implemented a progress indicator that outputs the percentage of pixels rendered. This is enabled by default and can be disabled in [Options.hpp](Options.hpp) by commenting `#define SHOW_PROGRESS`.

**Note**: I never issue more worker threads than the hardware concurrency limit defined in `<thread>`