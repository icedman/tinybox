#ifndef TINYBOX_LIBINPUT_H
#define TINYBOX_LIBINPUT_H

#include "tinybox/server.h"

void reset_libinput_device(struct tbx_input_device* device);
void configure_libinput_device(struct tbx_input_device* device);

#endif // TINYBOX_LIBINPUT_H