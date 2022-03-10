///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolTexture.h"
#include "TextureManager.h"
#include "GLRenderHelpers.h"
#include "GUI_Helpers.h"

extern udword gScreenWidth;
extern udword gScreenHeight;

///////////////////////////////////////////////////////////////////////////////

	class TextureComboBox : public IceComboBox
	{
		public:
						TextureComboBox(const ComboBoxDesc& desc) : IceComboBox(desc), mOwner(null)	{}
		virtual			~TextureComboBox()															{}

		virtual	void	OnComboBoxEvent(ComboBoxEvent event)
		{
			if(event==CBE_SELECTION_CHANGED)
			{
				const udword Selected = GetSelectedIndex();

				if(GetUserData())
				{
					const SystemTexture* ST = GetSystemTexture(Selected);
					if(ST)
					{
						ASSERT(mOwner);
						mOwner->mEditBox_Width->SetLabel(_F("%d", ST->mWidth));
						mOwner->mEditBox_Height->SetLabel(_F("%d", ST->mHeight));
						mOwner->mST = true;
					}
				}
				else
				{
					const ManagedTexture* MT = GetManagedTexture(Selected);
					if(MT)
					{
						ASSERT(mOwner);
						mOwner->mEditBox_Width->SetLabel(_F("%d", MT->mWidth));
						mOwner->mEditBox_Height->SetLabel(_F("%d", MT->mHeight));
						mOwner->mST = false;
					}
				}
			}
		}
				ToolTexture*	mOwner;
	};

///////////////////////////////////////////////////////////////////////////////

ToolTexture::ToolTexture() :
	mTextureComboBox(null),
	mTimestamp		(INVALID_ID),
	mTimestamp2		(INVALID_ID),
	mMinX			(10),
	mMinY			(10),
	mMaxX			(10),
	mMaxY			(10),
	mOffsetX		(0),
	mOffsetY		(0),
	mCapture		(false),
	mST				(false)
{
}

ToolTexture::~ToolTexture()
{
}

void ToolTexture::Select()
{
}

void ToolTexture::Deselect()
{
	//printf("ToolTexture::Deselect\n");
}

void ToolTexture::Reset(udword pint_index)
{
	//printf("ToolTexture::Reset\n");
	if(mTextureComboBox)
		mTextureComboBox->RemoveAll();
	if(mTextureComboBox2)
		mTextureComboBox2->RemoveAll();
	mMinX = mMinY = 10;
	mMaxX = mMaxY = 10;
	mOffsetX = mOffsetY = 0;
	mCapture = false;
	mST = false;
}

void ToolTexture::KeyboardCallback(Pint& pint, udword pint_index, unsigned char key, bool down)
{
}

void ToolTexture::RightDownCallback(Pint& pint, udword pint_index)
{
	if(mX>=mMinX && mX<=mMaxX && mY>=mMinY && mY<=mMaxY)
	{
		mCapture = true;
		mOffsetX = mMinX - mX;
		mOffsetY = mMinY - mY;
	}
	else mCapture = false;
}

void ToolTexture::RightDragCallback(Pint& pint, udword pint_index)
{
	if(mCapture)
	{
		mMinX = mX + mOffsetX;
		mMinY = mY + mOffsetY;
		//printf("%d %d\n", mMinX, mMinY);
	}
}

void ToolTexture::RightUpCallback(Pint& pint, udword pint_index)
{
	mCapture = false;
}

void ToolTexture::UpdateUI()
{
	if(mTextureComboBox)
	{
		mTextureComboBox->RemoveAll();
		const udword NbTextures = GetNbManagedTextures();
		for(udword i=0;i<NbTextures;i++)
		{
			const ManagedTexture* MT = GetManagedTexture(i);

			const char* UIName = MT->mFilename;
			if(!UIName)
				UIName = _F("(Unnamed texture %d)", i);

			mTextureComboBox->Add(UIName);
		}
		mTextureComboBox->Select(INVALID_ID);
	}

	if(mTextureComboBox2)
	{
		mTextureComboBox2->RemoveAll();
		const udword NbTextures = GetNbSystemTextures();
		for(udword i=0;i<NbTextures;i++)
		{
			const SystemTexture* ST = GetSystemTexture(i);

			const char* UIName = ST->mName;
			if(!UIName)
				UIName = _F("(Unnamed texture %d)", i);

			mTextureComboBox2->Add(UIName);
		}
		mTextureComboBox2->Select(INVALID_ID);
	}
}

