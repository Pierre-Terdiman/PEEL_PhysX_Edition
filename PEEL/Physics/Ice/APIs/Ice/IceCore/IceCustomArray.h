///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a custom array class.
 *	\file		IceCustomArray.h
 *	\author		Pierre Terdiman
 *	\date		January, 15, 1999
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECUSTOMARRAY_H
#define ICECUSTOMARRAY_H

	//! Default size of a memory block
	#define CUSTOMARRAY_BLOCKSIZE	(4*1024)		// 4 Kb => heap size

/*	//! A memory block
	class ICECORE_API CustomBlock
	{
		public:
							CustomBlock();
							~CustomBlock();

		void*				Addy;					//!< Stored data
		udword				Size;					//!< Length of stored data
		udword				Max;					//!< Heap size
	};
*/
	//! A linked list of blocks
	class ICECORE_API CustomCell : public Allocateable
	{
		public:
							CustomCell();
							~CustomCell();

//		CustomBlock			Item;
		// Memory block
		void*				Addy;					//!< Stored data
		udword				Size;					//!< Length of stored data
		udword				Max;					//!< Heap size
		CustomCell*			NextCustomCell;
	};

	class ICECORE_API CustomArray : public Allocateable
	{
		public:
								CustomArray(udword start_size=CUSTOMARRAY_BLOCKSIZE, void* input_buffer=null);
								CustomArray(CustomArray& array);
//								CustomArray(const char* filename);
								~CustomArray();

				CustomArray&	Clean();

		// Store methods

			// Store a BOOL
				CustomArray&	Store(BOOL bo);
			// Store a bool
				CustomArray&	Store(bool bo);

			// Store a char (different from "signed char" AND "unsigned char")
				CustomArray&	Store(char b);
			// Store a ubyte
		inline_	CustomArray&	Store(ubyte b)
								{
									ASSERT(sizeof(ubyte)==sizeof(char));
									return Store(char(b));
								}
			// Store a sbyte
		inline_	CustomArray&	Store(sbyte b)
								{
									ASSERT(sizeof(sbyte)==sizeof(char));
									return Store(char(b));
								}

			// Store a short / sword
				CustomArray&	Store(short w);
			// Store a uword
		inline_	CustomArray&	Store(uword w)
								{
									ASSERT(sizeof(uword)==sizeof(short));
									return Store(short(w));
								}

			// Store a unsigned int / udword. "int / sdword" is handled as a BOOL.
				CustomArray&	Store(udword d);
				CustomArray&	Store(uqword d);
			// Store a long
		inline_	CustomArray&	Store(long d)
								{
									ASSERT(sizeof(long)==sizeof(udword));
									return Store(udword(d));
								}
			// Store a unsigned long
		inline_	CustomArray&	Store(unsigned long d)
								{
									ASSERT(sizeof(unsigned long)==sizeof(udword));
									return Store(udword(d));
								}

			// Store a float
				CustomArray&	Store(float f);
			// Store a double
				CustomArray&	Store(double f);

			// Store a string
				CustomArray&	Store(const char* string);
			// Store a buffer
				CustomArray&	Store(const void* buffer, udword size);
			// Store a CustomArray
				CustomArray&	Store(CustomArray* array);

		// ASCII store methods

			// Store a BOOL in ASCII
				CustomArray&	StoreASCII(BOOL bo);
			// Store a bool in ASCII
				CustomArray&	StoreASCII(bool bo);

			// Store a char in ASCII
				CustomArray&	StoreASCII(char b);
			// Store a sbyte in ASCII
		inline_	CustomArray&	StoreASCII(sbyte b)		{ return StoreASCII(char(b));	}
			// Store a ubyte in ASCII
				CustomArray&	StoreASCII(ubyte b);

			// Store a short in ASCII
				CustomArray&	StoreASCII(short w);
			// Store a uword in ASCII
				CustomArray&	StoreASCII(uword w);

			// Store a long in ASCII
				CustomArray&	StoreASCII(long d);
			// Store a unsigned long in ASCII
				CustomArray&	StoreASCII(unsigned long d);

			// Store a int in ASCII
			//	CustomArray&	StoreASCII(int d);
			// Store a unsigned int in ASCII
				CustomArray&	StoreASCII(unsigned int d);

			// Store a float in ASCII
				CustomArray&	StoreASCII(float f);
			// Store a double in ASCII
				CustomArray&	StoreASCII(double f);

			// Store a string in ASCII
				CustomArray&	StoreASCII(const char* string);

		// Bit storage - inlined since mostly used in data compression where speed is welcome

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Stores a bit.
		 *	\param		bit		[in] the bit to store
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	CustomArray&	StoreBit(bool bit)
								{
									mBitMask<<=1;				// Shift current bitmask
//									if(bit)	mBitMask |= 1;		// Set the LSB if needed
									mBitMask |= (ubyte)bit;		// Set the LSB if needed - no conditional branch here
									mBitCount++;				// Count number of active bits in bitmask
									if(mBitCount==8)			// If bitmask is full...
									{
										mBitCount = 0;			// ...then reset counter...
										Store(mBitMask);		// ...and store bitmask.
									}
									return *this;
								}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Store bits.
		 *	\param		bits	[in] the bit pattern to store
		 *	\param		nb_bits	[in] the number of bits to store
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	CustomArray&	StoreBits(udword bits, udword nb_bits)
								{
									udword Mask=1<<(nb_bits-1);
									while(Mask)
									{
										StoreBit((bits&Mask)!=0);
										Mask>>=1;
									}
									return *this;
								}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Reads a bit.
		 *	\return		the bit
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	bool			GetBit()	const
								{
									if(!mBitCount)
									{
										mBitMask = ReadByte();
										mBitCount = 0x80;
									}
									bool Bit = (mBitMask&mBitCount)!=0;
									mBitCount>>=1;
									return Bit;
								}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Reads bits.
		 *	\param		nb_bits		[in] the number of bits to read
		 *	\return		the bits
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	udword			GetBits(udword nb_bits)	const
								{
									udword Bits = 0;
									while(nb_bits--)
									{
										Bits<<=1;
										bool Bit = GetBit();
										Bits|=udword(Bit);
									}
									return Bits;
								}

		// Padds extra bits to reach a byte
				CustomArray&	EndBits();

		// Management methods
				bool			ExportToDisk(const char* filename, const char* access=null);
				bool			ExportToDisk(FILE* fp);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets current number of bytes stored.
		 *	\return		number of bytes stored
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				udword			GetSize()	const;
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Padds offset on a 8 bytes boundary.
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				CustomArray&	Padd();
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Collapses a CustomArray into a single continuous buffer. This invalidates all pushed addies.
		 *	If you provide your destination buffer original bytes are copied into it, then it's safe using them.
		 *	If you don't, returned address is valid until the array's destructor is called. Beware of memory corruption...
		 *	\param		user_buffer		[out] destination buffer (provided or not)
		 *	\return		destination buffer
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				void*			Collapse(void* user_buffer=null);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets the current address within the current block.
		 *	\return		destination buffer
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	void*			GetAddress()	const
								{
									char* CurrentAddy = (char*)mCurrentCell->Addy;
									CurrentAddy += mCurrentCell->Size;
									return CurrentAddy;
								}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets used size within current block.
		 *	\return		used size / offset
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	udword			GetCellUsedSize()	const
								{
									return mCurrentCell->Size;
								}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets max size within current block.
		 *	\return		max size / limit
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	udword			GetCellMaxSize()	const
								{
									return mCurrentCell->Max;
								}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Gets remaining size within current block.
		 *	\return		remaining size / limit - offset
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	udword			GetCellRemainingSize()	const
								{
									return mCurrentCell->Max - mCurrentCell->Size;
								}

		// Address methods
				bool			PushAddress();
				CustomArray&	PopAddressAndStore(BOOL Bo);
				CustomArray&	PopAddressAndStore(bool Bo);
				CustomArray&	PopAddressAndStore(char b);
				CustomArray&	PopAddressAndStore(ubyte b);
				CustomArray&	PopAddressAndStore(short w);
				CustomArray&	PopAddressAndStore(uword w);
				CustomArray&	PopAddressAndStore(long d);
				CustomArray&	PopAddressAndStore(unsigned long d);
			//	CustomArray&	PopAddressAndStore(int d);
				CustomArray&	PopAddressAndStore(unsigned int d);
				CustomArray&	PopAddressAndStore(float f);
				CustomArray&	PopAddressAndStore(double f);

		// Read methods
				ubyte			ReadByte()								const;	//!< Read a byte from the array
				uword			ReadWord()								const;	//!< Read a word from the array
				udword			ReadDword()								const;	//!< Read a dword from the array
				uqword			ReadQword()								const;	//!< Read a qword from the array
				float			ReadFloat()								const;	//!< Read a float from the array
				const ubyte*	ReadString()							const;	//!< Read a string from the array
				const void*		ReadBuffer(udword size)					const;	//!< Read a buffer from the array
				bool			ReadBuffer(void* buffer, udword size)	const;	//!< Read a buffer from the array
		inline_	bool			ReadBool()								const	{ return ReadByte()!=0;	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Sets the current address within current block. Input offset can't be greater than current block's length.
		 *	\param		offset	[in] the wanted offset within the block.
		 *	\return		Self-Reference.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				CustomArray&	Reset(udword offset=0);
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Finds a given chunk (a sequence of bytes) in current buffer.
		 *	\param		chunk	[in] chunk you're looking for (must end with a null byte).
		 *	\return		address where the chunk has been found, or null if not found.
		 *	\warning	chunk length limited to 1024 bytes...
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				void*			GetChunk(const char* chunk);

				CustomArray&	operator=(CustomArray& array);
		private:
				CustomCell*		mCurrentCell;				//!< Current block cell
				CustomCell*		mInitCell;					//!< First block cell

				void*			mCollapsed;					//!< Possible collapsed buffer
				void**			mAddresses;					//!< Stack to store addresses
				void*			mLastAddress;				//!< Last address used in current block cell
				uword			mNbPushedAddies;			//!< Number of saved addies
				uword			mNbAllocatedAddies;			//!< Number of allocated addies
		// Bit storage
		mutable	ubyte			mBitCount;					//!< Current number of valid bits in the bitmask
		mutable	ubyte			mBitMask;					//!< Current bitmask
		// Management methods
				void			Init(udword start_size, void* input_buffer, FILE* fp, udword used_size);
				CustomArray&	Empty();
				CustomArray&	CheckArray(udword bytes_needed);
				bool			NewBlock(CustomCell* previous_cell, udword size=0);
				bool			SaveCell(CustomCell* p, FILE* fp);
				CustomArray&	StoreASCIICode(char code);
		inline_	void*			PrepareAccess(udword size);
	};

#endif // ICECUSTOMARRAY_H
