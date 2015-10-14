#pragma once

#include "generic.hpp"
#include "widget.hpp"

#include "png_image.hpp"

namespace sparta
{
	namespace gui
	{
		typedef std::function<void(bool)> treeview_function_t;

		enum class tree_level
		{
			root,
			parent,
			child
		};

		struct treeview_param
		{
			HTREEITEM tree_item;

			bool is_editable;
			bool has_checkbox;
			treeview_function_t on_check_function;
		};

		class treeview : public widget
		{
		public:
			treeview(HWND hwnd_parent = NULL, HINSTANCE instance = NULL)
				: widget(hwnd_parent, instance), has_checkboxes(false)
			{

			}

			~treeview()
			{
				this->clear_items();
			}

			bool assemble(widget& parent_widget, rectangle& rect = rectangle(), bool has_checkboxes = false)
			{
				if (!this->create(WC_TREEVIEW, "", rect, WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_EDITLABELS | TVS_HASLINES | TVS_LINESATROOT | TVS_INFOTIP |
					 TVS_FULLROWSELECT | (has_checkboxes ? TVS_CHECKBOXES : NULL), 0, parent_widget.get_handle()))
				{
					return false;
				}
				
				this->has_checkboxes = has_checkboxes;
				this->set_message_handlers();
				return true;
			}
			
			HTREEITEM add_item(std::string const& caption, tree_level level, bool is_editable = false, bool has_checkbox = false, treeview_function_t on_check_function = [](bool) -> void {  })
			{ 
				static HTREEITEM prev_tree_item = TVI_FIRST; 
				static HTREEITEM prev_root_item = NULL; 
				static HTREEITEM prev_child_item = NULL; 

				TVINSERTSTRUCT treeview_insert_struct; 
				memset(&treeview_insert_struct, 0, sizeof(TVINSERTSTRUCT));
				memset(&treeview_insert_struct.item, 0, sizeof(TVITEM));
				
				treeview_insert_struct.item.mask = TVIF_TEXT; 
				treeview_insert_struct.item.pszText = const_cast<char*>(caption.c_str()); 
				treeview_insert_struct.item.cchTextMax = caption.length() + 1; 

				treeview_insert_struct.hInsertAfter = prev_tree_item; 

				switch (level)
				{
				case tree_level::root:
					treeview_insert_struct.hParent = TVI_ROOT; 
					break;

				case tree_level::parent:
					treeview_insert_struct.hParent = prev_root_item; 
					break;

				default:
					treeview_insert_struct.hParent = prev_child_item; 
					break;
				}

				prev_tree_item = TreeView_InsertItem(this->get_handle(), &treeview_insert_struct);

				if (prev_tree_item == NULL)
				{
					return NULL;
				}

				switch (level)
				{
				case tree_level::root:
					prev_root_item = prev_tree_item; 
					break;

				case tree_level::parent:
					TreeView_Expand(this->get_handle(), prev_root_item, TVE_EXPAND);
					prev_child_item = prev_tree_item; 
					break;

				default:
					TreeView_Expand(this->get_handle(), prev_child_item, TVE_EXPAND);
					break;
				}

				treeview_param* param = new treeview_param;
				param->tree_item = prev_tree_item;
				param->is_editable = is_editable;
				param->has_checkbox = has_checkbox;
				param->on_check_function = on_check_function;

				treeview_insert_struct.item.mask = TVIF_PARAM; 
				treeview_insert_struct.item.hItem = prev_tree_item; 
				treeview_insert_struct.item.lParam = reinterpret_cast<LPARAM>(param); 

				if (!TreeView_SetItem(this->get_handle(), &treeview_insert_struct.item))
				{
					return NULL;
				}

				return prev_tree_item; 
			} 

			bool remove_item(HTREEITEM tree_item)
			{
				if (!this->delete_item(tree_item))
				{
					return false;
				}

				return (TreeView_DeleteItem(this->get_handle(), tree_item) != FALSE);
			}

			bool clear_items()
			{
				HTREEITEM root_item = TreeView_GetRoot(this->get_handle());

				if (!root_item)
				{
					return false;
				}

				std::vector<std::pair<HTREEITEM, int>> treeview_nodes;
				this->traverse_all_nodes(root_item, treeview_nodes);

				for (int i = 2; i >= 0; i--)
				{
					for (std::pair<HTREEITEM, int> node : treeview_nodes)
					{
						if (node.second == i)
						{
							if (!this->remove_item(node.first))
							{
								return false;
							}
						}
					}
				}

				return true;
			}
			
		protected:
			void traverse_all_nodes(HTREEITEM tree_item, std::vector<std::pair<HTREEITEM, int>>& treeview_nodes, int level = 0)
			{
				HTREEITEM current_tree_item = tree_item;

				while (current_tree_item != NULL)
				{
					treeview_nodes.push_back(std::pair<HTREEITEM, int>(current_tree_item, level));
					
					if (this->has_children(current_tree_item))
					{
						this->traverse_all_nodes(TreeView_GetChild(this->get_handle(), current_tree_item), treeview_nodes, level + 1);
					}

					current_tree_item = TreeView_GetNextSibling(this->get_handle(), current_tree_item);
				}
			}

