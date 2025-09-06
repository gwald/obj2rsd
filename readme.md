# RSD2OBJ - Wavefront .obj to PSX .rsd

OBJ2RSD - Wavefront .obj to PSX .rsd - Sept 2025

**obj2rsd file.obj [-s1.0] [-v]**

   file.obj is your .obj 3D model file

   -s1.0 is a float value to scale model up or down, default is no scale 1.0.

   -v is debug/verbose output.

**Note:**

	1) Multiple groups in OBJ will be turned into a single group in RSD.
	2) If you use a texture, you have to have it converted correctly to TIM using the same filename from the .mtl file (map_Kd tag) - some old programs require it to be a DOS 8.3 filename.
	3) Output Filename will be the input filename and if it's greater than 8 chars it will be trimmed. 




**Supported RSD output:**

	Everything Net Yaroze RSD/TMD primative support.
	

	Flat coloured polygon, no texture
	Per Vertex colour ( aka gradient) filled polygon, no texture
	
	Colored (tinted) textured polygon
	Per Vertex colour ( aka gradient) shaded textured polygon

	The above works with 	Triangles or Quads, Lit (using face or per vert normals) or not.

**Not supported:**

	Lines or sprites primatives or double sided etc, as per the Net Yaroze limits.
	



RSD2OBJ.exe was built with:

	tcc -m32 obj2rsd.c -o obj2rsd.exes


	
## TIPS:

Per Vertext colour (Gradient shading/filling) support via blender obj exporter (verts: x,y,z,r,g,b)
	
	
Note that the PlayStation RSDtool does NOT display gradients (per vertex colours), you need to view it using something else like a "real" RSD/TMD viewer on PlayStation ie:
https://github.com/gwald/MarbleClay_TMD_PSX/blob/main/Plugin/PSX/RSDN.bat

Test your obj file with Assimp first to make sure it's correct.
