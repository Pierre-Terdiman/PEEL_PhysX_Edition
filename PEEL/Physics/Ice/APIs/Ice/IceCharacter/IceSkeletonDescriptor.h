///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for the skeleton descriptor.
 *	\file		IceSkeletonDescriptor.h
 *	\author		Pierre Terdiman
 *	\date		May, 08, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESKELETONDESCRIPTOR_H
#define ICESKELETONDESCRIPTOR_H

	class ICECHARACTER_API SkeletonDescriptor : public Allocateable
	{
		public:
										SkeletonDescriptor();
										~SkeletonDescriptor();

				bool					Init(udword nb_bones, bool trans=false);

		inline_	udword					GetNbBones()				const	{ return mNbBones;				}
		inline_	const BoneDescriptor*	GetBones()					const	{ return mBones;				}

		// Translations
		inline_	const Point*			GetTranslations()			const	{ return mTranslations;			}
		inline_	const Point&			GetTranslation(udword i)	const	{ return mTranslations[i];		}

		private:
				udword					mNbBones;			//!< Number of bones
				BoneDescriptor*			mBones;				//!< mNbBones bone descriptors
				Point*					mTranslations;		//!< mNbBones bone translations (or null)
	};

#endif // ICESKELETONDESCRIPTOR_H
