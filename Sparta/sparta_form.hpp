#pragma once

#include "generic.hpp"
#include "widget.hpp"

#include "resource.hpp"
#include "png_image.hpp"

#include "button.hpp"
#include "panel.hpp"
#include "tab_control.hpp"
#include "textbox.hpp"
#include "treeview.hpp"
#include "label.hpp"

#include "maple_client.hpp"

namespace sparta
{
	WPARAM execute();

	namespace gui
	{
		#define COLOR_RED		RGB(255, 0, 0)
		#define COLOR_YELLOW	RGB(255, 255, 0)
		#define COLOR_WHITE		RGB(255, 255, 255)
	
		enum sparta_form_update_enum
		{
			level = 1,
			experience = 2,
			mesos = 3,
			map_id = 4
		};

		class sparta_form_widgets
		{
		protected:
			textbox username_textbox;
			textbox password_textbox;
			textbox pic_textbox;
			button activation_button;
			
			tab_control<unsigned int> control_tab_bar;
			std::vector<std::unique_ptr<panel>> tab_pages;

			std::vector<std::unique_ptr<widget>> widget_container;

			label level_label;
			label experience_label;
			label mesos_label;
			label map_id_label;
		};

		class sparta_form : public widget, public sparta_form_widgets
		{
		public:
			static sparta_form& get_instance()
			{
				static sparta_form instance("sparta_window_class", "Sparta Bot", MAKEINTRESOURCE(IDI_MAIN_ICON));
				return instance;
			}
			
			void update(sparta_form_update_enum type, unsigned int data);
			void update(sparta_form_update_enum type, unsigned __int64 data);

		protected:
			sparta_form(std::string const& class_name, std::string const& window_name, const char* icon_name = IDI_APPLICATION);
			~sparta_form();

			bool initialize();

			bool register_class(std::string const& class_name, const char* icon_name);
			bool create_window(std::string const& class_name, std::string const& window_name, rectangle& rect);

			void set_message_handlers();

		private:
			HBITMAP background;
			HFONT paint_font;

			std::shared_ptr<maplestory::maple_client> maplestory_instance;
		};
	}
}