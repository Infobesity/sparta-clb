#pragma once

#include "generic.hpp"

#include <Wincodec.h>
#pragma comment(lib, "Windowscodecs")

namespace sparta
{
	namespace gui
	{
		namespace png
		{
			HBITMAP create_bitmap_handle(IWICBitmapSource* bitmap_source);
			bool create_image_stream(const char* resource_name, IStream** image_stream);

			bool extract_bitmap(IStream* image_stream, IWICBitmapSource** bitmap_source);

			HBITMAP make_image(unsigned int resource_id);
		}
	}
}