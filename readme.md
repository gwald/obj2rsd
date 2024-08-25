#RSD2OBJ - Wavefront .obj to PSX .rsd
Build with:
tcc -m32 obj2rsd.c -o obj2rsd.exe

**obj2rsd obj [-s1.0] [-v]**

   obj is your .obj 3D model file

   -s1.0 is a float value to scale model up or down, default is no scale 1.0.

   -v is debug/verbose output.


**Supported RSD faces are:**

	C Colored polygon/straight line, no texture
	T Textured polygon/sprite
	D Colored textured polygon

**Not supported are:	**

	H Gradient (shaded) textured polygon
	G Gradient filled polygon/straight line, no texture


**Multiple groups in OBJ will be turned into a single group in RSD.**