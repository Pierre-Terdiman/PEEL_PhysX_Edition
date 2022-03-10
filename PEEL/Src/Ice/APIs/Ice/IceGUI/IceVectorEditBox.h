///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICEVECTOREDITBOX_H
#define ICEVECTOREDITBOX_H

	class VectorEditBox;
	typedef void (*VectorEditCB)	(const VectorEditBox& edit_box);

	class ICEGUI_API VectorEditDesc : public WindowDesc
	{
		public:
								VectorEditDesc() : mCallback(null)
								{
									mEditBoxWidth = 40;
									mValue.Zero();
								}

				Point			mValue;
				udword			mEditBoxWidth;
				VectorEditCB	mCallback;
	};

	class ICEGUI_API VectorEditBox : public IceWindow
	{
		public:
								VectorEditBox(const VectorEditDesc& desc);
		virtual					~VectorEditBox();

				bool			GetVector(Point& vector)	const;
				void			SetVector(const Point& vector);

		inline_	VectorEditCB	GetCallback()				{ return mCallback;	}
		private:
				VectorEditCB	mCallback;
				IceLabel*		mLabel[3];
				IceEditBox*		mEditBox[3];
				IceScrollbar*	mScrollBar[3];

				PREVENT_COPY(VectorEditBox);
	};

#endif	// ICEVECTOREDITBOX_H