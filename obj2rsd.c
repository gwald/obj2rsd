/*
 ============================================================================
 Name        : obj2rsd.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

//https://github.com/OneSadCookie/fcaseopen/blob/master/fcaseopen.c

#include <unistd.h>
#include <ctype.h>

void casechdir(char const *path)
{
#if !defined(_WIN32)
	char *r = alloca(strlen(path) + 2);
	if (casepath(path, r))
	{
		chdir(r);
	}
	else
	{
		errno = ENOENT;
	}
#else
	chdir(path);
#endif
}


#if !defined(_WIN32)
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>
#include <unistd.h>

// r must have strlen(path) + 2 bytes
static int casepath(char const *path, char *r)
{
	size_t l = strlen(path);
	char *p = alloca(l + 1);
	strcpy(p, path);
	size_t rl = 0;

	DIR *d;
	if (p[0] == '/')
	{
		d = opendir("/");
		p = p + 1;
	}
	else
	{
		d = opendir(".");
		r[0] = '.';
		r[1] = 0;
		rl = 1;
	}

	int last = 0;
	char *c = strsep(&p, "/");
	while (c)
	{
		if (!d)
		{
			return 0;
		}

		if (last)
		{
			closedir(d);
			return 0;
		}

		r[rl] = '/';
		rl += 1;
		r[rl] = 0;

		struct dirent *e = readdir(d);
		while (e)
		{
			if (strcasecmp(c, e->d_name) == 0)
			{
				strcpy(r + rl, e->d_name);
				rl += strlen(e->d_name);

				closedir(d);
				d = opendir(r);

				break;
			}

			e = readdir(d);
		}

		if (!e)
		{
			strcpy(r + rl, c);
			rl += strlen(c);
			last = 1;
		}

		c = strsep(&p, "/");
	}

	if (d) closedir(d);
	return 1;
}
#endif

FILE *fcaseopen(char const *path, char const *mode)
{
	FILE *f = fopen(path, mode);
#if !defined(_WIN32)
	if (!f)
	{
		char *r = alloca(strlen(path) + 2);
		if (casepath(path, r))
		{
			f = fopen(r, mode);
		}
	}
#endif
	return f;
}




// https://stackoverflow.com/questions/656542/trim-a-string-in-c
char *ltrim(char *s)
{
	while(isspace(*s)) s++;
	return s;
}

char *rtrim(char *s)
{
	char* back = s + strlen(s);
	while(isspace(*--back));
	*(back+1) = '\0';
	return s;
}

char *trim(char *s)
{
	return rtrim(ltrim(s));
}

char *string_compare(char *str, char *find) // return null if not found or start of find in str
{
	char *tstr, *tfind;
	char *ret;

	tstr = malloc(strlen(str)+1);
	tfind = malloc(strlen(find)+1);

	strcpy(tstr, str);
	strcpy(tfind, find);

	strupr(tstr);

	strupr(tfind);

	ret = strstr(tstr, tfind);

	if(ret)
	{
		return str+(ret-tstr); // correct offset
	}
	else
		return NULL; // not found

}


long file_length(FILE *handle)
{
	long retval;
	fseek(handle, 0, SEEK_END);
	retval = ftell(handle);
	fseek(handle, 0, SEEK_SET);
	return retval;
}



int LOG(char *format, ...)
{
	va_list args; // Declare a va_list variable to manage the variable arguments

	// Initialize the va_list 'args' to start at the argument after 'format'
	va_start(args, format);

	while (*format) // Loop through the format string
	{
		// If a format specifier is encountered
		if (*format == '%')
		{
			format++;
			if (*format == 'd')
			{
				// Fetch the next argument as an integer and print it
				printf("%d", va_arg(args, int));
			}
			else if (*format == 'f')
			{
				// Fetch the next argument as an integer and print it
				// printf("%f", va_arg(args, float));
			}
			else if (*format == 's')
			{
				// Fetch the next argument as a string and print it
				printf("%s", va_arg(args, char *));
			}
		}
		else
		{
			// Print regular characters
			putchar(*format);
		}
		format++; // Move to the next character
	}
	// Cleanup the va_list 'args' after processing
	va_end(args);

	fflush(0);
}



/*
 *
 *
 *
 *********************************************************************************************************************************************************************
 *
 *
 *
 * START
 *
 *
 *
 *
 *********************************************************************************************************************************************************************
 *
 *
 */

#define TOTAL_MAX_TIM_FILES
#define  TOTAL_MAX_PRIMATIVE_SIZE 1500000
#define OBJ2RSD_VERSION "OBJ2RSD - Wavefront .obj to PSX .rsd - Aug 2024 - First buggy version."



// Obj info form https://paulbourke.net/dataformats/obj/
// RSD info from fileformat.pdf

float g_scale;
char g_filename[255];
char g_verbose;


typedef struct mtl_s
{
	char *newmtl;
	int tim_height, tim_width, tim_depth;
	float r,g,b; // no w needed
	char *map_Kd; // texture
	char illum; // normal

}mtl;

mtl g_mtl_arr[1024];
int g_mtl_count;


int g_current_mtl;





// typedef float vert[3];

typedef struct vert_s
{
	float x,y,z; // no w needed

}vert;

// f v/vt/vn v/vt/vn v/vt/vn v/vt/vn
typedef struct point_s
{
	int vert, uv, normal; // no w needed


}point;


typedef struct face_s
{
	int mtl;
	point pnt[4]; // no w needed

}face;




int g_total_verts;
vert g_vert_arr[TOTAL_MAX_PRIMATIVE_SIZE];

int g_total_UVs;
vert g_UV_arr[TOTAL_MAX_PRIMATIVE_SIZE];

int g_total_normals;
vert g_normals_arr[TOTAL_MAX_PRIMATIVE_SIZE];

int g_total_faces;
face g_face_arr[TOTAL_MAX_PRIMATIVE_SIZE]={0};


//Primitives
#define V_TYPE 10 // VERT
#define VT_TYPE 20 // UV
#define VN_TYPE 30 // NORMAL

//Face
#define F_TYPE 40 // F V/VT/VN



