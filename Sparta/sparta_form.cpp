#include "sparta_form.hpp"

namespace sparta
{
	WPARAM execute()
	{
		MSG message;
		while (GetMessage(&message, 0, 0, 0) > 0)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		return message.wParam;
	}

	namespace gui
	{
		void sparta_form::update(sparta_form_update_enum type, unsigned int data)
		{
			if (type == level)
			{
				this->level_label.set_text(std::to_string(data));
			}
			else if (type == map_id)
			{
				this->map_id_label.set_text(std::to_string(data));
			}
		}

		void sparta_form::update(sparta_form_update_enum type, unsigned __int64 data)
		{
			if (type == experience)
			{
				this->experience_label.set_text(utility::delimited_numeric(data));
			}
			else if (type == mesos)
			{
				this->mesos_label.set_text(utility::delimited_numeric(data));
			}
		}

		sparta_form::sparta_form(std::string const& class_name, std::string const& window_name, const char* icon_name) 
			: widget(0, 0, true), background(NULL), paint_font(NULL)
		{
			this->set_background_color(RGB(53, 53, 53));

			if (!this->register_class(class_name, icon_name))
			{
				throw std::exception("Failed to register main window class!");
			}

			if (!this->create_window(class_name, window_name, rectangle(0, 0, 410, 415)))
			{
				throw std::exception("Failed to create main window!");
			}
			
			this->set_message_handlers();

			if (!this->initialize())
			{
				throw std::exception("Failed to initialize main window!");
			}
		}

		sparta_form::~sparta_form()
		{
			if (this->paint_font)
			{
				DeleteObject(this->paint_font);
			}
		}

		bool sparta_form::initialize()
		{
			/* tab controls */
			this->control_tab_bar.assemble(*this, rectangle(5, 5, 400, 405));
			this->control_tab_bar.add_tab("General");
			this->control_tab_bar.add_tab("Inventory");
			this->control_tab_bar.add_tab("Statistics");

			this->control_tab_bar.set_current_selection(0);

			/* general tab */
			this->tab_pages.push_back(std::make_unique<panel>(*this, rectangle(11, 37, 388, 368), false));

			this->username_textbox.assemble(*this->tab_pages.back().get(), rectangle(0, 0, 128, 20), false, false, true);
			this->username_textbox.set_cue_banner("Username..");

			this->password_textbox.assemble(*this->tab_pages.back().get(), rectangle(133, 0, 128, 20), false, true, true);
			this->password_textbox.set_cue_banner("Password..");

			this->pic_textbox.assemble(*this->tab_pages.back().get(), rectangle(266, 0, 128, 20), true, true, true);
			this->pic_textbox.set_cue_banner("Pic..");
			
			this->activation_button.assemble(*this->tab_pages.back().get(), "Start", rectangle(0, 25, 388, 25), [this]() -> bool
			{
				static bool start = true;
				
				this->username_textbox.set_enable(!start);
				this->password_textbox.set_enable(!start);
				this->pic_textbox.set_enable(!start);
				this->activation_button.set_text(start ? "Stop" : "Start");

				if (start)
				{
					this->maplestory_instance = std::make_shared<maplestory::maple_client>(maplestory::account(this->username_textbox.get_text(), 
						this->password_textbox.get_text(), this->pic_textbox.get_text(), 0, 12, 0));
					
					start ^= true;
					return this->maplestory_instance.get()->reset();
				}

				this->maplestory_instance.reset();
				start ^= true;
				return true;
			});
			
			this->widget_container.push_back(std::make_unique<label>(*this->tab_pages.back().get(), "Level:", rectangle(5, 100, 100, 20)));
			this->level_label.assemble(*this->tab_pages.back().get(), "0", rectangle(105, 100, 100, 20), true);

			this->widget_container.push_back(std::make_unique<label>(*this->tab_pages.back().get(), "Experience:", rectangle(5, 125, 100, 20)));
			this->experience_label.assemble(*this->tab_pages.back().get(), "0", rectangle(105, 125, 100, 20), true);
			
			this->widget_container.push_back(std::make_unique<label>(*this->tab_pages.back().get(), "Mesos:", rectangle(5, 150, 100, 20)));
			this->mesos_label.assemble(*this->tab_pages.back().get(), "0", rectangle(105, 150, 100, 20), true);

			this->widget_container.push_back(std::make_unique<label>(*this->tab_pages.back().get(), "Map ID:", rectangle(5, 175, 100, 20)));
			this->map_id_label.assemble(*this->tab_pages.back().get(), "0", rectangle(105, 175, 100, 20), true);

			/* inventory tab */

			/* statistics tab */

			this->control_tab_bar.relate(0, *this->tab_pages.back().get());

			return true;
		}

