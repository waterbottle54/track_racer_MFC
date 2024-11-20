#include "stdafx.h"
#include "surface.h"
#define _USE_MATH_DEFINES
#include <math.h>		// for calculation of normal vector
#include <cmath>
#include <malloc.h>		// for allocation of vertex array
#pragma warning(disable:4996)



void normal(double a[3], double b[3], double c[3], double normal[3])
{
	double ba[3];
	double ca[3];
	double n[3];
	double len;

	// calcalate two vectors from three given points
	ba[0] = b[0] - a[0]; 
	ba[1] = b[1] - a[1]; 
	ba[2] = b[2] - a[2];
	ca[0] = c[0] - a[0]; 
	ca[1] = c[1] - a[1]; 
	ca[2] = c[2] - a[2];

	// calculate perpendicular vector of the vectors
	n[0] = ba[1] * ca[2] - ca[1] * ba[2];
	n[1] = ca[0] * ba[2] - ba[0] * ca[2];
	n[2] = ba[0] * ca[1] - ca[0] * ba[1];

	// normalize perpendicular vector
	len = sqrt(n[0]*n[0] + n[1] * n[1] + n[2] * n[2]);
	normal[0] = n[0] / len; 
	normal[1] = n[1] / len;
	normal[2] = n[2] / len;
}



HSURFACE CreateSurface
(
	SURFACE_FUNC func, void* param,
	double u_0, double u_1,
	double v_0, double v_1,
	int n_u, int n_v,
	double mat[16], double org[3], 
	COLORREF clr
)
{
	SURFACE_DATA*	psur;
	GLdouble		*pv, *pn;
	double			u, v;
	double			d_u, d_v;
	int				i_u, i_v;

	// create surface data
	psur = (SURFACE_DATA*)malloc(sizeof(SURFACE_DATA));
	psur->avt = (GLdouble*)malloc(sizeof(GLdouble) * (3 * n_u*n_v));
	psur->anm = (GLdouble*)malloc(sizeof(GLdouble) * (3 * n_u*n_v));
	psur->n_u = n_u;
	psur->n_v = n_v;
	psur->org[0] = org[0];
	psur->org[1] = org[1];
	psur->org[2] = org[2];
	psur->clr = clr;
	if (!psur->avt || !psur->anm)
	{
		DeleteSurface((HSURFACE)psur);
		return NULL;
	}

	// calculate all vertice
	pv = psur->avt;
	d_u = (u_1 - u_0) / (n_u - 1);
	d_v = (v_1 - v_0) / (n_v - 1);
	for (i_u = 0, u = u_0; i_u < n_u; i_u++, u += d_u)
	{
		for (i_v = 0, v = v_0; i_v < n_v; i_v++, v += d_v)
		{
			func(u, v, pv, param);
			pv += 3;
		}
	}

	// apply matrix to all vertice
	pv = psur->avt;
	for (i_u = 0; i_u < n_u; i_u++)
	{
		for (i_v = 0; i_v < n_v; i_v++)
		{
			MultiplyMatrix(mat, pv, pv, false);
			pv += 3;
		}
	}

	// calculate normal vector of all vertice
	pv = psur->avt;
	pn = psur->anm;
	for (i_u = 0; i_u < n_u; i_u++)
	{
		for (i_v = 0; i_v < n_v; i_v++)
		{
			if (i_v < n_v - 1 && i_u < n_u - 1)
				normal(pv, pv + 3 * n_v, pv + 3, pn);
			else if (i_v == n_v - 1 && i_u == n_u - 1)
				normal(pv, pv - 3 * n_v, pv - 3, pn);
			else if (i_v == n_v - 1)
				normal(pv, pv - 3, pv + 3 * n_v, pn);
			else if (i_u == n_u - 1)
				normal(pv, pv + 3, pv - 3 * n_v, pn);

			pv += 3;
			pn += 3;
		}
	}
	return (HSURFACE)psur;
}

HSURFACE CreateSurface(SURFACE_CREATION_INFO& sci)
{
	HSURFACE hsur;
	hsur = CreateSurface(
		sci.func, sci.param, 
		sci.u_0, sci.u_1, sci.v_0, sci.v_1, 
		sci.n_u, sci.n_v, 
		sci.mat, sci.org, sci.clr);
	return hsur;
}

