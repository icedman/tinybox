#ifndef TBX_XDG_SHELL_H
#define TBX_XDG_SHELL_H

void xdg_shell_init();

void focus_view(struct tbx_view *view, struct wlr_surface *surface);
bool view_at(struct tbx_view *view,
    double lx, double ly, struct wlr_surface **surface,
    double *sx, double *sy);
struct tbx_view *desktop_view_at(
    struct tbx_server *server, double lx, double ly,
    struct wlr_surface **surface, double *sx, double *sy);

#endif // TBX_XDG_SHELL_H