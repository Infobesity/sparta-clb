#pragma once

#include "generic.hpp"
#include "widget.hpp"

#include "png_image.hpp"
#include "resource.hpp"

namespace sparta
{
	namespace gui
	{
		class listbox : public widget
		{
		public:
			listbox(HWND hwnd_parent = NULL, HINSTANCE instance = NULL)
				: widget(hwnd_parent, instance), use_time_stamp(false)
			{

			}
			
			listbox(widget& parent_widget, rectangle& rect = rectangle(), bool use_time_stamp = false)
				: widget(parent_widget.get_handle(), NULL), use_time_stamp(use_time_stamp)
			{
				if (!this->assemble(parent_widget, rect, use_time_stamp))
				{
					throw std::exception("Failed to create 'listbox' widget");
				}
			}

			~listbox()
			{

			}

			bool assemble(widget& parent_widget, rectangle& rect, bool use_time_stamp = false)
			{
				if (!this->create(WC_LISTBOX, "", rect, WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_HASSTRINGS |
					LBS_DISABLENOSCROLL | LBS_NOSEL | LBS_OWNERDRAWFIXED, NULL, parent_widget.get_handle()))
				{
					return false;
				}

				this->use_time_stamp = use_time_stamp;
				this->set_message_handlers();
				return true;
			}

			void add_string(const char* format, ...)
			{
				va_list va;
				va_start(va, format);

				char buffer[1024];
				vsprintf(buffer, format, va);

				if (this->use_time_stamp)
				{
					ListBox_AddString(this->get_handle(), ("[" + my_time::get_time() + "]" + " " + buffer).c_str());
				}
				else
				{
					ListBox_AddString(this->get_handle(), buffer);
				}

				va_end(va);
				ListBox_SetCurSel(this->get_handle(), ListBox_GetCount(this->get_handle()) - 1);
			}

			void add_string(COLORREF color, const char* format, ...)
			{
				va_list va;
				va_start(va, format);

				char buffer[1024];
				vsprintf(buffer, format, va);

				if (this->use_time_stamp)
				{
					ListBox_AddString(this->get_handle(), ("[" + my_time::get_time() + "]" + " " + buffer).c_str());
				}
				else
				{
					ListBox_AddString(this->get_handle(), buffer);
				}

				va_end(va);

				ListBox_SetItemData(this->get_handle(), ListBox_GetCount(this->get_handle()) - 1, color);
				ListBox_SetCurSel(this->get_handle(), ListBox_GetCount(this->get_handle()) - 1);
			}
			
			void clear()
			{
				ListBox_ResetContent(this->get_handle());
			}

		protected:
			void set_message_handlers()
			{
				this->add_message_handlers(3,
					message_pair(WM_ERASEBKGND, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						RECT rc_main_body;
						GetClientRect(hWnd, &rc_main_body);

						FillRect(reinterpret_cast<HDC>(wParam), &rc_main_body, CreateSolidBrush(RGB(255, 255, 255)));
						return 1;
					}),
					message_pair(OWNER_MEASUREITEM, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						reinterpret_cast<MEASUREITEMSTRUCT*>(lParam)->itemHeight = 15;
						return 0;
					}),
					message_pair(OWNER_DRAWITEM, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						return this->draw_item(reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
					})
				);
			}
	
			LRESULT draw_item(DRAWITEMSTRUCT* item_draw)
			{
				/* draw the item background*/
				FillRect(item_draw->hDC, &item_draw->rcItem, CreateSolidBrush(this->get_background_color()));

				/* draw the item text label */
				int background_mode = SetBkMode(item_draw->hDC, TRANSPARENT);
				COLORREF text_color = SetTextColor(item_draw->hDC, (item_draw->itemData ? item_draw->itemData : this->get_foreground_color()));
				
				int text_length = ListBox_GetTextLen(item_draw->hwndItem, item_draw->itemID);
				
				char* text_buffer = new char[text_length + 1];
				memset(text_buffer, 0, text_length + 1);
				
				text_length = ListBox_GetText(item_draw->hwndItem, item_draw->itemID, text_buffer);
           
				item_draw->rcItem.left += 2;

				DrawText(item_draw->hDC, text_buffer, -1, &item_draw->rcItem, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);
				DrawText(item_draw->hDC, text_buffer, -1, &item_draw->rcItem, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

				SetTextColor(item_draw->hDC, text_color);
				SetBkMode(item_draw->hDC, background_mode);

				delete[] text_buffer;
				return 0;
			}
			
		private:
			bool use_time_stamp;
		};
	}
}