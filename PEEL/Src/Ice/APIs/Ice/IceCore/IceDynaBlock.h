
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEDYNABLOCK_H
#define ICEDYNABLOCK_H

	// ### Experimental & obsolete, don't use

	struct ICECORE_API DynamicBlock
	{
						DynamicBlock()	{ Length = 0; Data = null;			}
						~DynamicBlock()	{ Release();						}

				bool	Release()		{ DELETEARRAY(Data); return true;	}

				bool	Init(udword length)
						{
								Release();
								Data = new ubyte[length];
								CHECKALLOC(Data);
								Length = length;
								return true;
						}

				bool	Compare(DynamicBlock* blc)
						{
							if(!blc)				return false;
							if(blc->Length!=Length)	return false;
							ubyte* Data2 = blc->Data;
							for(udword i=0;i<Length;i++)
								if(Data[i]!=Data2[i])	return false;
							return true;
						}

		udword			Length;
		ubyte*			Data;
	};

	class ICECORE_API DataBlock : public Cell
	{
						DECLARE_ICE_CLASS(DataBlock, Cell);
						DECLARE_FIELDS;

			// Initialize
						bool			Init(udword length)										{ return mBlock.Init(length);			}

						DynamicBlock	mBlock;
	};

#endif // ICEDYNABLOCK_H
