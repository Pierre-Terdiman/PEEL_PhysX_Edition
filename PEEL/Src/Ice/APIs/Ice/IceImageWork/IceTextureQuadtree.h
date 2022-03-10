///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Texture quadtree classes.
 *	\file		IceTextureQuadtree.h
 *	\author		Pierre Terdiman, original code by Thatcher Ulrich
 *	\date		February, 25, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETEXTUREQUADTREE_H
#define ICETEXTUREQUADTREE_H

	// Manages a disk-based texture quadtree.
	class ICEIMAGEWORK_API TextureQuadtree : public Allocateable
	{
		public:
							TextureQuadtree(const char* filename);
							~TextureQuadtree();

		inline_	bool		is_valid()		const { return m_source != NULL; }
		inline_	int			get_depth()		const { return m_depth; }
		inline_	int			get_tile_size()	const { return m_tile_size; }

				udword		get_texture_id(int level, int col, int row)	const;
				Picture*	load_image(int level, int col, int row)		const;
			
		private:
			Container		m_toc;
			int				m_depth;
			int				m_tile_size;
			IceFile*		m_source;
	};

	FUNCTION ICEIMAGEWORK_API bool	is_tqt_file(const char* filename);
	FUNCTION ICEIMAGEWORK_API int	node_count(int depth);
	FUNCTION ICEIMAGEWORK_API int	node_index(int level, int col, int row);

#endif // ICETEXTUREQUADTREE_H
