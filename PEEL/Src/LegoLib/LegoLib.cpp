///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LegoLib.h"
#include "..\GLTexture.h"	// ### hmmm
#include "..\Common.h"
#include "..\SupportFile.h"

//#pragma optimize( "", off )
//#define RETIRED

#ifdef RETIRED

#ifndef _WIN64
	#define COMPILE_DEVIL
#endif

#ifdef COMPILE_DEVIL
	#undef _UNICODE

	// DevIL wrapper copied from '\APPs\#Plugins\SystemPlugs\IceDevil.cpp' (ICE). Probably old and obsolete but it still works.
//	#include ".\DevIL\include\il\il_wrap.h"
//	#include ".\il\il_wrap.h"
	#include ".\il\il.h"
#endif

#ifdef COMPILE_DEVIL
static bool InitDone = false;
#endif
static bool LoadWithDevil(const char* filename, Picture& pic)
{
#ifdef COMPILE_DEVIL
	if(!InitDone)
	{
		InitDone = true;
		ilInit();
	}

//	ilImage DevilImage;
//	ILboolean b = DevilImage.Load((char*)filename);
//	if(!b)	return false;
		ILuint ImageName;
		ilGenImages(1, &ImageName);
		ilBindImage(ImageName);
		if(!ilLoadImage((char*)filename))
		{
			ilDeleteImages(1, &ImageName);
			return false;
		}

//	ILuint W = DevilImage.Width();
//	ILuint H = DevilImage.Height();
//	ILuint BPP = DevilImage.Bitpp();
//	ILubyte* Data = DevilImage.GetData();
//	ILenum F = DevilImage.Format();
		ILuint W = ilGetInteger(IL_IMAGE_WIDTH);
		ILuint H = ilGetInteger(IL_IMAGE_HEIGHT); 
		ILuint BPP = ilGetInteger(IL_IMAGE_BPP); 
		ILenum F = ilGetInteger(IL_IMAGE_FORMAT); 
		ILubyte* Data = ilGetData(); 

	if(F!=IL_RGB && F!=IL_RGBA && F!=IL_BGR && F!=IL_BGRA)
	{
		IceCore::MessageBox(null, "Unsupported format!\nImage will not be loaded.", "Error", MB_OK);
		ilDeleteImages(1, &ImageName);
		return false;
	}

	pic.Init(ToWord(W), ToWord(H));
	RGBAPixel* Pixels = pic.GetPixels();
	for(udword y=0;y<H;y++)
	{
		for(udword x=0;x<W;x++)
		{
			if(F==IL_RGB)
			{
				Pixels->R = *Data++;
				Pixels->G = *Data++;
				Pixels->B = *Data++;
				Pixels->A = 0;
			}
			else if(F==IL_RGBA)
			{
				Pixels->R = *Data++;
				Pixels->G = *Data++;
				Pixels->B = *Data++;
				Pixels->A = *Data++;
			}
			else if(F==IL_BGR)
			{
				Pixels->B = *Data++;
				Pixels->G = *Data++;
				Pixels->R = *Data++;
				Pixels->A = 0;
			}
			else if(F==IL_BGRA)
			{
				Pixels->B = *Data++;
				Pixels->G = *Data++;
				Pixels->R = *Data++;
				Pixels->A = *Data++;
			}
			Pixels++;
		}
	}
//	pic.FlipVertical();
	ilDeleteImages(1, &ImageName);
#endif
	return true;
}

#else
	#include "Devil.h"
#endif

static const bool gUseConstantManager = false;

LegoImage::LegoImage(const char* id) : mTexture(null)// : mTexId(0)
{
	mID.Set(id);
}

LegoImage::~LegoImage()
{
	ReleaseManagedTexture(mTexture);
}

LegoActor::LegoActor() : mMesh(null), mMaterial(null), mUserData(null)
{
}

LegoActor::~LegoActor()
{
}

LegoEffect::LegoEffect(const char* id) : mImage(null)
{
	mID.Set(id);

	mDiffuse.R = mDiffuse.G = mDiffuse.B = 1.0f;
//	mDiffuse.R = mDiffuse.G = 0.0f;
	mDiffuse.A = 1.0f;
}

LegoEffect::~LegoEffect()
{
}

LegoMaterial::LegoMaterial(const char* id) : mEffect(null)
{
	mID.Set(id);
}

LegoMaterial::~LegoMaterial()
{
}

LegoPartMesh::LegoPartMesh(const char* string_id, udword id) : mID(id), mPos(0.0f, 0.0f, 0.0f), mUserData(null)
{
	mStringID.Set(string_id);
}

LegoPartMesh::~LegoPartMesh()
{
}

void LegoPartMesh::Recenter()
{
	const udword NbVerts = GetNbVerts();
	Point* V = const_cast<Point*>(mVertices.GetVertices());

	const float Coeff = 1.0f / float(NbVerts);
	Point Center(0.0f, 0.0f, 0.0f);
	for(udword i=0;i<NbVerts;i++)
		Center += V[i]*Coeff;
	for(udword i=0;i<NbVerts;i++)
		V[i] -= Center;
	mPos = Center;
}


	class LegoPartDatabase : public Allocateable
	{
		public:
						LegoPartDatabase();
						~LegoPartDatabase();

		PtrContainer	mParts;
		PtrContainer	mMeshes;
		PtrContainer	mActors;
		PtrContainer	mEffects;
		PtrContainer	mMaterials;
		PtrContainer	mImages;
		ConstantManager	mMeshByName;
	};

LegoPartDatabase::LegoPartDatabase()
{
}

LegoPartDatabase::~LegoPartDatabase()
{
	DeleteOwnedObjects<LegoPartMesh>(mMeshes);
	DeleteOwnedObjects<LegoPartMesh>(mParts);
	DeleteOwnedObjects<LegoActor>(mActors);
	DeleteOwnedObjects<LegoEffect>(mEffects);
	DeleteOwnedObjects<LegoMaterial>(mMaterials);
	DeleteOwnedObjects<LegoImage>(mImages);
}

static	LegoPartDatabase*	gLegoDatabase = null;

namespace
{
	enum ColladaParsingState
	{
		CPS_UNDEFINED	= 0,
		CPS_IMAGES		= 1,
		CPS_EFFECTS		= 2,
		CPS_MATERIALS	= 3,
		CPS_GEOMETRIES	= 4,
		CPS_SCENES		= 5
	};

	enum ColladaParsingItem
	{
		CPI_UNDEFINED		= 0,
		CPI_DIFFUSE			= 1,
		CPI_TRANSPARENCY	= 2,
	};

	struct ParseContext
	{
		ParseContext() :
			mCurrentActor			(null),
			mCurrentMesh			(null),
			mCurrentEffect			(null),
			mCurrentMaterial		(null),
			mCurrentImage			(null),
			mIndexBase				(0),
			mCurrentID				(0),
			mColladaParsingStage	(CPS_UNDEFINED),
			mColladaParsingItem		(CPI_UNDEFINED),
			mNbUVSets				(0),
			mScale					(1.0f)
		{
		}

		void	FinishMesh()
		{
			mCurrentMesh->Recenter();
			gLegoDatabase->mParts.AddPtr(mCurrentMesh);
		}

		LegoActor*		mCurrentActor;
		LegoPartMesh*	mCurrentMesh;
		LegoEffect*		mCurrentEffect;
		LegoMaterial*	mCurrentMaterial;
		LegoImage*		mCurrentImage;
		Picture			mCurrentPic;
		udword			mIndexBase;
		udword			mCurrentID;
		//
		udword			mColladaParsingStage;
		udword			mColladaParsingItem;
		udword			mNbUVSets;
		//
		float			mScale;
	};
}

