///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a signature class.
 *	\file		IceSignature.h
 *	\author		Pierre Terdiman
 *	\date		October, 24, 2002
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESIGNATURE_H
#define ICESIGNATURE_H

	class ICECORE_API Signature
	{
		public:
						Signature();
						~Signature();

		inline_	void	Invalidate()		{ mTimestamp++;	}

						PREVENT_COPY(Signature)
		private:
				Handle	mStructureHandle;	//!< Object's structural validity
				udword	mTimestamp;			//!< Object's state validity

		friend	class	SignatureState;
	};

	class ICECORE_API SignatureState
	{
		public:
		inline_			SignatureState() :
							mStructureHandle(INVALID_ID),
							mTimestamp(INVALID_ID)
						{
						}
		inline_			SignatureState(const Signature& signature) :
							mStructureHandle(signature.mStructureHandle),
							mTimestamp(signature.mTimestamp)
						{
						}
		inline_			~SignatureState()		{}

		inline_	void	operator = (const Signature& signature)
						{
							mStructureHandle = signature.mStructureHandle;
							mTimestamp = signature.mTimestamp;
						}

		inline_	BOOL	operator == (const Signature& signature)	const
						{
							if(signature.mTimestamp!=mTimestamp)				return FALSE;
							if(signature.mStructureHandle!=mStructureHandle)	return FALSE;
							return TRUE;
						}

		inline_	BOOL	operator != (const Signature& signature)	const
						{
							if(signature.mTimestamp!=mTimestamp)				return TRUE;
							if(signature.mStructureHandle!=mStructureHandle)	return TRUE;
							return FALSE;
						}
		private:
				Handle	mStructureHandle;	//!< Object's structural validity
				udword	mTimestamp;			//!< Object's state validity
	};

#endif // ICESIGNATURE_H
