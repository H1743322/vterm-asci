#pragma once

extern "C" {
#include <libavutil/frame.h>
}

void frame_to_ASCII(AVFrame* frame, int h, int w, char* buffer);
void frame_to_color_ASCII(AVFrame* rgb, int h, int w, char* buffer);
