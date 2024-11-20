#pragma once
#include <gl/GL.h>



typedef void(*SURFACE_FUNC)(double, double, double[3], void*);

struct SURFACE_DATA
{
	GLdouble*	avt;
	GLdouble*	anm;
	int			n_u, n_v;
	double		org[3];
	COLORREF	clr;
};

struct SURFACE_CREATION_INFO
{
	SURFACE_FUNC	func;
	void*			param;
	double			u_0, u_1;
	double			v_0, v_1;
	int				n_u, n_v;
	double			org[3];
	double			mat[16];
	COLORREF		clr;
};

typedef SURFACE_DATA* HSURFACE;



HSURFACE CreateSurface
(
	SURFACE_FUNC func, void* param,
	double u_0, double u_1,
	double v_0, double v_1,
	int n_u, int n_v,
	double mat[16], double org[3],
	COLORREF clr
);

HSURFACE CreateSurface(SURFACE_CREATION_INFO& sci);

int LoadSurface(HSURFACE* ahsur, const wchar_t* filename);

bool SaveSurface(HSURFACE* ahsur, int size, const wchar_t* filename);

void DeleteSurface(HSURFACE hsur);

void RenderSurface(HSURFACE hsur);

void BorderSurface(HSURFACE hsur);


void GetSurfaceOrigin(HSURFACE hsur, double org[3]);

void SetSurfaceOrigin(HSURFACE hsur, double x, double y, double z);

void SetSurfaceColor(HSURFACE hsur, COLORREF clr);

COLORREF GetSurfaceColor(HSURFACE hsur);



void TransformSurface(HSURFACE hsur, double mat[16]);

void MultiplyMatrix(double mat[16], double* obj, double* dest, bool mat_or_vec);

void IdentityMatrix(double mat[16]);

void TranslationMatrix(double mat[16], double x, double y, double z);

void RotationMatrix(double mat[16], double ox, double oy, double oz, double angle, char axis);

void ScalingMatrix(double mat[16], double ox, double oy, double oz, double x, double y, double z);


bool PtOnSurface(HSURFACE hsur, double x, double y, double z, double err);


