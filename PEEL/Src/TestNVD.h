///////////////////////////////////////////////////////////////////////////////
/*
 *	PEEL - Physics Engine Evaluation Lab
 *	Copyright (C) 2012 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/blog.htm
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef TEST_NVD_H
#define TEST_NVD_H

	class TestNVD
	{
		public:
			TestNVD();
			~TestNVD();

		void	Init();
		void	Close();
		void	Sync();
		void	Update();
		void	SetGravity(const Point& gravity);
	};

#endif