/*static udword FindMinIndex()
{
	udword MinIndex = MAX_UDWORD;
	const udword NbTris = gCurrentMesh->GetNbTris();
	const udword* Indices = gCurrentMesh->GetIndices();
	for(udword i=0;i<NbTris*3;i++)
	{
		if(Indices[i]<MinIndex)
			MinIndex = Indices[i];
	}
	return MinIndex;
}*/

static bool ParseInitFromTag(char* text, String& str)
{
	if(strncmp(text, "<init_from>", 11)==0)
	{
		// The whole line appears in one entry of the parameter block:
		// <init_from>maps/bump/3040_bump.png</init_from>
		char* InitFrom = text;
		char* Start = InitFrom + 11;
		//char* Start = strchr(InitFrom, '>');
		//if(Start)
		{
//			char* End = strchr(++Start, '<');
			char* End = strchr(Start, '<');
			if(End)
			{
				*End = 0;
				str.Set(Start);
			}
		}
		return true;
	}
	return false;
}

static void gCollada_ParseRoot(const char* command, udword nb_params, const ParameterBlock& pb, size_t context, ParseContext* ctx, const ParameterBlock* cmd)
{
	ASSERT(ctx->mColladaParsingStage == CPS_UNDEFINED);
	if(nb_params==1)
	{
		if(pb[0]=="<library_images>")
		{
			ctx->mColladaParsingStage = CPS_IMAGES;
		}
		else if(pb[0]=="<library_effects>")
		{
			ctx->mColladaParsingStage = CPS_EFFECTS;
		}
		else if(pb[0]=="<library_geometries>")
		{
			ctx->mColladaParsingStage = CPS_GEOMETRIES;
		}
		else if(pb[0]=="<library_visual_scenes>")
		{
			ctx->mColladaParsingStage = CPS_SCENES;
		}
		else if(pb[0]=="<library_materials>")
		{
			ctx->mColladaParsingStage = CPS_MATERIALS;
		}
	}
}

static void gCollada_ParseImages(const char* command, udword nb_params, const ParameterBlock& pb, size_t context, ParseContext* ctx, const ParameterBlock* cmd)
{
	ASSERT(ctx->mColladaParsingStage == CPS_IMAGES);

	if(ctx->mCurrentImage)
	{
		if(nb_params==1)
		{
			if(pb[0]=="</image>")
			{
				gLegoDatabase->mImages.AddPtr(ctx->mCurrentImage);

				ctx->mCurrentImage = null;
			}
/*			else if(strncmp(pb[0], "<init_from>", 11)==0)
			{
				// The whole line appears in one entry of the parameter block:
				// <init_from>maps/bump/3040_bump.png</init_from>
				char* InitFrom = pb[0];
				char* Start = InitFrom + 11;
				//char* Start = strchr(InitFrom, '>');
				//if(Start)
				{
//					char* End = strchr(++Start, '<');
					char* End = strchr(Start, '<');
					if(End)
					{
						*End = 0;
						ctx->mCurrentImage->mFileName.Set(Start);
					}
				}
			}*/
			else if(ParseInitFromTag(pb[0], ctx->mCurrentImage->mFileName))
			{
			}
		}
	}
	else
	{
		if(nb_params==1)
		{
			if(pb[0]=="</library_images>")
			{
				ctx->mColladaParsingStage = CPS_UNDEFINED;
			}
		}
		else if(nb_params==6)
		{
			if(pb[0]=="<image")
			{
//				<image id="3040_bump_png" name="3040_bump_png">
				ASSERT(pb[1]=="id=");
				ASSERT(pb[3]=="name=");
	//			pb[2] = id
	//			pb[4] = name

				ASSERT(!ctx->mCurrentImage);
				ctx->mCurrentImage = ICE_NEW(LegoImage)(pb[2]);
			}
		}
	}
}

