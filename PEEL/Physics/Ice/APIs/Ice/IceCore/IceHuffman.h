///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for Huffman data compression.
 *	\file		IceHuffman.h
 *	\author		Pierre Terdiman / Guillaume Baron, original code from Jetpac 1.0
 *	\date		1996
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEHUFFMAN_H
#define ICEHUFFMAN_H

	struct ICECORE_API HuffNode : public Allocateable
	{
		inline_	HuffNode() : mCount(INVALID_ID), mChild0(INVALID_ID), mChild1(INVALID_ID)	{}

		udword	mCount;
		sdword	mChild0;
		sdword	mChild1;
	};

	struct ICECORE_API HuffCode : public Allocateable
	{
		inline_	HuffCode() : mCode(INVALID_ID), mCodeNbBits(INVALID_ID)	{}

		udword	mCode;
		sdword	mCodeNbBits;
	};

	class ICECORE_API Huffman0
	{
		public:
							Huffman0();
							~Huffman0();

				void		CountBytes(const ubyte* input, udword nb_bytes);
				void		ScaleCounts();
				sdword		BuildTree();
				void		_ConvertTreeToCode(udword code_so_far, sdword bits, sdword node);

				bool		Compress(const ubyte* buffer, udword size, CustomArray& packed);
				void		Decompress(const CustomArray& source, CustomArray& dest);

				udword*		mCounts;
				HuffNode*	mNodes;
				HuffCode*	mCodes;
		private:
				void		CompressData(const ubyte* buffer, udword size, CustomArray& packed);
				void		ExpandData(const CustomArray& source, CustomArray& dest, sdword root_node);
				void		OutputCounts(CustomArray& packed);
				void		InputCounts(const CustomArray& source);
	};

#endif // ICEHUFFMAN_H
