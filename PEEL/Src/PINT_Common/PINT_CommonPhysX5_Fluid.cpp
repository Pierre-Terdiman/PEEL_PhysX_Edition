///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

// WARNING: this file is compiled by all PhysX5 plug-ins, so put only the code here that is "the same" for all versions.

#include "stdafx.h"
#include "PINT_CommonPhysX5_Fluid.h"

#ifdef TEST_FLUIDS
#include "PINT_CommonPhysX3.h"

PxPBDParticleSystem* initParticles(PxPhysics& physics, PxScene& scene, const EditableParams& params, const PxU32 numX, const PxU32 numY, const PxU32 numZ, const PxVec3& position, const PxReal particleSpacing, const PxU32 maxDiffuseParticles)
{
//	printf("initParticles start\n");

	PxCudaContextManager* cudaContextManager = scene.getCudaContextManager();
	if(!cudaContextManager)
		return null;

	PxPBDMaterial* defaultMat = physics.createPBDMaterial(
		params.mFluidFriction,
		params.mFluidDamping,
		params.mFluidAdhesion,
		params.mFluidViscosity,
		params.mFluidVorticityConfinement,
		params.mFluidSurfaceTension,
		params.mFluidCohesion,
		params.mFluidLift,
		params.mFluidDrag,
		params.mFluidCflCoefficient,
		params.mFluidGravityScale
		);

	const PxU32 maxParticles = numX * numY * numZ;
	const PxU32 maxInflatables = 0;
	const PxU32 maxVolumes = 0;
	const PxU32 maxRigids = 0;
	const PxU32 maxCloths = 0;
	const PxU32 maxNeighborhood = 96;
	PxPBDParticleSystem* particleSystem = physics.createPBDParticleSystem(maxParticles, maxInflatables, maxVolumes, maxRigids, maxCloths, *cudaContextManager, maxNeighborhood, maxDiffuseParticles);

	// General particle system setting
	
	//const PxReal fluidDensity = params.mFluidDensity;
	const PxReal restOffset = 0.5f * particleSpacing / 0.6f;
	const PxReal solidRestOffset = restOffset;
	const PxReal fluidRestOffset = restOffset * 0.6f;
	//const PxReal particleMass = fluidDensity * 1.333f * 3.14159f * particleSpacing * particleSpacing * particleSpacing;
	const PxReal particleMass = params.mFluidParticleMass;
	//printf("particleMass: %f\n", particleMass);
	//const PxReal particleMass = 0.1f;

	particleSystem->setRestOffset(restOffset);
	particleSystem->setContactOffset(restOffset + 0.01f);
	particleSystem->setParticleContactOffset(fluidRestOffset / 0.6f);
	particleSystem->setSolidRestOffset(solidRestOffset);
	particleSystem->setFluidRestOffset(fluidRestOffset);
	particleSystem->enableCCD(params.mFluidParticleCCD);
	particleSystem->setMaxVelocity(solidRestOffset*100.f);

	scene.addParticleSystem(*particleSystem);

	// Diffuse particles setting
	PxDiffuseParticleParams dpParams;
	dpParams.maxParticles			= maxDiffuseParticles;
	dpParams.threshold				= 300.0f;
	dpParams.bubbleDrag				= 0.9f;
	dpParams.buoyancy				= 0.9f;
	dpParams.airDrag				= 0.0f;
	dpParams.kineticEnergyWeight	= 0.01f;
	dpParams.pressureWeight			= 1.0f;
	dpParams.divergenceWeight		= 10.f;
	dpParams.lifetime				= 2.0f;
	dpParams.useAccurateVelocity	= false;
	particleSystem->setDiffuseParticleParams(dpParams);

	// Create particles and add them to the particle system
	const PxU32 particlePhase = particleSystem->createPhase(defaultMat, PxParticlePhaseFlags(PxParticlePhaseFlag::eParticlePhaseFluid | PxParticlePhaseFlag::eParticlePhaseSelfCollide));

	PxBuffer* activeIndexBuf = physics.createBuffer(maxParticles * sizeof(PxU32), PxBufferType::eHOST, cudaContextManager);
	PxBuffer* phaseBuf = physics.createBuffer(maxParticles * sizeof(PxU32), PxBufferType::eHOST, cudaContextManager);
	PxBuffer* positionInvMassBuf = physics.createBuffer(maxParticles * sizeof(PxVec4), PxBufferType::eHOST, cudaContextManager);
	PxBuffer* velocityBuf = physics.createBuffer(maxParticles * sizeof(PxVec4), PxBufferType::eHOST, cudaContextManager);

//	printf("checkpoint 0\n");

	PxU32* activeIndex = (PxU32*)activeIndexBuf->map();
	PxU32* phase = (PxU32*)phaseBuf->map();
	PxVec4* positionInvMass = (PxVec4*)positionInvMassBuf->map();
	PxVec4* velocity = (PxVec4*)velocityBuf->map();

	PxReal x = position.x;
	PxReal y = position.y;
	PxReal z = position.z;
	PxReal maxY = y;
	PxReal maxZ = z;

	for (PxU32 i = 0; i < numX; ++i)
	{
		for (PxU32 j = 0; j < numY; ++j)
		{
			for (PxU32 k = 0; k < numZ; ++k)
			{
				const PxU32 index = i * (numY * numZ) + j * numZ + k;

				PxVec4 pos(x, y, z, 1.0f / particleMass);
				activeIndex[index] = index;
				phase[index] = particlePhase;
				positionInvMass[index] = pos;
				velocity[index] = PxVec4(0.0f);

				z += particleSpacing;
			}
			maxZ = z - particleSpacing;
			z = position.z;
			y += particleSpacing;
		}
		maxY = y - particleSpacing;
		y = position.y;
		x += particleSpacing;
	}

	activeIndexBuf->unmap();
	phaseBuf->unmap();
	positionInvMassBuf->unmap();
	velocityBuf->unmap();

//	printf("checkpoint 1\n");

	particleSystem->writeData(PxParticleDataFlag::eACTIVE_PARTICLE, *activeIndexBuf, true);
	particleSystem->writeData(PxParticleDataFlag::ePHASE, *phaseBuf, true);
	particleSystem->writeData(PxParticleDataFlag::ePOSITION_INVMASS, *positionInvMassBuf, true);
	particleSystem->writeData(PxParticleDataFlag::eVELOCITY, *velocityBuf, true);
	particleSystem->setNbActiveParticles(maxParticles);

//	printf("checkpoint 2\n");

	velocityBuf->release();
	positionInvMassBuf->release();
	phaseBuf->release();
	activeIndexBuf->release();

//	printf("initParticles end\n");

//	printf("checkpoint 3\n");

	return particleSystem;
}