static void gCollada_ParseEffects(const char* command, udword nb_params, const ParameterBlock& pb, size_t context, ParseContext* ctx, const ParameterBlock* cmd)
{
	ASSERT(ctx->mColladaParsingStage == CPS_EFFECTS);

	// Shortcut: of this whole block below, only <init_from> is parsed
/*
		<effect id="decoration_6636d21-effect">
			<profile_COMMON>
				<newparam sid="6636d21_png-surface">
					<surface type="2D">
						<init_from>6636d21_png</init_from>
					</surface>
				</newparam>
				<newparam sid="6636d21_png-sampler">
					<sampler2D>
						<source>6636d21_png-surface</source>
					</sampler2D>
				</newparam>
				<technique sid="common">
					<phong>
						<diffuse>
							<texture texture="6636d21_png-sampler" texcoord="decoration_6636d21-uvmap"/>
						</diffuse>
						<index_of_refraction>
							<float sid="index_of_refraction">0</float>
						</index_of_refraction>
					</phong>
				</technique>
			</profile_COMMON>
		</effect>

		This one for transparent cockpit:

		<effect id="decoration_18973d1-effect">
			<profile_COMMON>
				<newparam sid="18973d1_png-surface">
					<surface type="2D">
						<init_from>18973d1_png</init_from>
					</surface>
				</newparam>
				<newparam sid="18973d1_png-sampler">
					<sampler2D>
						<source>18973d1_png-surface</source>
					</sampler2D>
				</newparam>
				<technique sid="common">
					<phong>
						<diffuse>
							<texture texture="18973d1_png-sampler" texcoord="decoration_18973d1-uvmap"/>
						</diffuse>
						<transparent opaque="A_ONE">
							<texture texture="18973d1_png-sampler" texcoord="decoration_18973d1-uvmap"/>
						</transparent>
						<index_of_refraction>
							<float sid="index_of_refraction">0</float>
						</index_of_refraction>
					</phong>
				</technique>
			</profile_COMMON>
		</effect>


*/

	if(ctx->mCurrentEffect)
	{
		// Ignored:
		// <profile_COMMON>
		// <technique sid="common">
		// <phong>
		// <index_of_refraction>

		if(nb_params==1)
		{
			if(pb[0]=="</effect>")
			{
				ctx->mColladaParsingStage = CPS_EFFECTS;
				gLegoDatabase->mEffects.AddPtr(ctx->mCurrentEffect);
				ctx->mCurrentEffect = null;
			}
			else if(pb[0]=="<diffuse>")
			{
				ctx->mColladaParsingItem = CPI_DIFFUSE;
			}
			else if(pb[0]=="</diffuse>")
			{
				ctx->mColladaParsingItem = CPI_UNDEFINED;
			}
			if(pb[0]=="<transparency>")
			{
				ctx->mColladaParsingItem = CPI_TRANSPARENCY;
			}
			else if(pb[0]=="</transparency>")
			{
				ctx->mColladaParsingItem = CPI_UNDEFINED;
			}
			// Shortcut: we consider this is always for a diffuse texture and we ignore pretty much everything else
/*			else if(strncmp(pb[0], "<init_from>", 11)==0)
			{
				// The whole line appears in one entry of the parameter block:
				// <init_from>maps/bump/3040_bump.png</init_from>
				char* InitFrom = pb[0];
				char* Start = InitFrom + 11;
				//char* Start = strchr(InitFrom, '>');
				//if(Start)
				{
//					char* End = strchr(++Start, '<');
					char* End = strchr(Start, '<');
					if(End)
					{
						*End = 0;
						ctx->mCurrentEffect->mTexture.Set(Start);
					}
				}
			}*/
			else if(ParseInitFromTag(pb[0], ctx->mCurrentEffect->mTexture))
			{
				const udword NbImages = gLegoDatabase->mImages.GetNbEntries();
				for(udword i=0;i<NbImages;i++)
				{
					LegoImage* Current = (LegoImage*)gLegoDatabase->mImages[i];
					if(Current->mID==ctx->mCurrentEffect->mTexture)
					{
						ctx->mCurrentEffect->mImage = Current;

						if(Current->mFileName.IsValid())
						{
							const char* Filename = FindPEELFile(Current->mFileName);
							if(Filename)
							{
								Picture& Pic = ctx->mCurrentPic;
								if(LoadWithDevil(Filename, Pic))
								{
									const AlphaType at = Pic.AlphaUsage();
									if(at!=AT_NONE)
									{
										bool ApplyMB5Process = true;

										const char* name = ctx->mCurrentEffect->mID;
										if(name)
										{
											// Trying to fix issues with MB6 format
											//ctx->mCurrentMaterial->
											//ctx->mCurrentEffect->mDiffuse.R = 255;
											//ctx->mCurrentEffect->mDiffuse.A = 0.5f;

											const char* Start = strchr(name, ' ');
											if(Start)
											{
												const char* End = strchr(Start+1, ' ');
												if(End)
												{
													*(char*)End = 0;
													const udword MBColorID = atoi(Start);
													*(char*)End = ' ';
												
													// https://www.mecabricks.com/docs/colour_chart.pdf
													udword RGB = 0;
													float Alpha = 1.0f;
													switch(MBColorID)
													{
														case 1:	// White
														{
															RGB = 0xF4F4F4;
															break;
														}

														case 5:	// Brick Yellow
														{
															RGB = 0xD5BC7C;
															break;
														}

														case 21:	// Bright Red
														{
															RGB = 0xD6001E;
															break;
														}

														case 23:	// Bright Blue
														{
															RGB = 0x2653A7;
															break;
														}

														case 24:	// Bright yellow
														{
															RGB = 0xF8C718;
															break;
														}

														case 26:	// Black
														{
															RGB = 0x101010;
															break;
														}

														case 40:	// Transparent
														{
															RGB = 0xEEEEEE;
															Alpha = 0.4f;
															break;
														}

														case 111:	// Tr. Brown
														{
															RGB = 0x645A4C;
															Alpha = 0.7f;
															break;
														}

														case 106:	// Bright Orange
														{
															RGB = 0xF57D23;
															break;
														}

														case 119:	// Bright Yellowish Green
														{
															RGB = 0x94BC0E;
															break;
														}

														case 138:	// Sand Yellow
														{
															RGB = 0x8A7553;
															break;
														}

														case 140:	// Earth Blue
														{
															RGB = 0x0A2441;
															break;
														}

														case 141:	// Earth Green
														{
															RGB = 0x053515;
															break;
														}

														case 192:	// Reddish Brown
														{
															RGB = 0x5B2D0E;
															break;
														}

														case 194:	// Medium Stone Grey
														{
															RGB = 0xA3A2A4;
															break;
														}

														case 199:	// Dark Stone Grey
														{
															RGB = 0x4D5156;
															break;
														}

														case 283:	// Light Nougat
														{
															RGB = 0xFAD1B1;
															break;
														}

														case 308:	// Dark Brown
														{
															RGB = 0x2E0F06;
															break;
														}

														case 315:	// Silver Metallic
														{
															RGB = 0x8E9496;
															break;
														}

														case 316:	// Titanium Metallic
														{
															RGB = 0x3E3C39;
															break;
														}

														case 322:	// Medium Azuze
														{
															RGB = 0x2ACDE8;
															break;
														}

														default:
														{
															ASSERT(!"Color not found");
															printf("Color %d not found\n", MBColorID);
															break;
														}
													};


													if(RGB)
													{
														RGBAPixel* p = Pic.GetPixels();
														udword Nb = Pic.GetWidth()*Pic.GetHeight();
														while(Nb--)
														{
															if(!p->R && !p->G && !p->B && !p->A)
															{
																p->R = ((RGB>>16) & 255);
																p->G = ((RGB>>8) & 255);
																p->B = (RGB & 255);
															}
															p++;
														}
													}

													ctx->mCurrentEffect->mDiffuse.A = Alpha;

													ApplyMB5Process = false;
												}
											}
										}

										if(ApplyMB5Process)
										{
											if(0)
											{
												RGBAPixel* p = Pic.GetPixels();
												udword Nb = Pic.GetWidth()*Pic.GetHeight();
												while(Nb--)
												{
													p->R = 0;
													p->G = 0;
													p->B = 255;
													p->A = 255;
													p++;
												}
											}

											if(0)
											{
												RGBAPixel* p = Pic.GetPixels();
												udword Nb = Pic.GetWidth()*Pic.GetHeight();
												while(Nb--)
												{
													if(!p->R && !p->G && !p->B && !p->A)
													{
														p->B = 255;
													}
													p++;
												}
											}
											else
												ctx->mCurrentEffect->mDiffuse.A = 0.0f;
										}
									}
	/*								else if(1 && strstr(Filename, "18973d1.png"))
									{
										//Current->mPic.AlphaLuminance();
										ctx->mCurrentEffect->mDiffuse.A = 0.0f;
									}*/


	//									Current->mPic.AlphaLuminance();
	//									ctx->mCurrentEffect->mDiffuse.A = 0.5f;
	//								Current->mTexId = GLTexture::createTexture(Pic.GetWidth(), Pic.GetHeight(), Pic.GetPixels(), true);
									Current->mTexture = CreateManagedTexture(Pic.GetWidth(), Pic.GetHeight(), Pic.GetPixels(), Current->mFileName);
//									Current->mTexture = CreateManagedTexture(Pic.GetWidth(), Pic.GetHeight(), Pic.GetPixels(), Filename);

									//ctx->mCurrentEffect->mDiffuse = Pic.GetPixels()[0];
								}
							}
							else
							{
								OutputConsoleError(_F("File not found: %s\n", Current->mFileName));
							}
						}
						break;
					}
				}
			}
		}
		else if(pb[0]=="<float")
		{
			if(ctx->mColladaParsingItem == CPI_TRANSPARENCY)
			{
				// <float sid="transparent">0.7</float>
				ASSERT(pb[1]=="sid=");
				ASSERT(nb_params==4);
				char* Start = pb[3];
				Start++;	// Skip '>'
				char* End = strchr(Start, '<');
				if(End)
				{
					*End = 0;
					ctx->mCurrentEffect->mDiffuse.A = float(::atof(Start));
				}
			}
		}
		else if(pb[0]=="<color")
		{
			// The <color> tag can appear under both <diffuse> and <transparent> tags. They're sometimes both defined for
			// an effect and I don't know how to use both so I arbitrarily keep the one from the diffuse tag.
			if(ctx->mColladaParsingItem == CPI_DIFFUSE)
			{
				ASSERT(pb[1]=="sid=");
				ASSERT(nb_params==7);
				const char* tmp = pb[3];	tmp++;
				ctx->mCurrentEffect->mDiffuse.R = float(::atof(tmp));
				ctx->mCurrentEffect->mDiffuse.G = float(::atof(pb[4]));
				ctx->mCurrentEffect->mDiffuse.B = float(::atof(pb[5]));

				// Gamma
				{
					const float Gamma = 1.0f/2.2f;
					ctx->mCurrentEffect->mDiffuse.R = powf(ctx->mCurrentEffect->mDiffuse.R, Gamma);
					ctx->mCurrentEffect->mDiffuse.G = powf(ctx->mCurrentEffect->mDiffuse.G, Gamma);
					ctx->mCurrentEffect->mDiffuse.B = powf(ctx->mCurrentEffect->mDiffuse.B, Gamma);
/*
					float R2 = powf(R, 1.0f/Gamma);
					float G2 = powf(G, 1.0f/Gamma);
					float B2 = powf(B, 1.0f/Gamma);
					int stop=1;*/
				}
			}
		}
		// Shortcut: ignore this for now
/*		else if(pb[0]=="<newparam")
		{
		}
		else if(pb[0]=="<surface")
		{
		}*/
	}
	else
	{
		if(nb_params==1)
		{
			if(pb[0]=="</library_effects>")
			{
				ctx->mColladaParsingStage = CPS_UNDEFINED;
			}
		}
		else
		{
			if(pb[0]=="<effect")
			{
				//<effect id="color_1-effect">
				ASSERT(pb[1]=="id=");

				ASSERT(!ctx->mCurrentEffect);
				ctx->mCurrentEffect = ICE_NEW(LegoEffect)(pb[2]);
			}
		}
	}
}

