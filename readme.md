# RSD2OBJ - Wavefront .obj to PSX .rsd


**obj2rsd file.obj [-s1.0] [-v]**

   file.obj is your .obj 3D model file

   -s1.0 is a float value to scale model up or down, default is no scale 1.0.

   -v is debug/verbose output.


**Supported RSD output:**

	Triangles and Quads
	Colored polygon no texture
	Textured polygon
	Colored (tinted) textured polygon

**Not supported:**

	Lines or sprites primatives.
	Gradient (shaded) textured polygon
	Gradient filled polygon no texture


**Multiple groups in OBJ will be turned into a single group in RSD.**


RSD2OBJ.exe was built with:

	tcc -m32 obj2rsd.c -o obj2rsd.exes

## TODO:
	Vertext colour (Gradient shading/filling) support via
	https://paulbourke.net/dataformats/obj/colour.html