#include "../GL/glew.h"
//#define GLUT_NO_LIB_PRAGMA
//#include "../Glut/include/GL/glut.h"

#include "D:\packman-repo\chk\CUDA\10.0\include\cuda.h"
#include "D:\packman-repo\chk\CUDA\10.0\include\cudaGL.h"

#pragma comment(lib, "D:/packman-repo/chk/CUDA/10.0/lib/x64/cuda.lib")
#pragma comment(lib, "D:/packman-repo/chk/CUDA/10.0/lib/x64/cudart_static.lib")
//D:\packman-repo\chk\CUDA\10.0\lib\x64\cudart_static.lib

//#pragma comment(lib, "../GL/glew64.lib")
#pragma comment(lib, "D:/#Projects/PEEL_Timestamp62/Src/GL/glew64.lib")
#pragma comment(lib, "opengl32.lib")

/*void ReportGlutErrors()
{
	glutReportErrors();
}*/

namespace
{
	class SharedGLBuffer : public Allocateable
	{
	public:
		SharedGLBuffer(PxCudaContextManager* contextManager);
		~SharedGLBuffer();
		void allocate(PxU32 sizeInBytes, PxCudaInteropRegisterFlags flags = PxCudaInteropRegisterFlags());
		void release();
		void* mapForCudaAccess();
		void unmap();

	private:
		PxCudaContextManager*	mCudaContextManager;
		void*					mVboRes;
		void*					mDevicePointer;
	public:
		GLuint					mVbo; //Opengl vertex buffer object
		PxU32					mSize;
	};
}

static void createVBO(GLuint* vbo, CUgraphicsResource* vbo_res, PxU32 size, PxCudaInteropRegisterFlags flags = PxCudaInteropRegisterFlags())
{
//	printf("createVBO start\n");

	PX_ASSERT(vbo);

	// create buffer object
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);

	// initialize buffer object
	glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// register this buffer object with CUDA
	CUresult result = cuGraphicsGLRegisterBuffer(vbo_res, *vbo, flags);
	PX_UNUSED(result);
//	printf("cuGraphicsGLRegisterBuffer result: %d\n", result);

//	printf("createVBO end\n");
//	printf("createVBO end %d\n", *vbo);
}