static void gCollada_ParseMaterials(const char* command, udword nb_params, const ParameterBlock& pb, size_t context, ParseContext* ctx, const ParameterBlock* cmd)
{
	ASSERT(ctx->mColladaParsingStage == CPS_MATERIALS);

	if(ctx->mCurrentMaterial)
	{
		if(nb_params==1)
		{
			if(pb[0]=="</material>")
			{
				ctx->mColladaParsingStage = CPS_MATERIALS;
				gLegoDatabase->mMaterials.AddPtr(ctx->mCurrentMaterial);
				ctx->mCurrentMaterial = null;
			}
		}
		else if(pb[0]=="<instance_effect")
		{
//			<instance_effect url="#decoration_6238d1-effect"/>
			ASSERT(pb[1]=="url=");

			const char* Name = pb[2];
			Name++;	// Skips #

			bool Found = false;
			{
				for(udword i=0;i<gLegoDatabase->mEffects.GetNbEntries();i++)
				{
					LegoEffect* M = (LegoEffect*)gLegoDatabase->mEffects.GetEntry(i);
					if(M->mID==Name)
					{
						ASSERT(!ctx->mCurrentMaterial->mEffect);
						ctx->mCurrentMaterial->mEffect = M;
						Found = true;
						break;
					}
				}
			}
			ASSERT(Found);
		}
	}
	else
	{
		if(nb_params==1)
		{
			if(pb[0]=="</library_materials>")
			{
				ctx->mColladaParsingStage = CPS_UNDEFINED;
			}
		}
		else
		{
			if(pb[0]=="<material")
			{
//				<material id="111:6238d1:6238d1_bump:-material" name="111:6238d1:6238d1_bump:">
				ASSERT(pb[1]=="id=");

				ASSERT(!ctx->mCurrentMaterial);
				ctx->mCurrentMaterial = ICE_NEW(LegoMaterial)(pb[2]);
			}
		}
	}
}

