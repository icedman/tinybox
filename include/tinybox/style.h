#ifndef TINYBOX_STYLE_H
#define TINYBOX_STYLE_H

struct tbx_server;

enum {
  sf_solid = 1 << 1,
  sf_flat = 1 << 2,
  sf_raised = 1 << 3,
  sf_diagonal = 1 << 4,
  sf_crossdiagonal = 1 << 5,
  sf_border = 1 << 6,
  sf_bevel = 1 << 7,
  sf_bevel1 = 1 << 8,
  sf_bevel2 = 1 << 9,
  sf_gradient = 1 << 10,
  sf_interlaced = 1 << 11,
  sf_sunken = 1 << 12,
  sf_vertical = 1 << 13,
  sf_horizontal = 1 << 14
};

struct tbx_style {

  int background_color;                          // fb
  int background_colorTo;                        // fb
  int background_modX;                           // fb
  int background_modY;                           // fb
  int background;                                // fb
  int bbpager_desktop_focus_color;               // fb
  int bbpager_desktop_focus_colorTo;             // fb
  int bbpager_desktop_focus;                     // fb
  int bevelWidth;                                // bbwm fb
  int borderColor;                               // bbwm fb
  int borderWidth;                               // bbwm fb
  int frameWidth;                                // bbwm fb
  int handleWidth;                               // bbwm fb
  int menu_active_appearance;                    // bbwm
  int menu_active_backgroundColor;               // bbwm
  int menu_active_borderColor;                   // bbwm
  int menu_active_borderWidth;                   // bbwm
  int menu_active_color;                         // bbwm
  int menu_active_colorTo;                       // bbwm
  int menu_active_foregroundColor;               // bbwm
  int menu_active_textColor;                     // bbwm
  int menu_active;                               // bbwm
  int menu_bevelWidth;                           // fb
  int menu_borderColor;                          // fb
  int menu_borderWidth;                          // fb
  int menu_borderwidth;                          // fb
  int menu_bullet_position;                      // bbwm fb
  int menu_bullet;                               // bbwm fb
  int menu_frame_alignment;                      // bbwm
  int menu_frame_appearance;                     // bbwm
  int menu_frame_borderColor;                    // bbwm
  int menu_frame_borderWidth;                    // bbwm
  int menu_frame_color;                          // bbwm fb
  int menu_frame_colorTo;                        // bbwm fb
  int menu_frame_disableColor;                   // bbwm fb
  int menu_frame_disabledColor;                  // bbwm
  char *menu_frame_font;                         // bbwm fb
  int menu_frame_foregroundColor;                // bbwm
  int menu_frame_justify;                        // bbwm fb
  char *menu_frame_pixmap;                       // fb
  int menu_frame_textColor;                      // bbwm fb
  int menu_frame_underlineColor;                 // fb
  int menu_frame;                                // bbwm fb
  int menu_hilite_color;                         // bbwm fb
  int menu_hilite_colorTo;                       // bbwm fb
  char *menu_hilite_pixmap;                      // fb
  char *menu_hilite_selected_pixmap;             // fb
  char *menu_hilite_submenu_pixmap;              // fb
  int menu_hilite_textColor;                     // bbwm fb
  char *menu_hilite_unselected_pixmap;           // fb
  int menu_hilite;                               // bbwm fb
  char *menu_selected_pixmap;                    // fb
  char *menu_submenu_pixmap;                     // fb
  int menu_title_alignment;                      // bbwm
  int menu_title_appearance;                     // bbwm
  int menu_title_backgroundColor;                // bbwm
  int menu_title_borderColor;                    // bbwm
  int menu_title_borderWidth;                    // bbwm
  int menu_title_color;                          // bbwm fb
  int menu_title_colorTo;                        // bbwm fb
  char *menu_title_font_effect;                  // fb
  int menu_title_font_shadow_color;              // fb
  char *menu_title_font;                         // bbwm fb
  int menu_title_foregroundColor;                // bbwm
  int menu_title_justify;                        // bbwm fb
  char *menu_title_pixmap;                       // fb
  int menu_title_textColor;                      // bbwm fb
  int menu_title;                                // bbwm fb
  char *menu_unselected_pixmap;                  // fb
  char *menuFont;                                // fb
  int rootCommand;                               // bbwm
  int slit_appearance;                           // bbwm
  int slit_backgroundColor;                      // bbwm
  int slit_bevelWidth;                           // fb
  int slit_borderColor;                          // fb
  int slit_borderWidth;                          // fb
  int slit_color;                                // fb
  int slit_colorTo;                              // fb
  int slit_marginWidth;                          // bbwm
  char *slit_pixmap;                             // fb
  int slit;                                      // fb
  int style_author;                              // fb
  int style_comments;                            // fb
  int style_credits;                             // fb
  int style_date;                                // fb
  int style_name;                                // fb
  char *titleFont;                               // fb
  int toolbar_alignment;                         // bbwm
  int toolbar_alpha;                             // fb
  int toolbar_appearance;                        // bbwm
  int toolbar_bevelWidth;                        // fb
  int toolbar_borderColor;                       // fb
  int toolbar_borderWidth;                       // fb
  int toolbar_button_color;                      // bbwm fb
  int toolbar_button_colorTo;                    // bbwm fb
  int toolbar_button_picColor;                   // bbwm fb
  char *toolbar_button_pixmap;                   // fb
  int toolbar_button_pressed_color;              // bbwm fb
  int toolbar_button_pressed_colorTo;            // bbwm fb
  int toolbar_button_pressed_picColor;           // fb
  char *toolbar_button_pressed_pixmap;           // fb
  int toolbar_button_pressed;                    // bbwm fb
  int toolbar_button;                            // bbwm fb
  int toolbar_clock_appearance;                  // bbwm
  int toolbar_clock_borderColor;                 // fb
  int toolbar_clock_borderWidth;                 // fb
  int toolbar_clock_color;                       // bbwm fb
  int toolbar_clock_colorTo;                     // bbwm fb
  char *toolbar_clock_font;                      // fb
  int toolbar_clock_justify;                     // fb
  char *toolbar_clock_pixmap;                    // fb
  int toolbar_clock_textColor;                   // bbwm fb
  int toolbar_clock;                             // bbwm fb
  int toolbar_color;                             // bbwm fb
  int toolbar_colorTo;                           // bbwm fb
  char *toolbar_font;                            // bbwm fb
  int toolbar_height;                            // fb
  int toolbar_iconbar_borderColor;               // fb
  int toolbar_iconbar_borderWidth;               // fb
  int toolbar_iconbar_empty_color;               // fb
  int toolbar_iconbar_empty_colorTo;             // fb
  char *toolbar_iconbar_empty_pixmap;            // fb
  int toolbar_iconbar_empty;                     // fb
  int toolbar_iconbar_focused_borderColor;       // fb
  int toolbar_iconbar_focused_borderWidth;       // fb
  int toolbar_iconbar_focused_color;             // fb
  int toolbar_iconbar_focused_colorTo;           // fb
  char *toolbar_iconbar_focused_font_effect;     // fb
  int toolbar_iconbar_focused_font_shadow_color; // fb
  char *toolbar_iconbar_focused_font;            // fb
  int toolbar_iconbar_focused_justify;           // fb
  char *toolbar_iconbar_focused_pixmap;          // fb
  int toolbar_iconbar_focused_textColor;         // fb
  int toolbar_iconbar_focused;                   // fb
  int toolbar_iconbar_unfocused_borderColor;     // fb
  int toolbar_iconbar_unfocused_borderWidth;     // fb
  int toolbar_iconbar_unfocused_color;           // fb
  int toolbar_iconbar_unfocused_colorTo;         // fb
  char *toolbar_iconbar_unfocused_font;          // fb
  int toolbar_iconbar_unfocused_justify;         // fb
  char *toolbar_iconbar_unfocused_pixmap;        // fb
  int toolbar_iconbar_unfocused_textColor;       // fb
  int toolbar_iconbar_unfocused;                 // fb
  int toolbar_justify;                           // bbwm fb
  int toolbar_label_appearance;                  // bbwm
  int toolbar_label_color;                       // bbwm fb
  int toolbar_label_colorTo;                     // bbwm fb
  char *toolbar_label_pixmap;                    // fb
  int toolbar_label_textColor;                   // bbwm fb
  int toolbar_label;                             // bbwm fb
  char *toolbar_pixmap;                          // fb
  int toolbar_shaped;                            // fb
  int toolbar_textColor;                         // fb
  int toolbar_windowLabel_appearance;            // bbwm
  int toolbar_windowLabel_color;                 // bbwm fb
  int toolbar_windowLabel_colorTo;               // bbwm fb
  char *toolbar_windowLabel_pixmap;              // fb
  int toolbar_windowLabel_textColor;             // bbwm fb
  int toolbar_windowLabel;                       // bbwm fb
  int toolbar_workspace_borderColor;             // fb
  int toolbar_workspace_borderWidth;             // fb
  int toolbar_workspace_color;                   // fb
  int toolbar_workspace_colorTo;                 // fb
  char *toolbar_workspace_font;                  // fb
  int toolbar_workspace_justify;                 // fb
  char *toolbar_workspace_pixmap;                // fb
  int toolbar_workspace_textColor;               // fb
  int toolbar_workspace;                         // fb
  int toolbar;                                   // bbwm fb
  int window_alignment;                          // bbwm
  int window_alpha;                              // fb
  int window_bevelWidth;                         // fb
  int window_borderColor;                        // fb
  int window_borderWidth;                        // fb
  int window_button_focus_appearance;            // bbwm
  int window_button_focus_color;                 // bbwm fb
  int window_button_focus_colorTo;               // bbwm fb
  int window_button_focus_foregroundColor;       // bbwm
  int window_button_focus_picColor;              // bbwm fb
  int window_button_focus;                       // bbwm fb
  int window_button_pressed_color;               // bbwm fb
  int window_button_pressed_colorTo;             // bbwm fb
  int window_button_pressed_picColor;            // fb
  int window_button_pressed;                     // bbwm fb
  int window_button_unfocus_Color;               // fb
  int window_button_unfocus_ColorTo;             // fb
  int window_button_unfocus_appearance;          // bbwm
  int window_button_unfocus_color;               // bbwm fb
  int window_button_unfocus_colorTo;             // bbwm fb
  int window_button_unfocus_foregroundColor;     // bbwm
  int window_button_unfocus_picColor;            // bbwm fb
  int window_button_unfocus;                     // bbwm fb
  char *window_close_pixmap;                     // fb
  char *window_close_pressed_pixmap;             // fb
  char *window_close_unfocus_pixmap;             // fb
  char *window_font_effect;                      // fb
  int window_font_shadow_color;                  // fb
  char *window_font;                             // bbwm fb
  int window_frame_focus_color;                  // fb
  int window_frame_focus;                        // fb
  int window_frame_focusColor;                   // bbwm fb
  int window_frame_unfocus_color;                // fb
  int window_frame_unfocus;                      // fb
  int window_frame_unfocusColor;                 // bbwm fb
  int window_frameWidth;                         // bbwm
  int window_grip_focus_appearance;              // bbwm
  int window_grip_focus_backgroundColor;         // bbwm
  int window_grip_focus_color;                   // bbwm fb
  int window_grip_focus_colorTo;                 // bbwm fb
  char *window_grip_focus_pixmap;                // fb
  int window_grip_focus;                         // bbwm fb
  int window_grip_unfocus_appearance;            // bbwm
  int window_grip_unfocus_color;                 // bbwm fb
  int window_grip_unfocus_colorTo;               // bbwm fb
  char *window_grip_unfocus_pixmap;              // fb
  int window_grip_unfocus;                       // bbwm fb
  int window_handle_focus_appearance;            // bbwm
  int window_handle_focus_color;                 // bbwm fb
  int window_handle_focus_colorTo;               // bbwm fb
  char *window_handle_focus_pixmap;              // fb
  int window_handle_focus;                       // bbwm fb
  int window_handle_unfocus_appearance;          // bbwm
  int window_handle_unfocus_color;               // bbwm fb
  int window_handle_unfocus_colorTo;             // bbwm fb
  char *window_handle_unfocus_pixmap;            // fb
  int window_handle_unfocus;                     // bbwm fb
  int window_handleHeight;                       // bbwm
  int window_handleWidth;                        // fb
  char *window_iconify_pixmap;                   // fb
  char *window_iconify_pressed_pixmap;           // fb
  char *window_iconify_unfocus_pixmap;           // fb
  int window_justify;                            // bbwm fb
  int window_label_focus_appearance;             // bbwm
  int window_label_focus_color;                  // bbwm fb
  int window_label_focus_colorTo;                // bbwm fb
  char *window_label_focus_font;                 // fb
  int window_label_focus_justify;                // fb
  char *window_label_focus_pixmap;               // fb
  int window_label_focus_textColor;              // bbwm fb
  int window_label_focus;                        // bbwm fb
  int window_label_marginWidth;                  // bbwm
  char *window_label_pixmap;                     // fb
  int window_label_unfocus_appearance;           // bbwm
  int window_label_unfocus_color;                // bbwm fb
  int window_label_unfocus_colorTo;              // bbwm fb
  char *window_label_unfocus_pixmap;             // fb
  int window_label_unfocus_textColor;            // bbwm fb
  int window_label_unfocus;                      // bbwm fb
  char *window_maximize_pixmap;                  // fb
  char *window_maximize_pressed_pixmap;          // fb
  char *window_maximize_unfocus_pixmap;          // fb
  char *window_menuicon_pixmap;                  // fb
  char *window_menuicon_pressed_pixmap;          // fb
  char *window_menuicon_unfocus_pixmap;          // fb
  int window_roundCorners;                       // fb
  char *window_shade_pixmap;                     // fb
  char *window_shade_pressed_pixmap;             // fb
  char *window_shade_unfocus_pixmap;             // fb
  int window_shade;                              // fb
  char *window_stick_pixmap;                     // fb
  char *window_stick_pressed_pixmap;             // fb
  char *window_stick_unfocus_pixmap;             // fb
  char *window_stuck_pixmap;                     // fb
  char *window_stuck_pressed_pixmap;             // fb
  char *window_stuck_unfocus_pixmap;             // fb
  int window_tab_borderColor;                    // fb
  int window_tab_borderWidth;                    // fb
  char *window_tab_font;                         // fb
  int window_tab_justify;                        // fb
  int window_tab_label_focus_color;              // fb
  int window_tab_label_focus_colorTo;            // fb
  int window_tab_label_focus_textColor;          // fb
  int window_tab_label_focus;                    // fb
  int window_tab_label_unfocus_color;            // fb
  int window_tab_label_unfocus_colorTo;          // fb
  int window_tab_label_unfocus_textColor;        // fb
  int window_tab_label_unfocus;                  // fb
  int window_title_focus_appearance;             // bbwm
  int window_title_focus_color1;                 // bbwm
  int window_title_focus_color2;                 // bbwm
  int window_title_focus_color;                  // bbwm fb
  int window_title_focus_colorTo;                // bbwm fb
  char *window_title_focus_pixmap;               // fb
  int window_title_focus;                        // bbwm fb
  int window_title_height;                       // fb
  int window_title_unfocus_appearance;           // bbwm
  int window_title_unfocus_color;                // bbwm fb
  int window_title_unfocus_colorTo;              // bbwm fb
  char *window_title_unfocus_pixmap;             // fb
  int window_title_unfocus;                      // bbwm fb
  char *window_unshade_pixmap;                   // fb
  char *window_unshade_pressed_pixmap;           // fb
  char *window_unshade_unfocus_pixmap;           // fb

  int hash;

  char font[255];
};

void
load_style(struct tbx_server *server, const char *path);
void
free_style(struct tbx_style *style);

#endif // TINYBOX_STYLE_H