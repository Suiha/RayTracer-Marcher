# RayTracer & RayMarcher

A continuation of my raytracing project of a 3D scene of objects (planes and spheres), where both raytracing and raymarching is used to render the scene with lights and textures are applied to objects. Shading is implemented using lambert and phong shading. Lights include point lights and area lights, the latter of which creates a soft shadow effect. Textures are applied using a diffuse map and specular map (textures sourced from https://www.sketchuptextureclub.com/).

Additionally, raymarching is used to render 3D fractals such as mandelbulbs and menger sponges.

User interaction is enabled through the GUI panels. Selection of objects and lights can be made using the mouse; the object properties (such as position, color, size, and texture) can then be changed through their corresponding GUI panel, and objects can be also be moved by selecting it and dragging the mouse. The user is free to add more objects (currently only planes and sphere) and lights to the scene, or delete selected objects from the scene. The camera that the scene is rendered through can also be updated to match the current camera position. 

Coded using C++ and the OpenFrameworks library.
