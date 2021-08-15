///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains texture synthesis-related code.
 *	\file		IceTextureSynthesis.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICETEXTURESYNTHESIS_H
#define ICETEXTURESYNTHESIS_H

	// Constants got from Efros
	static const float _ERR_THRESHOLD			= 0.1f;
	static const float _INIT_MAX_ERR_THRESHOLD	= 0.3f; 
	static const float _MAX_ERR_THRESHOLD_SCALE	= 1.1f;

	/*
class Image
{
public:
  typedef float ElementType;

  Image(void);
  Image(const int height, const int width, const int depth
	) throw (Memory, Size);
  Image(const char *fileName) throw(Memory, Size, File);
  Image(constImage & original) throw(Memory);
  virtual ~Matrix3D(void);
  
  // assignment operator
  Image & operator = (constImage & original) throw(Memory);



  // clear 
  constImage & Clear(const ElementType & element);

  // file IO stuff
  // return 0 if fail, 1 if successful
  int Read(const char * filename);
  int Write(const char * filename) const;

};
*/

	class ICEIMAGEWORK_API EfrosTexture : public Allocateable
	{
	public:
							EfrosTexture();
			virtual			~EfrosTexture();

					bool	GrowImage(FloatPicture& sampleimage, FloatPicture& weightimage, FloatPicture& maskimage, FloatPicture& resultimage);
	protected:
			virtual	bool	SetProgress(float percent);
					bool	CheckArguments(FloatPicture& sampleimage, FloatPicture& weightimage, FloatPicture& maskimage, FloatPicture& resultimage);
					float	SSD(FloatPicture& sampleimage, FloatPicture& weightimage, FloatPicture& maskimage, FloatPicture& resultimage, sdword samplerow, sdword samplecol, sdword resultrow, sdword resultcol);
					sdword	FindMatches(FloatPicture& sampleimage, FloatPicture& weightimage, FloatPicture& maskimage, FloatPicture& resultimage, sdword row, sdword col, float errthreshold);
					sdword	GetUnfilledNeighbors(FloatPicture& maskimage, FloatPicture& weightimage);
					sdword	NumValidNeighbors(FloatPicture& maskimage, FloatPicture& weightimage, sdword row, sdword col);

					// The coordinates of the best matches
					sdword*	mBestMatches;
					// The coordinates of the unassigned pixels
					sdword*	mPixelList;

					// Temporary array for holding the ssd values
					float*	mSSDArray;
	};

#endif // ICETEXTURESYNTHESIS_H


#ifdef _EFROS_TEXTURE_HPP

protected:
  // no copy nor assignment
  EfrosTexture(const EfrosTexture & original);
  EfrosTexture & operator=(const EfrosTexture & original);
#endif
