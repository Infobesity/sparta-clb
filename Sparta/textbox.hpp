#pragma once

#include "generic.hpp"
#include "widget.hpp"

namespace sparta
{
	namespace gui
	{
		typedef std::function<bool()> textbox_function_t;

		class textbox : public widget
		{
		public:
			textbox(HWND hwnd_parent = NULL, HINSTANCE instance = NULL)
				: widget(hwnd_parent, instance), multi_line(false), on_enter_function(nullptr)
			{

			}
			
			textbox(widget& parent_widget, rectangle& rect = rectangle(), bool numeric = false, bool password = false, bool center = false, bool multi_line = false)
				: widget(parent_widget.get_handle(), NULL), on_enter_function(nullptr)
			{
				if (!this->assemble(parent_widget, rect, numeric, password, center, multi_line))
				{
					throw std::exception("Failed to create 'textbox' widget");
				}
			}

			~textbox()
			{

			}

			bool assemble(widget& parent_widget, rectangle& rect, bool numeric = false, bool password = false, bool center = false, bool multi_line = false)
			{
				this->multi_line = multi_line;

				if (!this->create(WC_EDIT, "", rect, WS_VISIBLE | WS_CHILD | (center ? ES_CENTER : ES_LEFT) | (numeric ? ES_NUMBER : 0) | (password ? ES_PASSWORD : 0) |
					(this->multi_line ? ES_MULTILINE | WS_VSCROLL : ES_AUTOHSCROLL), WS_EX_CLIENTEDGE, parent_widget.get_handle()))
				{
					return false;
				}

				this->set_message_handlers();
				return true;
			}
			
			bool set_text(std::string const& text)
			{
				return (SetWindowText(this->get_handle(), text.c_str()) != FALSE);
			}
	
			bool set_cue_banner(std::string const& caption)
			{
				std::wstring wide_text(caption.length(), L' ');
				std::copy(caption.begin(), caption.end(), wide_text.begin());

				return (Edit_SetCueBannerText(this->get_handle(), wide_text.c_str()) != FALSE);
			}

			void set_maximum_length(std::size_t max_length)
			{
				Edit_LimitText(this->get_handle(), max_length);
			}

			void set_on_enter_function(textbox_function_t on_enter_fucntion)
			{
				this->on_enter_function = on_enter_fucntion;
			}

			std::string get_text()
			{
				int text_length = Edit_GetTextLength(this->get_handle());
				
				char* text_buffer = new char[text_length + 1];
				memset(text_buffer, 0, text_length + 1);
				
				text_length = Edit_GetText(this->get_handle(), text_buffer, text_length + 1);

				if (text_length <= 0)
				{
					delete[] text_buffer;
					return std::string("");
				}

				std::string text_string(text_buffer);

				delete[] text_buffer;
				return text_string;
			}

		protected:
			void set_message_handlers()
			{
				this->add_message_handlers(5,
					message_pair(CUSTOM_CTLCOLOR, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						SetBkMode(reinterpret_cast<HDC>(wParam), TRANSPARENT);
						SetTextColor(reinterpret_cast<HDC>(wParam), RGB(0, 0, 0));
						//SetTextColor(reinterpret_cast<HDC>(wParam), RGB(240, 240, 240));
						return reinterpret_cast<LRESULT>(CreateSolidBrush(RGB(255, 255, 255)));
					}),
					message_pair(WM_CHAR, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						if (!this->multi_line && wParam == VK_RETURN)
						{
							if (on_enter_function)
							{
								return static_cast<LRESULT>(on_enter_function());
							}
						}

						return CallWindowProc(this->original_wndproc, hWnd, WM_CHAR, wParam, lParam);;
					}),
					message_pair(WM_SETFOCUS, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						//SetWindowPos(this->border_handle, 0, this->rect_focused.get_x(), this->rect_focused.get_y(), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
						//this->set_border(true);
						return CallWindowProc(this->original_wndproc, hWnd, WM_SETFOCUS, wParam, lParam);;
					}),
					message_pair(WM_KILLFOCUS, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						//SetWindowPos(this->border_handle, 0, this->rect_normal.get_x(), this->rect_normal.get_y(), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
						//this->set_border(false);
						return CallWindowProc(this->original_wndproc, hWnd, WM_KILLFOCUS, wParam, lParam);
					}),
					message_pair(WM_KEYDOWN, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						InvalidateRect(hWnd, NULL, TRUE);
						return CallWindowProc(this->original_wndproc, hWnd, WM_KEYDOWN, wParam, lParam);
					})
				);
			}

		private:
			bool multi_line;
			textbox_function_t on_enter_function;
		};
	}
}