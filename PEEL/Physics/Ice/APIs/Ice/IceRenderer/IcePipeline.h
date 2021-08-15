///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains pipeline-related code.
 *	\file		IcePipeline.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEPIPELINE_H
#define ICEPIPELINE_H

	enum TriangleSortKey
	{
		TSK_DEFAULT,			//!< Default sort key
		TSK_CENTER,				//!< Sort key = center of triangle
		TSK_CLOSEST_POINT,		//!< Sort key = closest vertex
		TSK_FARTHEST_POINT,		//!< Sort key = farthest vertex

		TSK_FORCE_DWORD		= 0x7fffffff
	};

	// Values used to mask out the TSK bits encoded in PipelineID
	#define	PIPELINE_SHIFT	3
	#define	PIPELINE_MASK	((1<<PIPELINE_SHIFT)-1)

	enum PipelineID
	{
		PIPELINE_OPAQUE,		//!< Opaque pipeline, rendered first, not sorted
		PIPELINE_ALPHA_BLEND,	//!< 1:1 pipeline, rendered second, not sorted
		PIPELINE_ALPHA_SORT,	//!< Alpha pipeline, rendered last, sorted
		PIPELINE_USER,			//!< User pipeline

		PIPELINE_NB,			//!< Number of pipelines

		// Customized alphasort pipelines
		PIPELINE_ALPHA_SORT_CENTER			= PIPELINE_ALPHA_SORT + (TSK_CENTER<<PIPELINE_SHIFT),
		PIPELINE_ALPHA_SORT_CLOSEST_POINT	= PIPELINE_ALPHA_SORT + (TSK_CLOSEST_POINT<<PIPELINE_SHIFT),
		PIPELINE_ALPHA_SORT_FARTHEST_POINT	= PIPELINE_ALPHA_SORT + (TSK_FARTHEST_POINT<<PIPELINE_SHIFT),

		PIPELINE_FORCE_DWORD = 0x7fffffff
	};

	inline_ TriangleSortKey	SortKeyFromPipeline(PipelineID id)
	{
		return TriangleSortKey(id>>PIPELINE_SHIFT);
	}

#endif // ICEPIPELINE_H
