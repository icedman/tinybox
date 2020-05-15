{ "background.color:",
        &style->background_color, parseColor },                  // fb
{ "background.colorTo:",
        &style->background_colorTo, parseColor },                // fb
{ "background.modX:",
        &style->background_modX, parseValue },                   // fb
{ "background.modY:",
        &style->background_modY, parseValue },                   // fb
{ "background:",
        &style->background, parseValue },                        // fb
{ "bbpager.desktop.focus.color:",
        &style->bbpager_desktop_focus_color, parseColor },       // fb
{ "bbpager.desktop.focus.colorTo:",
        &style->bbpager_desktop_focus_colorTo, parseColor },     // fb
{ "bbpager.desktop.focus:",
        &style->bbpager_desktop_focus, parseValue },             // fb
{ "bevelWidth:",
        &style->bevelWidth, parseInt },                          // bbwm fb
{ "border.Width:",
        &style->border_Width, parseInt },                        // ob
{ "border.color:",
        &style->border_color, parseColor },                      // ob
{ "border.width:",
        &style->border_width, parseInt },                        // ob
{ "borderColor:",
        &style->borderColor, parseColor },                       // bbwm fb
{ "borderWidth:",
        &style->borderWidth, parseInt },                         // bbwm fb ob
{ "focus.inner.color:",
        &style->focus_inner_color, parseColor },                 // ob
{ "focus.outer.color:",
        &style->focus_outer_color, parseColor },                 // ob
{ "frameWidth:",
        &style->frameWidth, parseInt },                          // bbwm fb
{ "handleWidth:",
        &style->handleWidth, parseInt },                         // bbwm fb
{ "menu.active.appearance:",
        &style->menu_active_appearance, parseValue },            // bbwm
{ "menu.active.backgroundColor:",
        &style->menu_active_backgroundColor, parseColor },       // bbwm
{ "menu.active.borderColor:",
        &style->menu_active_borderColor, parseColor },           // bbwm
{ "menu.active.borderWidth:",
        &style->menu_active_borderWidth, parseInt },             // bbwm
{ "menu.active.color:",
        &style->menu_active_color, parseColor },                 // bbwm
{ "menu.active.colorTo:",
        &style->menu_active_colorTo, parseColor },               // bbwm
{ "menu.active.foregroundColor:",
        &style->menu_active_foregroundColor, parseColor },       // bbwm
{ "menu.active.textColor:",
        &style->menu_active_textColor, parseColor },             // bbwm
{ "menu.active:",
        &style->menu_active, parseValue },                       // bbwm
{ "menu.bevelWidth:",
        &style->menu_bevelWidth, parseInt },                     // fb
{ "menu.border.color:",
        &style->menu_border_color, parseColor },                 // ob
{ "menu.border.width:",
        &style->menu_border_width, parseInt },                   // ob
{ "menu.borderColor:",
        &style->menu_borderColor, parseColor },                  // fb
{ "menu.borderWidth:",
        &style->menu_borderWidth, parseInt },                    // fb
{ "menu.borderwidth:",
        &style->menu_borderwidth, parseInt },                    // fb
{ "menu.bullet.image.color:",
        &style->menu_bullet_image_color, parseColor },           // ob
{ "menu.bullet.position:",
        &style->menu_bullet_position, parseValue },              // bbwm fb
{ "menu.bullet.selected.image.color:",
        &style->menu_bullet_selected_image_color, parseColor },  // ob
{ "menu.bullet:",
        &style->menu_bullet, parseValue },                       // bbwm fb
{ "menu.frame.alignment:",
        &style->menu_frame_alignment, parseValue },              // bbwm
{ "menu.frame.appearance:",
        &style->menu_frame_appearance, parseValue },             // bbwm
{ "menu.frame.borderColor:",
        &style->menu_frame_borderColor, parseColor },            // bbwm
{ "menu.frame.borderWidth:",
        &style->menu_frame_borderWidth, parseInt },              // bbwm
{ "menu.frame.color:",
        &style->menu_frame_color, parseColor },                  // bbwm fb
{ "menu.frame.colorTo:",
        &style->menu_frame_colorTo, parseColor },                // bbwm fb
{ "menu.frame.disableColor:",
        &style->menu_frame_disableColor, parseColor },           // bbwm fb
{ "menu.frame.disabledColor:",
        &style->menu_frame_disabledColor, parseColor },          // bbwm
{ "menu.frame.font:",
        &style->menu_frame_font, parseString },                  // bbwm fb
{ "menu.frame.foregroundColor:",
        &style->menu_frame_foregroundColor, parseColor },        // bbwm
{ "menu.frame.justify:",
        &style->menu_frame_justify, parseValue },                // bbwm fb ob
{ "menu.frame.pixmap:",
        &style->menu_frame_pixmap, parseString },                // fb
{ "menu.frame.textColor:",
        &style->menu_frame_textColor, parseColor },              // bbwm fb
{ "menu.frame.underlineColor:",
        &style->menu_frame_underlineColor, parseColor },         // fb
{ "menu.frame:",
        &style->menu_frame, parseValue },                        // bbwm fb
{ "menu.hilite.color:",
        &style->menu_hilite_color, parseColor },                 // bbwm fb
{ "menu.hilite.colorTo:",
        &style->menu_hilite_colorTo, parseColor },               // bbwm fb
{ "menu.hilite.pixmap:",
        &style->menu_hilite_pixmap, parseString },               // fb
{ "menu.hilite.selected.pixmap:",
        &style->menu_hilite_selected_pixmap, parseString },      // fb
{ "menu.hilite.submenu.pixmap:",
        &style->menu_hilite_submenu_pixmap, parseString },       // fb
{ "menu.hilite.textColor:",
        &style->menu_hilite_textColor, parseColor },             // bbwm fb
{ "menu.hilite.unselected.pixmap:",
        &style->menu_hilite_unselected_pixmap, parseString },    // fb
{ "menu.hilite:",
        &style->menu_hilite, parseValue },                       // bbwm fb
{ "menu.items.active.bg.border.color:",
        &style->menu_items_active_bg_border_color, parseColor }, // ob
{ "menu.items.active.bg.color.splitTo:",
        &style->menu_items_active_bg_color_splitTo, parseColor },// ob
{ "menu.items.active.bg.color:",
        &style->menu_items_active_bg_color, parseColor },        // ob
{ "menu.items.active.bg.colorTo.splitTo:",
        &style->menu_items_active_bg_colorTo_splitTo, parseColor },// ob
{ "menu.items.active.bg.colorTo:",
        &style->menu_items_active_bg_colorTo, parseColor },      // ob
{ "menu.items.active.bg:",
        &style->menu_items_active_bg, parseValue },              // ob
{ "menu.items.active.disabled.text.color:",
        &style->menu_items_active_disabled_text_color, parseColor },// ob
{ "menu.items.active.text.color:",
        &style->menu_items_active_text_color, parseColor },      // ob
{ "menu.items.bg.border.color:",
        &style->menu_items_bg_border_color, parseColor },        // ob
{ "menu.items.bg.color:",
        &style->menu_items_bg_color, parseColor },               // ob
{ "menu.items.bg.colorTo:",
        &style->menu_items_bg_colorTo, parseColor },             // ob
{ "menu.items.bg:",
        &style->menu_items_bg, parseValue },                     // ob
{ "menu.items.disable.text.color:",
        &style->menu_items_disable_text_color, parseColor },     // ob
{ "menu.items.disabled.text.color:",
        &style->menu_items_disabled_text_color, parseColor },    // ob
{ "menu.items.font:",
        &style->menu_items_font, parseString },                  // ob
{ "menu.items.justify:",
        &style->menu_items_justify, parseValue },                // ob
{ "menu.items.text.color:",
        &style->menu_items_text_color, parseColor },             // ob
{ "menu.overlap:",
        &style->menu_overlap, parseValue },                      // ob
{ "menu.selected.pixmap:",
        &style->menu_selected_pixmap, parseString },             // fb
{ "menu.separator.color:",
        &style->menu_separator_color, parseColor },              // ob
{ "menu.separator.padding.height:",
        &style->menu_separator_padding_height, parseInt },       // ob
{ "menu.separator.padding.width:",
        &style->menu_separator_padding_width, parseInt },        // ob
{ "menu.separator.width:",
        &style->menu_separator_width, parseInt },                // ob
{ "menu.submenu.pixmap:",
        &style->menu_submenu_pixmap, parseString },              // fb
{ "menu.title.alignment:",
        &style->menu_title_alignment, parseValue },              // bbwm
{ "menu.title.appearance:",
        &style->menu_title_appearance, parseValue },             // bbwm
{ "menu.title.backgroundColor:",
        &style->menu_title_backgroundColor, parseColor },        // bbwm
{ "menu.title.bg.border.color:",
        &style->menu_title_bg_border_color, parseColor },        // ob
{ "menu.title.bg.color:",
        &style->menu_title_bg_color, parseColor },               // ob
{ "menu.title.bg.colorTo:",
        &style->menu_title_bg_colorTo, parseColor },             // ob
{ "menu.title.bg:",
        &style->menu_title_bg, parseValue },                     // ob
{ "menu.title.borderColor:",
        &style->menu_title_borderColor, parseColor },            // bbwm
{ "menu.title.borderWidth:",
        &style->menu_title_borderWidth, parseInt },              // bbwm
{ "menu.title.color:",
        &style->menu_title_color, parseColor },                  // bbwm fb
{ "menu.title.colorTo:",
        &style->menu_title_colorTo, parseColor },                // bbwm fb
{ "menu.title.font.effect:",
        &style->menu_title_font_effect, parseString },           // fb
{ "menu.title.font.shadow.color:",
        &style->menu_title_font_shadow_color, parseColor },      // fb
{ "menu.title.font:",
        &style->menu_title_font, parseString },                  // bbwm fb
{ "menu.title.foregroundColor:",
        &style->menu_title_foregroundColor, parseColor },        // bbwm
{ "menu.title.justify:",
        &style->menu_title_justify, parseValue },                // bbwm fb
{ "menu.title.pixmap:",
        &style->menu_title_pixmap, parseString },                // fb
{ "menu.title.text.color:",
        &style->menu_title_text_color, parseColor },             // ob
{ "menu.title.text.font:",
        &style->menu_title_text_font, parseString },             // ob
{ "menu.title.text.justify:",
        &style->menu_title_text_justify, parseValue },           // ob
{ "menu.title.textColor:",
        &style->menu_title_textColor, parseColor },              // bbwm fb
{ "menu.title:",
        &style->menu_title, parseValue },                        // bbwm fb
{ "menu.unselected.pixmap:",
        &style->menu_unselected_pixmap, parseString },           // fb
{ "menuFont:",
        &style->menuFont, parseString },                         // fb
{ "osd.active.label.bg.border.color:",
        &style->osd_active_label_bg_border_color, parseColor },  // ob
{ "osd.active.label.bg.color:",
        &style->osd_active_label_bg_color, parseColor },         // ob
{ "osd.active.label.bg:",
        &style->osd_active_label_bg, parseValue },               // ob
{ "osd.active.label.text.color:",
        &style->osd_active_label_text_color, parseColor },       // ob
{ "osd.bg.border.color:",
        &style->osd_bg_border_color, parseColor },               // ob
{ "osd.bg.color.splitto:",
        &style->osd_bg_color_splitto, parseColor },              // ob
{ "osd.bg.color:",
        &style->osd_bg_color, parseColor },                      // ob
{ "osd.bg.colorTo.splitto:",
        &style->osd_bg_colorTo_splitto, parseColor },            // ob
{ "osd.bg.colorTo:",
        &style->osd_bg_colorTo, parseColor },                    // ob
{ "osd.bg:",
        &style->osd_bg, parseValue },                            // ob
{ "osd.border.color:",
        &style->osd_border_color, parseColor },                  // ob
{ "osd.border.width:",
        &style->osd_border_width, parseInt },                    // ob
{ "osd.hilight.bg.color:",
        &style->osd_hilight_bg_color, parseColor },              // ob
{ "osd.hilight.bg.colorTo:",
        &style->osd_hilight_bg_colorTo, parseColor },            // ob
{ "osd.hilight.bg:",
        &style->osd_hilight_bg, parseValue },                    // ob
{ "osd.inactive.label.bg:",
        &style->osd_inactive_label_bg, parseValue },             // ob
{ "osd.inactive.label.text.color:",
        &style->osd_inactive_label_text_color, parseColor },     // ob
{ "osd.label.bg:",
        &style->osd_label_bg, parseValue },                      // ob
{ "osd.unhilight.bg.color:",
        &style->osd_unhilight_bg_color, parseColor },            // ob
{ "osd.unhilight.bg.colorTo:",
        &style->osd_unhilight_bg_colorTo, parseColor },          // ob
{ "osd.unhilight.bg:",
        &style->osd_unhilight_bg, parseValue },                  // ob
{ "padding.height:",
        &style->padding_height, parseInt },                      // ob
{ "padding.width:",
        &style->padding_width, parseInt },                       // ob
{ "rootCommand:",
        &style->rootCommand, parseValue },                       // bbwm
{ "slit.appearance:",
        &style->slit_appearance, parseValue },                   // bbwm
{ "slit.backgroundColor:",
        &style->slit_backgroundColor, parseColor },              // bbwm
{ "slit.bevelWidth:",
        &style->slit_bevelWidth, parseInt },                     // fb
{ "slit.borderColor:",
        &style->slit_borderColor, parseColor },                  // fb
{ "slit.borderWidth:",
        &style->slit_borderWidth, parseInt },                    // fb
{ "slit.color:",
        &style->slit_color, parseColor },                        // fb
{ "slit.colorTo:",
        &style->slit_colorTo, parseColor },                      // fb
{ "slit.marginWidth:",
        &style->slit_marginWidth, parseInt },                    // bbwm
{ "slit.pixmap:",
        &style->slit_pixmap, parseString },                      // fb
{ "slit:",
        &style->slit, parseValue },                              // fb
{ "style.author:",
        &style->style_author, parseValue },                      // fb
{ "style.comments:",
        &style->style_comments, parseValue },                    // fb
{ "style.credits:",
        &style->style_credits, parseValue },                     // fb
{ "style.date:",
        &style->style_date, parseValue },                        // fb
{ "style.name:",
        &style->style_name, parseValue },                        // fb
{ "titleFont:",
        &style->titleFont, parseString },                        // fb
{ "toolbar.alignment:",
        &style->toolbar_alignment, parseValue },                 // bbwm
{ "toolbar.alpha:",
        &style->toolbar_alpha, parseValue },                     // fb
{ "toolbar.appearance:",
        &style->toolbar_appearance, parseValue },                // bbwm
{ "toolbar.bevelWidth:",
        &style->toolbar_bevelWidth, parseInt },                  // fb
{ "toolbar.borderColor:",
        &style->toolbar_borderColor, parseColor },               // fb
{ "toolbar.borderWidth:",
        &style->toolbar_borderWidth, parseInt },                 // fb
{ "toolbar.button.color:",
        &style->toolbar_button_color, parseColor },              // bbwm fb
{ "toolbar.button.colorTo:",
        &style->toolbar_button_colorTo, parseColor },            // bbwm fb
{ "toolbar.button.picColor:",
        &style->toolbar_button_picColor, parseColor },           // bbwm fb
{ "toolbar.button.pixmap:",
        &style->toolbar_button_pixmap, parseString },            // fb
{ "toolbar.button.pressed.color:",
        &style->toolbar_button_pressed_color, parseColor },      // bbwm fb
{ "toolbar.button.pressed.colorTo:",
        &style->toolbar_button_pressed_colorTo, parseColor },    // bbwm fb
{ "toolbar.button.pressed.picColor:",
        &style->toolbar_button_pressed_picColor, parseColor },   // fb
{ "toolbar.button.pressed.pixmap:",
        &style->toolbar_button_pressed_pixmap, parseString },    // fb
{ "toolbar.button.pressed:",
        &style->toolbar_button_pressed, parseValue },            // bbwm fb
{ "toolbar.button:",
        &style->toolbar_button, parseValue },                    // bbwm fb
{ "toolbar.clock.appearance:",
        &style->toolbar_clock_appearance, parseValue },          // bbwm
{ "toolbar.clock.borderColor:",
        &style->toolbar_clock_borderColor, parseColor },         // fb
{ "toolbar.clock.borderWidth:",
        &style->toolbar_clock_borderWidth, parseInt },           // fb
{ "toolbar.clock.color:",
        &style->toolbar_clock_color, parseColor },               // bbwm fb
{ "toolbar.clock.colorTo:",
        &style->toolbar_clock_colorTo, parseColor },             // bbwm fb
{ "toolbar.clock.font:",
        &style->toolbar_clock_font, parseString },               // fb
{ "toolbar.clock.justify:",
        &style->toolbar_clock_justify, parseValue },             // fb
{ "toolbar.clock.pixmap:",
        &style->toolbar_clock_pixmap, parseString },             // fb
{ "toolbar.clock.textColor:",
        &style->toolbar_clock_textColor, parseColor },           // bbwm fb
{ "toolbar.clock:",
        &style->toolbar_clock, parseValue },                     // bbwm fb
{ "toolbar.color:",
        &style->toolbar_color, parseColor },                     // bbwm fb
{ "toolbar.colorTo:",
        &style->toolbar_colorTo, parseColor },                   // bbwm fb
{ "toolbar.font:",
        &style->toolbar_font, parseString },                     // bbwm fb
{ "toolbar.height:",
        &style->toolbar_height, parseInt },                      // fb
{ "toolbar.iconbar.borderColor:",
        &style->toolbar_iconbar_borderColor, parseColor },       // fb
{ "toolbar.iconbar.borderWidth:",
        &style->toolbar_iconbar_borderWidth, parseInt },         // fb
{ "toolbar.iconbar.empty.color:",
        &style->toolbar_iconbar_empty_color, parseColor },       // fb
{ "toolbar.iconbar.empty.colorTo:",
        &style->toolbar_iconbar_empty_colorTo, parseColor },     // fb
{ "toolbar.iconbar.empty.pixmap:",
        &style->toolbar_iconbar_empty_pixmap, parseString },     // fb
{ "toolbar.iconbar.empty:",
        &style->toolbar_iconbar_empty, parseValue },             // fb
{ "toolbar.iconbar.focused.borderColor:",
        &style->toolbar_iconbar_focused_borderColor, parseColor },// fb
{ "toolbar.iconbar.focused.borderWidth:",
        &style->toolbar_iconbar_focused_borderWidth, parseInt }, // fb
{ "toolbar.iconbar.focused.color:",
        &style->toolbar_iconbar_focused_color, parseColor },     // fb
{ "toolbar.iconbar.focused.colorTo:",
        &style->toolbar_iconbar_focused_colorTo, parseColor },   // fb
{ "toolbar.iconbar.focused.font.effect:",
        &style->toolbar_iconbar_focused_font_effect, parseString },// fb
{ "toolbar.iconbar.focused.font.shadow.color:",
        &style->toolbar_iconbar_focused_font_shadow_color, parseColor },// fb
{ "toolbar.iconbar.focused.font:",
        &style->toolbar_iconbar_focused_font, parseString },     // fb
{ "toolbar.iconbar.focused.justify:",
        &style->toolbar_iconbar_focused_justify, parseValue },   // fb
{ "toolbar.iconbar.focused.pixmap:",
        &style->toolbar_iconbar_focused_pixmap, parseString },   // fb
{ "toolbar.iconbar.focused.textColor:",
        &style->toolbar_iconbar_focused_textColor, parseColor }, // fb
{ "toolbar.iconbar.focused:",
        &style->toolbar_iconbar_focused, parseValue },           // fb
{ "toolbar.iconbar.unfocused.borderColor:",
        &style->toolbar_iconbar_unfocused_borderColor, parseColor },// fb
{ "toolbar.iconbar.unfocused.borderWidth:",
        &style->toolbar_iconbar_unfocused_borderWidth, parseInt },// fb
{ "toolbar.iconbar.unfocused.color:",
        &style->toolbar_iconbar_unfocused_color, parseColor },   // fb
{ "toolbar.iconbar.unfocused.colorTo:",
        &style->toolbar_iconbar_unfocused_colorTo, parseColor }, // fb
{ "toolbar.iconbar.unfocused.font:",
        &style->toolbar_iconbar_unfocused_font, parseString },   // fb
{ "toolbar.iconbar.unfocused.justify:",
        &style->toolbar_iconbar_unfocused_justify, parseValue }, // fb
{ "toolbar.iconbar.unfocused.pixmap:",
        &style->toolbar_iconbar_unfocused_pixmap, parseString }, // fb
{ "toolbar.iconbar.unfocused.textColor:",
        &style->toolbar_iconbar_unfocused_textColor, parseColor },// fb
{ "toolbar.iconbar.unfocused:",
        &style->toolbar_iconbar_unfocused, parseValue },         // fb
{ "toolbar.justify:",
        &style->toolbar_justify, parseValue },                   // bbwm fb
{ "toolbar.label.appearance:",
        &style->toolbar_label_appearance, parseValue },          // bbwm
{ "toolbar.label.color:",
        &style->toolbar_label_color, parseColor },               // bbwm fb
{ "toolbar.label.colorTo:",
        &style->toolbar_label_colorTo, parseColor },             // bbwm fb
{ "toolbar.label.pixmap:",
        &style->toolbar_label_pixmap, parseString },             // fb
{ "toolbar.label.textColor:",
        &style->toolbar_label_textColor, parseColor },           // bbwm fb
{ "toolbar.label:",
        &style->toolbar_label, parseValue },                     // bbwm fb
{ "toolbar.pixmap:",
        &style->toolbar_pixmap, parseString },                   // fb
{ "toolbar.shaped:",
        &style->toolbar_shaped, parseValue },                    // fb
{ "toolbar.textColor:",
        &style->toolbar_textColor, parseColor },                 // fb
{ "toolbar.windowLabel.appearance:",
        &style->toolbar_windowLabel_appearance, parseValue },    // bbwm
{ "toolbar.windowLabel.color:",
        &style->toolbar_windowLabel_color, parseColor },         // bbwm fb
{ "toolbar.windowLabel.colorTo:",
        &style->toolbar_windowLabel_colorTo, parseColor },       // bbwm fb
{ "toolbar.windowLabel.pixmap:",
        &style->toolbar_windowLabel_pixmap, parseString },       // fb
{ "toolbar.windowLabel.textColor:",
        &style->toolbar_windowLabel_textColor, parseColor },     // bbwm fb
{ "toolbar.windowLabel:",
        &style->toolbar_windowLabel, parseValue },               // bbwm fb
{ "toolbar.workspace.borderColor:",
        &style->toolbar_workspace_borderColor, parseColor },     // fb
{ "toolbar.workspace.borderWidth:",
        &style->toolbar_workspace_borderWidth, parseInt },       // fb
{ "toolbar.workspace.color:",
        &style->toolbar_workspace_color, parseColor },           // fb
{ "toolbar.workspace.colorTo:",
        &style->toolbar_workspace_colorTo, parseColor },         // fb
{ "toolbar.workspace.font:",
        &style->toolbar_workspace_font, parseString },           // fb
{ "toolbar.workspace.justify:",
        &style->toolbar_workspace_justify, parseValue },         // fb
{ "toolbar.workspace.pixmap:",
        &style->toolbar_workspace_pixmap, parseString },         // fb
{ "toolbar.workspace.textColor:",
        &style->toolbar_workspace_textColor, parseColor },       // fb
{ "toolbar.workspace:",
        &style->toolbar_workspace, parseValue },                 // fb
{ "toolbar:",
        &style->toolbar, parseValue },                           // bbwm fb
{ "window.active.border.color:",
        &style->window_active_border_color, parseColor },        // ob
{ "window.active.button.disabled.bg.border.color:",
        &style->window_active_button_disabled_bg_border_color, parseColor },// ob
{ "window.active.button.disabled.bg.color:",
        &style->window_active_button_disabled_bg_color, parseColor },// ob
{ "window.active.button.disabled.bg.colorTo:",
        &style->window_active_button_disabled_bg_colorTo, parseColor },// ob
{ "window.active.button.disabled.bg:",
        &style->window_active_button_disabled_bg, parseValue },  // ob
{ "window.active.button.disabled.image.color:",
        &style->window_active_button_disabled_image_color, parseColor },// ob
{ "window.active.button.hover.bg.border.color:",
        &style->window_active_button_hover_bg_border_color, parseColor },// ob
{ "window.active.button.hover.bg.color.splitTo:",
        &style->window_active_button_hover_bg_color_splitTo, parseColor },// ob
{ "window.active.button.hover.bg.color:",
        &style->window_active_button_hover_bg_color, parseColor },// ob
{ "window.active.button.hover.bg.colorTo.splitTo:",
        &style->window_active_button_hover_bg_colorTo_splitTo, parseColor },// ob
{ "window.active.button.hover.bg.colorTo:",
        &style->window_active_button_hover_bg_colorTo, parseColor },// ob
{ "window.active.button.hover.bg:",
        &style->window_active_button_hover_bg, parseValue },     // ob
{ "window.active.button.hover.image.color:",
        &style->window_active_button_hover_image_color, parseColor },// ob
{ "window.active.button.pressed.bg.border.color:",
        &style->window_active_button_pressed_bg_border_color, parseColor },// ob
{ "window.active.button.pressed.bg.color:",
        &style->window_active_button_pressed_bg_color, parseColor },// ob
{ "window.active.button.pressed.bg.colorTo:",
        &style->window_active_button_pressed_bg_colorTo, parseColor },// ob
{ "window.active.button.pressed.bg:",
        &style->window_active_button_pressed_bg, parseValue },   // ob
{ "window.active.button.pressed.image.color:",
        &style->window_active_button_pressed_image_color, parseColor },// ob
{ "window.active.button.toggled.bg.border.color:",
        &style->window_active_button_toggled_bg_border_color, parseColor },// ob
{ "window.active.button.toggled.bg.color:",
        &style->window_active_button_toggled_bg_color, parseColor },// ob
{ "window.active.button.toggled.bg.colorTo:",
        &style->window_active_button_toggled_bg_colorTo, parseColor },// ob
{ "window.active.button.toggled.bg:",
        &style->window_active_button_toggled_bg, parseValue },   // ob
{ "window.active.button.toggled.hover.bg.border.color:",
        &style->window_active_button_toggled_hover_bg_border_color, parseColor },// ob
{ "window.active.button.toggled.hover.bg.color:",
        &style->window_active_button_toggled_hover_bg_color, parseColor },// ob
{ "window.active.button.toggled.hover.bg.colorTo:",
        &style->window_active_button_toggled_hover_bg_colorTo, parseColor },// ob
{ "window.active.button.toggled.hover.bg:",
        &style->window_active_button_toggled_hover_bg, parseValue },// ob
{ "window.active.button.toggled.hover.image.color:",
        &style->window_active_button_toggled_hover_image_color, parseColor },// ob
{ "window.active.button.toggled.image.color:",
        &style->window_active_button_toggled_image_color, parseColor },// ob
{ "window.active.button.unpressed.bg.border.color:",
        &style->window_active_button_unpressed_bg_border_color, parseColor },// ob
{ "window.active.button.unpressed.bg.color:",
        &style->window_active_button_unpressed_bg_color, parseColor },// ob
{ "window.active.button.unpressed.bg.colorTo:",
        &style->window_active_button_unpressed_bg_colorTo, parseColor },// ob
{ "window.active.button.unpressed.bg:",
        &style->window_active_button_unpressed_bg, parseValue }, // ob
{ "window.active.button.unpressed.image.color:",
        &style->window_active_button_unpressed_image_color, parseColor },// ob
{ "window.active.client.color:",
        &style->window_active_client_color, parseColor },        // ob
{ "window.active.grip.bg.border.color:",
        &style->window_active_grip_bg_border_color, parseColor },// ob
{ "window.active.grip.bg.color:",
        &style->window_active_grip_bg_color, parseColor },       // ob
{ "window.active.grip.bg.colorTo:",
        &style->window_active_grip_bg_colorTo, parseColor },     // ob
{ "window.active.grip.bg:",
        &style->window_active_grip_bg, parseValue },             // ob
{ "window.active.handle.bg.border.color:",
        &style->window_active_handle_bg_border_color, parseColor },// ob
{ "window.active.handle.bg.color:",
        &style->window_active_handle_bg_color, parseColor },     // ob
{ "window.active.handle.bg.colorTo:",
        &style->window_active_handle_bg_colorTo, parseColor },   // ob
{ "window.active.handle.bg:",
        &style->window_active_handle_bg, parseValue },           // ob
{ "window.active.label.bg.border.color:",
        &style->window_active_label_bg_border_color, parseColor },// ob
{ "window.active.label.bg.color:",
        &style->window_active_label_bg_color, parseColor },      // ob
{ "window.active.label.bg.colorTo:",
        &style->window_active_label_bg_colorTo, parseColor },    // ob
{ "window.active.label.bg:",
        &style->window_active_label_bg, parseValue },            // ob
{ "window.active.label.text.color:",
        &style->window_active_label_text_color, parseColor },    // ob
{ "window.active.label.text.font:",
        &style->window_active_label_text_font, parseString },    // ob
{ "window.active.padding.width:",
        &style->window_active_padding_width, parseInt },         // ob
{ "window.active.title.bg.border.color:",
        &style->window_active_title_bg_border_color, parseColor },// ob
{ "window.active.title.bg.color:",
        &style->window_active_title_bg_color, parseColor },      // ob
{ "window.active.title.bg.colorTo:",
        &style->window_active_title_bg_colorTo, parseColor },    // ob
{ "window.active.title.bg.highlight:",
        &style->window_active_title_bg_highlight, parseValue },  // ob
{ "window.active.title.bg.shadow:",
        &style->window_active_title_bg_shadow, parseValue },     // ob
{ "window.active.title.bg:",
        &style->window_active_title_bg, parseValue },            // ob
{ "window.active.title.separator.color:",
        &style->window_active_title_separator_color, parseColor },// ob
{ "window.alignment:",
        &style->window_alignment, parseValue },                  // bbwm
{ "window.alpha:",
        &style->window_alpha, parseValue },                      // fb
{ "window.bevelWidth:",
        &style->window_bevelWidth, parseInt },                   // fb
{ "window.borderColor:",
        &style->window_borderColor, parseColor },                // fb
{ "window.borderWidth:",
        &style->window_borderWidth, parseInt },                  // fb
{ "window.button.focus.appearance:",
        &style->window_button_focus_appearance, parseValue },    // bbwm
{ "window.button.focus.color:",
        &style->window_button_focus_color, parseColor },         // bbwm fb
{ "window.button.focus.colorTo:",
        &style->window_button_focus_colorTo, parseColor },       // bbwm fb
{ "window.button.focus.foregroundColor:",
        &style->window_button_focus_foregroundColor, parseColor },// bbwm
{ "window.button.focus.picColor:",
        &style->window_button_focus_picColor, parseColor },      // bbwm fb
{ "window.button.focus:",
        &style->window_button_focus, parseValue },               // bbwm fb
{ "window.button.pressed.color:",
        &style->window_button_pressed_color, parseColor },       // bbwm fb
{ "window.button.pressed.colorTo:",
        &style->window_button_pressed_colorTo, parseColor },     // bbwm fb
{ "window.button.pressed.picColor:",
        &style->window_button_pressed_picColor, parseColor },    // fb
{ "window.button.pressed:",
        &style->window_button_pressed, parseValue },             // bbwm fb
{ "window.button.unfocus.Color:",
        &style->window_button_unfocus_Color, parseColor },       // fb
{ "window.button.unfocus.ColorTo:",
        &style->window_button_unfocus_ColorTo, parseColor },     // fb
{ "window.button.unfocus.appearance:",
        &style->window_button_unfocus_appearance, parseValue },  // bbwm
{ "window.button.unfocus.color:",
        &style->window_button_unfocus_color, parseColor },       // bbwm fb
{ "window.button.unfocus.colorTo:",
        &style->window_button_unfocus_colorTo, parseColor },     // bbwm fb
{ "window.button.unfocus.foregroundColor:",
        &style->window_button_unfocus_foregroundColor, parseColor },// bbwm
{ "window.button.unfocus.picColor:",
        &style->window_button_unfocus_picColor, parseColor },    // bbwm fb
{ "window.button.unfocus:",
        &style->window_button_unfocus, parseValue },             // bbwm fb
{ "window.client.padding.height:",
        &style->window_client_padding_height, parseInt },        // ob
{ "window.client.padding.width:",
        &style->window_client_padding_width, parseInt },         // ob
{ "window.close.pixmap:",
        &style->window_close_pixmap, parseString },              // fb
{ "window.close.pressed.pixmap:",
        &style->window_close_pressed_pixmap, parseString },      // fb
{ "window.close.unfocus.pixmap:",
        &style->window_close_unfocus_pixmap, parseString },      // fb
{ "window.font.effect:",
        &style->window_font_effect, parseString },               // fb
{ "window.font.shadow.color:",
        &style->window_font_shadow_color, parseColor },          // fb
{ "window.font:",
        &style->window_font, parseString },                      // bbwm fb
{ "window.frame.focus.color:",
        &style->window_frame_focus_color, parseColor },          // fb
{ "window.frame.focus:",
        &style->window_frame_focus, parseValue },                // fb
{ "window.frame.focusColor:",
        &style->window_frame_focusColor, parseColor },           // bbwm fb
{ "window.frame.unfocus.color:",
        &style->window_frame_unfocus_color, parseColor },        // fb
{ "window.frame.unfocus:",
        &style->window_frame_unfocus, parseValue },              // fb
{ "window.frame.unfocusColor:",
        &style->window_frame_unfocusColor, parseColor },         // bbwm fb
{ "window.frameColor:",
        &style->window_frameColor, parseColor },                 // ob
{ "window.frameWidth:",
        &style->window_frameWidth, parseInt },                   // bbwm
{ "window.grip.focus.appearance:",
        &style->window_grip_focus_appearance, parseValue },      // bbwm
{ "window.grip.focus.backgroundColor:",
        &style->window_grip_focus_backgroundColor, parseColor }, // bbwm
{ "window.grip.focus.color:",
        &style->window_grip_focus_color, parseColor },           // bbwm fb
{ "window.grip.focus.colorTo:",
        &style->window_grip_focus_colorTo, parseColor },         // bbwm fb
{ "window.grip.focus.pixmap:",
        &style->window_grip_focus_pixmap, parseString },         // fb
{ "window.grip.focus:",
        &style->window_grip_focus, parseValue },                 // bbwm fb
{ "window.grip.unfocus.appearance:",
        &style->window_grip_unfocus_appearance, parseValue },    // bbwm
{ "window.grip.unfocus.color:",
        &style->window_grip_unfocus_color, parseColor },         // bbwm fb
{ "window.grip.unfocus.colorTo:",
        &style->window_grip_unfocus_colorTo, parseColor },       // bbwm fb
{ "window.grip.unfocus.pixmap:",
        &style->window_grip_unfocus_pixmap, parseString },       // fb
{ "window.grip.unfocus:",
        &style->window_grip_unfocus, parseValue },               // bbwm fb
{ "window.handle.focus.appearance:",
        &style->window_handle_focus_appearance, parseValue },    // bbwm
{ "window.handle.focus.color:",
        &style->window_handle_focus_color, parseColor },         // bbwm fb
{ "window.handle.focus.colorTo:",
        &style->window_handle_focus_colorTo, parseColor },       // bbwm fb
{ "window.handle.focus.pixmap:",
        &style->window_handle_focus_pixmap, parseString },       // fb
{ "window.handle.focus:",
        &style->window_handle_focus, parseValue },               // bbwm fb
{ "window.handle.unfocus.appearance:",
        &style->window_handle_unfocus_appearance, parseValue },  // bbwm
{ "window.handle.unfocus.color:",
        &style->window_handle_unfocus_color, parseColor },       // bbwm fb
{ "window.handle.unfocus.colorTo:",
        &style->window_handle_unfocus_colorTo, parseColor },     // bbwm fb
{ "window.handle.unfocus.pixmap:",
        &style->window_handle_unfocus_pixmap, parseString },     // fb
{ "window.handle.unfocus:",
        &style->window_handle_unfocus, parseValue },             // bbwm fb
{ "window.handle.width:",
        &style->window_handle_width, parseInt },                 // ob
{ "window.handleHeight:",
        &style->window_handleHeight, parseInt },                 // bbwm
{ "window.handleWidth:",
        &style->window_handleWidth, parseInt },                  // fb
{ "window.iconify.pixmap:",
        &style->window_iconify_pixmap, parseString },            // fb
{ "window.iconify.pressed.pixmap:",
        &style->window_iconify_pressed_pixmap, parseString },    // fb
{ "window.iconify.unfocus.pixmap:",
        &style->window_iconify_unfocus_pixmap, parseString },    // fb
{ "window.inactive.border.color:",
        &style->window_inactive_border_color, parseColor },      // ob
{ "window.inactive.button.disabled.bg.border.color:",
        &style->window_inactive_button_disabled_bg_border_color, parseColor },// ob
{ "window.inactive.button.disabled.bg.color:",
        &style->window_inactive_button_disabled_bg_color, parseColor },// ob
{ "window.inactive.button.disabled.bg.colorTo:",
        &style->window_inactive_button_disabled_bg_colorTo, parseColor },// ob
{ "window.inactive.button.disabled.bg:",
        &style->window_inactive_button_disabled_bg, parseValue },// ob
{ "window.inactive.button.disabled.image.color:",
        &style->window_inactive_button_disabled_image_color, parseColor },// ob
{ "window.inactive.button.hover.bg.border.color:",
        &style->window_inactive_button_hover_bg_border_color, parseColor },// ob
{ "window.inactive.button.hover.bg.color:",
        &style->window_inactive_button_hover_bg_color, parseColor },// ob
{ "window.inactive.button.hover.bg.colorTo:",
        &style->window_inactive_button_hover_bg_colorTo, parseColor },// ob
{ "window.inactive.button.hover.bg:",
        &style->window_inactive_button_hover_bg, parseValue },   // ob
{ "window.inactive.button.hover.image.color:",
        &style->window_inactive_button_hover_image_color, parseColor },// ob
{ "window.inactive.button.pressed.bg.border.color:",
        &style->window_inactive_button_pressed_bg_border_color, parseColor },// ob
{ "window.inactive.button.pressed.bg.color:",
        &style->window_inactive_button_pressed_bg_color, parseColor },// ob
{ "window.inactive.button.pressed.bg.colorTo:",
        &style->window_inactive_button_pressed_bg_colorTo, parseColor },// ob
{ "window.inactive.button.pressed.bg:",
        &style->window_inactive_button_pressed_bg, parseValue }, // ob
{ "window.inactive.button.pressed.image.color:",
        &style->window_inactive_button_pressed_image_color, parseColor },// ob
{ "window.inactive.button.toggled.bg.border.color:",
        &style->window_inactive_button_toggled_bg_border_color, parseColor },// ob
{ "window.inactive.button.toggled.bg.color:",
        &style->window_inactive_button_toggled_bg_color, parseColor },// ob
{ "window.inactive.button.toggled.bg.colorTo:",
        &style->window_inactive_button_toggled_bg_colorTo, parseColor },// ob
{ "window.inactive.button.toggled.bg:",
        &style->window_inactive_button_toggled_bg, parseValue }, // ob
{ "window.inactive.button.toggled.hover.bg.border.color:",
        &style->window_inactive_button_toggled_hover_bg_border_color, parseColor },// ob
{ "window.inactive.button.toggled.hover.bg.color:",
        &style->window_inactive_button_toggled_hover_bg_color, parseColor },// ob
{ "window.inactive.button.toggled.hover.bg.colorTo:",
        &style->window_inactive_button_toggled_hover_bg_colorTo, parseColor },// ob
{ "window.inactive.button.toggled.hover.bg:",
        &style->window_inactive_button_toggled_hover_bg, parseValue },// ob
{ "window.inactive.button.toggled.hover.image.color:",
        &style->window_inactive_button_toggled_hover_image_color, parseColor },// ob
{ "window.inactive.button.toggled.image.color:",
        &style->window_inactive_button_toggled_image_color, parseColor },// ob
{ "window.inactive.button.unpressed.bg.border.color:",
        &style->window_inactive_button_unpressed_bg_border_color, parseColor },// ob
{ "window.inactive.button.unpressed.bg.color:",
        &style->window_inactive_button_unpressed_bg_color, parseColor },// ob
{ "window.inactive.button.unpressed.bg.colorTo:",
        &style->window_inactive_button_unpressed_bg_colorTo, parseColor },// ob
{ "window.inactive.button.unpressed.bg:",
        &style->window_inactive_button_unpressed_bg, parseValue },// ob
{ "window.inactive.button.unpressed.image.color:",
        &style->window_inactive_button_unpressed_image_color, parseColor },// ob
{ "window.inactive.client.color:",
        &style->window_inactive_client_color, parseColor },      // ob
{ "window.inactive.grip.bg.border.color:",
        &style->window_inactive_grip_bg_border_color, parseColor },// ob
{ "window.inactive.grip.bg.color:",
        &style->window_inactive_grip_bg_color, parseColor },     // ob
{ "window.inactive.grip.bg.colorTo:",
        &style->window_inactive_grip_bg_colorTo, parseColor },   // ob
{ "window.inactive.grip.bg:",
        &style->window_inactive_grip_bg, parseValue },           // ob
{ "window.inactive.handle.bg.border.color:",
        &style->window_inactive_handle_bg_border_color, parseColor },// ob
{ "window.inactive.handle.bg.color:",
        &style->window_inactive_handle_bg_color, parseColor },   // ob
{ "window.inactive.handle.bg.colorTo:",
        &style->window_inactive_handle_bg_colorTo, parseColor }, // ob
{ "window.inactive.handle.bg:",
        &style->window_inactive_handle_bg, parseValue },         // ob
{ "window.inactive.label.bg.border.color:",
        &style->window_inactive_label_bg_border_color, parseColor },// ob
{ "window.inactive.label.bg.color:",
        &style->window_inactive_label_bg_color, parseColor },    // ob
{ "window.inactive.label.bg.colorTo:",
        &style->window_inactive_label_bg_colorTo, parseColor },  // ob
{ "window.inactive.label.bg:",
        &style->window_inactive_label_bg, parseValue },          // ob
{ "window.inactive.label.text.color:",
        &style->window_inactive_label_text_color, parseColor },  // ob
{ "window.inactive.label.text.font:",
        &style->window_inactive_label_text_font, parseString },  // ob
{ "window.inactive.title.bg.border.color:",
        &style->window_inactive_title_bg_border_color, parseColor },// ob
{ "window.inactive.title.bg.color.splitTo:",
        &style->window_inactive_title_bg_color_splitTo, parseColor },// ob
{ "window.inactive.title.bg.color:",
        &style->window_inactive_title_bg_color, parseColor },    // ob
{ "window.inactive.title.bg.colorTo.splitTo:",
        &style->window_inactive_title_bg_colorTo_splitTo, parseColor },// ob
{ "window.inactive.title.bg.colorTo:",
        &style->window_inactive_title_bg_colorTo, parseColor },  // ob
{ "window.inactive.title.bg.highlight:",
        &style->window_inactive_title_bg_highlight, parseValue },// ob
{ "window.inactive.title.bg.shadow:",
        &style->window_inactive_title_bg_shadow, parseValue },   // ob
{ "window.inactive.title.bg:",
        &style->window_inactive_title_bg, parseValue },          // ob
{ "window.inactive.title.separator.color:",
        &style->window_inactive_title_separator_color, parseColor },// ob
{ "window.justify:",
        &style->window_justify, parseValue },                    // bbwm fb
{ "window.label.focus.appearance:",
        &style->window_label_focus_appearance, parseValue },     // bbwm
{ "window.label.focus.color:",
        &style->window_label_focus_color, parseColor },          // bbwm fb
{ "window.label.focus.colorTo:",
        &style->window_label_focus_colorTo, parseColor },        // bbwm fb
{ "window.label.focus.font:",
        &style->window_label_focus_font, parseString },          // fb
{ "window.label.focus.justify:",
        &style->window_label_focus_justify, parseValue },        // fb
{ "window.label.focus.pixmap:",
        &style->window_label_focus_pixmap, parseString },        // fb
{ "window.label.focus.textColor:",
        &style->window_label_focus_textColor, parseColor },      // bbwm fb
{ "window.label.focus:",
        &style->window_label_focus, parseValue },                // bbwm fb
{ "window.label.marginWidth:",
        &style->window_label_marginWidth, parseInt },            // bbwm
{ "window.label.pixmap:",
        &style->window_label_pixmap, parseString },              // fb
{ "window.label.text.justify:",
        &style->window_label_text_justify, parseValue },         // ob
{ "window.label.unfocus.appearance:",
        &style->window_label_unfocus_appearance, parseValue },   // bbwm
{ "window.label.unfocus.color:",
        &style->window_label_unfocus_color, parseColor },        // bbwm fb
{ "window.label.unfocus.colorTo:",
        &style->window_label_unfocus_colorTo, parseColor },      // bbwm fb
{ "window.label.unfocus.pixmap:",
        &style->window_label_unfocus_pixmap, parseString },      // fb
{ "window.label.unfocus.textColor:",
        &style->window_label_unfocus_textColor, parseColor },    // bbwm fb
{ "window.label.unfocus:",
        &style->window_label_unfocus, parseValue },              // bbwm fb
{ "window.maximize.pixmap:",
        &style->window_maximize_pixmap, parseString },           // fb
{ "window.maximize.pressed.pixmap:",
        &style->window_maximize_pressed_pixmap, parseString },   // fb
{ "window.maximize.unfocus.pixmap:",
        &style->window_maximize_unfocus_pixmap, parseString },   // fb
{ "window.menuicon.pixmap:",
        &style->window_menuicon_pixmap, parseString },           // fb
{ "window.menuicon.pressed.pixmap:",
        &style->window_menuicon_pressed_pixmap, parseString },   // fb
{ "window.menuicon.unfocus.pixmap:",
        &style->window_menuicon_unfocus_pixmap, parseString },   // fb
{ "window.roundCorners:",
        &style->window_roundCorners, parseValue },               // fb
{ "window.shade.pixmap:",
        &style->window_shade_pixmap, parseString },              // fb
{ "window.shade.pressed.pixmap:",
        &style->window_shade_pressed_pixmap, parseString },      // fb
{ "window.shade.unfocus.pixmap:",
        &style->window_shade_unfocus_pixmap, parseString },      // fb
{ "window.shade:",
        &style->window_shade, parseValue },                      // fb
{ "window.stick.pixmap:",
        &style->window_stick_pixmap, parseString },              // fb
{ "window.stick.pressed.pixmap:",
        &style->window_stick_pressed_pixmap, parseString },      // fb
{ "window.stick.unfocus.pixmap:",
        &style->window_stick_unfocus_pixmap, parseString },      // fb
{ "window.stuck.pixmap:",
        &style->window_stuck_pixmap, parseString },              // fb
{ "window.stuck.pressed.pixmap:",
        &style->window_stuck_pressed_pixmap, parseString },      // fb
{ "window.stuck.unfocus.pixmap:",
        &style->window_stuck_unfocus_pixmap, parseString },      // fb
{ "window.tab.borderColor:",
        &style->window_tab_borderColor, parseColor },            // fb
{ "window.tab.borderWidth:",
        &style->window_tab_borderWidth, parseInt },              // fb
{ "window.tab.font:",
        &style->window_tab_font, parseString },                  // fb
{ "window.tab.justify:",
        &style->window_tab_justify, parseValue },                // fb
{ "window.tab.label.focus.color:",
        &style->window_tab_label_focus_color, parseColor },      // fb
{ "window.tab.label.focus.colorTo:",
        &style->window_tab_label_focus_colorTo, parseColor },    // fb
{ "window.tab.label.focus.textColor:",
        &style->window_tab_label_focus_textColor, parseColor },  // fb
{ "window.tab.label.focus:",
        &style->window_tab_label_focus, parseValue },            // fb
{ "window.tab.label.unfocus.color:",
        &style->window_tab_label_unfocus_color, parseColor },    // fb
{ "window.tab.label.unfocus.colorTo:",
        &style->window_tab_label_unfocus_colorTo, parseColor },  // fb
{ "window.tab.label.unfocus.textColor:",
        &style->window_tab_label_unfocus_textColor, parseColor },// fb
{ "window.tab.label.unfocus:",
        &style->window_tab_label_unfocus, parseValue },          // fb
{ "window.title.focus.appearance:",
        &style->window_title_focus_appearance, parseValue },     // bbwm
{ "window.title.focus.color1:",
        &style->window_title_focus_color1, parseColor },         // bbwm
{ "window.title.focus.color2:",
        &style->window_title_focus_color2, parseColor },         // bbwm
{ "window.title.focus.color:",
        &style->window_title_focus_color, parseColor },          // bbwm fb
{ "window.title.focus.colorTo:",
        &style->window_title_focus_colorTo, parseColor },        // bbwm fb
{ "window.title.focus.pixmap:",
        &style->window_title_focus_pixmap, parseString },        // fb
{ "window.title.focus:",
        &style->window_title_focus, parseValue },                // bbwm fb
{ "window.title.height:",
        &style->window_title_height, parseInt },                 // fb
{ "window.title.unfocus.appearance:",
        &style->window_title_unfocus_appearance, parseValue },   // bbwm
{ "window.title.unfocus.color:",
        &style->window_title_unfocus_color, parseColor },        // bbwm fb
{ "window.title.unfocus.colorTo:",
        &style->window_title_unfocus_colorTo, parseColor },      // bbwm fb
{ "window.title.unfocus.pixmap:",
        &style->window_title_unfocus_pixmap, parseString },      // fb
{ "window.title.unfocus:",
        &style->window_title_unfocus, parseValue },              // bbwm fb
{ "window.unshade.pixmap:",
        &style->window_unshade_pixmap, parseString },            // fb
{ "window.unshade.pressed.pixmap:",
        &style->window_unshade_pressed_pixmap, parseString },    // fb
{ "window.unshade.unfocus.pixmap:",
        &style->window_unshade_unfocus_pixmap, parseString },    // fb
