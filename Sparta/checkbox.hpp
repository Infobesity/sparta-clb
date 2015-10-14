#pragma once

#include "generic.hpp"
#include "widget.hpp"

#include "png_image.hpp"
#include "resource.hpp"

namespace sparta
{
	namespace gui
	{
		typedef std::function<void(bool)> checkbox_function_t;

		class checkbox : public widget
		{
		public:
			checkbox(HWND hwnd_parent = NULL, HINSTANCE instance = NULL)
				: widget(hwnd_parent, instance), checked(false)
			{

			}
			
			checkbox(widget& parent_widget, std::string const& caption = "", rectangle& rect = rectangle(), checkbox_function_t function_on_click = [](bool checked) -> void {  })
				: widget(parent_widget.get_handle(), NULL), checked(false)
			{
				if (!this->assemble(parent_widget, caption, rect, function_on_click))
				{
					throw std::exception("Failed to create 'checkbox' widget");
				}
			}

			~checkbox()
			{

			}
			
			bool assemble(widget& parent_widget, std::string const& caption = "", rectangle& rect = rectangle(), checkbox_function_t function_on_click = [](bool checked) -> void {  })
			{
				rect.set_height(16);

				if (!this->create(WC_BUTTON, caption, rect, WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, NULL, parent_widget.get_handle()))
				{
					return false;
				}

				this->function_on_click = function_on_click;
				this->set_message_handlers();
				return true;
			}

			void set_check(bool checked)
			{
				this->checked = checked;

				if (this->function_on_click)
				{
					this->function_on_click(this->checked);
				}

				RedrawWindow(this->get_handle(), NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
			}
				
			bool get_check()
			{
				return this->checked;
			}

		protected:
			void set_message_handlers()
			{
				this->add_message_handlers(4,
					message_pair(WM_ERASEBKGND, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						return 1;
					}),
					message_pair(WM_LBUTTONDBLCLK, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						SendMessage(hWnd, WM_LBUTTONDOWN, wParam, lParam);
						return 0;
					}),
					message_pair(CUSTOM_COMMAND, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						if (HIWORD(wParam) == BN_CLICKED)
						{
							this->checked ^= true;
							this->function_on_click(this->checked);
						}

						return 0;
					}),
					message_pair(OWNER_DRAWITEM, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						return this->draw_item(reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
					})
				);
			}
	
			LRESULT draw_item(DRAWITEMSTRUCT* checkbox_draw)
			{
				if ((checkbox_draw->itemAction & ODA_FOCUS) || (checkbox_draw->itemState & ODS_SELECTED))
				{
					/* Filter drawing for focus-events and key-down events */
					return 0;
				}
		
				/* draw toggle box */
				RECT rc_box(checkbox_draw->rcItem);
				rc_box.left++;
				rc_box.top++;
				rc_box.bottom--;
				rc_box.right = rc_box.left + 14;
				
				FillRect(checkbox_draw->hDC, &rc_box, CreateSolidBrush(RGB(195, 202, 206)));
				InflateRect(&rc_box, -1, -1);
				FillRect(checkbox_draw->hDC, &rc_box, CreateSolidBrush(RGB(255, 255, 255)));
				
				if (this->checked)
				{
					HBITMAP check_image = png::make_image(PNG_CHECKBOX_CHECK);

					BITMAP bitmap_info;
					GetObject(check_image, sizeof(bitmap_info), &bitmap_info);

					HDC hdc_source = CreateCompatibleDC(NULL);
					HBITMAP old_bitmap = reinterpret_cast<HBITMAP>(SelectObject(hdc_source, check_image));
					
					BitBlt(checkbox_draw->hDC, rc_box.left, rc_box.top, bitmap_info.bmWidth, bitmap_info.bmHeight, hdc_source, 0, 0, SRCCOPY);
					SelectObject(hdc_source, old_bitmap);

					DeleteDC(hdc_source);
					DeleteObject(check_image);
				}
				
				/* text */
				RECT rc_text = { checkbox_draw->rcItem.left + 16 + 4, checkbox_draw->rcItem.top + 1, 
					checkbox_draw->rcItem.right - 1, checkbox_draw->rcItem.bottom - 1 };
				
				/* draw text background */
				FillRect(checkbox_draw->hDC, &rc_text, CreateSolidBrush(this->get_widget(this->get_parent_handle()).get_background_color()));
				
				/* draw text labels */
				int background_mode = SetBkMode(checkbox_draw->hDC, TRANSPARENT);
				COLORREF text_color = SetTextColor(checkbox_draw->hDC, RGB(136, 136, 136));
		
				int text_length = Button_GetTextLength(checkbox_draw->hwndItem);

				char* text_buffer = new char[text_length + 1];
				memset(text_buffer, 0, text_length + 1);

				text_length = Button_GetText(checkbox_draw->hwndItem, text_buffer, text_length + 1);

				RECT rc_align(rc_text);
				DrawText(checkbox_draw->hDC, text_buffer, -1, &rc_align, DT_CALCRECT);
				DrawText(checkbox_draw->hDC, text_buffer, -1, &rc_text, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
				
				SetTextColor(checkbox_draw->hDC, text_color);
				SetBkMode(checkbox_draw->hDC, background_mode);

				delete[] text_buffer;
				return 0;
			}
			
		private:
			bool checked;
			checkbox_function_t function_on_click;
		};
	}
}