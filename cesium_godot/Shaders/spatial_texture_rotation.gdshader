shader_type spatial;

uniform sampler2D albedo_texture : source_color;
uniform vec2 uv_offset = vec2(0.0);
uniform vec2 uv_scale = vec2(1.0);
uniform float uv_rotation = 0.0;

vec2 rotate_uv(vec2 uv, float angle) {
	float s = sin(angle);
	float c = cos(angle);
	mat2 rot = mat2(c, -s, s, c);
	return rot * uv;
}

void fragment() {
	vec2 uv = UV;
	uv *= uv_scale;
	uv = rotate_uv(uv, uv_rotation);
	uv += uv_offset;
	vec4 albedo = texture(albedo_texture, uv);
	ALBEDO = albedo.rgb;
	ALPHA = albedo.a;
}