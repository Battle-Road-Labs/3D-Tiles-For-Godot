vec2 map_uv_wrapping(vec2 uv, int mode) {
	switch (mode)  {
		case 1:
			uv = clamp(uv, 0.0, 1.0);
			break;
		case 2:
			uv = fract(uv / 2.0);
			uv = min(uv, 1.0 - uv) * 2.0;
			break;
		case 3:
			uv = clamp(abs(uv), -1.0, 1.0);
			break;
	}
	return uv;
}

vec4 textureWrapped(sampler2D samp, vec2 uv, int mode) {
	float lod_level = textureQueryLod(samp, uv).x;
	uv = map_uv_wrapping(uv, mode);
	return textureLod(samp, uv, lod_level);
	//return texture(samp, uv);
}

vec4 textureLodWrapped(sampler2D samp, vec2 uv, float lod, int mode){
	uv = map_uv_wrapping(uv, mode);
	return textureLod(samp, uv, lod);
}