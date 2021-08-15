///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for text rendering.
 *	\file		IceTextRenderer.h
 *	\author		Pierre Terdiman
 *	\date		January, 1, 2005
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETEXTRENDERER_H
#define ICETEXTRENDERER_H

	ICERENDERER_API bool GenerateTextQuads(const char* text, udword nb_characters, 
							PRDT_Vertex* fnt_verts, uword* fnt_indices, const ClipBox& clip_box, const FntData* fnt_data, float& x, float& y, float scale_x, float scale_y, udword color=ARGB_WHITE,
							float* x_min=null, float* y_min=null, float* x_max=null, float* y_max=null, udword* nb_lines=null, udword* nb_active_characters=null);

	ICERENDERER_API bool GenerateTextQuads2(const char* text, udword nb_characters, 
							PRDT_Vertex* fnt_verts, uword* fnt_indices, const ClipBox& clip_box, const FntData2* fnt_data, float& x, float& y, float scale_x, float scale_y, udword color=ARGB_WHITE,
							float* x_min=null, float* y_min=null, float* x_max=null, float* y_max=null, udword* nb_lines=null, udword* nb_active_characters=null);

#endif // ICETEXTRENDERER_H
