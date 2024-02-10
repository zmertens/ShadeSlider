shader_type canvas_item;

uniform sampler2D SCREEN_TEXTURE : hint_screen_texture, filter_linear_mipmap;

void vertex() {
	// Called for every vertex the material is visible on.
}

void fragment() {
	// Called for every pixel the material is visible on.
	vec3 my_texture_bg = textureLod(SCREEN_TEXTURE, SCREEN_UV, 0.0).rgb;
	float avgs = (my_texture_bg.r + my_texture_bg.g + my_texture_bg.b) / 3.0f;
	COLOR = vec4(avgs, avgs, avgs, 1.0);
	
}

//void light() {
	// Called for every pixel for every light affecting the CanvasItem.
	// Uncomment to replace the default light processing function with this one.
//}
