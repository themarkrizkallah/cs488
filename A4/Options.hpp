#pragma once

// Comment this #define to hide the progress updates in cout
// Note: Might gain a *slight* performance boost if disabled
#define SHOW_PROGRESS

// Comment this #define to disable multithreading
#define ENABLE_MULTITHREADING

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

// Default bounding volume is a bounding box
#define BOUNDING_VOLUME BoundingBox