created by:	Colin Dyson
Student #:	7683407

Assignment was created and tested with:
OS:		Windows 10 Home
GPU:	NVIDIA GeForce GTX 1050 Ti

Files: (located in /a2/src/)
teapot
splinemaker.cpp
teaset.cpp
fshader.f.glsl
vshader.v.glsl
teapotfshader.f.glsl
teapotvshader.v.glsl

Question 1:
--------------------------
Build:
Load 'opengl.sln' into Visual Studio
Ensure the files 'splinemaker.cpp', 'common.h', and 'main.cpp' are loaded into the project from the solution explorer (remove any others from project)
Build and run (Configuration: Debug, x86)

Click on the screen to add control points
Click on a control point to select, then again to move the selected control point
Press spacebar to change the spline drawing mode
Press q or esc to quit


Question 2:
---------------------------
Build:
Load 'opengl.sln' into Visual Studio
Ensure the files 'teaset.cpp', 'common.h', and 'main.cpp' are loaded into the project from the solution explorer (remove any others from project)
Build and run (Configuration: Debug, x86)

Left click to rotate around x axis
Right click to rotate around y axis
Middle click to rotate around z axis
Press spacebar to toggle between drawing the interpolated curve points, the given control points, or both
Press q or esc to quit

NOTE: The patches are not properly connected. I haven't figured out how to do that yet.
