///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TOOL_TERRAIN_EDITOR_H
#define TOOL_TERRAIN_EDITOR_H

#include "ToolRayBased.h"

	class TerrainData;

	class ToolTerrainEditor : public ToolRayBased
	{
		public:
								ToolTerrainEditor();
		virtual					~ToolTerrainEditor();

		virtual	void			CreateUI			(PintGUIHelper& helper, IceWidget* parent, Widgets& owner);

		virtual	void			Reset				(udword pint_index);
		virtual	void			Deselect			()			{ Reset(INVALID_ID);	}

		virtual	void			OnObjectReleased	(Pint& pint, PintActorHandle removed_object);

		virtual	void			KeyboardCallback	(Pint& pint, udword pint_index, unsigned char key, bool down);

		virtual	void			MouseMoveCallback	(const MouseInfo& mouse);

		virtual	void			RightDownCallback	(Pint& pint, udword pint_index);
		virtual	void			RightDragCallback	(Pint& pint, udword pint_index);
		virtual	void			RightUpCallback		(Pint& pint, udword pint_index);

		// GUI_RenderInterface
		virtual	void			RenderCallback		(PintRender& render, Pint& pint, udword pint_index)	override;
		virtual	void			PostRenderCallback	()													override;
		//~GUI_RenderInterface

				bool			CreateGridMesh(Pint& pint);
				//bool			CreateTexture(Pint& pint);

				TerrainData*	mSelectedTerrain;
				PintActorHandle	mSelectedTerrainActor;
				PintShapeHandle	mSelectedTerrainShape;
				Pint*			mSelectedEngine;

				ComboBoxPtr		mLayerComboBox;
				ComboBoxPtr		mOpComboBox;
				ListBoxPtr		mLayerListBox;
				ButtonPtr		mAddLayer;
				ButtonPtr		mRemoveLayer;

				enum DelayedAction
				{
					NONE,
					CREATE_GRID,
					//CREATE_TEXTURE,
				};
				DelayedAction	mPendingAction;

				bool			mModifyTerrain;

				void			OnListBoxLayerSelected();
				void			OnComboBoxLayerSelected();
				void			UpdateUI();
				void			UpdateTerrain();
				void			UpdateTexture();
				HeightLayer*	GetSelectedLayer()	const;
				void			AddLayer();
				void			RemoveLayer();

		inline_	void			SetSelectedPointers(TerrainData* data, PintActorHandle actor, PintShapeHandle shape, Pint* engine)
								{
									mSelectedTerrain		= data;
									mSelectedTerrainActor	= actor;
									mSelectedTerrainShape	= shape;
									mSelectedEngine			= engine;
								}

		inline_	void			ResetSelectedPointers()
								{
									SetSelectedPointers(null, null, null, null);
								}
	};

#endif