		bool sparta_form::register_class(std::string const& class_name, const char* icon_name)
		{
			WNDCLASSEX wcex;
			memset(&wcex, 0, sizeof(WNDCLASSEX));

			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.lpfnWndProc = sparta_form::window_proc;
			wcex.hInstance = this->get_instance_handle();
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.hIcon = LoadIcon(this->get_instance_handle(), icon_name);
			wcex.hIconSm = LoadIcon(this->get_instance_handle(), icon_name);
			wcex.hbrBackground = CreateSolidBrush(this->get_background_color());
			wcex.lpszClassName = class_name.c_str();

			return (RegisterClassEx(&wcex) != NULL);
		}

		bool sparta_form::create_window(std::string const& class_name, std::string const& window_name, rectangle& rect)
		{
			RECT rc_window = { rect.get_x(), rect.get_y(), rect.get_width(), rect.get_height() };
			AdjustWindowRect(&rc_window, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
			
			rect.set_width_height(rc_window.right - rc_window.left, rc_window.bottom - rc_window.top);

			if (!rect.get_x() && !rect.get_y())
			{
				RECT rc;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);

				rect.set_x_y((rc.right / 2) - (rect.get_width() / 2), (rc.bottom / 2) - (rect.get_height() / 2));
			}
			
			return this->create(class_name, window_name, rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
		}
		
		void sparta_form::set_message_handlers()
		{
			this->add_message_handlers(6,
				message_pair(WM_CLOSE, [](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
				{
					DestroyWindow(hWnd);
					return 0;
				}),
				message_pair(WM_DESTROY, [](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
				{
					PostQuitMessage(0);
					return 0;
				}),
				message_pair(WM_PAINT, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
				{
					this->paint_font = CreateFont(28, 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5, DEFAULT_PITCH | FF_DONTCARE, "Tahoma");

					//this->background = gui::png_image::make_image(PNG_MAIN_BACKGROUND);

					//BITMAP bitmap_info;
					//GetObject(this->background, sizeof(BITMAP), &bitmap_info);

					PAINTSTRUCT paint_struct;

					HDC hdc_temp = BeginPaint(hWnd, &paint_struct);
					HDC hdc = CreateCompatibleDC(hdc_temp);

					//SelectObject(hdc, this->background);
					//SelectObject(hdc, this->paint_font);

					SetTextColor(hdc, RGB(255, 255, 255));
					SetBkMode(hdc, TRANSPARENT);

					//BitBlt(hdc_temp, 0, 0, bitmap_info.bmWidth, bitmap_info.bmHeight, hdc, 0, 0, SRCCOPY);

					DeleteDC(hdc);
					EndPaint(hWnd, &paint_struct);
					return 0;
				}),
				message_pair(WM_CTLCOLORLISTBOX, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
				{
					return SendMessage(reinterpret_cast<HWND>(lParam), CUSTOM_CTLCOLOR, wParam, lParam);
				}),
				message_pair(WM_NOTIFY, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
				{
					return SendMessage(reinterpret_cast<NMHDR*>(lParam)->hwndFrom, CUSTOM_NOTIFY, wParam, lParam);
				}),
				message_pair(WM_SOCKET, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
				{
					if (WSAGETSELECTERROR(lParam))
					{
						printf("WSAGETSELECTERROR - %d\n", WSAGETSELECTERROR(lParam));

						if (this->maplestory_instance.get())
						{
							return this->maplestory_instance.get()->notify_close();
						}

						return 0;
					}
					
					if (this->maplestory_instance.get())
					{
						switch (WSAGETSELECTEVENT(lParam))
						{
						case FD_CLOSE:
							return this->maplestory_instance.get()->notify_close();

						case FD_CONNECT:
							return this->maplestory_instance.get()->notify_connect();

						case FD_READ:
							return this->maplestory_instance.get()->notify_read();

						case FD_WRITE:
							return this->maplestory_instance.get()->notify_write();

						default:
							break;
						}
					}

					return 0;
				})		
			);
		}
	}
}