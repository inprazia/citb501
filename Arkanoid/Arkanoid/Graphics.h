#ifndef __GRAPHICS_H_
#define __GRAPHICS_H_

#include <windows.h>			// Windows API Definitions
#include <vector>

#include <gl/gl.h>									// Header File For The OpenGL32 Library
#include <gl/glu.h>									// Header File For The GLu32 Library

#include "utils.h"
#include "math.h"

enum RGB_COMPS
{
	RED, 
	GREEN, 
	BLUE, 
	ALPHA
};

/// the tga header (18 bytes!)
#pragma pack(push,1) // turn off data boundary alignment
struct TGAHeader
{
	// sometimes the tga file has a field with some custom info in. This 
	// just identifies the size of that field. If it is anything other
	// than zero, forget it.
	unsigned char m_iIdentificationFieldSize;
	// This field specifies if a colour map is present, 0-no, 1 yes...
	unsigned char m_iColourMapType;
	// only going to support RGB/RGBA/8bit - 2, colour mapped - 1
	unsigned char m_iImageTypeCode;
	// ignore this field....0
	unsigned short m_iColorMapOrigin;
	// size of the colour map
	unsigned short m_iColorMapLength;
	// bits per pixel of the colour map entries...
	unsigned char m_iColourMapEntrySize;
	// ignore this field..... 0
	unsigned short m_iX_Origin;
	// ignore this field..... 0
	unsigned short m_iY_Origin;
	// the image fWidth....
	unsigned short m_iWidth;
	// the image height.... 
	unsigned short m_iHeight;
	// the bits per pixel of the image, 8,16,24 or 32
	unsigned char m_iBPP;
	// ignore this field.... 0
	unsigned char m_ImageDescriptorByte;
};
#pragma pack(pop)

#define PIXEL_COMP_GRAY	1
#define PIXEL_COMP_RGB	3
#define PIXEL_COMP_RGBA	4

#define REPLACE(a,b) (a)^=(b)^=(a)^=(b)

void SetMemAlign(int nMemWidth, BOOL bPack);
void CreateLight(GLenum id, GLfloat *position = NULL, GLfloat *ambient = NULL, GLfloat *diffuse = NULL, GLfloat *specular = NULL, GLfloat *attenuate = NULL);

GLvoid DrawSphere(float R, int nDivs = 6);
void DrawCube(float side);

GLvoid DrawFrame(
		float x, float y, float z,
		float r = 1.0f, float w = 1);
GLvoid DrawLine3D(
		float x1, float y1, float z1, 
		float x2, float y2, float z2,
		int c1 = 0xffffffff, int c2 = 0, float w = 1);
GLvoid DrawCircle3D(
		float xc, float yc, float zc, float fRadius, int nColor = 0xffffffff,
		float xn = 0, float yn = 0, float zn = 1,
		float line = 1, int nDivs = 12);
GLvoid DrawSpline3D(
		const Point *coef, int nColor = 0xffffffff,
		float line = 1, int nDivs = 12);
GLvoid DrawLine2D(
		float x1, float y1, 
		float x2, float y2,
		int c = 0xffffffff, float w = 1);
void DrawBox(
		float left, float top,
		float fWidth, float height,
		DWORD nColor = 0, float line = 0);
void FillBox(
		float left, float top,
		float fWidth, float height,
		DWORD nColor = 0);

void DrawBox(
	int left, int top,
	int fWidth, int height,
	DWORD nColor = 0, float line = 0);
void FillBox(
	int left, int top,
	int fWidth, int height,
	DWORD nColor = 0);

ErrorCode glErrorToStr();

inline void SetColor(DWORD nColor)
{
	glColor4ubv(INT_TO_BYTE(nColor));
}

class Transform
{
public:
	GLint view[4];
	GLdouble model[16],project[16];
	void Update()
	{
		glGetIntegerv(GL_VIEWPORT, view);
		glGetDoublev(GL_MODELVIEW_MATRIX, model);
		glGetDoublev(GL_PROJECTION_MATRIX, project);
	}
	ErrorCode GetObjectCoor(double winX, double winY, double winZ, double &objX, double &objY, double &objZ)
	{
		gluUnProject(
			winX, winY, winZ,
			model, project, view,
			&objX, &objY, &objZ);
		return glErrorToStr();
	}
	ErrorCode GetWindowCoor(double objX, double objY, double objZ, double &winX, double &winY, double &winZ)
	{
		gluProject(
			objX, objY, objZ,
			model, project, view,
			&winX, &winY, &winZ);
		return glErrorToStr();
	}
	ErrorCode GetWindowDepth(LONG winX, LONG winY, double &winZ)
	{
		glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_DOUBLE, &winZ);
		return glErrorToStr();
	}
};

class Image
{
	std::vector<char> m_vBuffer;
	WORD m_uWidth, m_uHeight;
	BYTE m_uComps;
public:
	Image():m_uWidth(0), m_uHeight(0), m_uComps(PIXEL_COMP_RGB){}

