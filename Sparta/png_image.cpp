#include "png_image.hpp"

namespace sparta
{
	namespace gui
	{
		namespace png
		{
			HBITMAP create_bitmap_handle(IWICBitmapSource* bitmap_source)
			{
				unsigned int width = 0;
				unsigned int height = 0;

				if (bitmap_source->GetSize(&width, &height) != S_OK)
				{
					return NULL;
				}

				BITMAPINFO bitmap_info;
				memset(&bitmap_info, 0, sizeof(BITMAPINFO));

				bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bitmap_info.bmiHeader.biWidth = width;
				bitmap_info.bmiHeader.biHeight -= height;
				bitmap_info.bmiHeader.biPlanes = 1;
				bitmap_info.bmiHeader.biBitCount = 32;
				bitmap_info.bmiHeader.biCompression = BI_RGB;
					
				HDC hdc_screen = GetDC(NULL);

				void* image_bits;
				HBITMAP bitmap = CreateDIBSection(hdc_screen, &bitmap_info, DIB_RGB_COLORS, &image_bits, NULL, 0);

				ReleaseDC(NULL, hdc_screen);

				if (!bitmap)
				{
					return NULL;
				}

				const unsigned int stride = width * 4;
				const unsigned int image = stride * height;

				if (bitmap_source->CopyPixels(NULL, stride, image, static_cast<unsigned char*>(image_bits)) != S_OK)
				{
					DeleteObject(bitmap);
					return NULL;
				}

				return bitmap;
			}

			bool create_image_stream(const char* resource_name, IStream** image_stream)
			{
				HRSRC resource = FindResource(NULL, resource_name, "PNG");

				if (!resource)
				{
					return false;
				}

				unsigned long resource_size = SizeofResource(NULL, resource);
				HGLOBAL global_image = LoadResource(NULL, resource);

				if (!global_image)
				{
					return false;
				}

				void* source_resource_data = LockResource(global_image);

				if (!source_resource_data)
				{
					return false;
				}

				HGLOBAL global_resource_data = GlobalAlloc(GMEM_MOVEABLE, resource_size);

				if (!global_resource_data)
				{
					return false;
				}

				void* resource_data = GlobalLock(global_resource_data);

				if (!resource_data)
				{
					GlobalFree(global_resource_data);
					return false;
				}

				CopyMemory(resource_data, source_resource_data, resource_size);
				GlobalUnlock(global_resource_data);

				if (CreateStreamOnHGlobal(global_resource_data, TRUE, image_stream) != S_OK)
				{
					GlobalFree(global_resource_data);
					return false;
				}

				return true;
			}

			bool extract_bitmap(IStream* image_stream, IWICBitmapSource** bitmap_source)
			{
				IWICBitmapDecoder* image_decoder;

				if (CoCreateInstance(CLSID_WICPngDecoder1, NULL, CLSCTX_INPROC_SERVER, __uuidof(image_decoder), reinterpret_cast<void**>(&image_decoder)) != S_OK)
				{
					return false;
				}

				if (image_decoder->Initialize(image_stream, WICDecodeMetadataCacheOnLoad) != S_OK)
				{
					image_decoder->Release();
				}

				unsigned int frame_count = 0;

				if (image_decoder->GetFrameCount(&frame_count) != S_OK)
				{
					image_decoder->Release();
				}

				IWICBitmapFrameDecode* image_frame = NULL;

				if (image_decoder->GetFrame(0, &image_frame) != S_OK)
				{
					image_decoder->Release();
				}

				bool ret = (WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, image_frame, bitmap_source) == S_OK);
					
				image_frame->Release();
				image_decoder->Release();
				return ret;
			}

			HBITMAP make_image(unsigned int resource_id)
			{
				IStream* image_stream = NULL;

				if (!create_image_stream(MAKEINTRESOURCE(resource_id), &image_stream))
				{
					return NULL;
				}

				IWICBitmapSource* bitmap_source = NULL;

				if (!extract_bitmap(image_stream, &bitmap_source))
				{
					image_stream->Release();
					return NULL;
				}

				HBITMAP bitmap = create_bitmap_handle(bitmap_source);

				bitmap_source->Release();
				image_stream->Release();
				return bitmap;
			}
		}
	}
}