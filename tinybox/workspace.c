#include "tinybox/tbx_server.h"
#include "tinybox/tbx_workspace.h"
#include "tinybox/console.h"

struct tbx_workspace *workspace_create() {
    struct tbx_workspace *workspace = calloc(1, sizeof(struct tbx_workspace));
    if (workspace == NULL) {
        return NULL;
    }
    wl_list_insert(&server.workspaces, &workspace->link);
    return workspace;
}

void workspace_destroy(struct tbx_workspace *destroy) {
    // remove all views from workspace
    wl_list_remove(&destroy->link);
    free(destroy);
}

static struct tbx_workspace *find_workspace(int idx) {
    int i = 0;
    struct tbx_workspace *workspaces[24];
    struct tbx_workspace *workspace;
    wl_list_for_each(workspace, &server.workspaces, link) {
      workspace->id = i;
      workspaces[i++] = workspace;
      if (i == 24) break;
    }

    idx = idx % i;
    return workspaces[idx];
}

void assign_view_workspace(struct tbx_view *view)
{
    if (view->workspace) {
        return;
    }
    view->workspace = find_workspace(view->workspace_id);
    view->workspace_id = view->workspace->id;
}

void assign_server_workspace()
{
    if (server.active_workspace) {
        return;
    }
    server.active_workspace = find_workspace(server.active_workspace_id);
    server.active_workspace_id = server.active_workspace->id;
    console_clear();
    console_log("w:%d", server.active_workspace_id);
}


void workspace_init() {
  wl_list_init(&server.workspaces);

    workspace_create();
    workspace_create();
    // workspace_create();
    // workspace_create();
}