int LoadSurface(HSURFACE* ahsur, const wchar_t* filename)
{
	FILE*			pf;
	SURFACE_DATA*	psur;
	int				size, i;

	pf = _wfopen(filename, L"r");
	if (pf == NULL)
		return 0;

	fscanf(pf, "%d", &size);
	for (i = 0; i < size; i++)
	{
		psur = (SURFACE_DATA*)malloc(sizeof(SURFACE_DATA));

		fscanf(pf, "%d %d %lf %lf %lf %d",
			&psur->n_u, &psur->n_v,
			&psur->org[0], &psur->org[1], &psur->org[2],
			&psur->clr);

		psur->avt = (GLdouble*)malloc(sizeof(GLdouble) * (3 * psur->n_u*psur->n_v));
		psur->anm = (GLdouble*)malloc(sizeof(GLdouble) * (3 * psur->n_u*psur->n_v));

		for (int i = 0; i < psur->n_u*psur->n_v; i++)
		{
			fscanf(pf, "%lf %lf %lf ", &psur->avt[3 * i], &psur->avt[3 * i + 1], &psur->avt[3 * i + 2]);
		}
		for (int i = 0; i < psur->n_u*psur->n_v; i++)
		{
			fscanf(pf, "%lf %lf %lf ", &psur->anm[3 * i], &psur->anm[3 * i + 1], &psur->anm[3 * i + 2]);
		}

		ahsur[i] = (HSURFACE)psur;
	}

	fclose(pf);
	return size;
}

bool SaveSurface(HSURFACE* ahsur, int size, const wchar_t* filename)
{
	FILE*			pf;
	SURFACE_DATA*	psur;
	int				i;

	pf = _wfopen(filename, L"w");
	if (pf == NULL)
		return false;

	fprintf(pf, "%d ", size);
	for (i = 0; i < size; i++)
	{
		psur = (SURFACE_DATA*)ahsur[i];

		fprintf(pf, "%d %d %lf %lf %lf %d ",
			psur->n_u, psur->n_v,
			psur->org[0], psur->org[1], psur->org[2],
			psur->clr);

		for (int i = 0; i < psur->n_u*psur->n_v; i++)
		{
			fprintf(pf, "%lf %lf %lf ", psur->avt[3 * i], psur->avt[3 * i + 1], psur->avt[3 * i + 2]);
		}
		for (int i = 0; i < psur->n_u*psur->n_v; i++)
		{
			fprintf(pf, "%lf %lf %lf ", psur->anm[3 * i], psur->anm[3 * i + 1], psur->anm[3 * i + 2]);
		}
	}

	fclose(pf);
	return true;
}

void DeleteSurface(HSURFACE hsur)
{
	if (hsur != NULL)
	{
		free(hsur->avt);
		free(hsur->anm);
		free(hsur);
		hsur = NULL;
	}
}

void RenderSurface(HSURFACE hsur)
{
	SURFACE_DATA* psur;
	GLuint *id, *pi;
	GLdouble *pv, *pn;
	int n_u, n_v;
	int i_u, i_v;

	psur = (SURFACE_DATA*)hsur;

	// allocate array of indice
	n_u = psur->n_u;
	n_v = psur->n_v;
	id = (GLuint*)malloc(sizeof(GLuint) * (2 * n_v*(n_u - 1)));
	if (id == NULL)
		return;

	// define all indices (zigzag) 
	pi = id;
	for (i_u = 0; i_u < n_u - 1; i_u++)
	{
		if ((i_u % 2) == 0)
		{
			for (i_v = 0; i_v < n_v; i_v++)
			{
				*pi++ = i_u*n_v + i_v;
				*pi++ = (i_u + 1)*n_v + i_v;
			}
		}
		else
		{
			for (i_v = n_v - 1; i_v >= 0; i_v--)
			{
				*pi++ = i_u*n_v + i_v;
				*pi++ = (i_u + 1)*n_v + i_v;
			}
		}
	}

	// render surface polygons
	glColor3ub(GetRValue(psur->clr), GetGValue(psur->clr), GetBValue(psur->clr));
	glVertexPointer(3, GL_DOUBLE, 0, psur->avt);
	glNormalPointer(GL_DOUBLE, 0, psur->anm);
	glDrawElements(GL_TRIANGLE_STRIP, 2 * n_v*(n_u - 1), GL_UNSIGNED_INT, id);

	// delete memory
	delete[] id;
}

