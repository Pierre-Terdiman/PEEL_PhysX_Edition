///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for standard requests.
 *	\file		IceRequest.h
 *	\author		Pierre Terdiman
 *	\date		July, 8, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEREQUEST_H
#define ICEREQUEST_H

	// Request data

/*	extern ICECORE_API const char* RQ_MESSAGE_BOX;

		struct ICECORE_API MessageBoxParams : public RequestParams
		{
			// In
			String	mCaption;
			String	mMessage;
		};
*/
	//

	extern ICECORE_API const char* RQ_FILESELECT;

		struct ICECORE_API FileselectParams : public RequestParams
		{
			// In
			String	mCaption;
			String	mFiletype;
			// Out
			String	mFilename;
		};

	//

	extern ICECORE_API const char* RQ_BOOLEAN_QUERY;

		struct ICECORE_API BooleanQueryParams : public RequestParams
		{
			// In
			String	mCaption;
			String	mText;
			// Out
			bool	mAnswer;
		};

	//

	extern ICECORE_API const char* RQ_POPUP_MENU;

		struct ICECORE_API PopupMenuParams : public RequestParams
		{
			// In
			Strings	mEntries;
			// Out
			udword	mSelected;
		};

	//

/*	extern ICECORE_API const char* RQ_CREATE_PROGRESS_BAR;
	extern ICECORE_API const char* RQ_DELETE_PROGRESS_BAR;
	extern ICECORE_API const char* RQ_SET_RANGE_PROGRESS_BAR0;
	extern ICECORE_API const char* RQ_SET_RANGE_PROGRESS_BAR1;
	extern ICECORE_API const char* RQ_SET_TEXT_PROGRESS_BAR0;
	extern ICECORE_API const char* RQ_SET_TEXT_PROGRESS_BAR1;
	extern ICECORE_API const char* RQ_UPDATE_PROGRESS_BAR0;
	extern ICECORE_API const char* RQ_UPDATE_PROGRESS_BAR1;

		struct ICECORE_API ProgressBarParams : public RequestParams
		{
			ProgressBarParams()
			{
				mMin = mMax = 0;
				mProgress0		= null;
				mProgress1		= null;
				mProgressWindow	= null;
			}
			// In
			udword	mMin, mMax;			// Range
			String	mText;
			// Out
			HWND	mProgress0;			// Progress bar 0
			HWND	mProgress1;			// Progress bar 1
			HWND	mProgressWindow;	// Window that holds the bar
		};
*/
	class ICECORE_API ProgressBarRequest : public Allocateable
	{
		public:
			ProgressBarRequest(const char* title, udword length);
			~ProgressBarRequest();

			void RequestUpdate();
		private:
//			ProgressBarParams mParams;
			GUIProgressBar*	mPB;
	};

#endif // ICEREQUEST_H
