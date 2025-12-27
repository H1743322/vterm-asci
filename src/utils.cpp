#include "utils.hpp"

// static const char* ASCII = " .:-=+*#%@";
static const char* ASCII = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/*tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
const int ASCII_LEN = strlen(ASCII);

void frame_to_ASCII(AVFrame* frame, const int h, const int w, char* buffer) {
    int i = 0;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint8_t pixel = frame->data[0][y * frame->linesize[0] + x];
            buffer[i++] = ASCII[pixel * (ASCII_LEN - 1) / 255];
        }
        buffer[i++] = '\n';
    }
    buffer[i] = '\0';
}
void frame_to_color_ASCII(AVFrame* rgb, const int h, const int w, char* buffer) {
    int i = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint8_t r = rgb->data[0][y * rgb->linesize[0] + x * 3 + 0];
            uint8_t g = rgb->data[0][y * rgb->linesize[0] + x * 3 + 1];
            uint8_t b = rgb->data[0][y * rgb->linesize[0] + x * 3 + 2];

            int idx = (r + g + b) * (ASCII_LEN - 1) / (3 * 255);
            char c = ASCII[idx];

            i += sprintf(buffer + i, "\033[38;2;%d;%d;%dm%c\033[0m", r, g, b, c);
        }
        buffer[i++] = '\n';
    }
    buffer[i] = '\0';
}