void BorderSurface(HSURFACE hsur)
{
	SURFACE_DATA* psur;
	GLdouble *pv, *pn;
	int n_u, n_v;
	int i_u, i_v;

	psur = (SURFACE_DATA*)hsur;
	n_u = psur->n_u;
	n_v = psur->n_v;

	// render border of surface 
	pv = psur->avt;
	pn = psur->anm;
	glBegin(GL_LINE_LOOP);
	glColor3ub(GetRValue(psur->clr), GetGValue(psur->clr), GetBValue(psur->clr));
	for (i_v = 0; i_v < n_v - 1; i_v++)
	{
		glNormal3dv(pn += 3);
		glVertex3dv(pv += 3);
	}
	for (i_u = 0; i_u < n_u - 1; i_u++)
	{
		glNormal3dv(pn += 3 * n_v);
		glVertex3dv(pv += 3 * n_v);
	}
	for (i_v = 0; i_v < n_v - 1; i_v++)
	{
		glNormal3dv(pn -= 3);
		glVertex3dv(pv -= 3);
	}
	for (i_u = 0; i_u < n_u - 1; i_u++)
	{
		glNormal3dv(pn -= 3 * n_v);
		glVertex3dv(pv -= 3 * n_v);
	}
	glEnd();


}


void GetSurfaceOrigin(HSURFACE hsur, double org[3])
{
	org[0] = hsur->org[0];
	org[1] = hsur->org[1];
	org[2] = hsur->org[2];
}

void SetSurfaceOrigin(HSURFACE hsur, double x, double y, double z)
{
	hsur->org[0] = x;
	hsur->org[1] = y;
	hsur->org[2] = z;
}

void SetSurfaceColor(HSURFACE hsur, COLORREF clr)
{
	hsur->clr = clr;
}

COLORREF GetSurfaceColor(HSURFACE hsur)
{
	return hsur->clr;
}


void TransformSurface(HSURFACE hsur, double mat[16])
{
	GLdouble *pv, *pn;
	int i_u, i_v;
	int n_u, n_v;

	n_u = hsur->n_u;
	n_v = hsur->n_v;

	// apply matrix to all vertice
	pv = hsur->avt;
	for (i_u = 0; i_u < n_u; i_u++)
	{
		for (i_v = 0; i_v < n_v; i_v++)
		{
			MultiplyMatrix(mat, pv, pv, false);
			pv += 3;
		}
	}

	// apply matrix to origin point
	MultiplyMatrix(mat, hsur->org, hsur->org, false);

	// recalculate normal vector of all vertice
	pv = hsur->avt;
	pn = hsur->anm;
	for (i_u = 0; i_u < hsur->n_u; i_u++)
	{
		for (i_v = 0; i_v < hsur->n_v; i_v++)
		{
			if (i_v < n_v - 1 && i_u < n_u - 1)
				normal(pv, pv + 3 * n_v, pv + 3, pn);
			else if (i_v == n_v - 1 && i_u == n_u - 1)
				normal(pv, pv - 3 * n_v, pv - 3, pn);
			else if (i_v == n_v - 1)
				normal(pv, pv - 3, pv + 3 * n_v, pn);
			else if (i_u == n_u - 1)
				normal(pv, pv + 3, pv - 3 * n_v, pn);

			pv += 3;
			pn += 3;
		}
	}
}

