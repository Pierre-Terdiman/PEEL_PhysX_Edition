///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	This file contains code to compute inertia tensors and centers of mass.
 *	\file		IceVolumeIntegration.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEVOLUMEINTEGRATION_H
#define ICEVOLUMEINTEGRATION_H

	//! Creation structure
	struct ICEMATHS_API TENSORCREATE
	{
		// Properties
		double			mDensity;				//!< Solid density

		// Geometry
		udword			mNbVerts;				//!< Number of vertices
		const Point*	mVerts;					//!< List of vertices

		// Topology
		udword			mNbFaces;				//!< Number of faces
		const udword*	mFaces;					//!< List of faces
	};

	//! Result structure
	struct ICEMATHS_API INTEGRALSRESULT
	{
		Point		mCOM;						//!< Center of mass
		double		mMass;						//!< Total mass
		double		mInertiaTensor[3][3];		//!< Intertia tensor (mass matrix) relative to the origin
		double		mCOMInertiaTensor[3][3];	//!< Intertia tensor (mass matrix) relative to the COM

		void		GetInertia(Matrix3x3& inertia)
		{
			for(udword j=0;j<3;j++)
			{
				for(udword i=0;i<3;i++)
				{
					inertia.m[i][j] = (float)mCOMInertiaTensor[i][j];
				}
			}
		}
	};

	// A volume integrator
	class ICEMATHS_API VolumeIntegrator : public Allocateable
	{
		public:
									VolumeIntegrator();
									~VolumeIntegrator();

				bool				Init(const TENSORCREATE& sc);
				bool				ComputeVolumeIntegrals(INTEGRALSRESULT& ir);
		private:
		// Data structures
				struct TRIFACE
				{
					double			Norm[3];
					double			w;
					udword			Verts[3];
				};

				struct POLYHEDRON
				{
					udword			NbVerts;
					udword			NbFaces;
					double*			Verts;
					TRIFACE*		Faces;
				};

				double				mMass;					//!< Mass
				double				mDensity;				//!< Density

				int					mA;						//!< Alpha
				int					mB;						//!< Beta
				int					mC;						//!< Gamma

		// Projection integrals
				double				mP1;
				double				mPa;					//!< Pi Alpha
				double				mPb;					//!< Pi Beta
				double				mPaa;					//!< Pi Alpha^2
				double				mPab;					//!< Pi AlphaBeta
				double				mPbb;					//!< Pi Beta^2
				double				mPaaa;					//!< Pi Alpha^3
				double				mPaab;					//!< Pi Alpha^2Beta
				double				mPabb;					//!< Pi AlphaBeta^2
				double				mPbbb;					//!< Pi Beta^3

		// Face integrals
				double				mFa;					//!< FAlpha
				double				mFb;					//!< FBeta
				double				mFc;					//!< FGamma
				double				mFaa;					//!< FAlpha^2
				double				mFbb;					//!< FBeta^2
				double				mFcc;					//!< FGamma^2
				double				mFaaa;					//!< FAlpha^3
				double				mFbbb;					//!< FBeta^3
				double				mFccc;					//!< FGamma^3
				double				mFaab;					//!< FAlpha^2Beta
				double				mFbbc;					//!< FBeta^2Gamma
				double				mFcca;					//!< FGamma^2Alpha

		// The 10 volume integrals
				double				mT0;					//!< ~Total mass
				double				mT1[3];					//!< Location of the center of mass
				double				mT2[3];					//!< Moments of inertia
				double				mTP[3];					//!< Products of inertia

		// Working mesh
				POLYHEDRON			mP;						//!< The solid

		// Internal methods
				Point				ComputeCenterOfMass();
				void				ComputeInertiaTensor(double* J);
				void				ComputeCOMInertiaTensor(double* J);

				void				ComputeProjectionIntegrals(const TRIFACE& f);
				void				ComputeFaceIntegrals(const TRIFACE& f);
	};

#endif	// ICEVOLUMEINTEGRATION_H

