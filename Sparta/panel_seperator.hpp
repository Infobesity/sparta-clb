#pragma once

#include "generic.hpp"
#include "widget.hpp"

namespace sparta
{
	namespace gui
	{
		class panel_seperator : public widget
		{
		public:
			panel_seperator(HWND hwnd_parent = NULL, HINSTANCE instance = NULL)
				: widget(hwnd_parent, instance)
			{

			}
			
			panel_seperator(widget& parent_widget, std::string const& caption, rectangle& rect = rectangle())
				: widget(parent_widget.get_handle(), NULL)
			{
				if (!this->assemble(parent_widget, caption, rect))
				{
					throw std::exception("Failed to create 'panel seperator' widget");
				}
			}

			~panel_seperator()
			{

			}
			
			bool assemble(widget& parent_widget, std::string const& caption, rectangle& rect = rectangle())
			{
				if (!this->create(WC_STATIC, caption.c_str(), rect, WS_VISIBLE | WS_CHILD | SS_OWNERDRAW, NULL, parent_widget.get_handle()))
				{
					return false;
				}

				this->set_message_handlers();
				return true;
			}

		protected:
			void set_message_handlers()
			{
				this->add_message_handlers(2,
					message_pair(WM_ERASEBKGND, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						return 1;
					}),
					message_pair(OWNER_DRAWITEM, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						return this->draw_item(reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
					})
				);
			}
	
			LRESULT draw_item(DRAWITEMSTRUCT* panel_draw)
			{
				FillRect(panel_draw->hDC, &panel_draw->rcItem, CreateSolidBrush(this->get_widget(this->get_parent_handle()).get_background_color()));
						
				/* draw text */
				int background_mode = SetBkMode(panel_draw->hDC, TRANSPARENT);
				COLORREF text_color = SetTextColor(panel_draw->hDC, RGB(136, 136, 136));
		
				int text_length = Static_GetTextLength(panel_draw->hwndItem);

				char* text_buffer = new char[text_length + 1];
				memset(text_buffer, 0, text_length + 1);

				text_length = Static_GetText(panel_draw->hwndItem, text_buffer, text_length + 1);

				RECT rc_align(panel_draw->rcItem);
				DrawText(panel_draw->hDC, text_buffer, -1, &rc_align, DT_CALCRECT);
				DrawText(panel_draw->hDC, text_buffer, -1, &panel_draw->rcItem, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
				
				SetTextColor(panel_draw->hDC, text_color);
				SetBkMode(panel_draw->hDC, background_mode);

				delete[] text_buffer;

				/* draw seperator-lines */
				RECT rc_line(panel_draw->rcItem);
				
				rc_line.right -= (10 + (rc_align.right - rc_align.left));
				rc_line.right /= 2;

				rc_line.top += ((panel_draw->rcItem.bottom - panel_draw->rcItem.top) / 2);
				rc_line.bottom = rc_line.top + 3;

				FillRect(panel_draw->hDC, &rc_line, CreateSolidBrush(RGB(136, 136, 136)));

				int line_length = (rc_line.right - rc_line.left);
				rc_line.left += ((rc_align.right - rc_align.left) + 10) + line_length;
				rc_line.right += ((rc_align.right - rc_align.left) + 10) + line_length;

				FillRect(panel_draw->hDC, &rc_line, CreateSolidBrush(RGB(136, 136, 136)));
				return 0;
			}
			
		private:
		};
	}
}