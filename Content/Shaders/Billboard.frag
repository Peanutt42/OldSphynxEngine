#version 450

layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_color;
layout (location = 2) in flat float in_texindex;

layout (location = 0) out vec4 out_Color;

layout (binding = 1) uniform sampler2D image0;
layout (binding = 2) uniform sampler2D image1;
layout (binding = 3) uniform sampler2D image2;
layout (binding = 4) uniform sampler2D image3;
layout (binding = 5) uniform sampler2D image4;
layout (binding = 6) uniform sampler2D image5;
layout (binding = 7) uniform sampler2D image6;
layout (binding = 8) uniform sampler2D image7;
layout (binding = 9) uniform sampler2D image8;
layout (binding = 10) uniform sampler2D image9;
layout (binding = 11) uniform sampler2D image10;
layout (binding = 12) uniform sampler2D image11;
layout (binding = 13) uniform sampler2D image12;
layout (binding = 14) uniform sampler2D image13;
layout (binding = 15) uniform sampler2D image14;
layout (binding = 16) uniform sampler2D image15;
layout (binding = 17) uniform sampler2D image16;
layout (binding = 18) uniform sampler2D image17;
layout (binding = 19) uniform sampler2D image18;
layout (binding = 20) uniform sampler2D image19;
layout (binding = 21) uniform sampler2D image20;
layout (binding = 22) uniform sampler2D image21;
layout (binding = 23) uniform sampler2D image22;
layout (binding = 24) uniform sampler2D image23;
layout (binding = 25) uniform sampler2D image24;
layout (binding = 26) uniform sampler2D image25;
layout (binding = 27) uniform sampler2D image26;
layout (binding = 28) uniform sampler2D image27;
layout (binding = 29) uniform sampler2D image28;
layout (binding = 30) uniform sampler2D image29;
layout (binding = 31) uniform sampler2D image30;
layout (binding = 32) uniform sampler2D image31;

void main() {
	vec4 image_color = vec4(1.0);
	switch (int(in_texindex)) {
		case 0:  image_color = texture(image0, in_uv); break;
		case 1:  image_color = texture(image1, in_uv); break;
		case 2:  image_color = texture(image2, in_uv); break;
		case 3:  image_color = texture(image3, in_uv); break;
		case 4:  image_color = texture(image4, in_uv); break;
		case 5:  image_color = texture(image5, in_uv); break;
		case 6:  image_color = texture(image6, in_uv); break;
		case 7:  image_color = texture(image7, in_uv); break;
		case 8:  image_color = texture(image8, in_uv); break;
		case 9:  image_color = texture(image9, in_uv); break;
		case 10: image_color = texture(image10, in_uv); break;
		case 11: image_color = texture(image11, in_uv); break;
		case 12: image_color = texture(image12, in_uv); break;
		case 13: image_color = texture(image13, in_uv); break;
		case 14: image_color = texture(image14, in_uv); break;
		case 15: image_color = texture(image15, in_uv); break;
		case 16: image_color = texture(image16, in_uv); break;
		case 17: image_color = texture(image17, in_uv); break;
		case 18: image_color = texture(image18, in_uv); break;
		case 19: image_color = texture(image19, in_uv); break;
		case 20: image_color = texture(image20, in_uv); break;
		case 21: image_color = texture(image21, in_uv); break;
		case 22: image_color = texture(image22, in_uv); break;
		case 23: image_color = texture(image23, in_uv); break;
		case 24: image_color = texture(image24, in_uv); break;
		case 25: image_color = texture(image25, in_uv); break;
		case 26: image_color = texture(image26, in_uv); break;
		case 27: image_color = texture(image27, in_uv); break;
		case 28: image_color = texture(image28, in_uv); break;
		case 29: image_color = texture(image29, in_uv); break;
		case 30: image_color = texture(image30, in_uv); break;
		case 31: image_color = texture(image31, in_uv); break;
	}

	// just a hacky solution so that multiple billboards kinda can overlap
	// proper transparency would need to be implemented differently!
	if (image_color.a == 0.0) {
        discard;
    }

	out_Color = image_color * vec4(in_color, 1.0);
}