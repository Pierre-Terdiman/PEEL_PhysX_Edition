///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef PEEL_H
#define PEEL_H

#include "PintRenderPass.h"
#include "PintDef.h"

	HWND						GetWindowHandle();
	const char*					GetRoot();

	bool						IsPaused();
	void						SetPause(bool paused);

	void						ImportFile(const char* filename);
	void						MergeFile(const char* filename);
	void						ExportScene(String* string=null);

	struct CameraData;
	CameraData&					GetCamera();

	class DefaultControlInterface;
	DefaultControlInterface&	GetDefaultControlInterface();
	void						SetDefaultControlInterface();

	const char*					GetFilenameForExport(const char* extension, const char* target=null);
	float						GetPickingDistance();

	class Pint;
	class VisibilityManager;
	udword						GetNbEngines();
	bool						ValidateEngine(udword i);
	Pint*						GetEngine(udword i);
//	VisibilityManager*			GetVisHelper(udword i);

	void						DrawScene(PintRenderPass render_pass);
	void						DrawSQResults(PintRenderPass render_pass);

	class PhysicsTest;
	void						EnableRendering(bool b);
	void						EnableWireframeOverlay(bool b);
	void						ActivateTest(PhysicsTest* test=null);
	void						SetRandomizeOrder(bool b);
	void						SetTrashCache(bool b);

	void						UpdateToolComboBox();

	void						SetRectangleSelection(sdword x0, sdword x1, sdword y0, sdword y1);

	float						GetFrameSize();
	bool						GetSymmetricFrames();
	bool						UseFatFrames();

	Point						UI_GetGravity();
	void						UI_SetGravity(const Point& gravity);

	bool						UI_GetSceneDesc(String& text);
	void						UI_SetSceneDesc(const char*);

	void						UI_UpdateCameraSpeed();

	void						UI_HideEditWindows();

	float						ComputeConstantScale(const Point& pos, const Matrix4x4& view, const Matrix4x4& proj);

	void						SetFocusShape(PintActorHandle ah = null, PintShapeHandle sh = null);
	void						SetFocusActor(PintActorHandle h = null);

#endif