void MultiplyMatrix(double mat[16], double* obj, double* dest, bool mat_or_vec)
{
	double x[16], y[16];
	if (mat_or_vec == true)
	{
		memcpy(x, mat, sizeof(double) * 16);
		memcpy(y, obj, sizeof(double) * 16);
		dest[0] = (x[0] * y[0]) + (x[1] * y[4]) + (x[2] * y[8]) + (x[3] * y[12]);
		dest[1] = (x[0] * y[1]) + (x[1] * y[5]) + (x[2] * y[9]) + (x[3] * y[13]);
		dest[2] = (x[0] * y[2]) + (x[1] * y[6]) + (x[2] * y[10]) + (x[3] * y[14]);
		dest[3] = (x[0] * y[3]) + (x[1] * y[7]) + (x[2] * y[11]) + (x[3] * y[15]);
		dest[4] = (x[4] * y[0]) + (x[5] * y[4]) + (x[6] * y[8]) + (x[7] * y[12]);
		dest[5] = (x[4] * y[1]) + (x[5] * y[5]) + (x[6] * y[9]) + (x[7] * y[13]);
		dest[6] = (x[4] * y[2]) + (x[5] * y[6]) + (x[6] * y[10]) + (x[7] * y[14]);
		dest[7] = (x[4] * y[3]) + (x[5] * y[7]) + (x[6] * y[11]) + (x[7] * y[15]);
		dest[8] = (x[8] * y[0]) + (x[9] * y[4]) + (x[10] * y[8]) + (x[11] * y[12]);
		dest[9] = (x[8] * y[1]) + (x[9] * y[5]) + (x[10] * y[9]) + (x[11] * y[13]);
		dest[10] = (x[8] * y[2]) + (x[9] * y[6]) + (x[10] * y[10]) + (x[11] * y[14]);
		dest[11] = (x[8] * y[3]) + (x[9] * y[7]) + (x[10] * y[11]) + (x[11] * y[15]);
		dest[12] = (x[12] * y[0]) + (x[13] * y[4]) + (x[14] * y[8]) + (x[15] * y[12]);
		dest[13] = (x[12] * y[1]) + (x[13] * y[5]) + (x[14] * y[9]) + (x[15] * y[13]);
		dest[14] = (x[12] * y[2]) + (x[13] * y[6]) + (x[14] * y[10]) + (x[15] * y[14]);
		dest[15] = (x[12] * y[3]) + (x[13] * y[7]) + (x[14] * y[11]) + (x[15] * y[15]);
	}
	else
	{
		memcpy(x, mat, sizeof(double) * 16);
		memcpy(y, obj, sizeof(double) * 3);
		dest[0] = (x[0] * y[0]) + (x[1] * y[1]) + (x[2] * y[2]) + (x[3] * 1);
		dest[1] = (x[4] * y[0]) + (x[5] * y[1]) + (x[6] * y[2]) + (x[7] * 1);
		dest[2] = (x[8] * y[0]) + (x[9] * y[1]) + (x[10] * y[2]) + (x[11] * 1);
	}
}

void IdentityMatrix(double mat[16])
{
	double m[16] =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	memcpy(mat, m, sizeof(m));
}

void TranslationMatrix(double mat[16], double x, double y, double z)
{
	double m[16] =
	{
		1, 0, 0, x,
		0, 1, 0, y,
		0, 0, 1, z,
		0, 0, 0, 1
	};
	memcpy(mat, m, sizeof(m));
}

void RotationMatrix(double mat[16], double ox, double oy, double oz, double angle, char axis)
{
	double a = angle * (M_PI/180);
	double m_x[16] =
	{
		1,		0,		 0, 0,
		0, cos(a), -sin(a), 0,
		0, sin(a),  cos(a), 0,
		0,		0,		 0, 1
	};
	double m_y[16] =
	{
		cos(a), 0, -sin(a), 0,
			 0,	1,		 0, 0,
		sin(a), 0,  cos(a), 0,
			 0, 0,		 0, 1
	};
	double m_z[16] =
	{
		cos(a), -sin(a), 0, 0,
		sin(a),  cos(a), 0, 0,
			 0,		  0, 1, 0,
			 0,		  0, 0, 1
	};
	double m_t[16];

	TranslationMatrix(mat, -ox, -oy, -oz);
	if (axis == 'x') MultiplyMatrix(m_x, mat, mat, true);
	else if (axis == 'y') MultiplyMatrix(m_y, mat, mat, true);
	else if (axis == 'z') MultiplyMatrix(m_z, mat, mat, true);
	TranslationMatrix(m_t, ox, oy, oz);
	MultiplyMatrix(m_t, mat, mat, true);
}

void ScalingMatrix(double mat[16], double ox, double oy, double oz, double x, double y, double z)
{
	double m[16] =
	{
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1
	};
	double m_t[16];

	TranslationMatrix(mat, -ox, -oy, -oz);
	MultiplyMatrix(m, mat, mat, true);
	TranslationMatrix(m_t, ox, oy, oz);
	MultiplyMatrix(m_t, mat, mat, true);
}


bool PtOnSurface(HSURFACE hsur, double x, double y, double z, double err)
{
	int i_u, i_v;
	int n_u, n_v;
	GLdouble* pv;

	n_u = hsur->n_u;
	n_v = hsur->n_v;

	// check if any vertex that contects with given point exists
	pv = hsur->avt;
	for (i_u = 0; i_u < n_u; i_u++)
	{
		for (i_v = 0; i_v < n_v; i_v++)
		{
			if (abs(*pv++ - x) < err &&
				abs(*pv++ - y) < err &&
				abs(*pv++ - z) < err)
				return true;
		}
	}
	return false;
}


