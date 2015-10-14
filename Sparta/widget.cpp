#include "widget.hpp"
#include "sparta_form.hpp"


namespace sparta
{
	namespace gui
	{
		namespace color
		{
			/* factor should be > 1.0f */
			COLORREF lighter(COLORREF base_color, float factor)
			{
				factor = max(factor, 1.0f);

				unsigned char red = GetRValue(base_color);
				unsigned char blue = GetBValue(base_color);
				unsigned char green = GetGValue(base_color);

				unsigned char red_light = static_cast<unsigned char>(min(static_cast<int>(red * factor), 255));
				unsigned char blue_light = static_cast<unsigned char>(min(static_cast<int>(blue * factor), 255));
				unsigned char green_light = static_cast<unsigned char>(min(static_cast<int>(green * factor), 255));

				return RGB(red_light, blue_light, green_light);
			}
			
			/* factor should be between 0.0f and 1.0f */
			COLORREF darker(COLORREF base_color, float factor)
			{
				factor = min(factor, 1.0f);
				factor = max(factor, 0.0f);
				
				unsigned char red = GetRValue(base_color);
				unsigned char blue = GetBValue(base_color);
				unsigned char green = GetGValue(base_color);

				unsigned char red_dark = static_cast<unsigned char>(red * factor);
				unsigned char blue_dark = static_cast<unsigned char>(blue * factor);
				unsigned char green_dark = static_cast<unsigned char>(green * factor);

				return RGB(red_dark, blue_dark, green_dark);
			}
		}

		/* widget_base */
		widget_base::widget_base(HWND hwnd_parent, HINSTANCE instance, bool is_window)
			: font(NULL), rect(rectangle()), is_window(is_window), style(0), hwnd(NULL), hwnd_parent(hwnd_parent), instance(NULL), original_wndproc(nullptr)
		{
			if (instance != NULL)
			{
				this->instance = instance;
			}
			else if (hwnd_parent != NULL)
			{
				this->instance = reinterpret_cast<HINSTANCE>(GetWindowLong(this->hwnd_parent, GWLP_HINSTANCE));
			}
			else
			{
				this->instance = GetModuleHandle(NULL);
			}
		}

		widget_base::~widget_base()
		{
			
		}
		
		bool widget_base::set_enable(bool enable)
		{
			return (EnableWindow(this->hwnd, enable != false) != FALSE);
		}

		HFONT widget_base::get_font()
		{
			return this->font;
		}
		
		bool widget_base::set_font(std::string const& font, int rotation)
		{
			if (this->font)
			{
				DeleteObject(this->font);
			}

			this->font = CreateFont(13, 0, rotation, rotation, FW_DONTCARE, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS, 5, DEFAULT_PITCH | FF_DONTCARE, font.c_str());

			if (!this->font)
			{
				return false;
			}

			SendMessage(this->hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(this->font), TRUE);
			return true;
		}
		
		point widget_base::get_position()
		{
			return point(this->rect.get_x(), this->rect.get_y());
		}

		bool widget_base::set_position(point& new_position)
		{
			this->rect.set_x_y(new_position.get_x(), new_position.get_y());
			return (SetWindowPos(this->hwnd, NULL, this->rect.get_x(), this->rect.get_y(), 0, 0, SWP_NOZORDER | SWP_NOSIZE) != FALSE);
		}

		size widget_base::get_size(bool has_menu)
		{
			if (is_window)
			{
				RECT rc_window = { 0, 0, this->rect.get_width(), this->rect.get_height() };
				AdjustWindowRect(&rc_window, this->style, static_cast<BOOL>(has_menu));
				
				unsigned int width = this->rect.get_width() - ((rc_window.right - rc_window.left) - this->rect.get_width());
				unsigned int height = this->rect.get_height() - ((rc_window.bottom - rc_window.top) - this->rect.get_height());

				return size(width, height);
			}

			return size(this->rect.get_width(), this->rect.get_height());
		}

		bool widget_base::set_size(size& new_size)
		{
			this->rect.set_width_height(new_size.get_width(), new_size.get_height());
			return (SetWindowPos(this->hwnd, NULL, 0, 0, this->rect.get_width(), this->rect.get_height(), SWP_NOZORDER | SWP_NOMOVE) != FALSE);
		}
		
		std::string widget_base::get_text()
		{
			int text_length = GetWindowTextLength(this->hwnd);

			char* text_buffer = new char[text_length];
			memset(text_buffer, 0, text_length);

			std::string text(text_buffer, GetWindowText(this->hwnd, text_buffer, text_length) + 1);

			delete[] text_buffer;
			return text;
		}

		bool widget_base::set_text(std::string const& text)
		{
			return (SetWindowText(this->hwnd, text.c_str()) != FALSE);
		}
	