int process_primative(int vert_UV_normal, char *line)
{

	vert v;
	int i;
	char *str_p;
	char *pch;
	float f;

	str_p = trim(line);
	// LOG("%s\n", line );
	//V1
	pch = strtok (str_p," /");
	// printf("\n pch: %s\n", pch);
	f = (float) strtod(pch , NULL);
	// printf("float: %.4f\n", f);
	// mesh->verts[vert_length].x = NUM_MUL number;
	v.x  = f;

	//V2
	pch = strtok (NULL," /");
	// printf("\n pch: %s\n", pch);
	f =  (float) strtod(pch, NULL);
	// printf("float: %.4f\n", f);
	// mesh->verts[vert_length].x = NUM_MUL number;
	v.y = f;


	//V3
	pch = strtok (NULL," /");
	if(pch)
	{
		// printf("\n pch: %s\n", pch);
		f =  (float)  strtod(pch, NULL);
		// printf("float: %.4f\n", f);
		// mesh->verts[vert_length].x = NUM_MUL number;
		v.z  = f;
	}
	else
		v.z = 0.0;

	if(g_verbose)
		printf("%.4f, %.4f, %.4f\n",v.x ,v.y, v.z );


	/*
	 *
				vt_x2 := (vtx2.y^);
				vt_y2 :=  (vtx2.z^);
				vt_z2 := (vtx2.x^);
	 *
	 */
	switch(vert_UV_normal)
	{
	case V_TYPE: // VERT
		g_vert_arr[g_total_verts].x = v.x; // v.z * g_scale;
		g_vert_arr[g_total_verts].y = v.y; // v.y * -g_scale;
		g_vert_arr[g_total_verts].z = v.z; // v.x * g_scale;
		g_total_verts++;
		break;

	case VT_TYPE: // UV
		g_UV_arr[g_total_UVs].x = v.x ;
		g_UV_arr[g_total_UVs].y = v.y ;
		g_UV_arr[g_total_UVs].z = v.z ;
		g_total_UVs++;
		break;

	case VN_TYPE: // NORMAL
		g_normals_arr[g_total_normals].x = v.x * g_scale;
		g_normals_arr[g_total_normals].y = v.y * g_scale;
		g_normals_arr[g_total_normals].z = v.z * g_scale;
		g_total_normals++;
		break;

	default:
		printf("ERROR!! Process_primative() bad line type <%d> Line: <%s>\n",vert_UV_normal, line );
		return -111;

	}


}



