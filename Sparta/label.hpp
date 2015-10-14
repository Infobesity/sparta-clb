#pragma once

#include "generic.hpp"
#include "widget.hpp"

namespace sparta
{
	namespace gui
	{
		class label : public widget
		{
		public:
			label(HWND hwnd_parent = NULL, HINSTANCE instance = NULL)
				: widget(hwnd_parent, instance), center(false)
			{

			}
			
			label(widget& parent_widget, std::string const& caption, rectangle& rect = rectangle(), bool center = false)
				: widget(parent_widget.get_handle(), NULL), center(center)
			{
				if (!this->assemble(parent_widget, caption, rect, center))
				{
					throw std::exception("Failed to create 'label' widget");
				}
			}

			~label()
			{

			}
			
			bool assemble(widget& parent_widget, std::string const& caption, rectangle& rect = rectangle(), bool center = false)
			{
				if (!this->create(WC_STATIC, caption.c_str(), rect, WS_VISIBLE | WS_CHILD | SS_OWNERDRAW, NULL, parent_widget.get_handle()))
				{
					return false;
				}

				this->center = center;
				this->set_message_handlers();
				return true;
			}

		protected:
			void set_message_handlers()
			{
				this->add_message_handlers(2,
					message_pair(WM_ERASEBKGND, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						RECT rc_main_body;
						GetClientRect(hWnd, &rc_main_body);
							
						FillRect(reinterpret_cast<HDC>(wParam), &rc_main_body, CreateSolidBrush(this->get_widget(this->get_parent_handle()).get_background_color()));
						return 1;
					}),
					message_pair(OWNER_DRAWITEM, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						return this->draw_item(hWnd, reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
					})
				);
			}
	
			LRESULT draw_item(HWND hwnd, DRAWITEMSTRUCT* label_draw)
			{
				/* draw background */
				FillRect(label_draw->hDC, &label_draw->rcItem, CreateSolidBrush(this->get_widget(this->get_parent_handle()).get_background_color()));
				
				/* draw text */
				int background_mode = SetBkMode(label_draw->hDC, TRANSPARENT);
				COLORREF text_color = SetTextColor(label_draw->hDC, RGB(136, 136, 136));
		
				int text_length = Static_GetTextLength(label_draw->hwndItem);

				char* text_buffer = new char[text_length + 1];
				memset(text_buffer, 0, text_length + 1);

				text_length = Static_GetText(label_draw->hwndItem, text_buffer, text_length + 1);

				RECT rc_align(label_draw->rcItem);
				DrawText(label_draw->hDC, text_buffer, -1, &rc_align, DT_CALCRECT);
				DrawText(label_draw->hDC, text_buffer, -1, &label_draw->rcItem, (this->center ? DT_CENTER : DT_LEFT) | DT_SINGLELINE | DT_VCENTER);
				
				SetTextColor(label_draw->hDC, text_color);
				SetBkMode(label_draw->hDC, background_mode);

				delete[] text_buffer;
				return 0;
			}
			
		private:
			bool center;
		};
	}
}