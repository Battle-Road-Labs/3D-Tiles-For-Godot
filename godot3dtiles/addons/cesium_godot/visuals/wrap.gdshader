shader_type spatial;

#include "res://addons/cesium_godot/visuals/include/wrap_mode.gdshaderinc"

uniform sampler2D albedo_texture: repeat_enable;
uniform int albedo_texture_wrap_mode = 0;
uniform vec3 uv_offset = vec3(0.0);
uniform vec3 uv_scale = vec3(1.0);

void fragment() {
	vec2 base_uv = UV * uv_scale.xy + uv_offset.xy;
	ALBEDO = textureWrapped(albedo_texture, base_uv, albedo_texture_wrap_mode).rgb;
}