static void gCollada_ParseGeometries(const char* command, udword nb_params, const ParameterBlock& pb, size_t context, ParseContext* ctx, const ParameterBlock* cmd)
{
	ASSERT(ctx->mColladaParsingStage == CPS_GEOMETRIES);

/*
		<geometry id="11477uv-mesh" name="11477uv.50">
			<mesh>
				<source id="11477uv-mesh-positions">
					<float_array id="11477uv-mesh-positions-array" count="486">-3.96 0.02 0.04 -2.4 4.352 0.04 -3.96 0.02 7.96 -3.96 3.22 0.04 -3.96 6.248 -5.001 -3.96 4.717 2.026 -3.96 5.688 -1.443 3.96 0.02 0.04 3.96 3.22 0.04 3.96 6.38 -7.96 3.96 0.02 7.96 3.96 2 7.96 3.96 3.347 5.358 -3.96 2 7.96 -3.96 3.347 5.358 3.96 4.717 2.026 3.96 5.688 -1.443 3.96 6.248 -5.001 -3.96 6.38 -7.96 -0.6 4.352 0.04 -0.6 3.22 -1.6 -0.6 4.738 -1.6 0.6 3.22 -1.6 0.6 4.738 -1.6 0.6 4.352 0.04 0.6 3.22 0.04 -0.6 3.22 0.04 -2.4 3.22 -6.4 -2.4 5.14 -4 -2.4 3.22 0.04 -3.96 3.22 -7.96 -2.4 5.3 -6.4 2.4 5.3 -6.4 3.96 3.22 -7.96 -0.918 4.78 -1.783 2.217 5.011 -3.082 1.697 4.878 -2.303 2.4 3.22 -6.4 -1.697 4.878 -2.303 2.4 5.3 -4 2.4 3.22 0.04 2.4 5.14 -4 2.4 4.352 0.04 -2.4 0.02 1.6 2.4 0.02 1.6 2.4 0.02 6.4 -2.4 3.882 1.6 2.4 3.882 1.6 -2.4 2 6.4 -2.4 0.02 6.4 2.4 2 6.4 2.4 3.076 3.539 -2.4 3.076 3.539 2.4 2 5.503 -2.4 2 5.503 -2.4 5.3 -4 -2.217 5.3 -3.082 -1.697 5.3 -2.303 -0.918 5.3 -1.783 0 5.3 -1.6 0.918 5.3 -1.783 1.697 5.3 -2.303 2.217 5.3 -3.082 -2.217 5.011 -3.082 0 4.733 -1.6 0.918 4.78 -1.783 -3.96 2 7.96 -3.96 2 7.96 -3.96 3.347 5.358 3.96 2 7.96 3.96 2 7.96 3.96 3.347 5.358 0.6 3.22 -1.6 0.6 3.22 -1.6 0.6 4.738 -1.6 3.96 4.717 2.026 3.96 5.688 -1.443 -0.918 4.78 -1.783 -1.697 4.878 -2.303 -2.217 5.011 -3.082 -3.96 4.717 2.026 -3.96 5.688 -1.443 -3.96 0.02 0.04 -3.96 0.02 0.04 3.96 0.02 0.04 3.96 0.02 0.04 -3.96 6.248 -5.001 2.4 3.076 3.539 2.4 2 5.503 2.4 2 5.503 0.6 3.22 0.04 -2.4 3.076 3.539 -2.4 2 5.503 -2.4 2 5.503 2.4 0.02 1.6 2.4 0.02 1.6 2.4 0.02 6.4 2.4 0.02 6.4 -3.96 3.22 -7.96 -3.96 3.22 -7.96 3.96 3.22 -7.96 3.96 3.22 -7.96 2.4 5.3 -4 2.217 5.3 -3.082 3.96 6.248 -5.001 -2.4 3.882 1.6 -2.4 3.882 1.6 2.4 3.882 1.6 2.4 3.882 1.6 -2.4 5.3 -6.4 -2.4 5.3 -6.4 2.4 5.3 -6.4 2.4 5.3 -6.4 -3.96 0.02 7.96 -3.96 0.02 7.96 3.96 0.02 7.96 3.96 0.02 7.96 3.96 6.38 -7.96 3.96 6.38 -7.96 -3.96 6.38 -7.96 -3.96 6.38 -7.96 -2.4 3.22 -6.4 -2.4 3.22 -6.4 -2.4 3.22 0.04 -2.4 3.22 0.04 -2.4 4.352 0.04 -2.4 4.352 0.04 2.4 4.352 0.04 2.4 4.352 0.04 2.4 3.22 0.04 2.4 3.22 0.04 -2.4 0.02 1.6 -2.4 0.02 1.6 3.96 3.22 0.04 3.96 3.22 0.04 -2.4 5.3 -4 2.4 2 6.4 2.4 2 6.4 -2.4 2 6.4 -2.4 2 6.4 2.4 3.22 -6.4 2.4 3.22 -6.4 -2.4 5.14 -4 -3.96 3.22 0.04 -3.96 3.22 0.04 -2.4 0.02 6.4 -2.4 0.02 6.4 -0.6 3.22 -1.6 -0.6 3.22 -1.6 -0.6 4.738 -1.6 1.697 4.878 -2.303 2.217 5.011 -3.082 2.4 5.14 -4 0 4.733 -1.6 -0.6 3.22 0.04 0.918 4.78 -1.783 -2.217 5.3 -3.082 -1.697 5.3 -2.303 -0.918 5.3 -1.783 0 5.3 -1.6 0.918 5.3 -1.783 1.697 5.3 -2.303</float_array>
					<technique_common>
						<accessor source="#11477uv-mesh-positions-array" count="162" stride="3">
							<param name="X" type="float"/>
							<param name="Y" type="float"/>
							<param name="Z" type="float"/>
						</accessor>
					</technique_common>
				</source>
				<source id="11477uv-mesh-map">
					<float_array id="11477uv-mesh-map-array" count="720">0.422 0.333 0.422 0.671 0.269 0.333 0.422 0.671 0.269 0.671 0.269 0.333 0.422 0.333 0.575 0.333 0.422 0.671 0.575 0.333 0.575 0.671 0.422 0.671 0.514 0.842 0.514 0.979 0.43 0.779 0.514 0.979 0.177 0.979 0.43 0.779 0.918 0.065 0.918 0.056 0.931 0.068 0.918 0.056 0.931 0.056 0.931 0.068 0.936 0.072 0.932 0.072 0.936 0.069 0.932 0.072 0.932 0.068 0.936 0.069 0.936 0.072 0.936 0.069 0.941 0.072 0.936 0.069 0.941 0.07 0.941 0.072 0.878 0.047 0.878 0.047 0.893 0.055 0.878 0.047 0.893 0.055 0.893 0.055 0.269 0.333 0.269 0.671 0.144 0.333 0.269 0.671 0.144 0.671 0.144 0.333 0.918 0.065 0.931 0.068 0.918 0.056 0.931 0.068 0.931 0.056 0.918 0.056 0.966 0.056 0.966 0.056 0.977 0.056 0.966 0.056 0.977 0.056 0.977 0.056 0.907 0.033 0.893 0.055 0.872 0.033 0.893 0.055 0.878 0.047 0.872 0.033 0.918 0.033 0.918 0.056 0.918 0.056 0.966 0.056 0.977 0.056 0.918 0.056 0.977 0.056 0.918 0.056 0.918 0.056 0.918 0.056 0.918 0.033 0.918 0.056 0.86 0.033 0.872 0.033 0.86 0.033 0.872 0.033 0.872 0.033 0.86 0.033 0.514 0.152 0.577 0.258 0.43 0.216 0.918 0.056 0.918 0.065 0.966 0.056 0.918 0.065 0.948 0.071 0.966 0.056 0.86 0.047 0.86 0.047 0.86 0.033 0.86 0.047 0.86 0.033 0.86 0.033 0.729 0.713 0.855 0.708 0.514 0.842 0.855 0.708 0.855 0.842 0.514 0.842 0.729 0.281 0.855 0.152 0.855 0.287 0.872 0.047 0.872 0.047 0.878 0.047 0.872 0.047 0.878 0.047 0.878 0.047 0.855 0.671 0.729 0.671 0.855 0.333 0.729 0.671 0.729 0.333 0.855 0.333 0.918 0.033 0.918 0.033 0.918 0.056 0.918 0.033 0.918 0.056 0.918 0.056 0.729 0.713 0.514 0.842 0.577 0.737 0.514 0.842 0.43 0.779 0.577 0.737 0.918 0.033 0.86 0.033 0.907 0.033 0.86 0.033 0.872 0.033 0.907 0.033 0.966 0.056 0.966 0.056 0.966 0.072 0.966 0.056 0.966 0.072 0.966 0.072 0.941 0.07 0.948 0.071 0.918 0.065 0.931 0.056 0.918 0.056 0.931 0.056 0.918 0.056 0.918 0.056 0.931 0.056 0.288 0.837 0.43 0.779 0.177 0.894 0.43 0.779 0.177 0.979 0.177 0.894 0.948 0.071 0.941 0.07 0.948 0.072 0.941 0.07 0.941 0.072 0.948 0.072 0.907 0.033 0.907 0.033 0.907 0.061 0.907 0.033 0.907 0.061 0.907 0.061 0.288 0.158 0.514 0.016 0.43 0.216 0.514 0.016 0.514 0.152 0.43 0.216 0.575 0.333 0.729 0.333 0.575 0.671 0.729 0.333 0.729 0.671 0.575 0.671 0.872 0.033 0.872 0.033 0.872 0.047 0.872 0.033 0.872 0.047 0.872 0.047 0.86 0.033 0.918 0.033 0.872 0.033 0.918 0.033 0.907 0.033 0.872 0.033 0.966 0.056 0.948 0.071 0.918 0.056 0.948 0.071 0.918 0.065 0.918 0.056 0.918 0.065 0.948 0.071 0.941 0.07 0.893 0.055 0.893 0.055 0.907 0.061 0.893 0.055 0.907 0.061 0.907 0.061 0.918 0.056 0.977 0.056 0.918 0.056 0.977 0.056 0.966 0.056 0.918 0.056 0.941 0.072 0.941 0.07 0.936 0.072 0.941 0.07 0.936 0.069 0.936 0.072 0.931 0.056 0.931 0.068 0.931 0.056 0.931 0.068 0.931 0.068 0.931 0.056 0.936 0.072 0.936 0.069 0.932 0.072 0.936 0.069 0.932 0.068 0.932 0.072 0.855 0.152 0.729 0.281 0.514 0.152 0.729 0.281 0.577 0.258 0.514 0.152 0.948 0.071 0.966 0.056 0.948 0.072 0.966 0.056 0.966 0.072 0.948 0.072 0.932 0.072 0.932 0.068 0.931 0.072 0.932 0.068 0.931 0.068 0.931 0.072 0.872 0.033 0.878 0.047 0.907 0.033 0.878 0.047 0.893 0.055 0.907 0.033 0.977 0.056 0.977 0.056 0.977 0.08 0.977 0.056 0.977 0.08 0.977 0.08 0.907 0.061 0.907 0.033 0.893 0.055 0.907 0.033 0.918 0.033 0.907 0.033 0.918 0.033 0.918 0.033 0.907 0.033 0.948 0.071 0.948 0.072 0.966 0.056 0.948 0.072 0.966 0.072 0.966 0.056 0.288 0.158 0.177 0.1 0.514 0.016 0.177 0.1 0.177 0.016 0.514 0.016 0.878 0.047 0.872 0.033 0.872 0.047 0.872 0.047 0.872 0.033 0.878 0.047 0.931 0.072 0.931 0.068 0.932 0.072 0.931 0.068 0.932 0.068 0.932 0.072 0.907 0.033 0.907 0.061 0.893 0.055 0.941 0.07 0.948 0.071 0.941 0.072 0.948 0.071 0.948 0.072 0.941 0.072 0.918 0.065 0.918 0.065 0.918 0.056 0.918 0.065 0.918 0.056 0.918 0.056 0.918 0.065 0.941 0.07 0.936 0.069 0.936 0.069 0.941 0.07 0.918 0.065 0.918 0.065 0.936 0.069 0.932 0.068 0.932 0.068 0.936 0.069 0.918 0.065 0.918 0.065 0.932 0.068 0.931 0.068 0.931 0.068 0.932 0.068 0.918 0.065 0.918 0.065 0.931 0.068 0.918 0.065 0.948 0.072 0.948 0.072 0.941 0.072 0.948 0.072 0.941 0.072 0.941 0.072 0.936 0.072 0.936 0.072 0.941 0.072 0.936 0.072 0.941 0.072 0.941 0.072 0.932 0.072 0.932 0.072 0.936 0.072 0.932 0.072 0.936 0.072 0.936 0.072 0.932 0.072 0.931 0.072 0.932 0.072 0.966 0.072 0.948 0.072 0.966 0.072 0.948 0.072 0.948 0.072 0.966 0.072</float_array>
					<technique_common>
						<accessor source="#11477uv-mesh-map-array" count="360" stride="2">
							<param name="S" type="float"/>
							<param name="T" type="float"/>
						</accessor>
					</technique_common>
				</source>
				<vertices id="11477uv-mesh-vertices">
					<input semantic="POSITION" source="#11477uv-mesh-positions"/>
				</vertices>
				<polylist material="1:11477d14:11477_bump:-material" count="120">
					<input semantic="VERTEX" source="#11477uv-mesh-vertices" offset="0"/>
					<input semantic="TEXCOORD" source="#11477uv-mesh-map" offset="1" set="0"/>
					<vcount>3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3</vcount>
					<p>75 0 5 1 71 2 5 3 68 4 71 5 75 6 76 7 5 8 76 9 81 10 5 11 143 12 83 13 80 14 83 15 113 16 80 17 24 18 90 19 74 20 90 21 72 22 74 23 157 24 158 25 78 26 158 27 77 28 78 29 157 30 78 31 156 32 78 33 79 34 156 35 88 36 92 37 87 38 92 39 91 40 87 41 71 42 68 43 69 44 68 45 66 46 69 47 19 48 149 49 154 50 149 51 147 52 154 53 141 54 122 55 100 56 122 57 98 58 100 59 132 60 52 61 146 62 52 63 93 64 146 65 82 66 144 67 124 68 141 69 100 70 40 71 100 72 133 73 40 74 8 75 84 76 130 77 114 78 49 79 115 80 49 81 96 82 115 83 134 84 16 85 15 86 129 87 42 88 140 89 42 90 152 91 140 92 70 93 67 94 116 95 67 96 2 97 116 98 86 99 119 100 143 101 119 102 99 103 143 104 17 105 101 106 117 107 136 108 138 109 53 110 138 111 54 112 53 113 120 114 4 115 118 116 4 117 104 118 118 119 84 120 82 121 130 122 82 123 124 124 130 125 86 126 143 127 6 128 143 129 80 130 6 131 85 132 115 133 94 134 115 135 96 136 94 137 27 138 37 139 109 140 37 141 111 142 109 143 151 144 41 145 127 146 73 147 25 148 148 149 25 150 26 151 148 152 14 153 80 154 13 155 80 156 113 157 13 158 152 159 35 160 102 161 35 162 103 163 102 164 131 165 44 166 105 167 44 168 107 169 105 170 12 171 7 172 15 173 7 174 134 175 15 176 76 177 104 178 81 179 104 180 4 181 81 182 45 183 145 184 137 185 145 186 139 187 137 188 114 189 0 190 49 191 0 192 43 193 49 194 121 195 142 196 123 197 142 198 1 199 123 200 125 201 28 202 63 203 87 204 91 205 108 206 91 207 106 208 108 209 3 210 98 211 29 212 98 213 122 214 29 215 103 216 35 217 161 218 35 219 36 220 161 221 20 222 21 223 22 224 21 225 23 226 22 227 161 228 36 229 160 230 36 231 155 232 160 233 101 234 17 235 134 236 17 237 16 238 134 239 142 240 121 241 135 242 121 243 110 244 135 245 160 246 155 247 159 248 155 249 153 250 159 251 97 252 89 253 95 254 89 255 51 256 95 257 33 258 30 259 9 260 30 261 18 262 9 263 47 264 95 265 51 266 43 267 0 268 94 269 0 270 85 271 94 272 152 273 102 274 140 275 102 276 112 277 140 278 12 279 11 280 7 281 11 282 10 283 7 284 89 285 97 286 50 287 48 288 146 289 93 290 159 291 153 292 158 293 153 294 77 295 158 296 132 297 46 298 52 299 79 300 142 301 156 302 142 303 135 304 156 305 126 306 128 307 124 308 128 309 130 310 124 311 125 312 63 313 38 314 150 315 151 316 127 317 125 318 38 319 34 320 65 321 150 322 127 323 125 324 34 325 64 326 64 327 65 328 127 329 125 330 64 331 127 332 55 333 39 334 56 335 39 336 62 337 56 338 61 339 57 340 62 341 57 342 56 343 62 344 60 345 58 346 61 347 58 348 57 349 61 350 60 351 59 352 58 353 32 354 39 355 31 356 39 357 55 358 31 359</p>
				</polylist>
			</mesh>
		</geometry>
*/


	if(ctx->mCurrentMesh)
	{
		if(nb_params==1)
		{
			if(pb[0]=="</geometry>")
			{
				if(gUseConstantManager)
				{
					if(!gLegoDatabase->mMeshes.GetNbEntries())
						gLegoDatabase->mMeshByName.Init(32);
				}

//				ctx->FinishMesh();
				gLegoDatabase->mMeshes.AddPtr(ctx->mCurrentMesh);

				if(gUseConstantManager)
					gLegoDatabase->mMeshByName.Add(ctx->mCurrentMesh->mStringID, ctx->mCurrentMesh);

				ctx->mCurrentMesh = null;
			}
		}
		else if(pb[0]=="<float_array")
		{
			// I took some shortcuts here and just assumed that the first float array was for vertices, and the second for UVs.
			// In theory there would be more parsing & book-keeping to do here.

			// <float_array id="2412-mesh-positions-array" count="642">7.96 0.5 ... -0.8</float_array>
			//###last one works by accident
			//###################### CHECK LAST FLOAT IS VALID
			// maybe drop < and > and rely on the PB
			ASSERT(pb[1]=="id=");
			ASSERT(pb[3]=="count=");
			//pb[4]=count
			// Verts data starts with pb[5] but starts with ">"  stupid format

			if(!ctx->mCurrentMesh->mVertices.GetNbVertices())
			{
				ASSERT(!ctx->mNbUVSets);
				const udword NbVerts = ::atoi(pb[4])/3;
				udword Index = 5;
				ASSERT(nb_params==NbVerts*3+Index);
				for(udword i=0;i<NbVerts;i++)
				{
					const udword Skip = i==0 ? 1 : 0;
					const char* Data = pb[Index++];
					const float x = float(::atof(Data+Skip));
					const float y = float(::atof(pb[Index++]));
/*					if(i==NbVerts-1)
					{
						const char* check = pb[Index];
						check = check;
					}*/
					const float z = float(::atof(pb[Index++]));
					const float Scale = ctx->mScale;
					ctx->mCurrentMesh->mVertices.AddVertex(x*Scale, y*Scale, z*Scale);
				}
			}
			else
			{
				ctx->mCurrentMesh->mUVs.Reset();

				if(!ctx->mCurrentMesh->mUVs.GetNbVertices())
				{
					const udword NbVerts = ::atoi(pb[4])/2;
					udword Index = 5;
					ASSERT(nb_params==NbVerts*2+Index);
					for(udword i=0;i<NbVerts;i++)
					{
						const udword Skip = i==0 ? 1 : 0;
						char* Data = pb[Index++];
						const float u = float(::atof(Data+Skip));
						Data = pb[Index++];
						if(i==NbVerts-1)
						{
							char* End = strchr(Data, '<');
							if(End)
								*End = 0;
						}
						const float v = float(::atof(Data));
//						const float v = float(::atof(pb[Index++]));
						ctx->mCurrentMesh->mUVs.AddVertex(u, 1.0f-v, 0.0f);
//						printf("%f %f\n", u, v);
					}
				}
				else ASSERT(0);

				ASSERT(!ctx->mNbUVSets);
				ctx->mNbUVSets++;
			}
		}
		else
		{
			// OMG this format is awful
//			const char* Data = pb[0];
			if(strncmp(pb[0], "<p>", 3)==0)
			{
				//###last one works by accident
				//##### CHECK LAST INDEX IS VALID
				const udword NbVerts = ctx->mCurrentMesh->mVertices.GetNbVertices();
				const udword NbUVs = ctx->mCurrentMesh->mUVs.GetNbVertices();

				const udword NbToGo = ctx->mNbUVSets ? nb_params/2 : nb_params;	//****OMFG
				const udword Multiplier = ctx->mNbUVSets ? 2 : 1;	//****OMFG

				for(udword i=0;i<NbToGo;i++)
				{
					const udword Skip = i==0 ? 3 : 0;
					const char* Data = pb[i*Multiplier];
					const udword VRef = ::atoi(Data+Skip);
					ASSERT(VRef<NbVerts);
					ctx->mCurrentMesh->mIndices.Add(VRef);

					if(ctx->mNbUVSets)
					{
						const char* Data2 = pb[i*Multiplier+1];
						const udword TRef = ::atoi(Data2/*+Skip*/);	// There's nothing to skip for UVs
						ASSERT(TRef<NbUVs);
						ctx->mCurrentMesh->mTIndices.Add(TRef);
					}
				}
			}
		}
	}
	else
	{
		if(nb_params==1)
		{
			if(pb[0]=="</library_geometries>")
			{
				ctx->mColladaParsingStage = CPS_UNDEFINED;
			}
		}
		else if(nb_params==6)
		{
			if(pb[0]=="<geometry")
			{
//				<geometry id="3005-mesh" name="3005.1">
				ASSERT(pb[1]=="id=");
				ASSERT(pb[3]=="name=");
	//			pb[2] = id
	//			pb[4] = name
				ASSERT(!ctx->mCurrentMesh);
				ctx->mCurrentMesh = ICE_NEW(LegoPartMesh)(pb[2], ctx->mCurrentID++);
				ctx->mNbUVSets = 0;
			}
		}
	}
}

