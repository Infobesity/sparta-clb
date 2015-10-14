#pragma once

#include "generic.hpp"
#include "widget.hpp"

namespace sparta
{
	namespace gui
	{
		typedef std::function<void(std::size_t)> listview_function_t;

		class listview : public widget
		{
		public:
			listview(HWND hwnd_parent = NULL, HINSTANCE instance = NULL)
				: widget(hwnd_parent, instance), auto_scroll(false), column_index(0)
			{

			}
			
			listview(widget& parent_widget, rectangle& rect = rectangle(), bool auto_scroll = false, listview_function_t on_click_function = [](std::size_t) -> void {  })
				: widget(parent_widget.get_handle(), NULL), auto_scroll(auto_scroll), column_index(0)
			{
				if (!this->assemble(parent_widget, rect, auto_scroll, on_click_function))
				{
					throw std::exception("Failed to create 'listview' widget");
				}
			}

			~listview()
			{

			}

			bool assemble(widget& parent_widget, rectangle& rect = rectangle(), bool auto_scroll = false, listview_function_t on_click_function = [](std::size_t) -> void {  })
			{
				if (!this->create(WC_LISTVIEW, "", rect, WS_VISIBLE | WS_CHILD | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS | 
					LVS_SINGLESEL | LVS_NOSORTHEADER | LVS_OWNERDRAWFIXED, 0, parent_widget.get_handle()))
				{
					return false;
				}
				
				ListView_SetExtendedListViewStyle(this->get_handle(), LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | (false ? LVS_EX_GRIDLINES : 0));
				ListView_SetTextBkColor(this->get_handle(), CLR_NONE);

				this->auto_scroll = auto_scroll;
				this->on_click_function = on_click_function;
				this->menu = CreatePopupMenu();
				this->set_message_handlers();
				return true;
			}
			
			bool add_column(std::string const& column_name, std::size_t width)
			{
				LVCOLUMN listview_column;
				listview_column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
				listview_column.fmt = LVCFMT_FIXED_WIDTH;
				listview_column.pszText = const_cast<char*>(column_name.c_str());
				listview_column.cx = static_cast<int>(width);

				if (ListView_InsertColumn(this->get_handle(), this->column_index, &listview_column) == -1)
				{
					return false;
				}

				this->column_index++;
				return true;
			}
			
			bool remove_column(std::size_t index)
			{
				if (!ListView_DeleteColumn(this->get_handle(), index))
				{
					return false;
				}

				this->column_index--;
				return true;
			}
			
			bool add_item(std::size_t sub_items, ...)
			{
				std::size_t item_index = ListView_GetItemCount(this->get_handle());

				LVITEM listview_item;
				memset(&listview_item, 0, sizeof(LVITEM));

				listview_item.mask = LVIF_TEXT | LVIF_PARAM;
				listview_item.iItem = item_index;
				listview_item.lParam = NULL;

				if (ListView_InsertItem(this->get_handle(), &listview_item) == -1)
				{
					return false;
				}

				listview_item.mask &= ~LVIF_PARAM;

				va_list va;
				va_start(va, sub_items);

				for (std::size_t index = 0; index < sub_items; index++)
				{
					const char* sub_item = va_arg(va, const char*);
					listview_item.iSubItem = static_cast<int>(index);
					listview_item.pszText = const_cast<char*>(sub_item);
					StringCchLength(sub_item, 2048 * 3, reinterpret_cast<size_t*>(&listview_item.cchTextMax));

					if (!ListView_SetItem(this->get_handle(), &listview_item))
					{
						return false;
					}
				}

				va_end(va);

				if (this->auto_scroll)
				{
					return (ListView_EnsureVisible(this->get_handle(), item_index, FALSE) != FALSE);
				}

				return true;
			}