		COLORREF widget_base::get_foreground_color()
		{
			return this->foreground_color;
		}

		void widget_base::set_foreground_color(COLORREF foreground_color)
		{
			this->foreground_color = foreground_color;
		}
			
		COLORREF widget_base::get_background_color()
		{
			return this->background_color;
		}

		void widget_base::set_background_color(COLORREF background_color)
		{
			this->background_color = background_color;
		}

		HINSTANCE widget_base::get_instance_handle()
		{
			return this->instance;
		}
		
		void widget_base::set_instance_handle(HINSTANCE instance)
		{
			this->instance = instance;
		}
		
		WNDPROC widget_base::get_original_window_proc()
		{
			return this->original_wndproc;
		}

		void widget_base::set_window_proc(WNDPROC window_proc)
		{
			this->original_wndproc = reinterpret_cast<WNDPROC>(SetWindowLong(this->hwnd,
					GWLP_WNDPROC, reinterpret_cast<long>(window_proc)));
		}

		HWND widget_base::get_handle()
		{
			return this->hwnd;
		}

		HWND widget_base::get_parent_handle()
		{
			return this->hwnd_parent;
		}

		rectangle& widget_base::get_rect()
		{
			return this->rect;
		}

		/* widget */
		std::unordered_map<HWND, widget*> widget::widget_list;
		
		widget& widget::get_widget(HWND hwnd)
		{
			return (*widget::widget_list[hwnd]);
		}

		LRESULT CALLBACK widget::window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			widget* current_widget = widget::widget_list[hWnd];
			
			if (current_widget)
			{
				if (current_widget->custom_messages[message])
				{
					return current_widget->custom_messages[message](hWnd, wParam, lParam);
				}
			}

			switch (message)
			{
			case WM_MEASUREITEM:
				return SendMessage(GetDlgItem(hWnd, wParam), OWNER_MEASUREITEM, wParam, lParam);
		
			case WM_DRAWITEM:
				return SendMessage(reinterpret_cast<DRAWITEMSTRUCT*>(lParam)->hwndItem, OWNER_DRAWITEM, wParam, lParam);
		
			default:
				break;
			}
		
			if (current_widget)
			{
				if (current_widget->is_window)
				{
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
				else
				{
					return CallWindowProc(current_widget->get_original_window_proc(), hWnd, message, wParam, lParam);
				}
			}

			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		widget::widget(HWND hwnd_parent, HINSTANCE instance, bool is_window) 
			: widget_base(hwnd_parent, instance, is_window)
		{
			this->set_background_color(GetSysColor(COLOR_BTNFACE));
			this->set_foreground_color(GetSysColor(COLOR_WINDOWTEXT));
		}

		widget::~widget()
		{
			if (this->hwnd)
			{
				this->widget_list.erase(this->hwnd);
				DestroyWindow(this->hwnd);
			}

			if (this->font)
			{
				DeleteObject(this->font);
			}
		}

		bool widget::create(std::string const& class_name, std::string const& widget_name, rectangle& rect, 
			unsigned int style, unsigned int ex_style, HWND hwnd_parent, unsigned int menu_handle)
		{
			this->rect = rect;
			this->style = style;

			if (hwnd_parent)
			{
				this->hwnd_parent = hwnd_parent;
			}

			if (!(this->hwnd = CreateWindowEx(ex_style, class_name.c_str(), widget_name.c_str(), style, rect.get_x(), rect.get_y(), 
				rect.get_width(), rect.get_height(), this->hwnd_parent, reinterpret_cast<HMENU>(menu_handle), this->instance, NULL)))
			{
				return false;
			}

			if (!(this->font = CreateFont(13, 0, 0, 0, FW_DONTCARE, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS, 5, DEFAULT_PITCH | FF_DONTCARE, "Tahoma")))
			{
				return false;
			}
			else
			{
				SendMessage(this->hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(this->font), TRUE);
			}

			if (!this->is_window)
			{
				this->set_window_proc(widget::window_proc);
			}

			this->widget_list[this->hwnd] = this;
			return true;
		}

		void widget::add_message_handlers(int count, ...)
		{
			va_list va;
			va_start(va, count);

			for (int i = 0; i < count; i++)
			{
				message_pair message = va_arg(va, message_pair);
				this->custom_messages[message.first] = message.second;
			}

			va_end(va);
		}

		void widget::remove_message_handler(unsigned int message)
		{
			this->custom_messages.erase(message);
		}
		
		void widget::show(bool show)
		{
			ShowWindow(this->hwnd, show ? SW_SHOW : SW_HIDE);
		}
		
		bool widget::has_message(unsigned int message)
		{
			for (auto x : this->custom_messages)
			{
				if (x.first == message)
				{
					return true;
				}
			}

			return false;
		}
	}
}