static void gCollada_ParseActor(const char* command, udword nb_params, const ParameterBlock& pb, size_t context, ParseContext* ctx, const ParameterBlock* cmd)
{
	ASSERT(ctx->mCurrentActor);

	if(pb[0]=="</node>")
	{
		ctx->mColladaParsingStage = CPS_SCENES;	//### this assumes nodes are only defined in scenes
		gLegoDatabase->mActors.AddPtr(ctx->mCurrentActor);
		ctx->mCurrentActor = null;

	}
	else if(pb[0]=="<matrix")
	{
//		<matrix sid="transform">1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1</matrix>
		//###last one works by accident
		ASSERT(pb[1]=="sid=");
		udword Index = 3;
//		float* Dest = &Ctx->mCurrentActor->mPose.m[0][0];
		float Values[16];
		for(udword i=0;i<16;i++)
		{
			const udword Skip = i==0 ? 1 : 0;
			const char* Data = pb[Index++];
//			*Dest++ = ::atof(Data+Skip);
			Values[i] = float(::atof(Data+Skip));
		}
		ASSERT(Values[15]==1.0f);
		ASSERT(Values[14]==0.0f);
		ASSERT(Values[13]==0.0f);
		ASSERT(Values[12]==0.0f);
/*		Ctx->mCurrentActor->mPose.Transpose();
		Point Pos = Ctx->mCurrentActor->mPose.GetTrans();
		Pos *= gGlobalScale;
		Ctx->mCurrentActor->mPose.SetTrans(Pos);*/

		Matrix3x3 Rot;
		Rot.SetRow(0, Point(Values[0], Values[1], Values[2]));
		Rot.SetRow(1, Point(Values[4], Values[5], Values[6]));
		Rot.SetRow(2, Point(Values[8], Values[9], Values[10]));
		Rot.Transpose();

//		float det = Rot.Determinant();
//		ASSERT(det==1.0f);

		ctx->mCurrentActor->mPose = Rot;
		const float Scale = ctx->mScale;
		ctx->mCurrentActor->mPose.SetTrans(Point(Values[3]*Scale, Values[7]*Scale, Values[11]*Scale));
	}
	else if(pb[0]=="<instance_geometry")
	{
		// <instance_geometry url="#47457-mesh">
		ASSERT(pb[1]=="url=");
		const char* Name = pb[2];
		Name++;	// Skips #
		ASSERT(!ctx->mCurrentActor->mMesh);

		if(!gUseConstantManager)
		{
			for(udword i=0;i<gLegoDatabase->mMeshes.GetNbEntries();i++)
			{
				LegoPartMesh* Mesh = (LegoPartMesh*)gLegoDatabase->mMeshes.GetEntry(i);
				if(Mesh->mStringID==Name)
				{
					ctx->mCurrentActor->mMesh = Mesh;
					break;
				}
			}
		}
		else
		{
			void* UserData;
			if(gLegoDatabase->mMeshByName.Find(Name, &UserData))
			{
				LegoPartMesh* Mesh = (LegoPartMesh*)UserData;
				ASSERT(Mesh->mStringID==Name);
				ctx->mCurrentActor->mMesh = Mesh;
			}
		}

		ASSERT(ctx->mCurrentActor->mMesh);
	}
	else if(pb[0]=="<instance_material")
	{
		// <instance_material symbol="26:::-material" target="#26:::-material"/>
		ASSERT(pb[1]=="symbol=");
		const char* Name = pb[2];

//		Name++;	// Skips #

		bool Found = false;
		{
			for(udword i=0;i<gLegoDatabase->mMaterials.GetNbEntries();i++)
			{
				LegoMaterial* M = (LegoMaterial*)gLegoDatabase->mMaterials.GetEntry(i);
				if(M->mID==Name)
				{
					ASSERT(!ctx->mCurrentActor->mMaterial);
					ctx->mCurrentActor->mMaterial = M;
					Found = true;
					break;
				}
			}
		}
		ASSERT(Found);

	}
}