			bool add_item(COLORREF text_color, std::size_t sub_items, ...)
			{
				std::size_t item_index = ListView_GetItemCount(this->get_handle());

				LVITEM listview_item;
				memset(&listview_item, 0, sizeof(LVITEM));

				listview_item.mask = LVIF_TEXT | LVIF_PARAM;
				listview_item.iItem = item_index;
				listview_item.lParam = static_cast<LPARAM>(text_color);

				if (ListView_InsertItem(this->get_handle(), &listview_item) == -1)
				{
					return false;
				}

				listview_item.mask &= ~LVIF_PARAM;

				va_list va;
				va_start(va, sub_items);
				
				for (std::size_t index = 0; index < sub_items; index++)
				{
					const char* sub_item = va_arg(va, const char*);
					listview_item.iSubItem = static_cast<int>(index);
					listview_item.pszText = const_cast<char*>(sub_item);
					StringCchLength(sub_item, 2048 * 3, reinterpret_cast<size_t*>(&listview_item.cchTextMax));

					if (!ListView_SetItem(this->get_handle(), &listview_item))
					{
						return false;
					}
				}

				va_end(va);

				if (this->auto_scroll)
				{
					return (ListView_EnsureVisible(this->get_handle(), item_index, FALSE) != FALSE);
				}

				return true;
			}

			bool remove_item(std::size_t index)
			{
				return (ListView_DeleteItem(this->get_handle(), index) != FALSE);
			}

			bool clear_items()
			{
				return (ListView_DeleteAllItems(this->get_handle()) != FALSE);
			}
			
			bool add_menu_item(std::string const& menu_name, listview_function_t menu_function)
			{
				if (!AppendMenu(this->menu, MF_STRING, this->menu_index, menu_name.c_str()))
				{
					return false;
				}
				
				this->menu_functions[this->menu_index++] = menu_function;
				return true;
			}

			bool remove_menu_item(std::size_t index)
			{
				if (!RemoveMenu(this->menu, this->menu_index, MF_BYPOSITION))
				{
					return false;
				}

				this->menu_functions.erase(this->menu_index--);
				return true;
			}
			
			std::size_t get_item_count()
			{
				return ListView_GetItemCount(this->get_handle());
			}

			std::size_t get_item_data(std::size_t item_index, std::size_t sub_item_index)
			{
				return atoi(this->get_item_text(item_index, sub_item_index).c_str());
			}

			std::string get_item_text(std::size_t item_index, std::size_t sub_item_index)
			{
				char item_text[4096];
				memset(item_text, 0, sizeof(item_text));
				
				LV_ITEM listview_item;
				memset(&listview_item, 0, sizeof(LV_ITEM));

				listview_item.iSubItem = sub_item_index;
				listview_item.pszText = item_text;
				listview_item.cchTextMax = sizeof(item_text);

				int text_length = SendMessage(this->get_handle(), LVM_GETITEMTEXT, item_index, reinterpret_cast<LPARAM>(&listview_item));
				return (text_length == 0 ? std::string("") : std::string(item_text));
			}