int process_face(char *line)
{

#if 0

	The following is a portion of a sample file for a four-sided face
	element:

	f 1/1/1 2/2/2 3/3/3 4/4/4

	Using v, vt, and vn to represent geometric vertices, texture vertices,
	and vertex normals, the statement would read:

	f v/vt/vn v/vt/vn v/vt/vn v/vt/vn

	If there are only vertices and vertex normals for a face element (no
			texture vertices), you would enter two slashes (//). For example, to
					specify only the vertex and vertex normal reference numbers, you would
					enter:

					f 1//1 2//2 3//3 4//4

					When you are using a series of triplets, you must be consistent in the
							way you reference the vertex data. For example, it is illegal to give
							vertex normals for some vertices, but not all.

#endif


							int i, place=0,slot=0,
							vert[4]={0},
							text[4]={0},
							norm[4]={0};

					char *str_p;
					char *pch;
					int face;




					str_p = trim(line);

					for(i=0;i<strlen(str_p);) // count quad 8 or 6 for 3 values  t 4 3 for only 2
					{


						switch(place)
						{
						case 0:
							vert[slot] = atoi(&str_p[i]) ;
							break;


						case 1:
							text[slot] = atoi(&str_p[i]) ;
							break;


						case 2:
							norm[slot] = atoi(&str_p[i]) ;
							break;

						}


						while(str_p[i]>47 && str_p[i]<58) // between 0-9
							i++;



						if(isspace(str_p[i]))
						{

							while(isspace(str_p[i]))
								i++;

							place=0;
							slot++;

							continue;
						}


						if(str_p[i]='/')
						{
							place++;
							i++;
							continue;
						}



					}






					if(g_verbose)
					{
						printf("%s\n",line);
						printf("verts: %d, %d, %d, %d\n",vert[0], vert[1], vert[2], vert[3] );
						printf("norm: %d, %d, %d, %d\n",norm[0], norm[1], norm[2], norm[3] );
						printf("text: %d, %d, %d, %d\n",text[0], text[1], text[2], text[3] );
					}




					//	g_mtl_arr[g_current_mtl].




					g_face_arr[g_total_faces].pnt[0].vert = vert[0]-1;
					g_face_arr[g_total_faces].pnt[1].vert = vert[1]-1;
					g_face_arr[g_total_faces].pnt[2].vert = vert[2]-1;
					g_face_arr[g_total_faces].pnt[3].vert = vert[3]-1;

					g_face_arr[g_total_faces].pnt[0].normal = norm[0];
					g_face_arr[g_total_faces].pnt[1].normal = norm[1];
					g_face_arr[g_total_faces].pnt[2].normal = norm[2];
					g_face_arr[g_total_faces].pnt[3].normal = norm[3];

					g_face_arr[g_total_faces].pnt[0].uv = text[0]-1;
					g_face_arr[g_total_faces].pnt[1].uv = text[1]-1;
					g_face_arr[g_total_faces].pnt[2].uv = text[2]-1;
					g_face_arr[g_total_faces].pnt[3].uv = text[3]-1;


					g_face_arr[g_total_faces].mtl = g_current_mtl;
					g_total_faces++;

}

void delete_old_files(void)
{
	char filename[512];
	int ret;



	sprintf(filename, "%s.rsd",g_filename);
	ret = remove("filename");

	if(ret)
	{
		// printf("ERROR: Can not remove %s\n", filename);
		//exit(1);
	}



	sprintf(filename, "%s.ply",g_filename);
	ret = remove("filename");

	if(ret)
	{
		// printf("ERROR: Can not remove %s\n", filename);
		//exit(1);
	}



	sprintf(filename, "%s.mat",g_filename);
	ret = remove("filename");

	if(ret)
	{
		// printf("ERROR: Can not remove %s\n", filename);
		//exit(1);
	}



	sprintf(filename, "%s.grp",g_filename);
	ret = remove("filename");

	if(ret)
	{
		// printf("ERROR: Can not remove %s\n", filename);
		//exit(1);
	}


}
int create_RSD(void)
{

	/*
	 * # created with RSD_Exporter (July 2024) for MarbleClay.
@RSD940102
PLY=new.ply
MAT=new.mat
GRP=new.grp
NTEX=1
TEX[0]=new0.tim
	 *
	 */


	//


	char filename[512];
	char line[1024];
	FILE *RSD_fp;
	int i;

	sprintf(filename, "%s.rsd",g_filename);


	RSD_fp = fcaseopen(filename, "wb+");
	if (!RSD_fp)
	{

		//fail
		printf("Could not create file <%s>\n",filename);
		return 999;
	}



	sprintf(line, "# Created with %s\n", OBJ2RSD_VERSION);
	fwrite(line,  strlen(line),1, RSD_fp ); //write size with the same alignment


	sprintf(line, "@RSD940102\n");
	fwrite(line,  strlen(line),1, RSD_fp ); //write size with the same alignment

	sprintf(line, "PLY=%s.ply\n", g_filename);
	fwrite(line,  strlen(line),1, RSD_fp ); //write size with the same alignment



	sprintf(line, "MAT=%s.mat\n", g_filename);
	fwrite(line,  strlen(line),1, RSD_fp ); //write size with the same alignment


	sprintf(line, "GRP=%s.grp\n", g_filename);
	fwrite(line,  strlen(line),1, RSD_fp ); //write size with the same alignment


	//	sprintf(line, "NTEX=%d\n", g_mtl_count);
	//	fwrite(line,  strlen(line),1, RSD_fp ); //write size with the same alignment

	{
		int cnt=0;
		char string[10240]={0};

		if(g_mtl_count > 0)
		{
			for(i=0;i<g_mtl_count; i++)
			{
				if( g_mtl_arr[i].map_Kd)
				{

					if(cnt)
						sprintf(string, "%s\nTEX[%d]=%s.tim\n", string,  cnt, g_mtl_arr[i].map_Kd);
					else
						sprintf(string, "TEX[%d]=%s.tim\n", cnt, g_mtl_arr[i].map_Kd);

					cnt++;

				}
			}



			sprintf(line, "NTEX=%d\n",cnt);
			fwrite(line,  strlen(line),1, RSD_fp ); //write size with the same alignment

			if(cnt)
			{
				fwrite(string,  strlen(string),1, RSD_fp ); //write size with the same alignment
			}
			else
			{
				sprintf(line, "TEX[%d]=\n",0);
				fwrite(line,  strlen(line),1, RSD_fp ); //write size with the same alignment
			}



		}

	}

	fflush(RSD_fp);
	fclose(RSD_fp);

	printf("%s created correctly.\n", filename);



}




int create_GRP(void)
{

	/*
	 * @GRP940102
# created with RSD_Exporter (July 2024) for MarbleClay.
# dummy GRP file - all faces belong to the same group
# Number of Groups
1
# Groups
faces 1 1
0-492

	 *
	 */


	//


	char filename[512];
	char line[1024];
	FILE *GRP_fp;
	int i;

	sprintf(filename, "%s.grp",g_filename);


	GRP_fp = fcaseopen(filename, "wb+");
	if (!GRP_fp)
	{

		//fail
		printf("Could not create file <%s>\n",filename);
		return 999;
	}



	sprintf(line, "# Created with %s\n", OBJ2RSD_VERSION);
	fwrite(line,  strlen(line),1, GRP_fp ); //write size with the same alignment


	sprintf(line, "@GRP940102\n");
	fwrite(line,  strlen(line),1, GRP_fp ); //write size with the same alignment

	sprintf(line, "# Number of Groups\n1\n# Groups\n");
	fwrite(line,  strlen(line),1, GRP_fp ); //write size with the same alignment

	sprintf(line, "faces 1 1\n0-%d\n", g_total_faces);
	fwrite(line,  strlen(line),1, GRP_fp ); //write size with the same alignment

	fflush(GRP_fp);
	fclose(GRP_fp);

	printf("%s created correctly.\n", filename);

}







int create_PLY(void)
{

	/*
	 *

# created with RSD_Exporter (July 2024) for MarbleClay.
@PLY940102
# Number of Items - verts, normals, polygons
296 492 492
#verts = 296

	 *
	 */


	//

	face f;
	char filename[512];
	char line[1024];
	FILE *PLY_fp;
	int i;

	sprintf(filename, "%s.ply",g_filename);


	PLY_fp = fcaseopen(filename, "wb+");
	if (!PLY_fp)
	{

		//fail
		printf("Could not create file <%s>\n",filename);
		return 999;
	}



	sprintf(line, "# Created with %s\n", OBJ2RSD_VERSION);
	fwrite(line,  strlen(line),1, PLY_fp ); //write size with the same alignment


	sprintf(line, "@PLY940102\n# Number of Items - verts, normals, polygons\n");
	fwrite(line,  strlen(line),1, PLY_fp ); //write size with the same alignment

	sprintf(line, "%d %d %d\n", g_total_verts, g_total_faces, g_total_faces);
	fwrite(line,  strlen(line),1, PLY_fp ); //write size with the same alignment

	sprintf(line, "#verts = %d\n", g_total_verts);
	fwrite(line,  strlen(line),1, PLY_fp ); //write size with the same alignment


	/*
	 *
	 *
	 	x := vtx.y^ * param_scale;
		y := vtx.z^ * -param_scale;
		z := vtx.x^ * param_scale;
	 *
	 *
	 */

	for(i=0;i<g_total_verts; i++)
	{
		sprintf( line, "%.4f %.4f %.4f\n", g_vert_arr[i].z * g_scale, g_vert_arr[i].y * -g_scale, g_vert_arr[i].x * g_scale);
		fwrite(line,  strlen(line),1, PLY_fp ); //write size with the same alignment
	}




	sprintf(line, "#\n#polygon normals = %d\n#\n", g_total_faces);
	fwrite(line,  strlen(line),1, PLY_fp ); //write size with the same alignment
	g_total_normals=4;





	//-- normal calculation from:
	//-- Net Yaroze usenet newsgroup post: Subject: Re: Calculating RSD Normals Subject: Re: Calculating Normals
	//-- get valid verts and faces
	for(i=0;i<g_total_faces; i++)
	{
		int quad =0;
		double vt_x1,vt_y1,vt_z1,
		vt_x2,vt_y2,vt_z2,
		vt_x3,vt_y3,vt_z3;

		double vt_cross_x, vt_cross_y, vt_cross_z;

		double vt_norm_x, vt_norm_y, vt_norm_z;


		double vert_len;



		if(g_face_arr[i].pnt[3].vert>0) // QUAD
			quad = 1;

		/*
		 *
 	g_vert_arr[g_total_verts].x = v.x; // v.z * g_scale;
				g_vert_arr[g_total_verts].y = v.y; // v.y * -g_scale;
				g_vert_arr[g_total_verts].z = v.z; // v.x * g_scale;
	for(i=0;i<g_total_verts; i++)
	{
		sprintf( line, "%.4f %.4f %.4f\n", g_vert_arr[i].z * g_scale, g_vert_arr[i].y * -g_scale, g_vert_arr[i].x * g_scale);
		fwrite(line,  strlen(line),1, PLY_fp ); //write size with the same alignment
	}
g_scale+0.05;
		 *
		 *
		 */



		vt_x1 = g_vert_arr[ g_face_arr[i].pnt[0].vert ].y *  g_scale;
		vt_y1 = g_vert_arr[ g_face_arr[i].pnt[0].vert ].z * g_scale;
		vt_z1 = g_vert_arr[ g_face_arr[i].pnt[0].vert ].x  *  g_scale;



		vt_x2 = g_vert_arr[ g_face_arr[i].pnt[1].vert ].y*  g_scale;
		vt_y2 = g_vert_arr[ g_face_arr[i].pnt[1].vert ].z*  g_scale;
		vt_z2 = g_vert_arr[ g_face_arr[i].pnt[1].vert ].x*  g_scale;



		vt_x3 = g_vert_arr[ g_face_arr[i].pnt[2].vert ].y*  g_scale;
		vt_y3 = g_vert_arr[ g_face_arr[i].pnt[2].vert ].z*  g_scale;
		vt_z3 = g_vert_arr[ g_face_arr[i].pnt[2].vert ].x*  g_scale;





		// use 4th vert instead of 3rd
		if(quad)
		{
			vt_x3 = g_vert_arr[ g_face_arr[i].pnt[3].vert ].y*  g_scale;
			vt_y3 = g_vert_arr[ g_face_arr[i].pnt[3].vert ].z*  g_scale;
			vt_z3 = g_vert_arr[ g_face_arr[i].pnt[3].vert ].x*  g_scale;
		}

		/*
		 *
							vt_x1 := (vtx1.y^);
						vt_y1 :=  (vtx1.z^);
						vt_z1 := (vtx1.x^);



		 *
g_scale=20.0;


		g_scale=1.0;

		vt_x1 = g_scale *vt_y1;
		vt_y1 =  -g_scale*vt_z1;
		vt_z1 = g_scale*vt_x1;


		vt_x2 =g_scale*vt_y2;
		vt_y2 =  - g_scale*vt_z2;
		vt_z2 =g_scale*vt_x2;


		vt_x3 =g_scale*vt_y3;
		vt_y3 = -g_scale*vt_z3;
		vt_z3 = g_scale*vt_x3;

		 */

		vt_cross_x = ((vt_y2-vt_y1)*(vt_z3-vt_z1))-((vt_z2-vt_z1)*(vt_y3-vt_y1));
		vt_cross_y = ((vt_z2-vt_z1)*(vt_x3-vt_x1))-((vt_x2-vt_x1)*(vt_z3-vt_z1));
		vt_cross_z = ((vt_x2-vt_x1)*(vt_y3-vt_y1))-((vt_y2-vt_y1)*(vt_x3-vt_x1));
		vert_len = ( (vt_cross_x*vt_cross_x ) + (vt_cross_y*vt_cross_y) + (vt_cross_z*vt_cross_z) );

		vert_len = sqrt( vert_len );

		vt_norm_x = vt_cross_x/vert_len;
		vt_norm_y = vt_cross_y/vert_len;
		vt_norm_z = vt_cross_z/vert_len;


		if(vt_norm_x != vt_norm_x)
			vt_norm_x = 0.0;


		if(vt_norm_y != vt_norm_y)
			vt_norm_y = 0.0;


		if(vt_norm_z != vt_norm_z)
			vt_norm_z = 0.0;


		/*
		 *


									temp := Format('v %f %f %f ',[vt_x1, vt_y1, vt_z1]);
									WriteString( objfile, temp);

									temp := Format('v %f %f %f ',[vt_x3, vt_y3, vt_z3]);
									WriteString( objfile, temp);

									temp := Format('v %f %f %f ',[vt_x2, vt_y2, vt_z2]);
									WriteString( objfile, temp);

		 *
		 */
		sprintf( line, "%.4f %.4f %.4f\n",
				vt_norm_y, -vt_norm_x, vt_norm_z	);
		fwrite(line,  strlen(line),1, PLY_fp ); //write size with the same alignment

	}

#if 0


	for(i=0;i<g_total_normals; i++)
	{
		sprintf( line,"0.0000 -1.0000 0.0000\n");// "%.4f %.4f %.4f\n", g_normals_arr[i].x, g_normals_arr[i].y, g_normals_arr[i].z);
		fwrite(line,  strlen(line),1, PLY_fp ); //write size with the same alignment
	}



	NY fileformat.pdf pg11.
	polygon group is composed of a flag for representing the type of a polygon, and eight parameters constituting the polygon.
	The meaning of the parameters varies with the type of polygon specified in theflag
	.Figure 1–6: PLY File Polygon Descriptor Flag Parameter #1 Parameter #2... Parameter #3
	Flag bit configuration bit 7 (MSB) 0 (LSB) TYP Polygon (Triangle or Quadrangle)

	The parameter section describes the vertices and normals for the polygon.
	Each vertex value is an integer index, numbered from zero, to the proper position of the vertex data within the vertex group.
	Normal values are a similar index into the normal group. For a polygon to be subjected to flat shading,
	the normal of each vertex has the same value, and the value of the first vertex is adopted. For a polygon to be subjected
	to smooth shading gourand, the normal of each vertex has a different value.The flag is a hexadecimal integer value
	( although not prefixed with “0x”, as would be expected) that specifies the type of polygon.

	For a triangular polygon, the data for the fourth vertex and normal are assigned a value of zero.
	For a quadrangular polygon, the vertices are described in the proper order so that the first three vertices form a triangle,
	and the second through fourth vertices form another triangle (i.e. to sub divide the quad as shown in Figure 2-7).
	Figure 1–7: Vertex ordering for quad subdivision 1432
	1-63D GraphicsFile FormatsFigure 1–8: PolygonVertex 0     Vertex 1     Vertex 2     Vertex 3     Normal 0     Normal 1     Normal
	2
	Normal 3FlagStraight line
	The parameter section describes the vertex numbers of two end points.
	Figure 1–9: Straight LineVertex 0     Vertex 1     Vertex 2     Vertex 3     Normal 0     Normal 1     Normal 2     Normal 3FlagSpriteA sprite in model data is rectangular image data located in a 3D space. It can be considered to be atextured polygon always facing the visual point.The parameter section describes vertices indicating sprite positions, and the width and height of images(sprite patterns).Figure 1–10: Sprite

#endif



	sprintf(line, "#\n#polygons=%d  0=tri 1=quad\n#\n", g_total_faces);
	fwrite(line,  strlen(line),1, PLY_fp ); //write size with the same alignment

	for(i=0;i<g_total_faces; i++)
	{
		if(g_face_arr[i].pnt[3].vert>0) // 1=quad
		{

			sprintf( line, "%d   %d %d %d %d  %d %d %d %d\n",
					1, g_face_arr[i].pnt[3].vert, g_face_arr[i].pnt[2].vert, g_face_arr[i].pnt[0].vert, g_face_arr[i].pnt[1].vert, i, i, i, i
			);
		}
		else
		{
			sprintf( line, "%d   %d %d %d %d  %d %d %d %d\n",
					0, g_face_arr[i].pnt[2].vert, g_face_arr[i].pnt[1].vert, g_face_arr[i].pnt[0].vert, 0, i, i, i, 0
			);

		}
		fwrite(line,  strlen(line),1, PLY_fp ); //write size with the same alignment





		//	uvline  :=  Format(' %.0f %.0f    %.0f %.0f    %.0f %.0f    %.0f %.0f   '
		//[ ( map_w * (vtx4.y^) ),  ( map_h * (1.0-vtx4.z^) ) ,			( map_w * (vtx3.y^) ),  ( map_h * (1.0-vtx3.z^) ) ,		( map_w * (vtx1.y^) ),  ( map_h * (1.0-vtx1.z^) ) ,		( map_w * (vtx2.y^) ),  ( map_h * (1.0-vtx2.z^) )  ]);

		//flag   Vertex 0     Vertex 1     Vertex 2     Vertex 3     Normal 0     Normal 1     Normal 2     Normal 3F
#if 0
		if(g_face_arr[i].pnt[3].vert>0) // QUAD
		{//0   134 135 133 133  204 204 204 200

			sprintf( line, "1 %d %d %d %d %d %d %d %d\n",
					g_face_arr[i].pnt[4].vert, g_face_arr[i].pnt[3].vert, g_face_arr[i].pnt[1].vert, g_face_arr[i].pnt[2].vert,
					g_face_arr[i].pnt[4].normal, g_face_arr[i].pnt[3].normal, g_face_arr[i].pnt[1].normal, g_face_arr[i].pnt[2].normal
			);
			fwrite(line,  strlen(line),1, PLY_fp ); //write size with the same alignment
		}
		else // TRI
		{ // 0   134 135 133 0  204 204 204 0
			//uvline   :=  Format(' %.0f %.0f    %.0f %.0f    %.0f %.0f    %.0f %.0f  ',[ ( map_w * (vtx3.y^) ),  ( map_h * (1.0-vtx3.z^) ) , 	( map_w * (vtx2.y^) ),  ( map_h * (1.0-vtx2.z^) ) ,	( map_w * (vtx1.y^) ),  ( map_h * (1.0-vtx1.z^) ) ,	0.0, 0.0  ]);

			sprintf( line, "0 %d %d %d %d %d %d %d %d\n",
					g_face_arr[i].pnt[3].vert, g_face_arr[i].pnt[1].vert, g_face_arr[i].pnt[2].vert,0,
					g_face_arr[i].pnt[3].normal, g_face_arr[i].pnt[1].normal, g_face_arr[i].pnt[2].normal,0
			);
			fwrite(line,  strlen(line),1, PLY_fp ); //write size with the same alignment

		}
#endif

	}


	fflush(PLY_fp);
	fclose(PLY_fp);

	printf("%s created correctly.\n", filename);

}

int create_MAT(void)
{





	/*
	 *
# created with RSD_Exporter (July 2024) for MarbleClay.
@MAT940801
# Number of Items
492
# Materials

	 *
	 */


	//

	face f;
	char filename[512];
	char line[1024];
	FILE *MAT_fp;
	int i;


	sprintf(filename, "%s.mat",g_filename);


	MAT_fp = fcaseopen(filename, "wb+");
	if (!MAT_fp)
	{

		//fail
		printf("Could not create file <%s>\n",filename);
		return 999;
	}



	sprintf(line, "# Created with %s\n", OBJ2RSD_VERSION);
	fwrite(line,  strlen(line),1, MAT_fp ); //write size with the same alignment


	sprintf(line, "@MAT940801\n# Number of Faces to material mapping\n");
	fwrite(line,  strlen(line),1, MAT_fp ); //write size with the same alignment

	sprintf(line, "%d\n",  g_total_faces);
	fwrite(line,  strlen(line),1, MAT_fp ); //write size with the same alignment


#if 0


# Materials
	0 0 F   T   0  14 21    28 21    33 43    0 0
	1 0 F   T   0  14 21    33 43    14 51    0 0
	2 0 F   T   0  0 39    14 21    14 51    0 0
	3 0 F   T   0  0 39    14 51    1 55    0 0
	4 0 F   T   0  14 21    0 39    1 55    0 0
	5 0 F   T   0  14 21    1 55    14 51    0 0

	Face# : i
	count of each face, correlates to count of face in .ply

	Flag: Normal check on verts
	0: Light source calculation supported
	1: Fixed col


	Shading: F (G not supported)
	This is an ASCII character indicating the shading mode.
	“F” = Flat shading (shading is based on the normal for the first vertex of the polygon, as specified in the PLYfile)
	“G” = Smooth shading Material information The format of the remainder of each line is different dpending on the material type.

	Material info: C, T or D
	There are several different material types. Each is designated by a special type code, as follows:
	Table 1–2 Type Meaning
	C Colored polygon/straight line, no texture
	G Gradient filled polygon/straight line, no texture
	T Textured polygon/sprite
	D Colored textured polygon
	H Gradient (shaded) textured polygon




	C  R G B
	R, G, B:  RGB components of polygon color (0 to 255


			T TNO  U0  V0   U1  V1  U2   V2    U3    V3
			extured Polygon/Sprite
			TYPETYPE:     Material type, whose value is "T"
			TNO:  TIM data file to be used (Texture number described in the RSD file)
			Un, Vn:   Position of vertex n in the texture space.
			For a triangular polygon,the value (U3, V3) of the fourth vertex is zero.

			D TNO U0 V0  U1 V1  U2 V2 U3 V3  R  G   B

			Colored Textured Polygon

			TYPETYPE: Material type, whose value is "D"
			TNO: TIM data file to be used. (Texture number described in the RSD file)
			Un, Vn: Position of vertex n in the texture space.
			For a triangular polygon, the value (U3, V3) of the fourth vertex is zero.
			R, G, B: RGB components of polygon color (0 to 255)* The colored textured polygon is used to make the texture of a polygon bright without light source calculation.
			This type allows the three-dimensional drawing of a textured object without light sourcecalculation.
			It is valid only in the fixed color light source calculation mode.


			NY fileformat.pdf pg12.


#endif



			sprintf(line, "# Materials %d\n", g_total_faces);
			fwrite(line,  strlen(line),1, MAT_fp ); //write size with the same alignment




			for(i=0;i<g_total_faces; i++)
			{

				char flag_material_c, flag_light;
				mtl *mtl_p;
				char RGB_str[512]={0};
				char isRGB_used = 0;

				char material_str[512]={0};
				int quad;


				if(g_face_arr[i].pnt[3].vert>0)
					quad = 1;
				else
					quad = 0;


				// default colour?
				sprintf(RGB_str, " %d %d %d ", 126,126,126);


				mtl_p = &g_mtl_arr[ g_face_arr[i].mtl ];
				flag_light = 1; // off

				if(mtl_p->b != 1.0 || mtl_p->r != 1.0 || mtl_p->g != 1.0 )
				{
					sprintf(RGB_str, "  %d %d %d ",  (int)(mtl_p->r*255.00), (int)(mtl_p->g*255.00), (int) (mtl_p->b*255.00) );
					isRGB_used = 1;
				}



				if(mtl_p->illum )
					flag_light=0;//On


				//material

				if(mtl_p->map_Kd) // uses a texture
				{
					char texture[255];
					float u1,v1, u2,v2, u3,v3, u4,v4;

					// T or D
					flag_material_c = 'T';


					u1 = g_UV_arr[g_face_arr[i].pnt[0].uv].x;
					v1 = 1.0- g_UV_arr[g_face_arr[i].pnt[0].uv].y;

					u2 = g_UV_arr[g_face_arr[i].pnt[1].uv].x;
					v2 = 1.0-g_UV_arr[g_face_arr[i].pnt[1].uv].y;

					u3 = g_UV_arr[g_face_arr[i].pnt[2].uv].x;
					v3 = 1.0-g_UV_arr[g_face_arr[i].pnt[2].uv].y;

					u4 = 0.0;
					v4= 0.0;
					if(g_verbose)
					{
						LOG("%d UV0 %d  UV1 %d  UV2 %d  UV3 %d \n",i,
								g_face_arr[i].pnt[0].uv,
								g_face_arr[i].pnt[1].uv,
								g_face_arr[i].pnt[2].uv,
								g_face_arr[i].pnt[3].uv );

#if 0
						printf("U %f V %f ", u1, v1);
						printf("U %f V %f ", u2, v2);

						printf("U %f V %f ", u3, v3);
						printf("U %f V %f \n", u4, v4);
						fflush(0);
#endif
					}


					if(quad)
					{
						//uvline  :=  Format(' %.0f %.0f    %.0f %.0f    %.0f %.0f    %.0f %.0f   ',
						///[ ( map_w * (vtx4.y^) ),  ( map_h * (1.0-vtx4.z^) ) ,
						//( map_w * (vtx3.y^) ),  ( map_h * (1.0-vtx3.z^) ) ,
						//( map_w * (vtx1.y^) ),  ( map_h * (1.0-vtx1.z^) ) ,
						//( map_w * (vtx2.y^) ),  ( map_h * (1.0-vtx2.z^) )  ]);

						u4 = g_UV_arr[g_face_arr[i].pnt[3].uv].x;
						v4 = 1.0-g_UV_arr[g_face_arr[i].pnt[3].uv].y;



						sprintf(texture, "%d %d   %d %d   %d %d  %d %d",
								(int)	(mtl_p->tim_width*(u4)),
								(int)	(mtl_p->tim_height*(v4)),
								(int)	(mtl_p->tim_width*(u3)),
								(int)	(mtl_p->tim_height*(v3)),
								(int)	(mtl_p->tim_width*(u1)),
								(int)	(mtl_p->tim_height*(v1)),
								(int)	(mtl_p->tim_width*(u2)),
								(int)	(mtl_p->tim_height*(v2))
						);

					}
					else
					{
						// uvline   :=  Format(' %.0f %.0f    %.0f %.0f    %.0f %.0f    %.0f %.0f  ',
						//[ ( map_w * (vtx3.y^) ),  ( map_h * (1.0-vtx3.z^) ) ,
						//( map_w * (vtx2.y^) ),  ( map_h * (1.0-vtx2.z^) ) ,
						//( map_w * (vtx1.y^) ),  ( map_h * (1.0-vtx1.z^) ) ,	0.0, 0.0  ]);

						u4 = 0.0;
						v4= 0.0;



						sprintf(texture, "%d %d   %d %d   %d %d  %d %d",
								(int)	(mtl_p->tim_width*(u3)),
								(int)	(mtl_p->tim_height*(v3)),
								(int)	(mtl_p->tim_width*(u2)),
								(int)	(mtl_p->tim_height*(v2)),
								(int)	(mtl_p->tim_width*(u1)),
								(int)	(mtl_p->tim_height*(v1)),
								(int)	0,
								(int)	0
						);

					}




					if(isRGB_used) // has RBG been set?
					{
						flag_material_c = 'D';

						//D TNO U0 V0  U1 V1  U2 V2 U3 V3  R  G   B
						sprintf(material_str, "%d %d   F   %c  %d   %s   %s \n",
								i,
								flag_light,
								// F
								flag_material_c,
								g_face_arr[i].mtl,
								texture,
								RGB_str
						);

					}
					else
					{
						flag_material_c = 'T';

						//# lit F T TNO U0 V0  U1 V1  U2 V2 U3 V3

						sprintf(material_str, "%d %d   F  %c  %d   %s  \n",
								i,
								flag_light,
								// F
								flag_material_c, // T
								g_face_arr[i].mtl, // TNO
								texture
						);

					}


				}
				else // no texture just a colour
				{
					// C  R G B
					flag_material_c = 'C';

					sprintf(material_str, "%d %d   F  %c  %s \n",
							i,
							flag_light,
							// F
							flag_material_c,
							RGB_str
					);
				}




				//	sprintf(line, "#newmtl %s %d \n", mtl_p->newmtl, g_face_arr[i].mtl);
				//	fwrite(line,  strlen(line),1, MAT_fp ); //write size with the same alignment

				fwrite(material_str,  strlen(material_str),1, MAT_fp ); //write size with the same alignment


			}



			fflush(MAT_fp);
			fclose(MAT_fp);

			printf("%s created correctly.\n", filename);

}



int mtllib(char *filename)
{
	char mat_line[512];
	FILE *mat_fp;
	int i, linestotal, file_size;
	char *str_p;

	if(g_verbose)
		LOG("Processing mtllib file <%s>\n",filename);


	mat_fp = fcaseopen(filename, "rb");
	if (!mat_fp)
	{

		//fail
		printf("Could not open mtllib file <%s>\n",filename);
		return 999;
	}

	// Read script into RAM
	file_size = file_length(mat_fp);

	if(g_verbose)
		printf("mtllib file <%s> opened, size %d\n", filename, file_size);

	g_mtl_count=-1;

	while( !feof(mat_fp) )
	{
		sprintf(mat_line,"#000000000000000000000");

		fgets(mat_line,512,mat_fp);
		linestotal++;




		/*
		 *
		 *
		 *
		 *
typedef struct mtl_s
{
	char *name;
	float r,g,b; // no w needed
	char *texture;

}mtl;

mtl mtl_arr[1024];
int g_mtl_count;
		 *
		 */



		str_p = trim(mat_line);



		if(str_p[0] == 0 || str_p[0] == '#')
			continue;// blank line or comment


		//	LOG("%s\n", mat_line);




		//scanned first!
		str_p = string_compare(mat_line, "newmtl");


		if(str_p)
		{
			g_mtl_count++;

			str_p = trim(str_p+strlen( "newmtl"));

			if(g_verbose)
				LOG("Processing newmtl: <%s> <%d>\n", str_p, g_mtl_count);

			g_mtl_arr[g_mtl_count].newmtl = malloc(strlen(str_p)+1);
			strcpy(g_mtl_arr[g_mtl_count].newmtl ,str_p);


			//newmtl bowser~256x256x8

			str_p = strstr(str_p, "~");
			if(str_p) // ~widthxhieghtxbitwidth
			{
				char *pch;
				str_p++;
				int cc, w,h,d;

				for (cc=0; cc<strlen(str_p); cc++)
				{
					str_p[cc] = (char) tolower(str_p[cc]);

				}


				pch = strtok (str_p,"x");
				// printf("\n pch: %s\n", pch);
				w = atoi(pch ) ;


				pch = strtok (NULL,"x");
				// printf("\n pch: %s\n", pch);
				h = atoi(pch ) ;


				pch = strtok (NULL,"x");
				// printf("\n pch: %s\n", pch);
				d = atoi(pch ) ;

				g_mtl_arr[g_mtl_count].tim_height=h;
				g_mtl_arr[g_mtl_count].tim_width=w;
				g_mtl_arr[g_mtl_count].tim_depth=d;


			}

			continue;
		}



		str_p = string_compare(mat_line, "illum");


		if(str_p)
		{

			str_p += strlen("illum");

			str_p =  trim(str_p);

			g_mtl_arr[g_mtl_count].illum = str_p[0];

			continue;
		}


		str_p = string_compare(mat_line, "map_Kd");

		if(str_p)
		{
			str_p = trim(str_p+strlen( "map_Kd"));


			if(g_mtl_count==-1)
			{
				printf("ERROR: File %s - Sections must start with newmtl.\n", filename);
				return 999;

			}

			g_mtl_arr[g_mtl_count].map_Kd =  malloc(strlen(str_p)+1);

			strcpy(g_mtl_arr[g_mtl_count].map_Kd,str_p);

			if(strstr(g_mtl_arr[g_mtl_count].map_Kd, "."))
			{
				*strstr(g_mtl_arr[g_mtl_count].map_Kd, ".") = 0; // remove .obj
			}

			if(g_verbose)
				LOG("g_TEXT_count: %d file <%s>\n", g_mtl_count, g_mtl_arr[g_mtl_count].map_Kd);


			continue;
		}




		str_p = string_compare(mat_line, "Kd");

		if(str_p)
		{

			if(g_mtl_count==-1)
			{
				printf("ERROR: File %s - Sections must start with newmtl.\n", filename);
				return 999;

			}
			str_p = trim(str_p+strlen( "Kd"));

			if(g_verbose)
				LOG("Processing Kd: <%s> <%d>\n", str_p, g_mtl_count);


			sscanf(str_p, "%f %f %f",
					&g_mtl_arr[g_mtl_count].r,
					&g_mtl_arr[g_mtl_count].g,
					&g_mtl_arr[g_mtl_count].b);


			continue;
		}



	}

	g_mtl_count++;

}

int main(int argc, char *argv[])
{
	FILE *obj_fp;
	int obj_size, linestotal;
	char obj_line[512];
	char *str_p;


	g_scale = 1.0;
	g_current_mtl=g_mtl_count=linestotal=g_verbose =0;

	printf("\n%s\nobj2rsd obj [-s1.0] [-v]\n"
			"  obj is your .obj 3D model file\n"
			"   -s1.0 is a float value to scale model up or down, default is no scale 1.0.\n"
			"   -v is debug/verbose output.\n"
			"See readme.md.\n"
			"\n",OBJ2RSD_VERSION);

	if( argc == 1)
		return 0;

	strcpy(g_filename, argv[1]);

	if(strstr(g_filename, "."))
	{
		*strstr(g_filename, ".") = 0; // remove .obj
	}

	//first check for verbose mode
	if( argc > 2)
	{
		if (strcmp(argv[2], "-v")==0 || strcmp(argv[2], "-V") ==0)
			g_verbose =1;

		if (argv[2][1] == 's' || argv[2][1] == 'S')
		{
			char *p = (argv[2]);
			p++;
			p++;
			if(g_verbose)
			{
				printf("Scale %s\n",p);
				fflush(0);
			}
			g_scale = (float) strtod(p, NULL);
		}

	}

	if( argc > 3)
	{
		if (strcmp(argv[3], "-v")==0 || strcmp(argv[3], "-V") ==0)
			g_verbose =1;

		if (argv[3][1] == 's' || argv[3][1] == 'S')
		{
			char *p = (argv[2]);
			p++;
			p++;
			if(g_verbose)
			{
				printf("Scale %s\n",p);
				fflush(0);
			}
			g_scale = (float) strtod(p, NULL);
		}


	}




	obj_fp = fcaseopen(argv[1], "rb");
	if (!obj_fp)
	{

		//fail
		printf("Could not open file <%s>\n", argv[1]);
		return 999;
	}



	// Read script into RAM
	obj_size = file_length(obj_fp);

	if(g_verbose)
		LOG("File <%s> opened, size %d\n", argv[1], obj_size);


	// fread((char *)obj_buffer, obj_size, 1, obj_fp);

	if(g_verbose)
		LOG("Processing mtllib files...\n");

	// do mtllib files.
	while( !feof(obj_fp) )
	{
		sprintf(obj_line,"#000000000000000000000");


		fgets(obj_line,512,obj_fp);
		linestotal++;



		str_p = trim(obj_line);


		if(str_p[0] == 0 || str_p[0] == '#')
			continue;// blank line or comment



		str_p = string_compare(obj_line, "mtllib");

		if(str_p)
		{
			str_p = trim(str_p+strlen( "mtllib"));
			mtllib(str_p);
			break;

		}



	}
	// do mtllib files.


	rewind(obj_fp);
	linestotal=0;

	if(g_verbose)
		LOG("Storing all verts, UVs and normals...\n");

	// process all primatives.
	while( !feof(obj_fp) )
	{

		sprintf(obj_line,"#000000000000000000000");



		fgets(obj_line,512,obj_fp);
		linestotal++;

		str_p = trim(obj_line);

		if(str_p[0] == 0 || str_p[0] == '#')
			continue;// blank line or comment


		if(str_p[0] == 'v' && str_p[1] == ' ')
		{

			if(g_verbose)
				printf("line: %d %s Vert: ", linestotal,str_p);

			process_primative(V_TYPE, str_p+2);
		}
		else if(str_p[0] == 'v' && str_p[1] == 't')
		{
			if(g_verbose)
				printf("line: %d %s UV: ",  linestotal,str_p);

			process_primative(VT_TYPE, str_p+2);
		}
		else if(str_p[0] == 'v' && str_p[1] == 'n')
		{
			if(g_verbose)
				printf("line: %d %s Normal: ", linestotal, str_p);

			process_primative(VN_TYPE, str_p+2);
		}

	}



	rewind(obj_fp);
	linestotal=0;
	g_current_mtl = 0; // default first entry

	if(g_verbose)
		LOG("Processing all faces...\n");

	// process all faces.
	while( !feof(obj_fp) )
	{

		sprintf(obj_line,"#000000000000000000000");

		fgets(obj_line,512,obj_fp);
		linestotal++;

		str_p = trim(obj_line);


		if(str_p[0] == 0 || str_p[0] == '#')
			continue;// blank line or comment


		//usemtl
		if(str_p[0] == 'u' && str_p[1] == 's' && str_p[2] == 'e' && str_p[3] == 'm' && str_p[4] == 't' && str_p[5] == 'l')
		{
			int j;


			str_p = trim(str_p+strlen( "usemtl"));

			if(g_verbose)
				printf("line: %d <%s> \n", linestotal,str_p);

			//g_current_mtl=g_mtl_count
			for(j=0; j<g_mtl_count; j++)
			{
				if(strstr(g_mtl_arr[j].newmtl,str_p))
				{
					g_current_mtl = j;
					break;
				}

			}

			continue;
		}



		str_p = trim(obj_line);

		if(str_p[0] == 'f' && str_p[1] == ' ')
		{
			process_face(str_p+2);

			continue;
		}

	}





	if(g_verbose)
		LOG("OBJ Stats- Verts: %d UVs: %d Normals: %d Faces: %d\n", g_total_verts, g_total_UVs, g_total_normals, g_total_faces);



	fclose(obj_fp);

	delete_old_files();

	create_RSD();
	create_GRP();
	create_PLY();
	create_MAT();

}