static void gCollada_ParseVisualScenes(const char* command, udword nb_params, const ParameterBlock& pb, size_t context, ParseContext* ctx, const ParameterBlock* cmd)
{
	ASSERT(ctx->mColladaParsingStage == CPS_SCENES);

	//### We only support one scene for now

	if(ctx->mCurrentActor)
	{
		gCollada_ParseActor(command, nb_params, pb, context, ctx, cmd);
	}
	else
	{
		if(nb_params==1)
		{
			if(pb[0]=="</library_visual_scenes>")
			{
				ctx->mColladaParsingStage = CPS_UNDEFINED;
			}
		}
		else
		{
			if(pb[0]=="<node")
			{
				//<node id="Part_1" name="Part_30029.1" type="NODE">
				ASSERT(pb[1]=="id=");
				ASSERT(pb[3]=="name=");

				ASSERT(!ctx->mCurrentActor);
				ctx->mCurrentActor = ICE_NEW(LegoActor);
			}
		}
	}
}

//#define DEBUG_COLLADA_PARSER
#ifdef DEBUG_COLLADA_PARSER
static udword gCount = 0;
#endif

// Collada instancing might not even work if it uses mirror matrices

// This parses both Wavefront & Collada files :)
static bool gParseCallback(const char* command, const ParameterBlock& pb, size_t context, void* user_data, const ParameterBlock* cmd)
{
	ParseContext* Ctx = (ParseContext*)user_data;

#ifdef DEBUG_COLLADA_PARSER
	gCount++;
	printf("Line: %d\n", gCount);
#endif

	const udword NbParams = pb.GetNbParams();

	// Collada
	switch(Ctx->mColladaParsingStage)
	{
		case CPS_UNDEFINED:		{ gCollada_ParseRoot		(command, NbParams, pb, context, Ctx, cmd);	}	break;
		case CPS_IMAGES:		{ gCollada_ParseImages		(command, NbParams, pb, context, Ctx, cmd);	}	break;
		case CPS_EFFECTS:		{ gCollada_ParseEffects		(command, NbParams, pb, context, Ctx, cmd);	}	break;
		case CPS_MATERIALS:		{ gCollada_ParseMaterials	(command, NbParams, pb, context, Ctx, cmd);	}	break;
		case CPS_GEOMETRIES:	{ gCollada_ParseGeometries	(command, NbParams, pb, context, Ctx, cmd);	}	break;
		case CPS_SCENES:		{ gCollada_ParseVisualScenes(command, NbParams, pb, context, Ctx, cmd);	}	break;
	};
	//~Collada

	// Wavefront
	if(1 && pb[0]=="o")
	{
		// New mesh
		if(Ctx->mCurrentMesh)
		{
//			const udword MinIndex = FindMinIndex();
//			Ctx->mCurrentMesh->Recenter();
//			gLegoDatabase->mParts.Add(udword(Ctx->mCurrentMesh));
			Ctx->FinishMesh();

			Ctx->mIndexBase += Ctx->mCurrentMesh->GetNbVerts();
		}
		Ctx->mCurrentMesh = ICE_NEW(LegoPartMesh)(null, Ctx->mCurrentID++);
	}

	if(NbParams==4)
	{
		if(pb[0]=="v")
		{
			const float x = float(::atof(pb[1]));
			const float y = float(::atof(pb[2]));
			const float z = float(::atof(pb[3]));
			const float Scale = Ctx->mScale;
			Ctx->mCurrentMesh->mVertices.AddVertex(x*Scale, y*Scale, z*Scale);
		}
		else if(pb[0]=="f")
		{
			const udword VRef0 = ::atoi(pb[1]);
			const udword VRef1 = ::atoi(pb[2]);
			const udword VRef2 = ::atoi(pb[3]);
			Ctx->mCurrentMesh->mIndices.Add(VRef0-Ctx->mIndexBase-1);
			Ctx->mCurrentMesh->mIndices.Add(VRef1-Ctx->mIndexBase-1);
			Ctx->mCurrentMesh->mIndices.Add(VRef2-Ctx->mIndexBase-1);
		}
	}
	//~Wavefront
	return true;
}