static void deleteVBO(GLuint* vbo, CUgraphicsResource vbo_res)
{
//	printf("deleteVBO start\n");

	// unregister this buffer object with CUDA
	CUresult result = cuGraphicsUnregisterResource(vbo_res);
	PX_UNUSED(result);
//	printf("cuGraphicsUnregisterResource result: %d\n", result);

//	printf("deleteVBO start %d\n", *vbo);

	glBindBuffer(1, *vbo);
	glDeleteBuffers(1, vbo);

	*vbo = 0;
 
//	printf("deleteVBO end\n");
}

//Returns the pointer to the cuda buffer
static void* mapCudaGraphicsResource(CUgraphicsResource* vbo_resource, size_t& numBytes, CUstream stream = 0)
{
//	printf("mapCudaGraphicsResource start\n");

	CUresult result0 = cuGraphicsMapResources(1, vbo_resource, stream);
	PX_UNUSED(result0);
	void* dptr;
	CUresult result1 = cuGraphicsResourceGetMappedPointer((CUdeviceptr*)&dptr, &numBytes, *vbo_resource);
	PX_UNUSED(result1);

//	printf("mapCudaGraphicsResource end\n");
	return dptr;
}

static void unmapCudaGraphicsResource(CUgraphicsResource* vbo_resource, CUstream stream = 0)
{
//	printf("unmapCudaGraphicsResource start\n");
	CUresult result2 = cuGraphicsUnmapResources(1, vbo_resource, stream);
	PX_UNUSED(result2);
//	printf("unmapCudaGraphicsResource end\n");
}

SharedGLBuffer::SharedGLBuffer(PxCudaContextManager* contextManager) : mCudaContextManager(contextManager), mVboRes(null), mDevicePointer(null), mVbo(0), mSize(0)
{
}

void SharedGLBuffer::allocate(PxU32 sizeInBytes, PxCudaInteropRegisterFlags flags)
{
	release();
	mCudaContextManager->acquireContext();
	createVBO(&mVbo, reinterpret_cast<CUgraphicsResource*>(&mVboRes), sizeInBytes, flags);
	mCudaContextManager->releaseContext();
	mSize = sizeInBytes;
}

void SharedGLBuffer::release()
{
	if(mVboRes)
	{
		mCudaContextManager->acquireContext();
		deleteVBO(&mVbo, reinterpret_cast<CUgraphicsResource>(mVboRes));
		mCudaContextManager->releaseContext();
		mVbo = 0;
		mVboRes = null;
	}
}

SharedGLBuffer::~SharedGLBuffer()
{
	//release();
}

void* SharedGLBuffer::mapForCudaAccess()
{
	if(mDevicePointer)
		return mDevicePointer;
	size_t numBytes;
	if(mCudaContextManager)
		mCudaContextManager->acquireContext();
	if(mVboRes)
		mDevicePointer = mapCudaGraphicsResource(reinterpret_cast<CUgraphicsResource*>(&mVboRes), numBytes, 0);
	if(mCudaContextManager)
		mCudaContextManager->releaseContext();
	return mDevicePointer;
}

void SharedGLBuffer::unmap()
{
	if(!mDevicePointer)
		return;
	if(mCudaContextManager)
		mCudaContextManager->acquireContext();
	if(mVboRes)
		unmapCudaGraphicsResource(reinterpret_cast<CUgraphicsResource*>(&mVboRes), 0);
	if(mCudaContextManager)
		mCudaContextManager->releaseContext();
	mDevicePointer = NULL;
}

static SharedGLBuffer* gPosBuffer = null;
static SharedGLBuffer* gDiffusePosLifeBuffer = null;

static PxBuffer* gPositionInvMassCPUBuf = null;

void onBeforeRenderParticles(PxPBDParticleSystem* particleSystem)
{
//	return;
	if(particleSystem) 
	{
		if(gPosBuffer)
			particleSystem->copyData(gPosBuffer->mapForCudaAccess(), PxParticleDataFlag::ePOSITION_INVMASS);
		if(gDiffusePosLifeBuffer)
			particleSystem->copyDiffuseParticleData(gDiffusePosLifeBuffer->mapForCudaAccess(), PxDiffuseParticleDataFlag::eDIFFUSE_POSITION_LIFETIME);
	}
}

