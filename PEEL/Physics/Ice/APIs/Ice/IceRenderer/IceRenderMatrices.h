///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for render matrices.
 *	\file		IceRenderMatrices.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICERENDERMATRICES_H
#define ICERENDERMATRICES_H

	class ICERENDERER_API WorldMatrix : public Matrix4x4
	{
		public:
		//! Constructor
		inline_						WorldMatrix()								{}
		//! Copy constructor
		inline_						WorldMatrix(const Matrix4x4& mat)			{ CopyMemory(m, &mat.m, 16*sizeof(float));	}
		//! Copy constructor
		inline_						WorldMatrix(const WorldMatrix& mat)			{ CopyMemory(m, &mat.m, 16*sizeof(float));	}
		//! Destructor.
		inline_						~WorldMatrix()								{}
		// Compute a world matrix.
					Matrix4x4&		World(const PRS& prs);
					Matrix4x4&		World(const PR& pr);
	};

	class ICERENDERER_API ViewMatrix : public Matrix4x4
	{
		public:
		//! Constructor
		inline_						ViewMatrix()								{}
		//! Constructor
		inline_						ViewMatrix(const Point& o, const Point& t)	{ View(o, t);	}
		//! Copy constructor
		inline_						ViewMatrix(const Matrix4x4& mat)			{ CopyMemory(m, &mat.m, 16*sizeof(float));	}
		//! Copy constructor
		inline_						ViewMatrix(const ViewMatrix& mat)			{ CopyMemory(m, &mat.m, 16*sizeof(float));	}
		//! Destructor.
		inline_						~ViewMatrix()								{}
		// Compute a view matrix.
					Matrix4x4&		View(const PRS& prs);
					Matrix4x4&		View(const PR& pr);
					Matrix4x4&		View(const Point& origin, const Point& target);
		// Undo previous transform
					Matrix4x4&		Unview(const ViewMatrix& view);
					Quat			GetViewQuat()						const;

		inline_		void			GetUpVector(Point& up)				const	{ GetCol(0, up);	}
		inline_		void			GetRightVector(Point& right)		const	{ GetCol(1, right);	}
		inline_		void			GetViewVector(Point& view)			const	{ GetCol(2, view);	}

		inline_		void			SetUpVector(const Point& up)				{ SetCol(0, up);	}
		inline_		void			SetRightVector(const Point& right)			{ SetCol(1, right);	}
		inline_		void			SetViewVector(const Point& view)			{ SetCol(2, view);	}
	};

	class ICERENDERER_API ProjMatrix : public Matrix4x4
	{
		public:
		//! Constructor
		inline_						ProjMatrix()								{}
		//! Copy constructor
		inline_						ProjMatrix(const Matrix4x4& mat)			{ CopyMemory(m, &mat.m, 16*sizeof(float));	}
		//! Copy constructor
		inline_						ProjMatrix(const ProjMatrix& mat)			{ CopyMemory(m, &mat.m, 16*sizeof(float));	}
		//! Destructor.
		inline_						~ProjMatrix()								{}
		// Compute a perspective projection matrix.
					Matrix4x4&		Perspective(float width, float height, float fov, float znear, float zfar, bool infinite_clip);
		// Compute a orthographic projection matrix.
					Matrix4x4&		Orthographic(float width, float height, float fov, float znear, float zfar, bool infinite_clip);

					BOOL			IsOrtho()							const;
					float			ExtractZNear()						const;
					float			ExtractZFar()						const;
					void			Tweak(float coeff);
	};

#endif // ICERENDERMATRICES_H
