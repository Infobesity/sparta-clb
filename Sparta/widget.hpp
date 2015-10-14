#pragma once

#include "generic.hpp"
#include "basic_types.hpp"

#include <unordered_map>

namespace sparta
{
	namespace gui
	{
		#define CUSTOM_NOTIFY		(WM_USER + 1)
		#define CUSTOM_COMMAND		(WM_USER + 2)
		#define CUSTOM_CTLCOLOR		(WM_USER + 3)
		#define	OWNER_MEASUREITEM	(WM_USER + 4)
		#define OWNER_DRAWITEM		(WM_USER + 5)
	
		typedef std::function<int(HWND, WPARAM, LPARAM)> message_function;
		typedef std::pair<unsigned int, message_function> message_pair;
	
		namespace color
		{
			COLORREF lighter(COLORREF base_color, float factor);
			COLORREF darker(COLORREF base_color, float factor);
		}

		class widget_base
		{
		public:
			widget_base(HWND hwnd_parent = NULL, HINSTANCE instance = NULL, bool is_window = false);
			~widget_base();

			bool set_enable(bool enable);

			HFONT get_font();
			bool set_font(std::string const& font, int rotation = 0);
			
			point get_position();
			bool set_position(point& new_position);

			size get_size(bool has_menu = false);
			bool set_size(size& new_size);
			
			std::string get_text();
			bool set_text(std::string const& text);
			
			COLORREF get_foreground_color();
			void set_foreground_color(COLORREF foreground_color);
			
			COLORREF get_background_color();
			void set_background_color(COLORREF background_color);

			HINSTANCE get_instance_handle();
			void set_instance_handle(HINSTANCE instance);
			
			WNDPROC get_original_window_proc();
			void set_window_proc(WNDPROC window_proc);

			HWND get_handle();
			HWND get_parent_handle();
			rectangle& get_rect();

		protected:
			HFONT font;
			rectangle rect;

			bool is_window;
			long style;
			
			COLORREF background_color;
			COLORREF foreground_color;

			HWND hwnd;
			HWND hwnd_parent;
			HINSTANCE instance;
			WNDPROC original_wndproc;
		};

		class widget : public widget_base
		{
		public:
			static std::unordered_map<HWND, widget*> widget_list;

			static widget& get_widget(HWND hwnd);
			static LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

			widget(HWND hwnd_parent = NULL, HINSTANCE instance = NULL, bool is_window = false);
			~widget();

			bool create(std::string const& class_name, std::string const& widget_name, rectangle& rect, 
				unsigned int style, unsigned int ex_style = 0, HWND hwnd_parent = NULL, unsigned int menu_handle = 0);

			void add_message_handlers(int count, ...);
			void remove_message_handler(unsigned int message);
			
			void show(bool show);
			
		protected:
			bool has_message(unsigned int message);

		private:
			std::unordered_map<unsigned int, message_function> custom_messages;
		};
	}
}