///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	This file contains code from NeuQuant by Anthony Dekker.
 *	\file		IceNeuralNetQuantizer.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICENEURALNETQUANTIZER_H
#define ICENEURALNETQUANTIZER_H

	// CONSTANTS
	#define		NNQ_netsize		256									// number of colours used

	// For 256 colours, fixed arrays need 8kb, plus space for the image

	// four primes near 500 - assume no image has a length so large that it is divisible by all four primes
	#define		NNQ_prime1		499
	#define		NNQ_prime2		491
	#define		NNQ_prime3		487
	#define		NNQ_prime4		503

	#define		NNQ_minpicturebytes		(3*NNQ_prime4)				// minimum size for input image

	typedef int	NNQ_pixel[4];										// BGRc

	// Network Definitions

	#define		NNQ_maxnetpos		(NNQ_netsize-1)
	#define		NNQ_netbiasshift	4								// bias for colour values
	#define		NNQ_ncycles			100								// no. of learning cycles

	// defs for freq and bias
	#define		NNQ_intbiasshift	16								// bias for fractions
	#define		NNQ_intbias			(((int) 1)<<NNQ_intbiasshift)
	#define		NNQ_gammashift		10								// gamma = 1024
	#define		NNQ_gamma			(((int) 1)<<NNQ_gammashift)
	#define		NNQ_betashift		10
	#define		NNQ_beta			(NNQ_intbias>>NNQ_betashift)	// beta = 1/1024
	#define		NNQ_betagamma		(NNQ_intbias<<(NNQ_gammashift-NNQ_betashift))

	// defs for decreasing radius factor
	#define		NNQ_initrad			(NNQ_netsize>>3)				// for 256 cols, radius starts
	#define		NNQ_radiusbiasshift	6								// at 32.0 biased by 6 bits
	#define		NNQ_radiusbias		(((int) 1)<<NNQ_radiusbiasshift)
	#define		NNQ_initradius		(NNQ_initrad*NNQ_radiusbias)	// and decreases by a
	#define		NNQ_radiusdec		30								// factor of 1/30 each cycle

	// defs for decreasing alpha factor
	#define		NNQ_alphabiasshift	10								// alpha starts at 1.0
	#define		NNQ_initalpha		(((int) 1)<<NNQ_alphabiasshift)

	// radbias and alpharadbias used for radpower calculation
	#define		NNQ_radbiasshift	8
	#define		NNQ_radbias			(((int) 1)<<NNQ_radbiasshift)
	#define		NNQ_alpharadbshift	(NNQ_alphabiasshift+NNQ_radbiasshift)
	#define		NNQ_alpharadbias	(((int) 1)<<NNQ_alpharadbshift)

	//! Creation structure
	struct ICEIMAGEWORK_API QUANTIZERCREATE
	{
				// In
				ubyte*			Field;				//!< Input field
				udword			Length;				//!< H*W*3
				udword			Sample;				//!< 0...30
	};

	class ICEIMAGEWORK_API NeuralNetQuantizer : public Allocateable
	{
		public:
								NeuralNetQuantizer();
								~NeuralNetQuantizer();
		// Main method
				bool			Quantize(const QUANTIZERCREATE& qc);

		inline_	ubyte*			GetPalette()			const	{ return mPalette;		}
		inline_	ubyte*			GetQuantizedPic()		const	{ return mQuantizedPic;	}

		private:
				NNQ_pixel		mNetwork[NNQ_netsize];		//!< the network itself
				int				mNetindex[256];				//!< for network lookup - really 256
				int				mBias[NNQ_netsize];			//!< bias and freq arrays for learning
				int				mFreq[NNQ_netsize];
				int				mRadpower[NNQ_initrad];		//!< radpower for precomputation
		// Results
				ubyte*			mPalette;					//!< RGB palette
				ubyte*			mQuantizedPic;				//!< List of color indices
		// Internal methods
				int				Search(int b, int g, int r);
				int				Contest(int b, int g, int r);
				void			AlterSingle(int alpha, int i, int b, int g, int r);
				void			AlterNeigh(int rad, int i, int b, int g, int r);
	};

#endif // ICENEURALNETQUANTIZER_H
