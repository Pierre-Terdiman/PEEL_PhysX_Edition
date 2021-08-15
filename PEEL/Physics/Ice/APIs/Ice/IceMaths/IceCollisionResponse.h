///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for standard collision response.
 *	\file		IceCollisionResponse.h
 *	\author		Pierre Terdiman
 *	\date		September, 12, 2004
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef ICECOLLISIONRESPONSE_H
#define ICECOLLISIONRESPONSE_H

	inline_ void CollisionResponse(Point& target_position, const Point& current_position, const Point& current_dir, const Point& hit_normal, float bump, float friction, bool normalize=false)
	{
		// Compute reflect direction
		Point ReflectDir;
		ComputeReflexionVector(ReflectDir, current_dir, hit_normal);
		ReflectDir.Normalize();

		// Decompose it
		Point NormalCompo, TangentCompo;
		DecomposeVector(NormalCompo, TangentCompo, ReflectDir, hit_normal);
		if(normalize)
		{
			NormalCompo.Normalize();
			TangentCompo.Normalize();
		}

		// Compute new destination position
		const float Amplitude = target_position.Distance(current_position);

		target_position = current_position;
		if(IR(bump))		target_position += NormalCompo*(bump*Amplitude);
		if(IR(friction))	target_position += TangentCompo*(friction*Amplitude);
	// ### Hack introduced for Konoko Payne
	//if(target_position.y<current_position.y)	target_position.y = current_position.y;
	//target_position.y = current_position.y;
	}

#endif	// ICECOLLISIONRESPONSE_H
