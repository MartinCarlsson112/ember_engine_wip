#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
#include <iostream>
#include "buffer_object.h"
#include "transition_image_layout.h"

namespace em
{
	void texture::destroy(const em::device& device)
	{
		device.destroy_image(io);
	}

	void texture::load(const em::device& device, const char* file_path)
	{
		int tex_width, tex_height, tex_channels;
		stbi_uc* pixels = stbi_load(file_path, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

		if (!pixels) {
			std::cout << std::string("failed to load image: ") + file_path << std::endl;
		}
		else
		{
			device.load_image((char*)pixels, tex_width, tex_height, io, image_options{});
			stbi_image_free(pixels);
		}
	}

}