			bool delete_item(HTREEITEM tree_item)
			{
				TVITEM treeview_item;
				memset(&treeview_item, 0, sizeof(TVITEM));

				treeview_item.mask = TVIF_PARAM; 
				treeview_item.hItem = tree_item; 

				if (!TreeView_GetItem(this->get_handle(), &treeview_item))
				{
					return false;
				}

				delete reinterpret_cast<treeview_param*>(treeview_item.lParam);
				return true;
			}

			bool has_children(HTREEITEM tree_item)
			{
				TVITEM treeview_item;
				memset(&treeview_item, 0, sizeof(TVITEM));
				
				treeview_item.mask = TVIF_CHILDREN; 
				treeview_item.hItem = tree_item; 

				if (!TreeView_GetItem(this->get_handle(), &treeview_item))
				{
					return false;
				}

				return (treeview_item.cChildren > 0);
			}

			void set_message_handlers()
			{
				this->add_message_handlers(2,
					message_pair(WM_ERASEBKGND, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						return 1;
					}),
					message_pair(CUSTOM_NOTIFY, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
					{
						NMHDR* custom_notify = reinterpret_cast<NMHDR*>(lParam);
						
						switch (custom_notify->code)
						{
						case TVN_BEGINLABELEDIT:
							{
								NMTVDISPINFO* treeview_display_info = reinterpret_cast<NMTVDISPINFO*>(lParam);
								treeview_param* param = reinterpret_cast<treeview_param*>(treeview_display_info->item.lParam);

								if (!param->is_editable)
								{
									return TRUE;
								}
							
								HWND treeview_edit_control = TreeView_GetEditControl(this->get_handle());
								SetFocus(treeview_edit_control);
								break;
							}

						case TVN_ENDLABELEDIT:
							{
								NMTVDISPINFO* treeview_display_info = reinterpret_cast<NMTVDISPINFO*>(lParam);

								if (treeview_display_info->item.pszText != NULL)
								{
									return TRUE;
								}
								
								break;
							}

						case NM_TVSTATEIMAGECHANGING:
							{
								NMTVSTATEIMAGECHANGING* state_image_changing = reinterpret_cast<NMTVSTATEIMAGECHANGING*>(lParam);

								TV_ITEM treeview_item;
								memset(&treeview_item, 0, sizeof(TV_ITEM));

								treeview_item.mask = TVIF_PARAM;
								treeview_item.hItem = state_image_changing->hti;

								if (!TreeView_GetItem(this->get_handle(), &treeview_item))
								{
									return 0;
								}

								treeview_param* param = reinterpret_cast<treeview_param*>(treeview_item.lParam);

								if (param->has_checkbox && param->on_check_function)
								{
									param->on_check_function(state_image_changing->iNewStateImageIndex == 2);
								}
								else
								{
									TreeView_SelectItem(this->get_handle(), state_image_changing->hti);
								}

								return 0;
							}

						case NM_CUSTOMDRAW:
							{
								NMTVCUSTOMDRAW* custom_draw = reinterpret_cast<NMTVCUSTOMDRAW*>(lParam);
								
								if (custom_draw->nmcd.dwDrawStage == CDDS_PREPAINT)
								{
									FillRect(custom_draw->nmcd.hdc, &custom_draw->nmcd.rc, CreateSolidBrush(RGB(53, 53, 53)));
									return CDRF_NOTIFYITEMDRAW;
								}
								else if (custom_draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
								{
									BLENDFUNCTION blend_function; 
									blend_function.BlendOp = AC_SRC_OVER;
									blend_function.BlendFlags = 0;
									blend_function.SourceConstantAlpha = 255;
									blend_function.AlphaFormat = AC_SRC_ALPHA;

									FillRect(custom_draw->nmcd.hdc, &custom_draw->nmcd.rc, CreateSolidBrush(RGB(53, 53, 53)));

									treeview_param* param = reinterpret_cast<treeview_param*>(custom_draw->nmcd.lItemlParam);

									if (param)
									{
										/* retrieve treeview item information */
										TV_ITEM treeview_item;
										memset(&treeview_item, 0, sizeof(TV_ITEM));

										char treeview_item_caption[256];
										memset(treeview_item_caption, 0, sizeof(treeview_item_caption));

										treeview_item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
										treeview_item.pszText = treeview_item_caption;
										treeview_item.cchTextMax = sizeof(treeview_item_caption);
										treeview_item.hItem = param->tree_item;

										if (!TreeView_GetItem(this->get_handle(), &treeview_item))
										{
											return CDRF_SKIPDEFAULT;
										}

										/* draw expansion-icon */
										if (treeview_item.cChildren > 0)
										{
											RECT rc_expand_button(custom_draw->nmcd.rc);
											rc_expand_button.left += 5 + (19 * custom_draw->iLevel);
											rc_expand_button.top += 3;
											rc_expand_button.right = rc_expand_button.left + 11;
											rc_expand_button.bottom -= 2;
								
											HBITMAP expand_button_image = png::make_image((treeview_item.state & TVIS_EXPANDED) ? PNG_TREEVIEW_ARROW_COLLAPSED : PNG_TREEVIEW_ARROW_COLLAPSIBLE);

											BITMAP bitmap_info;
											GetObject(expand_button_image, sizeof(bitmap_info), &bitmap_info);
								
											HDC hdc_source = CreateCompatibleDC(NULL);
											HBITMAP old_bitmap = reinterpret_cast<HBITMAP>(SelectObject(hdc_source, expand_button_image));
					
											((treeview_item.state & TVIS_EXPANDED) ? rc_expand_button.top : rc_expand_button.left) += 2;
								
											AlphaBlend(custom_draw->nmcd.hdc, rc_expand_button.left, rc_expand_button.top, bitmap_info.bmWidth, bitmap_info.bmHeight,
												hdc_source, 0, 0, bitmap_info.bmWidth, bitmap_info.bmHeight, blend_function);

											SelectObject(hdc_source, old_bitmap);

											DeleteDC(hdc_source);
											DeleteObject(expand_button_image);
										}

										/* draw checkbox */
										if (this->has_checkboxes && param->has_checkbox)
										{
											RECT rc_checkbox(custom_draw->nmcd.rc);
											rc_checkbox.left += 22 + (19 * custom_draw->iLevel);
											rc_checkbox.top += 1;
											rc_checkbox.right = rc_checkbox.left + 14;
											rc_checkbox.bottom -= 1;
								
											FillRect(custom_draw->nmcd.hdc, &rc_checkbox, CreateSolidBrush(RGB(40, 40, 40)));
											InflateRect(&rc_checkbox, -1, -1);
											FillRect(custom_draw->nmcd.hdc, &rc_checkbox, CreateSolidBrush(RGB(53, 53, 53)));

											if (TreeView_GetCheckState(this->get_handle(), param->tree_item))
											{
												HBITMAP check_image = png::make_image(PNG_TREEVIEW_CHECKBOX_CHECK);

												BITMAP bitmap_info;
												GetObject(check_image, sizeof(bitmap_info), &bitmap_info);
								
												HDC hdc_source = CreateCompatibleDC(NULL);
												HBITMAP old_bitmap = reinterpret_cast<HBITMAP>(SelectObject(hdc_source, check_image));
									
												AlphaBlend(custom_draw->nmcd.hdc, rc_checkbox.left, rc_checkbox.top, bitmap_info.bmWidth, bitmap_info.bmHeight,
													hdc_source, 0, 0, bitmap_info.bmWidth, bitmap_info.bmHeight, blend_function);

												SelectObject(hdc_source, old_bitmap);

												DeleteDC(hdc_source);
												DeleteObject(check_image);
											}
										}

										/* draw text */
										RECT rc_text(custom_draw->nmcd.rc);
										rc_text.left += 24 + (((this->has_checkboxes && param->has_checkbox && custom_draw->iLevel > 0) || param->is_editable) ? 16 : 0) + (19 * custom_draw->iLevel);

										SetBkMode(custom_draw->nmcd.hdc, TRANSPARENT);
										SetTextColor(custom_draw->nmcd.hdc, RGB(255, 255, 255));
							
										if (custom_draw->nmcd.uItemState & CDIS_SELECTED)
										{
											RECT rc_selection(rc_text);
											rc_selection.left -= 3;

											FillRect(custom_draw->nmcd.hdc, &rc_selection, CreateSolidBrush(RGB(73, 90, 97)));
											InflateRect(&rc_selection, -1, -1);
											FillRect(custom_draw->nmcd.hdc, &rc_selection, CreateSolidBrush(RGB(60, 70, 80)));
										}
							
										HFONT new_font = CreateFont(13, 0, 0, 0, (custom_draw->iLevel > 0 ? FW_DONTCARE : FW_BOLD), 0, 0, 0, ANSI_CHARSET, 
											OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5, DEFAULT_PITCH | FF_DONTCARE, "Tahoma");

										HGDIOBJ old_font = SelectObject(custom_draw->nmcd.hdc, new_font);

										DrawText(custom_draw->nmcd.hdc, treeview_item_caption, -1, &rc_text, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

										SelectObject(custom_draw->nmcd.hdc, old_font);
										DeleteObject(new_font);
									}
								
									return CDRF_SKIPDEFAULT;
								}

								return CDRF_DODEFAULT;
							}

						default:
							break;
						}

						return CallWindowProc(this->original_wndproc, hWnd, WM_NOTIFY, wParam, lParam);
					})
				);
			}
	
		private:
			bool has_checkboxes;
		};
	}
}