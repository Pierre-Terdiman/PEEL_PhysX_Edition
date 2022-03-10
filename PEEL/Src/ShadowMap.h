#ifndef SHADOWMAP_H
#define SHADOWMAP_H

//#define FAR_DIST 200.0
#define MAX_SPLITS 4
//#define LIGHT_FOV 45.0

//--------------------------------------------------------------------------
class ShadowMap
{
public:
	ShadowMap( int w, int h, float fovi, int matOffseti, int resolution, int nb_splits);

	void makeShadowMap(const Point& cameraPos, const Point& cameraDir, const Point& lightDir, float znear, float zfar,
		void (*renderShadowCasters)());

	// call before the map is used for rendering
	void prepareForRender(const float* cam_modelview, const float* cam_proj);
	// call when rendering is done
	void doneRender();

	inline_	int    getTextureSize()		{ return depth_size;		}
	inline_	GLuint getDepthTexArray()	{ return depth_tex_ar;		}
	inline_	float  getFarBound(int i)	{ return far_bound[i];		}
	inline_	int    getNumSplits()		{ return cur_num_splits;	}

private:
	struct Frustum {
		float neard;
		float fard;
		float fov;
		float ratio;
		Point point[8];
	};

	struct Vec4;
	struct Matrix44;

	void updateFrustumPoints(Frustum& f, const Point& center, const Point& view_dir);
	void updateSplitDist(Frustum f[MAX_SPLITS], float nd, float fd);
	float applyCropMatrix(Frustum& f);

	void init();

	float far_bound[MAX_SPLITS];

	float minZAdd;
	float maxZAdd;

	float shadowOff;
	float shadowOff2;
	float fov;

	int cur_num_splits;
	int width;
	int height;
	int depth_size ;

	GLuint depth_fb;
	GLuint depth_tex_ar;
	
	Frustum f[MAX_SPLITS];
	float shad_cpm[MAX_SPLITS][16];

	float split_weight;
	int matOffset;
};
#endif