void ToolTexture::FinalRenderCallback()
{
	if(!mTextureComboBox && !mTextureComboBox2)
		return;

	const udword CurrentTimestamp = GetManagedTexturesTimestamp();
	if(CurrentTimestamp!=mTimestamp)
	{
		mTimestamp = CurrentTimestamp;
		UpdateUI();
	}

	const udword CurrentTimestamp2 = GetSystemTexturesTimestamp();
	if(CurrentTimestamp2!=mTimestamp2)
	{
		mTimestamp2 = CurrentTimestamp2;
		UpdateUI();
	}

	sdword TexWidth = 0;
	sdword TexHeight = 0;
	const Point color(1.0f, 1.0f, 1.0f);
	const float Alpha = 1.0f;
	udword screen_width = gScreenWidth;
	udword screen_height = gScreenHeight;
	GLuint GLID = 0;
	if(mST)
	{
		const udword Selected = mTextureComboBox2->GetSelectedIndex();
		const SystemTexture* ST = GetSystemTexture(Selected);
		if(ST)
		{
			TexWidth = ST->mWidth;
			TexHeight = ST->mHeight;
			GLID = ST->mGLID;
		}
	}
	else
	{
		const udword Selected = mTextureComboBox->GetSelectedIndex();
		const ManagedTexture* MT = GetManagedTexture(Selected);
		if(MT)
		{
			TexWidth = MT->mWidth;
			TexHeight = MT->mHeight;
			GLID = MT->mGLID;
		}
	}

	const float Scale = GetFloat(1.0f, mEditBox_Scale);
	//printf("Scale: %f\n", Scale);
	TexWidth = sdword(float(TexWidth) * Scale);
	TexHeight = sdword(float(TexHeight) * Scale);

	mMaxX = mMinX + TexWidth;
	mMaxY = mMinY + TexHeight;

	{
		const sdword BorderSize = 20;
		const sdword LimitX = sdword(gScreenWidth)-BorderSize;
		const sdword LimitY = sdword(gScreenHeight)-BorderSize;
		if(mMinX>LimitX)
			mMinX = LimitX;
		if(mMinY>LimitY)
			mMinY = LimitY;
		if(mMaxX<BorderSize)
		{
			mMaxX = BorderSize;
			mMinX = BorderSize - TexWidth;
		}
		if(mMaxY<BorderSize)
		{
			mMaxY = BorderSize;
			mMinY = BorderSize - TexHeight;
		}
	}

	if(GLID)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, GLID);
		//glBindTexture(GL_TEXTURE_RECTANGLE_ARB, GLID);

		const float x_start = float(mMinX)/float(screen_width);
		const float y_start = 1.0f - (float(mMinY)/float(screen_height));
		const float x_end = float(mMinX+TexWidth)/float(screen_width);
		const float y_end = 1.0f - float(mMinY+TexHeight)/float(screen_height);

		//if(x_end>0.0f && y_end>0.0f && x_start<1.0f && y_start<1.0f)
			GLRenderHelpers::DrawRectangle(x_start, x_end, y_start, y_end, color, color, Alpha, screen_width, screen_height, false, GLRenderHelpers::SQT_TEXTURING);
		glDisable(GL_TEXTURE_2D);
	}
}

///////////////////////////////////////////////////////////////////////////////

#include "GUI_Helpers.h"

void ToolTexture::CreateUI(PintGUIHelper& helper, IceWidget* parent, Widgets& owner)
{
	const sdword OffsetX = 130;
	const sdword EditBoxWidth = 60;
	const sdword LabelWidth = 100;
	const sdword LabelOffsetY = 2;
	const sdword YStep = 20;
	const sdword x = 4;

	sdword y = 0;

	helper.CreateLabel(parent, x, y+LabelOffsetY, LabelWidth, 20, "Managed textures:", &owner);
	mTextureComboBox = CreateComboBox<TextureComboBox>(parent, 0, x+OffsetX, y, 250, 20, "Texture", &owner, null);
	mTextureComboBox->mOwner = this;
	mTextureComboBox->SetUserData(null);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, LabelWidth, 20, "System textures:", &owner);
	mTextureComboBox2 = CreateComboBox<TextureComboBox>(parent, 0, x+OffsetX, y, 250, 20, "Texture", &owner, null);
	mTextureComboBox2->mOwner = this;
	mTextureComboBox2->SetUserData(this);
	y += YStep;

	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 100, 20, "Width:", &owner);
	mEditBox_Width = helper.CreateEditBox(parent, 0, x+OffsetX, y, EditBoxWidth, 20, "-", &owner, EDITBOX_INTEGER_POSITIVE, null, null);
	mEditBox_Width->SetEnabled(false);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 100, 20, "Height:", &owner);
	mEditBox_Height = helper.CreateEditBox(parent, 1, x+OffsetX, y, EditBoxWidth, 20, "-", &owner, EDITBOX_INTEGER_POSITIVE, null, null);
	mEditBox_Height->SetEnabled(false);
	y += YStep;

	helper.CreateLabel(parent, x, y+LabelOffsetY, 100, 20, "Scale:", &owner);
	mEditBox_Scale = helper.CreateEditBox(parent, 1, x+OffsetX, y, EditBoxWidth, 20, "1.0", &owner, EDITBOX_FLOAT_POSITIVE, null, null);
	y += YStep;
}


