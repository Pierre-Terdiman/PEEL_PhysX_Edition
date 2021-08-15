
float x = 0;
float y = 0;
float z = 0;

DWORD startTime;

#define NUMBER_OF_STARS		500

	struct Star
	{
		Point	position;
		Point	direction;
		float	lifetime;
	};

	Star AllStars[NUMBER_OF_STARS];

	static bool init = false;

Function(PrepareFireworks)
{
	startTime = timeGetTime();

	SetCamera(Point(4.01f, 48.33f, 41.51f), Point(-0.12f, 0.11f, -0.99f));
}

Function(ResetFireworks)
{
	startTime = timeGetTime();
	x = y = z = 0;
	init = false;
}

static void DrawStar(const Point& pt)
{
	const float scale = 0.1f;

	float red = UnitRandomFloat();
	float green = UnitRandomFloat();
	float blue = UnitRandomFloat();
	Point color(red, green, blue);

	DrawLine(pt - Point(scale, 0.0f, 0.0f), pt + Point(scale, 0.0f, 0.0f), color);
	DrawLine(pt - Point(0.0f, scale, 0.0f), pt + Point(0.0f, scale, 0.0f), color);
	DrawLine(pt - Point(0.0f, 0.0f, scale), pt + Point(0.0f, 0.0f, scale), color);
}

Function(DrawFireworks)
{
	DWORD currentTime = timeGetTime();
	if(currentTime<startTime + 3000)
	{
		DrawStar(Point(x, y, z));
		// Fireworks is moving up
//		x = x + 0.001;
		if(!gPaused)
			y = y + 0.02f;
	}
	else
	{
		if(!init)
		{
			init = true;
			// Fireworks should explode here
			for(int i=0;i<NUMBER_OF_STARS;i++)
			{
				AllStars[i].position = Point(x, y, z);
				AllStars[i].direction = Point(UnitRandomFloat()-0.5f, UnitRandomFloat()-0.5f, UnitRandomFloat()-0.5f);
				AllStars[i].direction.Normalize();
				AllStars[i].lifetime = 1000 + UnitRandomFloat()*500.0f;
			}
		}

		for(int i=0;i<NUMBER_OF_STARS;i++)
		{
			if(AllStars[i].lifetime>0.0f)
				DrawStar(AllStars[i].position);
		}

		if(!gPaused)
		{
		for(int i=0;i<NUMBER_OF_STARS;i++)
		{
			AllStars[i].position += AllStars[i].direction * 0.02f;
			AllStars[i].direction.y -= 9.81f * 0.0001f;
			AllStars[i].lifetime -= 1.0f;
		}
		}

	}
}