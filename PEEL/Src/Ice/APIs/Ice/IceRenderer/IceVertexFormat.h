///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains Vertex-Format code.
 *	\file		IceVertexFormat.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEVERTEXFORMAT_H
#define ICEVERTEXFORMAT_H

	typedef unsigned int VertexFormat;

	#define VF_POSITION_MASK	0x00E
	#define VF_TEXCOUNT_MASK	0xf00
	#define VF_TEXCOUNT_SHIFT	8

	#define VF_TEXTUREFORMAT2	0			//!< Two floating point values
	#define VF_TEXTUREFORMAT1	3			//!< One floating point value
	#define VF_TEXTUREFORMAT3	1			//!< Three floating point values
	#define VF_TEXTUREFORMAT4	2			//!< Four floating point values

	#define VF_TEXCOORDSIZE3(CoordIndex)	(VF_TEXTUREFORMAT3 << (CoordIndex*2 + 16))
	#define VF_TEXCOORDSIZE2(CoordIndex)	(VF_TEXTUREFORMAT2)
	#define VF_TEXCOORDSIZE4(CoordIndex)	(VF_TEXTUREFORMAT4 << (CoordIndex*2 + 16))
	#define VF_TEXCOORDSIZE1(CoordIndex)	(VF_TEXTUREFORMAT1 << (CoordIndex*2 + 16))

	// Vertex helpers
	//
	// Word list:
	// P = vertex position
	// R = RHW
	// N = normal
	// D = diffuse color
	// S = specular color
	// T = texture uv
	//
	// For more complex vertex formats, use your own structures.

	//																Vertex		1/w			Normal		Diffuse			Specular		Texture0		Texture1
	//! Point + Normal + UV
	struct ICERENDERER_API PNT_Vertex : public Allocateable		{	Point p;				Point n;									float u,v;						};
	//! Point + diffuse color
	struct ICERENDERER_API PD_Vertex : public Allocateable		{	Point p;							udword color;													};
	//! Point + diffuse color + UV
	struct ICERENDERER_API PDT_Vertex : public Allocateable		{	Point p;							udword color;					float u,v;						};
	//! Point + normal
	struct ICERENDERER_API PN_Vertex : public Allocateable		{	Point p;				Point n;																	};
	//! Point + UV
	struct ICERENDERER_API PT_Vertex : public Allocateable		{	Point p;															float u,v;						};
	//! Vertex-color vertex
	struct ICERENDERER_API PNDT_Vertex : public Allocateable	{	Point p;				Point n;	udword color;					float u,v;						};
	//! Transformed vertex
	struct ICERENDERER_API PRT_Vertex : public Allocateable		{	Point p;	float rhw;												float u,v;						};
	//! 
	struct ICERENDERER_API PRD_Vertex : public Allocateable		{	Point p;	float rhw;				udword color;													};
	//! 
	struct ICERENDERER_API PRDT_Vertex : public Allocateable	{	Point p;	float rhw;				udword color;					float u,v;						};
	//! Standard TL vertex
	struct ICERENDERER_API PRDST_Vertex : public Allocateable	{	Point p;	float rhw;				udword color;	float specular;	float u,v;						};
	//!
	struct ICERENDERER_API PRDTT_Vertex : public Allocateable	{	Point p;	float rhw;				udword color;					float u0,v0;	float u1,v1;	};


	//! Vertex Buffer format flags
	enum VBFormatFlag
	{
		VF_XYZ					= 0x002,	//!< Vertex format includes the position of an untransformed vertex.
		VF_XYZRHW				= 0x004,	//!< Vertex format includes the position of a transformed vertex.
		VF_NORMAL				= 0x010,	//!< Vertex format includes a vertex normal vector. Can't be used with the VF_XYZRHW flag.
		VF_PSIZE				= 0x020,	//!< Vertex format specified in point size. This size is expressed in camera space units for
											//!< vertices that are not transformed and lit, and in device-space units for transformed and lit vertices.
		VF_DIFFUSE				= 0x040,	//!< Vertex format includes a diffuse color component.
		VF_SPECULAR				= 0x080,	//!< Vertex format includes a specular color component.
		VF_TEX0					= 0x000,	//!< Number of texture coordinate sets for the vertex...
		VF_TEX1					= 0x100,
		VF_TEX2					= 0x200,
		VF_TEX3					= 0x300,
		VF_TEX4					= 0x400,
		VF_TEX5					= 0x500,
		VF_TEX6					= 0x600,
		VF_TEX7					= 0x700,
		VF_TEX8					= 0x800,
		VF_XYZB1				= 0x006,
		VF_XYZB2				= 0x008,
		VF_XYZB3				= 0x00a,
		VF_XYZB4				= 0x00c,
		VF_XYZB5				= 0x00e,
		VF_XYZW					= 0x4002,	//!< [DX9]

		VF_PNT					= VF_XYZ|VF_NORMAL|VF_TEX1,
		VF_PD					= VF_XYZ|VF_DIFFUSE,
		VF_PDT					= VF_XYZ|VF_DIFFUSE|VF_TEX1,
		VF_PN					= VF_XYZ|VF_NORMAL,
		VF_PT					= VF_XYZ|VF_TEX1,
		VF_PNDT					= VF_XYZ|VF_NORMAL|VF_DIFFUSE|VF_TEX1,
		VF_PRT					= VF_XYZRHW|VF_TEX1,
		VF_PRD					= VF_XYZRHW|VF_DIFFUSE,
		VF_PRDT					= VF_XYZRHW|VF_DIFFUSE|VF_TEX1,
		VF_PRDST				= VF_XYZRHW|VF_DIFFUSE|VF_SPECULAR|VF_TEX1,
		VF_PRDTT				= VF_XYZRHW|VF_DIFFUSE|VF_TEX2,

		VF_FORCE_DWORD			= 0x7fffffff,
	};

	enum FVFComponent
	{
		FVF_POSITION			= 0,
		FVF_NORMAL				= 1,
		FVF_PSIZE				= 2,
		FVF_DIFFUSE				= 3,
		FVF_SPECULAR			= 4,
		FVF_UV0					= 5,
		FVF_UV1					= 6,
		FVF_UV2					= 7,
		FVF_UV3					= 8,
		FVF_UV4					= 9,
		FVF_UV5					= 10,
		FVF_UV6					= 11,
		FVF_UV7					= 12,

		FVF_FORCE_DWORD			= 0x7fffffff
	};

	// Helpers
	FUNCTION ICERENDERER_API udword ComputeFVFSize(VertexFormat fvf);
	FUNCTION ICERENDERER_API udword ComputeOffset(VertexFormat fvf, FVFComponent component);

#endif // ICEVERTEXFORMAT_H
