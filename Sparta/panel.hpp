#pragma once

#include "generic.hpp"
#include "widget.hpp"

namespace sparta
{
	namespace gui
	{
		class panel : public widget
		{
		public:
			panel(HWND hwnd_parent = NULL, HINSTANCE instance = NULL)
				: widget(hwnd_parent, instance)
			{

			}
			
			panel(widget& parent_widget, rectangle& rect = rectangle(), bool has_background = false)
				: widget(parent_widget.get_handle(), NULL)
			{
				if (!this->assemble(parent_widget, rect, has_background))
				{
					throw std::exception("Failed to create 'panel' widget");
				}
			}

			~panel()
			{

			}
			
			bool assemble(widget& parent_widget, rectangle& rect = rectangle(), bool has_background = false)
			{
				if (!this->create(WC_STATIC, "", rect, WS_VISIBLE | WS_CHILD | SS_OWNERDRAW, NULL, parent_widget.get_handle()))
				{
					return false;
				}

				this->has_background = has_background;

				if (!this->has_background)
				{
					this->set_background_color(this->get_widget(this->get_parent_handle()).get_background_color());
				}

				this->set_message_handlers();
				return true;
			}

		protected:
			void set_message_handlers()
			{
				this->add_message_handlers(6,
					message_pair(WM_ERASEBKGND, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						if (has_background)
						{
							RECT rc_main_body;
							GetClientRect(hWnd, &rc_main_body);
							
							FillRect(reinterpret_cast<HDC>(wParam), &rc_main_body, CreateSolidBrush(this->get_background_color()));

							rc_main_body.top = rc_main_body.bottom - 1;
							FillRect(reinterpret_cast<HDC>(wParam), &rc_main_body, CreateSolidBrush(RGB(0, 0, 0)));
						}

						return 1;
					}),
					message_pair(OWNER_DRAWITEM, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						return this->draw_item(hWnd, reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
					}),
					message_pair(WM_CTLCOLOREDIT, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						return SendMessage(reinterpret_cast<HWND>(lParam), CUSTOM_CTLCOLOR, wParam, lParam);
					}),
					message_pair(WM_CTLCOLORLISTBOX, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						return SendMessage(reinterpret_cast<HWND>(lParam), CUSTOM_CTLCOLOR, wParam, lParam);
					}),
					message_pair(WM_NOTIFY, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						return SendMessage(reinterpret_cast<NMHDR*>(lParam)->hwndFrom, CUSTOM_NOTIFY, wParam, lParam);
					}),	
					message_pair(WM_COMMAND, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						return SendMessage(reinterpret_cast<HWND>(lParam), CUSTOM_COMMAND, wParam, lParam);
					})
				);
			}
	
			LRESULT draw_item(HWND hwnd, DRAWITEMSTRUCT* panel_draw)
			{
				//FillRect(panel_draw->hDC, &panel_draw->rcItem, CreateSolidBrush(RGB(255, 0, 0)));
				return 0;
			}
			
		private:
			bool has_background;
		};
	}
}