static bool LoadLegoFile(udword id, const char* name, float scale_factor)
{
	const char* Filename = FindPEELFile(name);
	if(!Filename)
		return false;

#ifdef DEBUG_COLLADA_PARSER
	gCount = 0;
#endif

	void SetDropFile(const char*);
	SetDropFile(name);

	ParseContext Ctx;
	Ctx.mIndexBase = 0;
	Ctx.mCurrentID = id;
	Ctx.mScale = scale_factor;

	ScriptFile2 WavefrontParser;
	WavefrontParser.Enable(BFC_MAKE_LOWER_CASE);
	WavefrontParser.Enable(BFC_REMOVE_TABS);
	WavefrontParser.Disable(BFC_REMOVE_SEMICOLON);
	WavefrontParser.Enable(BFC_DISCARD_COMMENTS);
	WavefrontParser.Disable(BFC_DISCARD_UNKNOWNCMDS);
	WavefrontParser.Disable(BFC_DISCARD_INVALIDCMDS);
	WavefrontParser.Disable(BFC_DISCARD_GLOBALCMDS);

	WavefrontParser.SetUserData(&Ctx);
	WavefrontParser.SetParseCallback(gParseCallback);
	WavefrontParser.Execute(Filename);

	if(Ctx.mCurrentMesh)
	{
		Ctx.FinishMesh();
//		Ctx.mCurrentMesh->Recenter();
//		gLegoDatabase->mParts.Add(udword(Ctx.mCurrentMesh));
	}
//	SetDropFile(null);

	return true;
}

bool InitLegoLib(const char* filename, float scale_factor)
{
//	return false;
	printf("Initializing Lego lib...\n");
	udword Time = TimeGetTime();

	gLegoDatabase = ICE_NEW(LegoPartDatabase);

	if(1)
	{	
		LoadLegoFile(0, filename, scale_factor);

		Time = TimeGetTime() - Time;

		printf("Done: (%d)\n", Time);
		printf("- %d parts\n", GetNbLegoParts());
		printf("- %d meshes\n", gLegoDatabase->mMeshes.GetNbEntries());
		printf("- %d actors\n", GetNbLegoActors());
		return true;
	}

	LoadLegoFile(2432, "2432.obj", scale_factor);
	LoadLegoFile(3001, "3001.obj", scale_factor);
	LoadLegoFile(3004, "3004.obj", scale_factor);
	LoadLegoFile(3005, "3005.obj", scale_factor);
	LoadLegoFile(3009, "3009.obj", scale_factor);
	LoadLegoFile(3010, "3010.obj", scale_factor);
	LoadLegoFile(3024, "3024.obj", scale_factor);
	LoadLegoFile(3070, "3070.obj", scale_factor);
	LoadLegoFile(3622, "3622.obj", scale_factor);
	LoadLegoFile(11208, "11208.obj", scale_factor);
	LoadLegoFile(15068, "15068.obj", scale_factor);
	LoadLegoFile(18892, "18892.obj", scale_factor);
	LoadLegoFile(22889, "22889.obj", scale_factor);
	LoadLegoFile(24309, "24309.obj", scale_factor);
	LoadLegoFile(28324, "28324.obj", scale_factor);
	LoadLegoFile(30363, "30363.obj", scale_factor);
	LoadLegoFile(30414, "30414.obj", scale_factor);
	LoadLegoFile(50745, "50745.obj", scale_factor);
	LoadLegoFile(50950, "50950.obj", scale_factor);
	LoadLegoFile(57783, "57783.obj", scale_factor);
	LoadLegoFile(58090, "58090.obj", scale_factor);
	LoadLegoFile(60477, "60477.obj", scale_factor);
	printf("Done (%d parts).\n", GetNbLegoParts());

	return true;
}

void CloseLegoLib()
{
	DELETESINGLE(gLegoDatabase);
}

udword GetNbLegoParts()
{
	return gLegoDatabase ? gLegoDatabase->mParts.GetNbEntries() : 0;
}

const LegoPartMesh* GetLegoPartByID(udword id)
{
	if(!gLegoDatabase)
		return null;
	const udword NbParts = gLegoDatabase->mParts.GetNbEntries();
	for(udword i=0;i<NbParts;i++)
	{
		const LegoPartMesh* P = (const LegoPartMesh*)gLegoDatabase->mParts.GetEntry(i);
		if(P->mID==id)
			return P;
	}
	return null;
}

const LegoPartMesh* GetLegoPartByIndex(udword i)
{
	if(!gLegoDatabase)
		return null;
	const udword NbParts = gLegoDatabase->mParts.GetNbEntries();
	if(i>=NbParts)
		return null;
	return (const LegoPartMesh*)gLegoDatabase->mParts.GetEntry(i);
}

udword GetNbLegoActors()
{
	return gLegoDatabase ? gLegoDatabase->mActors.GetNbEntries() : 0;
}

const LegoActor* GetLegoActorByIndex(udword i)
{
	if(!gLegoDatabase)
		return null;
	const udword NbActors = gLegoDatabase->mActors.GetNbEntries();
	if(i>=NbActors)
		return null;
	return (const LegoActor*)gLegoDatabase->mActors.GetEntry(i);
}

//#pragma optimize( "", on )
