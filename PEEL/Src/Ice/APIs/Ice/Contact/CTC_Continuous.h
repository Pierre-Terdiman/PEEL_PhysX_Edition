///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for continuous collision detection.
 *	\file		CTC_Continuous.h
 *	\author		Pierre Terdiman
 *	\date		September, 18, 2003
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef CTCCONTINUOUS_H
#define CTCCONTINUOUS_H

#define USE_EDGE_NORMALS

	struct CONTACT_API Velocity6D
	{
		Point	Linear;		//!< Linear velocity
		Point	Angular;	//!< Angular velocity
	};

	class CONTACT_API IntervalPoint
	{
		public:

		inline_					IntervalPoint()		{}
		inline_					IntervalPoint(const IntervalPoint& p) : x(p.x), y(p.y), z(p.z)	{}
		inline_					IntervalPoint(const IceInterval& _x, const IceInterval& _y, const IceInterval& _z) : x(_x), y(_y), z(_z)	{}
		inline_					IntervalPoint(const Point& p0, const Point& p1)
								{
									x = IceInterval(p0.x, p1.x);	if(x.a>x.b)	TSwap(x.a, x.b);
									y = IceInterval(p0.y, p1.y);	if(y.a>y.b)	TSwap(y.a, y.b);
									z = IceInterval(p0.z, p1.z);	if(z.a>z.b)	TSwap(z.a, z.b);
								}
		inline_					~IntervalPoint()	{}

		//! Operator for IntervalPoint Plus = IntervalPoint + IntervalPoint.
		inline_	IntervalPoint	operator+(const IntervalPoint& p)	const	{ return IntervalPoint(x + p.x, y + p.y, z + p.z);	}

		//! Operator for IntervalPoint Minus = IntervalPoint - IntervalPoint.
		inline_	IntervalPoint	operator-(const IntervalPoint& p)	const	{ return IntervalPoint(x - p.x, y - p.y, z - p.z);	}

		//! Operator for Interval DotProd = IntervalPoint | IntervalPoint.
		inline_	IceInterval		operator|(const IntervalPoint& p)	const	{ return x*p.x + y*p.y + z*p.z;						}
		//! Operator for IntervalPoint VecProd = IntervalPoint ^ IntervalPoint.
		inline_	IntervalPoint	operator^(const IntervalPoint& p)	const
								{
									return IntervalPoint(
										y * p.z - z * p.y,
										z * p.x - x * p.z,
										x * p.y - y * p.x);
								}
		//! Computes square magnitude
		inline_	IceInterval		SquareMagnitude()	const		{ return x*x + y*y + z*z;										}

		//! Operator for IntervalPoint *= Matrix4x4.
		inline_	IntervalPoint&	operator*=(const Matrix4x4& mat)
								{
									IceInterval xp = x * mat.m[0][0] + y * mat.m[1][0] + z * mat.m[2][0] + mat.m[3][0];
									IceInterval yp = x * mat.m[0][1] + y * mat.m[1][1] + z * mat.m[2][1] + mat.m[3][1];
									IceInterval zp = x * mat.m[0][2] + y * mat.m[1][2] + z * mat.m[2][2] + mat.m[3][2];

									x = xp;
									y = yp;
									z = zp;
									return *this;
								}

		//! Operator for "IntervalPoint A = IntervalPoint B"
		inline_	void			operator = (const IntervalPoint& interval)
								{
									x = interval.x;
									y = interval.y;
									z = interval.z;
								}

		inline_	void			Set(const IceInterval& _x, const IceInterval& _y, const IceInterval& _z)
								{
									x = _x;
									y = _y;
									z = _z;
								}

		inline_	void			GetMinPoint(Point& min)
								{
									min.x = x.a;
									min.y = y.a;
									min.z = z.a;
								}

		inline_	void			GetMaxPoint(Point& max)
								{
									max.x = x.b;
									max.y = y.b;
									max.z = z.b;
								}

		inline_					operator	const	IceInterval*() const	{ return &x; }
		inline_					operator			IceInterval*()			{ return &x; }

				IceInterval		x,y,z;
	};

	inline_ void CreateBounds(AABB& bounds, const IntervalPoint& pt)
	{
		// Create bounds out of intervals
		bounds.SetMinMax(Point(pt.x.a, pt.y.a, pt.z.a), Point(pt.x.b, pt.y.b, pt.z.b));
	}

	class CONTACT_API IntervalMatrix3x3
	{
		public:
		//! Empty constructor
		inline_					IntervalMatrix3x3()									{}
		//! Constructor from 9 values
		inline_					IntervalMatrix3x3(IceInterval m00, IceInterval m01, IceInterval m02, IceInterval m10, IceInterval m11, IceInterval m12, IceInterval m20, IceInterval m21, IceInterval m22)
								{
									m[0][0] = m00;	m[0][1] = m01;	m[0][2] = m02;
									m[1][0] = m10;	m[1][1] = m11;	m[1][2] = m12;
									m[2][0] = m20;	m[2][1] = m21;	m[2][2] = m22;
								}
		//! Copy constructor
		inline_					IntervalMatrix3x3(const IntervalMatrix3x3& mat)		{ CopyMemory(m, &mat.m, 9*sizeof(IceInterval));	}
		//! Destructor
		inline_					~IntervalMatrix3x3()								{}

		inline_	const IntervalPoint&	operator[](int row)		const	{ return *(const IntervalPoint*)&m[row][0];	}
		inline_	IntervalPoint&			operator[](int row)				{ return *(IntervalPoint*)&m[row][0];		}

		IceInterval				m[3][3];
	};

	CONTACT_API bool ContinuousOBBOBB(const OBB& world_box0, const OBB& world_box1, const Screwing& screw0, const Screwing& screw1);
	CONTACT_API BOOL IsCollidingContact(const Point& world_pt, const Point& world_normal, const Screwing& s0, const Screwing& s1);
	CONTACT_API BOOL IsCollidingContact(	const Point& world_pt, const Point& world_normal,
											const Point& pivot0, const Point& velocity0, const Point& omega0,
											const Point& pivot1, const Point& velocity1, const Point& omega1);
	CONTACT_API void ComputeBounds(AABB& bounds, const Point& world_pt, const IceInterval& it, const Screwing& screwing);
	CONTACT_API BOOL CloseEdgeCollision(const Point& p1, const Point& p2, const Point& p3, const Point& p4, float* dist=null);

	struct CONTACT_API INCLUSIONCREATE
	{
		inline_		INCLUSIONCREATE() : mFlipNormal(FALSE)	{}
		Screwing	mScrewing0;
		Screwing	mScrewing1;
		BOOL		mFlipNormal;
	};

	class CONTACT_API ContinuousTest
	{
		public:
		inline_					ContinuousTest()	{}
		virtual					~ContinuousTest()	{}

		virtual	bool			Init(const INCLUSIONCREATE* create);

		inline_	const Screwing&	GetScrewing0()	const	{ return mScrew0;	}
		inline_	const Screwing&	GetScrewing1()	const	{ return mScrew1;	}
		inline_	udword			GetNbIter()		const	{ return mNbIter;	}

		virtual	void			Inclusion(IceInterval& dst, const IceInterval& src) const	= 0;

		virtual	BOOL			ValidateSolution(float collision_time)	= 0;
		virtual	float			ComputeEpsilon()						= 0;

		virtual	float			Solve(float current_collision_time=MAX_FLOAT);

//				RigidContact	mContact;
//				Contacts		mContacts;
				Container		mContactData;

		protected:
				void			_Solve(const IceInterval& it, float epsilon/*, Container& solutions*/, float& first_time);
				float			Solve(float epsilon, float current_collision_time);

		// Screwing from object 0
				Screwing		mScrew0;

		// Screwing from object 1
				Screwing		mScrew1;

		// Precomputed data
				Matrix4x4		m0to1;

		// Solution
//				Interval		mFirstTime;
//float mExpectedBest;
		// Stats
				udword			mNbIter;
				BOOL			mValidSolution;
				BOOL			mFlipNormal;
	};

	struct CONTACT_API EDGEEDGECREATE : public INCLUSIONCREATE
	{
		Point		mWA;
		Point		mWB;
		Point		mWC;
		Point		mWD;
	};

	class CONTACT_API EdgeEdgeTest : public ContinuousTest
	{
		public:
		inline_					EdgeEdgeTest()		{}
		virtual					~EdgeEdgeTest()		{}

		virtual	bool			Init(const INCLUSIONCREATE* create);
		virtual void			Inclusion(IceInterval& dst, const IceInterval& src) const;
		virtual	BOOL			ValidateSolution(float collision_time);
		virtual	float			ComputeEpsilon();

		protected:
		// Original vertices in world space
				Point			mWA, mWB, mWC, mWD;

				Point			mSA, mSB, mSC, mSD;

		// Current vertices in a common screw space
		mutable	IntervalPoint	mPA,mPB,mPC,mPD;
	};

	struct CONTACT_API VERTEXFACECREATE : public INCLUSIONCREATE
	{
		Point		mWA;
		Point		mWB;
		Point		mWC;
		Point		mWD;
	};

	class CONTACT_API VertexFaceTest : public ContinuousTest
	{
		public:
		inline_					VertexFaceTest()	{}
		virtual					~VertexFaceTest()	{}

		virtual	bool			Init(const INCLUSIONCREATE* create);
		virtual void			Inclusion(IceInterval& dst, const IceInterval& src) const;
		virtual	BOOL			ValidateSolution(float collision_time);
		virtual	float			ComputeEpsilon();

		protected:
		// Original vertices in world space
				Point			mWA, mWB, mWC, mWD;

				Point			mSA, mSB, mSC, mSD;

		// Current vertices in a common screw space
		mutable	IntervalPoint	mPA,mPB,mPC,mPD;
	};

	struct CONTACT_API SPHERESPHERECREATE : public INCLUSIONCREATE
	{
		Point		mWA;
		Point		mWB;
	};

	class CONTACT_API SphereSphereTest : public ContinuousTest
	{
		public:
		inline_					SphereSphereTest()	{}
		virtual					~SphereSphereTest()	{}

		virtual	bool			Init(const INCLUSIONCREATE* create);
		virtual void			Inclusion(IceInterval& dst, const IceInterval& src) const;
		virtual	BOOL			ValidateSolution(float collision_time);
		virtual	float			ComputeEpsilon();

//		protected:
				float			mRadius0, mRadius1;

		// Original vertices in world space
				Point			mWA, mWB;

				Point			mSA, mSB;
	};

	CONTACT_API float VertexFaceTest_(udword nb, const Point& pa, const Point& pb, const Point& pc, const Point& pd,
							const PR& init_pose0, const Point& velocity0, const Point& omega0,
							const PR& init_pose1, const Point& velocity1, const Point& omega1,
							Container* data, BOOL flip_normal);

CONTACT_API float EdgeEdgeTest_(udword nb, const Point& pa, const Point& pb, const Point& pc, const Point& pd,
							const PR& init_pose0, const Point& velocity0, const Point& omega0,
							const PR& init_pose1, const Point& velocity1, const Point& omega1,
							Container* data, BOOL flip_normal
#ifdef USE_EDGE_NORMALS
							,const Point& edge_normal0, const Point& edge_normal1
#endif
							);


#endif // CTCCONTINUOUS_H

