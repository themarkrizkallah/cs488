#pragma once

// Comment this #define to hide the progress updates in cout
// Note: Might gain a *slight* performance boost if disabled
#define SHOW_PROGRESS

// Comment this #define to disable multithreading
#define ENABLE_MULTITHREADING

/** Bounding Volumes **/

// Comment this #define to disable bounding volume acceleration
#define ENABLE_BOUNDING_VOLUMES

// Uncomment this #define to render bounding volumes around objects
//  * Bounding volume type is defined in Mesh.hpp
//  * Automatically disabled if ENABLE_BOUNDING_VOLUMES is not defined
// #define RENDER_BOUNDING_VOLUMES

enum BoundingVolume {
	BoundingBox,
	BoundingSphere
};

// Default bounding volume is a bounding box (do not comment this out)
#define BOUNDING_VOLUME BoundingBox


/** Supersampling (Main Additional Feature)**/
// Comment this #define to enable Supersampling
// #define ENABLE_SUPERSAMPLING

// Super sampling factor
#ifdef ENABLE_SUPERSAMPLING
#define SS_FACTOR 3.0
#endif

/** Reflection (BONUS) **/

// Uncomment this #define to enable reflections
// #define ENABLE_REFLECTIONS

#ifdef ENABLE_REFLECTIONS
// Comment this to use the default MAX_HITS in A4.hpp (1)
#define MAX_HITS 5
#define REFLECTION_MIX_FACTOR 0.25
#endif
