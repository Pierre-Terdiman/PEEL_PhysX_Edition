#ifdef _FRAGMENT_

uniform vec3 camPos;
varying vec3 varyPos;

uniform vec3 topCol;
uniform vec3 middleCol;
uniform vec3 bottomCol;
uniform float colorSpread;

vec3 render( in vec3 ro, in vec3 rd )
{ 
	vec3 col;
	if(rd.y>0.0)
		col = mix(middleCol, topCol, rd.y*colorSpread);
	else
		col = mix(middleCol, bottomCol, -rd.y*colorSpread);
	return vec3(clamp(col, 0.0, 1.0));
}

void main()
{
	vec3 rd = normalize(varyPos.xyz);

	vec3 col = render( camPos, rd );
//	col = pow( col, vec3(0.4545) );	   
	gl_FragColor = vec4( col, 1.0 );
}

#endif