		protected:
			void set_message_handlers()
			{
				this->add_message_handlers(6,
					message_pair(WM_LBUTTONDBLCLK, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						this->try_click_functionality();
						return 0;
					}),
					message_pair(WM_RBUTTONDOWN, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						this->try_menu_functionality();
						return 0;
					}),
					message_pair(WM_NOTIFY, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						NMHDR* custom_notify = reinterpret_cast<NMHDR*>(lParam);
						
						if (custom_notify->code == HDN_BEGINTRACKW)
						{
							return TRUE;
						}
						else if (custom_notify->code != NM_CUSTOMDRAW)
						{
							return CallWindowProc(this->original_wndproc, hWnd, WM_NOTIFY, wParam, lParam);
						}
				
						NMCUSTOMDRAW* custom_draw = reinterpret_cast<NMCUSTOMDRAW*>(lParam);
				
						if (custom_draw->dwDrawStage == CDDS_PREPAINT)
						{
							return CDRF_NOTIFYITEMDRAW;
						}
						else if (custom_draw->dwDrawStage == CDDS_ITEMPREPAINT && custom_draw->dwItemSpec >= 0)
						{
							//return this->draw_column(hWnd, custom_draw);
						}

						return CDRF_DODEFAULT;
					}),
					message_pair(CUSTOM_NOTIFY, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						NMHDR* custom_notify = reinterpret_cast<NMHDR*>(lParam);
						NMLVCUSTOMDRAW* custom_draw = reinterpret_cast<NMLVCUSTOMDRAW*>(lParam);
						
						if (custom_notify->code != NM_CUSTOMDRAW || custom_draw->nmcd.dwDrawStage != CDDS_PREPAINT)
						{
							return CallWindowProc(this->original_wndproc, hWnd, WM_NOTIFY, wParam, lParam);
						}
				
						FillRect(custom_draw->nmcd.hdc, &custom_draw->nmcd.rc, CreateSolidBrush(this->get_background_color()));
						return CDRF_DODEFAULT;
					}),
					message_pair(OWNER_MEASUREITEM, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						reinterpret_cast<MEASUREITEMSTRUCT*>(lParam)->itemHeight = 15;
						return 0;
					}),
					message_pair(OWNER_DRAWITEM, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						DRAWITEMSTRUCT* item_draw = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
						return (item_draw->itemID >= 0 ? this->draw_item(item_draw) : 0);
					})
				);
			}
	
			void try_click_functionality()
			{
				if (this->on_click_function)
				{
					POINT mouse_location;
					memset(&mouse_location, 0, sizeof(POINT));

					GetCursorPos(&mouse_location);
					ScreenToClient(this->get_handle(), &mouse_location);

					LV_HITTESTINFO listview_hittest_info;
					listview_hittest_info.pt.x = mouse_location.x;
					listview_hittest_info.pt.y = mouse_location.y;

					int item_index = ListView_HitTest(this->get_handle(), &listview_hittest_info);

					if (item_index != -1)
					{
						this->on_click_function(item_index);
					}
				}
			}

			void try_menu_functionality()
			{
				POINT mouse_location;
				memset(&mouse_location, 0, sizeof(POINT));

				GetCursorPos(&mouse_location);
				ScreenToClient(this->get_handle(), &mouse_location);

				LV_HITTESTINFO listview_hittest_info;
				listview_hittest_info.pt.x = mouse_location.x;
				listview_hittest_info.pt.y = mouse_location.y;

				int item_index = ListView_HitTest(this->get_handle(), &listview_hittest_info);

				if (item_index != -1)
				{
					ListView_SetItemState(this->get_handle(), item_index, LVIS_SELECTED, LVIS_SELECTED);

					ClientToScreen(this->get_handle(), &mouse_location);

					std::size_t selected_menu = static_cast<std::size_t>(TrackPopupMenu(this->menu, TPM_RETURNCMD, 
						mouse_location.x, mouse_location.y, 0, this->get_handle(), NULL));

					if (this->menu_functions[selected_menu] != 0)
					{
						return this->menu_functions[selected_menu](item_index);
					}
				}
			}
			
			//LRESULT draw_column(HWND hwnd_listview, NMCUSTOMDRAW* column_draw)
			//{
			//	HWND hwnd_header = ListView_GetHeader(hwnd_listview);

			//	if (!hwnd_header)
			//	{
			//		return CDRF_DODEFAULT;
			//	}

			//	char header_text[256];

			//	HDITEM header_item;
			//	memset(&header_item, 0, sizeof(HDITEM));

			//	header_item.mask = HDI_TEXT;
			//	header_item.pszText = header_text;
			//	header_item.cchTextMax = 256;

			//	if (!Header_GetItem(hwnd_header, column_draw->dwItemSpec, &header_item))
			//	{
			//		return CDRF_DODEFAULT;
			//	}
			//	
			//	/* draw the column background */
			//	HBITMAP background_bitmap = gui::png_image::make_image(PNG_COLUMN_BACKGROUND);
		
			//	HDC hdc_source = CreateCompatibleDC(NULL);
			//	HBITMAP old_bitmap = reinterpret_cast<HBITMAP>(SelectObject(hdc_source, background_bitmap));

			//	BITMAP bitmap_info;
			//	GetObject(background_bitmap, sizeof(bitmap_info), &bitmap_info);
		
			//	column_draw->rc.left -= (column_draw->dwItemSpec == 0);

			//	BitBlt(column_draw->hdc, column_draw->rc.left, column_draw->rc.top, column_draw->rc.right - column_draw->rc.left, 
			//		column_draw->rc.bottom - column_draw->rc.top, hdc_source, 0, 0, SRCCOPY);
		
			//	SelectObject(hdc_source, old_bitmap);
			//	DeleteDC(hdc_source);

			//	/* draw the column captions */
			//	int background_mode = SetBkMode(column_draw->hdc, TRANSPARENT);
			//	COLORREF text_color = SetTextColor(column_draw->hdc, RGB(100, 100, 100));
		
			//	HFONT column_font = CreateFont(13, 0, 0, 0, FW_DONTCARE, 0, 0, 0, ANSI_CHARSET, 
			//		OUT_DEFAULT_PRECIS,	CLIP_DEFAULT_PRECIS, 5, DEFAULT_PITCH | FF_DONTCARE, "Tahoma");
		
			//	HFONT old_font = reinterpret_cast<HFONT>(SelectObject(column_draw->hdc, column_font));

			//	RECT rc_align(column_draw->rc);
			//	column_draw->rc.top -= 3;
			//	DrawText(column_draw->hdc, header_text, -1, &rc_align, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);
			//	DrawText(column_draw->hdc, header_text, -1, &column_draw->rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
		
			//	SelectObject(column_draw->hdc, old_font);
			//	SetTextColor(column_draw->hdc, text_color);
			//	SetBkMode(column_draw->hdc, background_mode);
			//	return CDRF_SKIPDEFAULT;
			//}

			LRESULT draw_item(DRAWITEMSTRUCT* item_draw)
			{
				int background_mode = SetBkMode(item_draw->hDC, TRANSPARENT);
				COLORREF text_color = SetTextColor(item_draw->hDC, (item_draw->itemState & ODS_SELECTED ?
					GetSysColor(COLOR_HIGHLIGHTTEXT) : (item_draw->itemData ? item_draw->itemData : this->get_foreground_color())));
			
				LVITEM listview_item;
				memset(&listview_item, 0, sizeof(LVITEM));
				
				/* draw the listview item labels */
				for (std::size_t sub_item_index = 0; sub_item_index < this->column_index; sub_item_index++)
				{
					/* get item information */
					bool is_sub_item = (sub_item_index > 0);
			
					RECT rc_item;
			
					if (!is_sub_item)
					{
						ListView_GetItemRect(item_draw->hwndItem, item_draw->itemID, &rc_item, LVIR_BOUNDS);
					}
					else
					{
						ListView_GetSubItemRect(item_draw->hwndItem, item_draw->itemID, sub_item_index, LVIR_BOUNDS, &rc_item);
					}

					std::string item_text = this->get_item_text(item_draw->itemID, sub_item_index);

					/* draw item background */
					RECT rc_background(rc_item);
					FillRect(item_draw->hDC, &rc_background, CreateSolidBrush(this->get_background_color()));
		
					if ((item_draw->itemState & ODS_SELECTED) && (item_draw->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
					{
						rc_background.left++;
						rc_background.bottom--;
						FillRect(item_draw->hDC, &rc_background, CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT)));
					}

					/* draw the actual label */
					RECT rc_align(rc_item);
					DrawText(item_draw->hDC, item_text.c_str(), -1, &rc_align, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);
					
					rc_item.left += 5;
				
					DrawText(item_draw->hDC, item_text.c_str(), -1, &rc_item, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
				}

				SetTextColor(item_draw->hDC, text_color);
				SetBkMode(item_draw->hDC, background_mode);
				return 0;
			}

		private:
			bool auto_scroll;

			std::size_t column_index;
			listview_function_t on_click_function;
			
			HMENU menu;
			std::size_t menu_index;
			std::unordered_map<std::size_t, listview_function_t> menu_functions;
		};
	}
}