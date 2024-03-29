{ "background.color:", &style->background_color, parseColor },         // fb
    { "background.colorTo:", &style->background_colorTo, parseColor }, // fb
    { "background.modX:", &style->background_modX, parseValue },       // fb
    { "background.modY:", &style->background_modY, parseValue },       // fb
    { "background:", &style->background, parseValue },                 // fb
    { "bbpager.desktop.focus.color:",
      &style->bbpager_desktop_focus_color,
      parseColor }, // fb
    { "bbpager.desktop.focus.colorTo:",
      &style->bbpager_desktop_focus_colorTo,
      parseColor }, // fb
    {
      "bbpager.desktop.focus:", &style->bbpager_desktop_focus, parseValue
    },                                                   // fb
    { "bevelWidth:", &style->bevelWidth, parseInt },     // bbwm fb
    { "borderColor:", &style->borderColor, parseColor }, // bbwm fb
    { "borderWidth:", &style->borderWidth, parseInt },   // bbwm fb
    { "frameWidth:", &style->frameWidth, parseInt },     // bbwm fb
    { "handleWidth:", &style->handleWidth, parseInt },   // bbwm fb
    {
      "menu.active.appearance:", &style->menu_active_appearance, parseValue
    }, // bbwm
    { "menu.active.backgroundColor:",
      &style->menu_active_backgroundColor,
      parseColor }, // bbwm
    {
      "menu.active.borderColor:", &style->menu_active_borderColor, parseColor
    }, // bbwm
    {
      "menu.active.borderWidth:", &style->menu_active_borderWidth, parseInt
    },                                                                   // bbwm
    { "menu.active.color:", &style->menu_active_color, parseColor },     // bbwm
    { "menu.active.colorTo:", &style->menu_active_colorTo, parseColor }, // bbwm
    { "menu.active.foregroundColor:",
      &style->menu_active_foregroundColor,
      parseColor }, // bbwm
    {
      "menu.active.textColor:", &style->menu_active_textColor, parseColor
    },                                                             // bbwm
    { "menu.active:", &style->menu_active, parseValue },           // bbwm
    { "menu.bevelWidth:", &style->menu_bevelWidth, parseInt },     // fb
    { "menu.borderColor:", &style->menu_borderColor, parseColor }, // fb
    { "menu.borderWidth:", &style->menu_borderWidth, parseInt },   // fb
    { "menu.borderwidth:", &style->menu_borderwidth, parseInt },   // fb
    {
      "menu.bullet.position:", &style->menu_bullet_position, parseValue
    },                                                   // bbwm fb
    { "menu.bullet:", &style->menu_bullet, parseValue }, // bbwm fb
    {
      "menu.frame.alignment:", &style->menu_frame_alignment, parseValue
    }, // bbwm
    {
      "menu.frame.appearance:", &style->menu_frame_appearance, parseValue
    }, // bbwm
    {
      "menu.frame.borderColor:", &style->menu_frame_borderColor, parseColor
    }, // bbwm
    {
      "menu.frame.borderWidth:", &style->menu_frame_borderWidth, parseInt
    },                                                             // bbwm
    { "menu.frame.color:", &style->menu_frame_color, parseColor }, // bbwm fb
    {
      "menu.frame.colorTo:", &style->menu_frame_colorTo, parseColor
    }, // bbwm fb
    {
      "menu.frame.disableColor:", &style->menu_frame_disableColor, parseColor
    }, // bbwm fb
    {
      "menu.frame.disabledColor:", &style->menu_frame_disabledColor, parseColor
    },                                                            // bbwm
    { "menu.frame.font:", &style->menu_frame_font, parseString }, // bbwm fb
    { "menu.frame.foregroundColor:",
      &style->menu_frame_foregroundColor,
      parseColor }, // bbwm
    {
      "menu.frame.justify:", &style->menu_frame_justify, parseValue
    },                                                                // bbwm fb
    { "menu.frame.pixmap:", &style->menu_frame_pixmap, parseString }, // fb
    {
      "menu.frame.textColor:", &style->menu_frame_textColor, parseColor
    }, // bbwm fb
    { "menu.frame.underlineColor:",
      &style->menu_frame_underlineColor,
      parseColor },                                                  // fb
    { "menu.frame:", &style->menu_frame, parseValue },               // bbwm fb
    { "menu.hilite.color:", &style->menu_hilite_color, parseColor }, // bbwm fb
    {
      "menu.hilite.colorTo:", &style->menu_hilite_colorTo, parseColor
    }, // bbwm fb
    { "menu.hilite.pixmap:", &style->menu_hilite_pixmap, parseString }, // fb
    { "menu.hilite.selected.pixmap:",
      &style->menu_hilite_selected_pixmap,
      parseString }, // fb
    { "menu.hilite.submenu.pixmap:",
      &style->menu_hilite_submenu_pixmap,
      parseString }, // fb
    {
      "menu.hilite.textColor:", &style->menu_hilite_textColor, parseColor
    }, // bbwm fb
    { "menu.hilite.unselected.pixmap:",
      &style->menu_hilite_unselected_pixmap,
      parseString },                                     // fb
    { "menu.hilite:", &style->menu_hilite, parseValue }, // bbwm fb
    {
      "menu.selected.pixmap:", &style->menu_selected_pixmap, parseString
    },                                                                    // fb
    { "menu.submenu.pixmap:", &style->menu_submenu_pixmap, parseString }, // fb
    {
      "menu.title.alignment:", &style->menu_title_alignment, parseValue
    }, // bbwm
    {
      "menu.title.appearance:", &style->menu_title_appearance, parseValue
    }, // bbwm
    { "menu.title.backgroundColor:",
      &style->menu_title_backgroundColor,
      parseColor }, // bbwm
    {
      "menu.title.borderColor:", &style->menu_title_borderColor, parseColor
    }, // bbwm
    {
      "menu.title.borderWidth:", &style->menu_title_borderWidth, parseInt
    },                                                             // bbwm
    { "menu.title.color:", &style->menu_title_color, parseColor }, // bbwm fb
    {
      "menu.title.colorTo:", &style->menu_title_colorTo, parseColor
    }, // bbwm fb
    {
      "menu.title.font.effect:", &style->menu_title_font_effect, parseString
    }, // fb
    { "menu.title.font.shadow.color:",
      &style->menu_title_font_shadow_color,
      parseColor },                                               // fb
    { "menu.title.font:", &style->menu_title_font, parseString }, // bbwm fb
    { "menu.title.foregroundColor:",
      &style->menu_title_foregroundColor,
      parseColor }, // bbwm
    {
      "menu.title.justify:", &style->menu_title_justify, parseValue
    },                                                                // bbwm fb
    { "menu.title.pixmap:", &style->menu_title_pixmap, parseString }, // fb
    {
      "menu.title.textColor:", &style->menu_title_textColor, parseColor
    },                                                 // bbwm fb
    { "menu.title:", &style->menu_title, parseValue }, // bbwm fb
    {
      "menu.unselected.pixmap:", &style->menu_unselected_pixmap, parseString
    },                                                           // fb
    { "menuFont:", &style->menuFont, parseString },              // fb
    { "rootCommand:", &style->rootCommand, parseValue },         // bbwm
    { "slit.appearance:", &style->slit_appearance, parseValue }, // bbwm
    {
      "slit.backgroundColor:", &style->slit_backgroundColor, parseColor
    },                                                                   // bbwm
    { "slit.bevelWidth:", &style->slit_bevelWidth, parseInt },           // fb
    { "slit.borderColor:", &style->slit_borderColor, parseColor },       // fb
    { "slit.borderWidth:", &style->slit_borderWidth, parseInt },         // fb
    { "slit.color:", &style->slit_color, parseColor },                   // fb
    { "slit.colorTo:", &style->slit_colorTo, parseColor },               // fb
    { "slit.marginWidth:", &style->slit_marginWidth, parseInt },         // bbwm
    { "slit.pixmap:", &style->slit_pixmap, parseString },                // fb
    { "slit:", &style->slit, parseValue },                               // fb
    { "style.author:", &style->style_author, parseValue },               // fb
    { "style.comments:", &style->style_comments, parseValue },           // fb
    { "style.credits:", &style->style_credits, parseValue },             // fb
    { "style.date:", &style->style_date, parseValue },                   // fb
    { "style.name:", &style->style_name, parseValue },                   // fb
    { "titleFont:", &style->titleFont, parseString },                    // fb
    { "toolbar.alignment:", &style->toolbar_alignment, parseValue },     // bbwm
    { "toolbar.alpha:", &style->toolbar_alpha, parseValue },             // fb
    { "toolbar.appearance:", &style->toolbar_appearance, parseValue },   // bbwm
    { "toolbar.bevelWidth:", &style->toolbar_bevelWidth, parseInt },     // fb
    { "toolbar.borderColor:", &style->toolbar_borderColor, parseColor }, // fb
    { "toolbar.borderWidth:", &style->toolbar_borderWidth, parseInt },   // fb
    {
      "toolbar.button.color:", &style->toolbar_button_color, parseColor
    }, // bbwm fb
    {
      "toolbar.button.colorTo:", &style->toolbar_button_colorTo, parseColor
    }, // bbwm fb
    {
      "toolbar.button.picColor:", &style->toolbar_button_picColor, parseColor
    }, // bbwm fb
    {
      "toolbar.button.pixmap:", &style->toolbar_button_pixmap, parseString
    }, // fb
    { "toolbar.button.pressed.color:",
      &style->toolbar_button_pressed_color,
      parseColor }, // bbwm fb
    { "toolbar.button.pressed.colorTo:",
      &style->toolbar_button_pressed_colorTo,
      parseColor }, // bbwm fb
    { "toolbar.button.pressed.picColor:",
      &style->toolbar_button_pressed_picColor,
      parseColor }, // fb
    { "toolbar.button.pressed.pixmap:",
      &style->toolbar_button_pressed_pixmap,
      parseString }, // fb
    {
      "toolbar.button.pressed:", &style->toolbar_button_pressed, parseValue
    },                                                         // bbwm fb
    { "toolbar.button:", &style->toolbar_button, parseValue }, // bbwm fb
    {
      "toolbar.clock.appearance:", &style->toolbar_clock_appearance, parseValue
    }, // bbwm
    { "toolbar.clock.borderColor:",
      &style->toolbar_clock_borderColor,
      parseColor }, // fb
    {
      "toolbar.clock.borderWidth:", &style->toolbar_clock_borderWidth, parseInt
    }, // fb
    {
      "toolbar.clock.color:", &style->toolbar_clock_color, parseColor
    }, // bbwm fb
    {
      "toolbar.clock.colorTo:", &style->toolbar_clock_colorTo, parseColor
    }, // bbwm fb
    { "toolbar.clock.font:", &style->toolbar_clock_font, parseString }, // fb
    {
      "toolbar.clock.justify:", &style->toolbar_clock_justify, parseValue
    }, // fb
    {
      "toolbar.clock.pixmap:", &style->toolbar_clock_pixmap, parseString
    }, // fb
    {
      "toolbar.clock.textColor:", &style->toolbar_clock_textColor, parseColor
    },                                                           // bbwm fb
    { "toolbar.clock:", &style->toolbar_clock, parseValue },     // bbwm fb
    { "toolbar.color:", &style->toolbar_color, parseColor },     // bbwm fb
    { "toolbar.colorTo:", &style->toolbar_colorTo, parseColor }, // bbwm fb
    { "toolbar.font:", &style->toolbar_font, parseString },      // bbwm fb
    { "toolbar.height:", &style->toolbar_height, parseInt },     // fb
    { "toolbar.iconbar.borderColor:",
      &style->toolbar_iconbar_borderColor,
      parseColor }, // fb
    { "toolbar.iconbar.borderWidth:",
      &style->toolbar_iconbar_borderWidth,
      parseInt }, // fb
    { "toolbar.iconbar.empty.color:",
      &style->toolbar_iconbar_empty_color,
      parseColor }, // fb
    { "toolbar.iconbar.empty.colorTo:",
      &style->toolbar_iconbar_empty_colorTo,
      parseColor }, // fb
    { "toolbar.iconbar.empty.pixmap:",
      &style->toolbar_iconbar_empty_pixmap,
      parseString }, // fb
    {
      "toolbar.iconbar.empty:", &style->toolbar_iconbar_empty, parseValue
    }, // fb
    { "toolbar.iconbar.focused.borderColor:",
      &style->toolbar_iconbar_focused_borderColor,
      parseColor }, // fb
    { "toolbar.iconbar.focused.borderWidth:",
      &style->toolbar_iconbar_focused_borderWidth,
      parseInt }, // fb
    { "toolbar.iconbar.focused.color:",
      &style->toolbar_iconbar_focused_color,
      parseColor }, // fb
    { "toolbar.iconbar.focused.colorTo:",
      &style->toolbar_iconbar_focused_colorTo,
      parseColor }, // fb
    { "toolbar.iconbar.focused.font.effect:",
      &style->toolbar_iconbar_focused_font_effect,
      parseString }, // fb
    { "toolbar.iconbar.focused.font.shadow.color:",
      &style->toolbar_iconbar_focused_font_shadow_color,
      parseColor }, // fb
    { "toolbar.iconbar.focused.font:",
      &style->toolbar_iconbar_focused_font,
      parseString }, // fb
    { "toolbar.iconbar.focused.justify:",
      &style->toolbar_iconbar_focused_justify,
      parseValue }, // fb
    { "toolbar.iconbar.focused.pixmap:",
      &style->toolbar_iconbar_focused_pixmap,
      parseString }, // fb
    { "toolbar.iconbar.focused.textColor:",
      &style->toolbar_iconbar_focused_textColor,
      parseColor }, // fb
    {
      "toolbar.iconbar.focused:", &style->toolbar_iconbar_focused, parseValue
    }, // fb
    { "toolbar.iconbar.unfocused.borderColor:",
      &style->toolbar_iconbar_unfocused_borderColor,
      parseColor }, // fb
    { "toolbar.iconbar.unfocused.borderWidth:",
      &style->toolbar_iconbar_unfocused_borderWidth,
      parseInt }, // fb
    { "toolbar.iconbar.unfocused.color:",
      &style->toolbar_iconbar_unfocused_color,
      parseColor }, // fb
    { "toolbar.iconbar.unfocused.colorTo:",
      &style->toolbar_iconbar_unfocused_colorTo,
      parseColor }, // fb
    { "toolbar.iconbar.unfocused.font:",
      &style->toolbar_iconbar_unfocused_font,
      parseString }, // fb
    { "toolbar.iconbar.unfocused.justify:",
      &style->toolbar_iconbar_unfocused_justify,
      parseValue }, // fb
    { "toolbar.iconbar.unfocused.pixmap:",
      &style->toolbar_iconbar_unfocused_pixmap,
      parseString }, // fb
    { "toolbar.iconbar.unfocused.textColor:",
      &style->toolbar_iconbar_unfocused_textColor,
      parseColor }, // fb
    { "toolbar.iconbar.unfocused:",
      &style->toolbar_iconbar_unfocused,
      parseValue },                                              // fb
    { "toolbar.justify:", &style->toolbar_justify, parseValue }, // bbwm fb
    {
      "toolbar.label.appearance:", &style->toolbar_label_appearance, parseValue
    }, // bbwm
    {
      "toolbar.label.color:", &style->toolbar_label_color, parseColor
    }, // bbwm fb
    {
      "toolbar.label.colorTo:", &style->toolbar_label_colorTo, parseColor
    }, // bbwm fb
    {
      "toolbar.label.pixmap:", &style->toolbar_label_pixmap, parseString
    }, // fb
    {
      "toolbar.label.textColor:", &style->toolbar_label_textColor, parseColor
    },                                                               // bbwm fb
    { "toolbar.label:", &style->toolbar_label, parseValue },         // bbwm fb
    { "toolbar.pixmap:", &style->toolbar_pixmap, parseString },      // fb
    { "toolbar.shaped:", &style->toolbar_shaped, parseValue },       // fb
    { "toolbar.textColor:", &style->toolbar_textColor, parseColor }, // fb
    { "toolbar.windowLabel.appearance:",
      &style->toolbar_windowLabel_appearance,
      parseValue }, // bbwm
    { "toolbar.windowLabel.color:",
      &style->toolbar_windowLabel_color,
      parseColor }, // bbwm fb
    { "toolbar.windowLabel.colorTo:",
      &style->toolbar_windowLabel_colorTo,
      parseColor }, // bbwm fb
    { "toolbar.windowLabel.pixmap:",
      &style->toolbar_windowLabel_pixmap,
      parseString }, // fb
    { "toolbar.windowLabel.textColor:",
      &style->toolbar_windowLabel_textColor,
      parseColor }, // bbwm fb
    {
      "toolbar.windowLabel:", &style->toolbar_windowLabel, parseValue
    }, // bbwm fb
    { "toolbar.workspace.borderColor:",
      &style->toolbar_workspace_borderColor,
      parseColor }, // fb
    { "toolbar.workspace.borderWidth:",
      &style->toolbar_workspace_borderWidth,
      parseInt }, // fb
    {
      "toolbar.workspace.color:", &style->toolbar_workspace_color, parseColor
    }, // fb
    { "toolbar.workspace.colorTo:",
      &style->toolbar_workspace_colorTo,
      parseColor }, // fb
    {
      "toolbar.workspace.font:", &style->toolbar_workspace_font, parseString
    }, // fb
    { "toolbar.workspace.justify:",
      &style->toolbar_workspace_justify,
      parseValue }, // fb
    {
      "toolbar.workspace.pixmap:", &style->toolbar_workspace_pixmap, parseString
    }, // fb
    { "toolbar.workspace.textColor:",
      &style->toolbar_workspace_textColor,
      parseColor },                                                  // fb
    { "toolbar.workspace:", &style->toolbar_workspace, parseValue }, // fb
    { "toolbar:", &style->toolbar, parseValue },                     // bbwm fb
    { "window.alignment:", &style->window_alignment, parseValue },   // bbwm
    { "window.alpha:", &style->window_alpha, parseValue },           // fb
    { "window.bevelWidth:", &style->window_bevelWidth, parseInt },   // fb
    { "window.borderColor:", &style->window_borderColor, parseColor }, // fb
    { "window.borderWidth:", &style->window_borderWidth, parseInt },   // fb
    { "window.button.focus.appearance:",
      &style->window_button_focus_appearance,
      parseValue }, // bbwm
    { "window.button.focus.color:",
      &style->window_button_focus_color,
      parseColor }, // bbwm fb
    { "window.button.focus.colorTo:",
      &style->window_button_focus_colorTo,
      parseColor }, // bbwm fb
    { "window.button.focus.foregroundColor:",
      &style->window_button_focus_foregroundColor,
      parseColor }, // bbwm
    { "window.button.focus.picColor:",
      &style->window_button_focus_picColor,
      parseColor }, // bbwm fb
    {
      "window.button.focus:", &style->window_button_focus, parseValue
    }, // bbwm fb
    { "window.button.pressed.color:",
      &style->window_button_pressed_color,
      parseColor }, // bbwm fb
    { "window.button.pressed.colorTo:",
      &style->window_button_pressed_colorTo,
      parseColor }, // bbwm fb
    { "window.button.pressed.picColor:",
      &style->window_button_pressed_picColor,
      parseColor }, // fb
    {
      "window.button.pressed:", &style->window_button_pressed, parseValue
    }, // bbwm fb
    { "window.button.unfocus.Color:",
      &style->window_button_unfocus_Color,
      parseColor }, // fb
    { "window.button.unfocus.ColorTo:",
      &style->window_button_unfocus_ColorTo,
      parseColor }, // fb
    { "window.button.unfocus.appearance:",
      &style->window_button_unfocus_appearance,
      parseValue }, // bbwm
    { "window.button.unfocus.color:",
      &style->window_button_unfocus_color,
      parseColor }, // bbwm fb
    { "window.button.unfocus.colorTo:",
      &style->window_button_unfocus_colorTo,
      parseColor }, // bbwm fb
    { "window.button.unfocus.foregroundColor:",
      &style->window_button_unfocus_foregroundColor,
      parseColor }, // bbwm
    { "window.button.unfocus.picColor:",
      &style->window_button_unfocus_picColor,
      parseColor }, // bbwm fb
    {
      "window.button.unfocus:", &style->window_button_unfocus, parseValue
    }, // bbwm fb
    { "window.close.pixmap:", &style->window_close_pixmap, parseString }, // fb
    { "window.close.pressed.pixmap:",
      &style->window_close_pressed_pixmap,
      parseString }, // fb
    { "window.close.unfocus.pixmap:",
      &style->window_close_unfocus_pixmap,
      parseString },                                                    // fb
    { "window.font.effect:", &style->window_font_effect, parseString }, // fb
    {
      "window.font.shadow.color:", &style->window_font_shadow_color, parseColor
    },                                                    // fb
    { "window.font:", &style->window_font, parseString }, // bbwm fb
    {
      "window.frame.focus.color:", &style->window_frame_focus_color, parseColor
    },                                                                 // fb
    { "window.frame.focus:", &style->window_frame_focus, parseValue }, // fb
    {
      "window.frame.focusColor:", &style->window_frame_focusColor, parseColor
    }, // bbwm fb
    { "window.frame.unfocus.color:",
      &style->window_frame_unfocus_color,
      parseColor },                                                        // fb
    { "window.frame.unfocus:", &style->window_frame_unfocus, parseValue }, // fb
    { "window.frame.unfocusColor:",
      &style->window_frame_unfocusColor,
      parseColor },                                                // bbwm fb
    { "window.frameWidth:", &style->window_frameWidth, parseInt }, // bbwm
    { "window.grip.focus.appearance:",
      &style->window_grip_focus_appearance,
      parseValue }, // bbwm
    { "window.grip.focus.backgroundColor:",
      &style->window_grip_focus_backgroundColor,
      parseColor }, // bbwm
    {
      "window.grip.focus.color:", &style->window_grip_focus_color, parseColor
    }, // bbwm fb
    { "window.grip.focus.colorTo:",
      &style->window_grip_focus_colorTo,
      parseColor }, // bbwm fb
    {
      "window.grip.focus.pixmap:", &style->window_grip_focus_pixmap, parseString
    },                                                               // fb
    { "window.grip.focus:", &style->window_grip_focus, parseValue }, // bbwm fb
    { "window.grip.unfocus.appearance:",
      &style->window_grip_unfocus_appearance,
      parseValue }, // bbwm
    { "window.grip.unfocus.color:",
      &style->window_grip_unfocus_color,
      parseColor }, // bbwm fb
    { "window.grip.unfocus.colorTo:",
      &style->window_grip_unfocus_colorTo,
      parseColor }, // bbwm fb
    { "window.grip.unfocus.pixmap:",
      &style->window_grip_unfocus_pixmap,
      parseString }, // fb
    {
      "window.grip.unfocus:", &style->window_grip_unfocus, parseValue
    }, // bbwm fb
    { "window.handle.focus.appearance:",
      &style->window_handle_focus_appearance,
      parseValue }, // bbwm
    { "window.handle.focus.color:",
      &style->window_handle_focus_color,
      parseColor }, // bbwm fb
    { "window.handle.focus.colorTo:",
      &style->window_handle_focus_colorTo,
      parseColor }, // bbwm fb
    { "window.handle.focus.pixmap:",
      &style->window_handle_focus_pixmap,
      parseString }, // fb
    {
      "window.handle.focus:", &style->window_handle_focus, parseValue
    }, // bbwm fb
    { "window.handle.unfocus.appearance:",
      &style->window_handle_unfocus_appearance,
      parseValue }, // bbwm
    { "window.handle.unfocus.color:",
      &style->window_handle_unfocus_color,
      parseColor }, // bbwm fb
    { "window.handle.unfocus.colorTo:",
      &style->window_handle_unfocus_colorTo,
      parseColor }, // bbwm fb
    { "window.handle.unfocus.pixmap:",
      &style->window_handle_unfocus_pixmap,
      parseString }, // fb
    {
      "window.handle.unfocus:", &style->window_handle_unfocus, parseValue
    }, // bbwm fb
    { "window.handleHeight:", &style->window_handleHeight, parseInt }, // bbwm
    { "window.handleWidth:", &style->window_handleWidth, parseInt },   // fb
    {
      "window.iconify.pixmap:", &style->window_iconify_pixmap, parseString
    }, // fb
    { "window.iconify.pressed.pixmap:",
      &style->window_iconify_pressed_pixmap,
      parseString }, // fb
    { "window.iconify.unfocus.pixmap:",
      &style->window_iconify_unfocus_pixmap,
      parseString },                                           // fb
    { "window.justify:", &style->window_justify, parseValue }, // bbwm fb
    { "window.label.focus.appearance:",
      &style->window_label_focus_appearance,
      parseValue }, // bbwm
    {
      "window.label.focus.color:", &style->window_label_focus_color, parseColor
    }, // bbwm fb
    { "window.label.focus.colorTo:",
      &style->window_label_focus_colorTo,
      parseColor }, // bbwm fb
    {
      "window.label.focus.font:", &style->window_label_focus_font, parseString
    }, // fb
    { "window.label.focus.justify:",
      &style->window_label_focus_justify,
      parseValue }, // fb
    { "window.label.focus.pixmap:",
      &style->window_label_focus_pixmap,
      parseString }, // fb
    { "window.label.focus.textColor:",
      &style->window_label_focus_textColor,
      parseColor }, // bbwm fb
    {
      "window.label.focus:", &style->window_label_focus, parseValue
    }, // bbwm fb
    {
      "window.label.marginWidth:", &style->window_label_marginWidth, parseInt
    }, // bbwm
    { "window.label.pixmap:", &style->window_label_pixmap, parseString }, // fb
    { "window.label.unfocus.appearance:",
      &style->window_label_unfocus_appearance,
      parseValue }, // bbwm
    { "window.label.unfocus.color:",
      &style->window_label_unfocus_color,
      parseColor }, // bbwm fb
    { "window.label.unfocus.colorTo:",
      &style->window_label_unfocus_colorTo,
      parseColor }, // bbwm fb
    { "window.label.unfocus.pixmap:",
      &style->window_label_unfocus_pixmap,
      parseString }, // fb
    { "window.label.unfocus.textColor:",
      &style->window_label_unfocus_textColor,
      parseColor }, // bbwm fb
    {
      "window.label.unfocus:", &style->window_label_unfocus, parseValue
    }, // bbwm fb
    {
      "window.maximize.pixmap:", &style->window_maximize_pixmap, parseString
    }, // fb
    { "window.maximize.pressed.pixmap:",
      &style->window_maximize_pressed_pixmap,
      parseString }, // fb
    { "window.maximize.unfocus.pixmap:",
      &style->window_maximize_unfocus_pixmap,
      parseString }, // fb
    {
      "window.menuicon.pixmap:", &style->window_menuicon_pixmap, parseString
    }, // fb
    { "window.menuicon.pressed.pixmap:",
      &style->window_menuicon_pressed_pixmap,
      parseString }, // fb
    { "window.menuicon.unfocus.pixmap:",
      &style->window_menuicon_unfocus_pixmap,
      parseString },                                                      // fb
    { "window.roundCorners:", &style->window_roundCorners, parseValue },  // fb
    { "window.shade.pixmap:", &style->window_shade_pixmap, parseString }, // fb
    { "window.shade.pressed.pixmap:",
      &style->window_shade_pressed_pixmap,
      parseString }, // fb
    { "window.shade.unfocus.pixmap:",
      &style->window_shade_unfocus_pixmap,
      parseString },                                                      // fb
    { "window.shade:", &style->window_shade, parseValue },                // fb
    { "window.stick.pixmap:", &style->window_stick_pixmap, parseString }, // fb
    { "window.stick.pressed.pixmap:",
      &style->window_stick_pressed_pixmap,
      parseString }, // fb
    { "window.stick.unfocus.pixmap:",
      &style->window_stick_unfocus_pixmap,
      parseString },                                                      // fb
    { "window.stuck.pixmap:", &style->window_stuck_pixmap, parseString }, // fb
    { "window.stuck.pressed.pixmap:",
      &style->window_stuck_pressed_pixmap,
      parseString }, // fb
    { "window.stuck.unfocus.pixmap:",
      &style->window_stuck_unfocus_pixmap,
      parseString }, // fb
    {
      "window.tab.borderColor:", &style->window_tab_borderColor, parseColor
    }, // fb
    {
      "window.tab.borderWidth:", &style->window_tab_borderWidth, parseInt
    },                                                                 // fb
    { "window.tab.font:", &style->window_tab_font, parseString },      // fb
    { "window.tab.justify:", &style->window_tab_justify, parseValue }, // fb
    { "window.tab.label.focus.color:",
      &style->window_tab_label_focus_color,
      parseColor }, // fb
    { "window.tab.label.focus.colorTo:",
      &style->window_tab_label_focus_colorTo,
      parseColor }, // fb
    { "window.tab.label.focus.textColor:",
      &style->window_tab_label_focus_textColor,
      parseColor }, // fb
    {
      "window.tab.label.focus:", &style->window_tab_label_focus, parseValue
    }, // fb
    { "window.tab.label.unfocus.color:",
      &style->window_tab_label_unfocus_color,
      parseColor }, // fb
    { "window.tab.label.unfocus.colorTo:",
      &style->window_tab_label_unfocus_colorTo,
      parseColor }, // fb
    { "window.tab.label.unfocus.textColor:",
      &style->window_tab_label_unfocus_textColor,
      parseColor }, // fb
    {
      "window.tab.label.unfocus:", &style->window_tab_label_unfocus, parseValue
    }, // fb
    { "window.title.focus.appearance:",
      &style->window_title_focus_appearance,
      parseValue }, // bbwm
    { "window.title.focus.color1:",
      &style->window_title_focus_color1,
      parseColor }, // bbwm
    { "window.title.focus.color2:",
      &style->window_title_focus_color2,
      parseColor }, // bbwm
    {
      "window.title.focus.color:", &style->window_title_focus_color, parseColor
    }, // bbwm fb
    { "window.title.focus.colorTo:",
      &style->window_title_focus_colorTo,
      parseColor }, // bbwm fb
    { "window.title.focus.pixmap:",
      &style->window_title_focus_pixmap,
      parseString }, // fb
    {
      "window.title.focus:", &style->window_title_focus, parseValue
    }, // bbwm fb
    { "window.title.height:", &style->window_title_height, parseInt }, // fb
    { "window.title.unfocus.appearance:",
      &style->window_title_unfocus_appearance,
      parseValue }, // bbwm
    { "window.title.unfocus.color:",
      &style->window_title_unfocus_color,
      parseColor }, // bbwm fb
    { "window.title.unfocus.colorTo:",
      &style->window_title_unfocus_colorTo,
      parseColor }, // bbwm fb
    { "window.title.unfocus.pixmap:",
      &style->window_title_unfocus_pixmap,
      parseString }, // fb
    {
      "window.title.unfocus:", &style->window_title_unfocus, parseValue
    }, // bbwm fb
    {
      "window.unshade.pixmap:", &style->window_unshade_pixmap, parseString
    }, // fb
    { "window.unshade.pressed.pixmap:",
      &style->window_unshade_pressed_pixmap,
      parseString }, // fb
    { "window.unshade.unfocus.pixmap:",
      &style->window_unshade_unfocus_pixmap,
      parseString }, // fb
