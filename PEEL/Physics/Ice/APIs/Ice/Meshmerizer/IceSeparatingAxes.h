///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICESEPARATINGAXES_H
#define ICESEPARATINGAXES_H

	// ### TODO better later

	// This class holds a list of potential separating axes.
	// - the orientation is irrelevant so V and -V should be the same vector
	// - the scale is irrelevant so V and n*V should be the same vector
	// - a given separating axis should appear only once in the class
	class SeparatingAxes
	{
	/*	SeparatingAxes()
		{
		}
		~SeparatingAxes()
		{
		}
	*/
		public:

		bool AddAxis(const Point& axis);

	//	private:

		Vertices	mAxes;
	};

#endif	// ICESEPARATINGAXES_H