	char *GetDataPtr() { return m_vBuffer.size() > 0 ? &m_vBuffer[0] : NULL; }
	const char *GetDataPtr() const { return m_vBuffer.size() > 0 ? &m_vBuffer[0] : NULL; }
	size_t GetDataSize() const { return m_vBuffer.size(); }
	WORD GetWidth() const { return m_uWidth; }
	WORD GetHeight() const { return m_uHeight; }
	BYTE GetComp() const { return m_uComps; }
	void Clear() { m_vBuffer.clear(); }
	
	BOOL SetSize(int nNewWidth, int nNewHeight, int nNewComp = PIXEL_COMP_RGB);
	
	void FlipV();
	void FlipC();

	// Read TGA format rgb or rgba image
	ErrorCode ReadTGA(FILE* fp);
	ErrorCode ReadTGA(const char *pchFileName);

	GLuint GetPixelFormat() const
	{
		switch(m_uComps)
		{
		case PIXEL_COMP_RGBA:
			return GL_RGBA;
		case PIXEL_COMP_RGB:
			return GL_RGB;
		default:
			return GL_LUMINANCE;
		}
	}
	void Draw(float x, float y) const;
};

class Texture
{
protected:
	GLuint				id; // GL Texture Identification
public:
	BOOL				mipmapped;
	GLint				fWidth; // For Not Mipmapped Must be 2^n + 2(border) for some integer n. 
	GLint				height;	// For Not Mipmapped Must be 2^m + 2(border) for some integer m. 
	GLint				levelDetail; // The level-of-detail number. Level 0 is the base image level. Level n is the nth mipmap reduction image. 
    GLint				components; // The number of nColor components in the texture. Must be 1, 2, 3, or 4. 	
	GLint				border;	// The fWidth of the border. Must be either 0 or 1. 
    GLuint				format; // GL_COLOR_INDEX, GL_STENCIL_INDEX, GL_DEPTH_COMPONENT,GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE,GL_LUMINANCE_ALPHA, GL_BGR_EXT, GL_BGRA_EXT 
    GLuint				dataFormat; // GL_UNSIGNED_BYTE, GL_BYTE, GL_BITMAP, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, and GL_FLOAT	
	GLint				magFilter; // Mag Filter To Use (GL_NEAREST, GL_LINEAR)
	GLint				minFilter; // Min Filter To Use (GL_NEAREST, GL_LINEAR)
	GLint				wrap; // Clamped or Repeated (GL_CLAMP, GL_REPEAT)

	Texture():
		fWidth(0), height(0), id(-1), mipmapped(TRUE), levelDetail(0), border(0),
		minFilter(GL_LINEAR_MIPMAP_NEAREST), magFilter(GL_LINEAR), wrap(GL_REPEAT),
		components(PIXEL_COMP_RGB), dataFormat(GL_UNSIGNED_BYTE), format(GL_RGB)
	{}
	~Texture() { Destroy(); }
	ErrorCode Create(const Image &image);
	void Destroy();
	ErrorCode Bind() const;
	int GetID() const { return id; }
};

class DisplayList
{
	GLuint m_uList; // Display List
public:
	DisplayList():m_uList(0){}
	~DisplayList(){ Destroy(); }
	ErrorCode Generate()
	{
		if( m_uList )
			return NULL;
		m_uList = glGenLists(1);
		return glErrorToStr();
	}
	void Destroy()
	{
		if(m_uList)
		{
			glDeleteLists(m_uList, 1);
			m_uList = 0;
		}
	}
	ErrorCode Start()
	{
		ErrorCode err = Generate();
		if(err)
			return err;
		glNewList(m_uList, GL_COMPILE);
		return glErrorToStr();
	}
	ErrorCode End() const
	{
		if( !m_uList )
			return FALSE;
		glEndList();
		return glErrorToStr();
	}
	void Execute() const
	{
		if( m_uList )
			glCallList(m_uList);
	}
	operator GLuint(){return m_uList;}
};

class CompileDisplayList
{
protected:
	DisplayList &m_list;
public:
	CompileDisplayList(DisplayList &list):m_list(list)
	{
		m_list.Start();
	}
	~CompileDisplayList()
	{
		m_list.End();
	}
};

#ifdef _DEBUG
void DbgDraw();
void DbgClear();
void DbgAddVector(
	Point ptPos, Point ptDir,
	int nColor1 = 0xffffffff, int nColor2 = 0, float fWidth = 1.0f);
void DbgAddCircle(
	Point ptCenter, float fRadius, int nColor = 0xffffffff,
	Point ptNormal = Point(0, 0, 1), float fWidth = 1.0f, int nDivs = 12);
void DbgAddSpline(
	Point ptPos1, Point ptDir1, Point ptPos2, Point ptDir2,
	int nColor = 0xffffffff, float fWidth = 1.0f,
	float fStep = 0, int nDivs = 12);
void DbgAddSpline(
	Point *ptSpline,
	int nColor = 0xffffffff, float fWidth = 1.0f,
	float fStep = 0, int nDivs = 12);
#else
#	define DbgDraw() ((void)0)
#	define DbgClear() ((void)0)
#	define DbgAddVector(...) ((void)0)
#	define DbgAddCircle(...) ((void)0)
#	define DbgAddSpline(...) ((void)0)
#endif _DEBUG

#endif __GRAPHICS_H_