void DrawPoints(GLuint vbo, PxU32 numPoints, const PxVec3& color, float scale, PxU32 coordinatesPerPoint=3, PxU32 stride=4 * sizeof(float), size_t offset=0)
{
//	renderer.DrawPoints(MAX_TEST_PARTICLES, reinterpret_cast<const Point*>(particles), sizeof(Particle));

//glDepthMask(FALSE);
//glEnable(GL_BLEND);
//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPointSize(scale);	
	glDisable(GL_LIGHTING);
	glColor4f(color.x, color.y, color.z, 1.0f);
	//glColor4f(color.x, color.y, color.z, 0.1f);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(coordinatesPerPoint, GL_FLOAT, stride, (void*)offset /*offsetof(Vertex, pos)*/);
	glDrawArrays(GL_POINTS, 0, numPoints);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);

//glDisable(GL_BLEND);
//glDepthMask(TRUE);
}

void renderParticles(PintRender& renderer, PxPBDParticleSystem* particleSystem)
{
//	return;

	const float Size = 2.0f;
	//const float Size = 4.0f;

	if(gPosBuffer)
		gPosBuffer->unmap();
	if(gDiffusePosLifeBuffer)
		gDiffusePosLifeBuffer->unmap();

	//const PxVec3 color(0.5f, 0.5f, 1.0f);
	const PxVec3 color(110.0f/255.0f, 147.0f/255.0f, 171.0f/255.0f);
	if(gPosBuffer)
	{
		if(gPositionInvMassCPUBuf)
		{
			particleSystem->readData(PxParticleDataFlag::ePOSITION_INVMASS, *gPositionInvMassCPUBuf);

			PxVec4* positionsInvMass = static_cast<PxVec4*>(gPositionInvMassCPUBuf->map());
			renderer.DrawPoints(gPosBuffer->mSize / sizeof(PxVec4), (const Point*)positionsInvMass, sizeof(PxVec4));

			gPositionInvMassCPUBuf->unmap();	
		}
		DrawPoints(gPosBuffer->mVbo, gPosBuffer->mSize / sizeof(PxVec4), color, Size);
	}

	const PxU32 numActiveDiffuseParticles = particleSystem->getNumActiveDiffuseParticles();
	if (numActiveDiffuseParticles > 0)
	{
		const PxVec3 colorDiffuseParticles(1.0f);
		if(gDiffusePosLifeBuffer)
			DrawPoints(gDiffusePosLifeBuffer->mVbo, numActiveDiffuseParticles, colorDiffuseParticles, Size);
	}

	//Snippets::DrawFrame(PxVec3(0, 0, 0));
}

void allocParticleBuffers(PxPhysics& physics, PxScene& scene, PxPBDParticleSystem* particleSystem)
{
//	printf("allocParticleBuffers start\n");

	PxCudaContextManager* cudaContexManager = scene.getCudaContextManager();
	gPosBuffer = ICE_NEW(SharedGLBuffer)(cudaContexManager);
	gDiffusePosLifeBuffer = ICE_NEW(SharedGLBuffer)(cudaContexManager);

	const PxU32 maxDiffuseParticles = particleSystem->getDiffuseParticleParams().maxParticles;
	const PxU32 maxParticles = particleSystem->getMaxParticles();

	if(gDiffusePosLifeBuffer)
		gDiffusePosLifeBuffer->allocate(maxDiffuseParticles * sizeof(PxVec4));

	if(gPosBuffer)
		gPosBuffer->allocate(maxParticles * sizeof(PxVec4));

	if(0)
		gPositionInvMassCPUBuf = physics.createBuffer(maxParticles * sizeof(PxVec4), PxBufferType::eHOST, cudaContexManager);

//	printf("allocParticleBuffers end\n");
}

void clearupParticleBuffers()
{
//	printf("clearupParticleBuffers start\n");

	PX_RELEASE(gPositionInvMassCPUBuf);

	if(gPosBuffer)
		gPosBuffer->release();
	if(gDiffusePosLifeBuffer)
		gDiffusePosLifeBuffer->release();

	DELETESINGLE(gDiffusePosLifeBuffer);
	DELETESINGLE(gPosBuffer);

//	printf("clearupParticleBuffers end\n");
}


#ifdef REMOVED
void renderCallback(PxPBDParticleSystem* particleSystem)
{
	onBeforeRenderParticles(particleSystem);

/*	stepPhysics(true);

	Snippets::startRender(sCamera);

	PxScene* scene;
	PxGetPhysics().getScenes(&scene,1);
	PxU32 nbActors = scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
	if(nbActors)
	{
		std::vector<PxRigidActor*> actors(nbActors);
		scene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
		Snippets::renderActors(&actors[0], static_cast<PxU32>(actors.size()), true);
	}*/
	
	renderParticles(particleSystem);

//	Snippets::showFPS();

//	Snippets::finishRender();
}
#endif

#endif
