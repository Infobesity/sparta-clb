#pragma once

#include "generic.hpp"
#include "widget.hpp"

namespace sparta
{
	namespace gui
	{
		typedef std::function<bool()> button_function_t;

		class button : public widget
		{
		public:
			button(HWND hwnd_parent = NULL, HINSTANCE instance = NULL)
				: widget(hwnd_parent, instance), is_hovered(false)
			{

			}
			
			button(widget& parent_widget, std::string const& caption, rectangle& rect = rectangle(), button_function_t function_on_click = []() -> bool { return true; })
				: widget(parent_widget.get_handle(), NULL), is_hovered(false)
			{
				if (!this->assemble(parent_widget, caption, rect, function_on_click))
				{
					throw std::exception("Failed to create 'button' widget");
				}
			}

			~button()
			{

			}

			bool assemble(widget& parent_widget, std::string const& caption, rectangle& rect = rectangle(), button_function_t function_on_click = []() -> bool { return true; })
			{
				if (!this->create(WC_BUTTON, caption, rect, WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, NULL, parent_widget.get_handle()))
				{
					return false;
				}
				
				this->function_on_click = function_on_click;
				this->set_message_handlers();
				return true;
			}
			
		protected:
			void set_message_handlers()
			{
				this->add_message_handlers(5,
					message_pair(WM_MOUSEMOVE, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						if (!this->is_hovered)
						{
							this->is_hovered = true;

							TRACKMOUSEEVENT track_event;
							track_event.cbSize = sizeof(TRACKMOUSEEVENT);
							track_event.dwFlags = TME_LEAVE;
							track_event.hwndTrack = hWnd;
							TrackMouseEvent(&track_event);

							RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
						}

						return 0;
					}),
					message_pair(WM_MOUSELEAVE, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						this->is_hovered = false;
						RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
						return 0;
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
							if (this->function_on_click)
							{
								this->function_on_click();
							}
						}

						return 0;
					}),
					message_pair(OWNER_DRAWITEM, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						return this->draw_item(reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
					})
				);
			}
			
			LRESULT draw_item(DRAWITEMSTRUCT* button_draw)
			{
				if (button_draw->itemAction & ODA_FOCUS)
				{
					/* Filter drawing for focus-events and key-down events */
					return 0;
				}

				/* draw the background */
				RECT rc_background(button_draw->rcItem);
				
				if (button_draw->itemState & ODS_SELECTED)
				{
					FillRect(button_draw->hDC, &rc_background, CreateSolidBrush(RGB(0, 0, 0)));
					InflateRect(&rc_background, -2, -2);
				}
				else if (this->is_hovered)
				{
					FillRect(button_draw->hDC, &rc_background, CreateSolidBrush(RGB(60, 126, 176)));
					InflateRect(&rc_background, -2, -2);
				}
				else
				{
					FillRect(button_draw->hDC, &rc_background, CreateSolidBrush(RGB(195, 202, 206)));
					InflateRect(&rc_background, -2, -2);
				}
				
				int y_old = rc_background.bottom;
				rc_background.bottom = rc_background.top + ((rc_background.bottom - rc_background.top) / 2);

				FillRect(button_draw->hDC, &rc_background, CreateSolidBrush(RGB(255, 255, 255)));
				
				rc_background.bottom = y_old;
				rc_background.top = rc_background.bottom - ((rc_background.bottom - rc_background.top) / 2);

				FillRect(button_draw->hDC, &rc_background, CreateSolidBrush(RGB(214, 214, 214)));

				/* draw text labels */
				RECT rc_text(button_draw->rcItem);

				int background_mode = SetBkMode(button_draw->hDC, TRANSPARENT);
				COLORREF text_color = SetTextColor(button_draw->hDC, RGB(0, 0, 0));
		
				std::size_t text_length = Button_GetTextLength(button_draw->hwndItem);

				char* text_buffer = new char[text_length + 1];
				text_length = Button_GetText(button_draw->hwndItem, text_buffer, text_length + 1);
		
				RECT rc_align(rc_text);
				DrawText(button_draw->hDC, text_buffer, -1, &rc_align, DT_CALCRECT);
				DrawText(button_draw->hDC, text_buffer, -1, &rc_text, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
				
				SetTextColor(button_draw->hDC, text_color);
				SetBkMode(button_draw->hDC, background_mode);

				delete[] text_buffer;
				return 0;
			}

		private:
			bool is_hovered;
			button_function_t function_on_click;
		};
	}
}