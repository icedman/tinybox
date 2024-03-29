#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/arrange.h"
#include "tinybox/command.h"
#include "tinybox/config.h"
#include "tinybox/damage.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"
#include "tinybox/view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tinybox/workspace.h"

void
exec_workspace(struct tbx_command *cmd, int argc, char **argv)
{
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  int w = strtol(argv[0], NULL, 10);
  cmd->data = get_workspace(cmd->server, w);
  if (cmd->server->started) {

    if (strcmp(argv[0], "left") == 0) {
      w = cmd->server->workspace - 1;
    } else if (strcmp(argv[0], "right") == 0) {
      w = cmd->server->workspace + 1;
    }

    workspace_activate(cmd->server, w, true);
  }
}

void
exec_set_background(struct tbx_command *cmd, int argc, char **argv)
{
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_workspace *workspace = cmd->context->data;
  if (workspace) {

    strip_quotes(argv[0]);

    char *expanded = calloc(strlen(argv[0]) + 1, sizeof(char));
    strcpy(expanded, argv[0]);
    expand_path(&expanded);

    workspace->background = expanded;
    console_log("set background %s", expanded);
  }
}

void
exec_move_window_to_workspace(struct tbx_command *cmd, int argc, char **argv)
{
  if (!command_check_args(cmd, argc, 4)) {
    return;
  }

  if (!cmd->server->started) {
    return;
  }

  // activate top most
  struct tbx_view *current_view =
      workspace_get_top_view(cmd->server, cmd->server->workspace);
  if (!current_view) {
    return;
  }

  // current_view->interface->set_activated(current_view, true);
  view_set_focus(current_view, NULL);

  int w = 0;
  if (strcmp(argv[3], "left") == 0) {
    w = current_view->workspace - 1;
  } else if (strcmp(argv[3], "right") == 0) {
    w = current_view->workspace + 1;
  } else {
    w = strtol(argv[3], NULL, 10);
  }

  view_send_to_workspace(current_view, w, true);
}

void
exec_window_close(struct tbx_command *cmd, int argc, char **argv)
{
  console_log("window close!");

  if (!command_check_args(cmd, argc, 0)) {
    return;
  }

  if (!cmd->server->started) {
    return;
  }

  // activate top most
  struct tbx_view *current_view =
      workspace_get_top_view(cmd->server, cmd->server->workspace);
  if (!current_view) {
    return;
  }

  current_view->interface->close(current_view);
}

void
exec_shade_window(struct tbx_command *cmd, int argc, char **argv)
{
  // if (!command_check_args(cmd, argc, 0)) {
  //   return;
  // }

  if (!cmd->server->started) {
    return;
  }

  // struct tbx_view* current_view = wl_container_of(cmd->server->views.next,
  // current_view, link);
  struct tbx_view *current_view =
      workspace_get_top_view(cmd->server, cmd->server->workspace);
  if (!current_view) {
    return;
  }

  bool shade = !current_view->shaded;
  if (argc && strcmp(argv[0], "up") == 0) {
    shade = true;
  }
  if (argc && strcmp(argv[0], "down") == 0) {
    shade = false;
  }

  current_view->shaded = shade;
}

void
exec_cycle(struct tbx_command *cmd, int argc, char **argv)
{
  workspace_cycle_views(cmd->server, cmd->server->workspace);
}

void
exec_arrange(struct tbx_command *cmd, int argc, char **argv)
{
  console_log("arrange");
  arrange_begin(cmd->server, cmd->server->workspace, 4, 8);
  arrange_run(cmd->server);
  arrange_end(cmd->server);
}

void
register_workspace_commands(struct tbx_server *server)
{
  struct tbx_command *wks =
      register_command(server->command, "workspace", exec_workspace);
  register_command(wks, "background", exec_set_background);
  register_command(server->command, "move", exec_move_window_to_workspace);
  register_command(server->command, "shade", exec_shade_window);
  register_command(server->command, "cycle", exec_cycle);
  register_command(server->command, "arrange", exec_arrange);
  register_command(server->command, "window_close", exec_window_close);
}