#ifndef TBX_WORKSPACE
#define TBX_WORKSPACE

struct tbx_workspace {
  struct wl_list link;
  // struct tbx_output* output;
  int id;
};

struct tbx_workspace *workspace_create();
void workspace_destroy(struct tbx_workspace *workspace);
void workspace_init();

void assign_view_workspace(struct tbx_view *view);
void assign_server_workspace();

#endif //  TBX_WORKSPACE