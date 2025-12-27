#include "utils.hpp"

#include <cstdio>
#include <iostream>
#include <libavutil/frame.h>
#include <sys/ioctl.h>
#include <unistd.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

int WIDTH = 120;
int HEIGHT = 60;
const double FPS = 60;
const bool USE_COLOR = true;

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <video_path>\n";
        return 1;
    }

#if defined(_WIN32)
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    WIDTH = w.ws_col - 1;
    HEIGHT = w.ws_row - 1;
#endif

    const char* path = argv[1];

    AVFormatContext* fmtCtx = avformat_alloc_context();
    if (!fmtCtx) {
        std::cerr << "ERROR could not allocate memory format codecCtx";
        return 1;
    }

    if (avformat_open_input(&fmtCtx, path, nullptr, nullptr) != 0) {
        std::cerr << "ERROR could not open file\n";
        return 1;
    }

    if (avformat_find_stream_info(fmtCtx, nullptr) != 0) {
        std::cerr << "ERROR could not find stream info\n";
        return 1;
    }

    // Video
    int vStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (vStreamIndex < 0) {
        std::cerr << "ERROR could not find index for video stream\n";
        return 1;
    }

    AVStream* stream = fmtCtx->streams[vStreamIndex];

    // Decoder
    const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec)
        return 1;

    AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(codecCtx, stream->codecpar) < 0) {
        std::cerr << "ERROR could not get codec params\n";
        return 1;
    }
    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        std::cerr << "ERROR could not open codec\n";
        return 1;
    }

    // Frame
    AVFrame* frame = av_frame_alloc();
    AVFrame* out = av_frame_alloc();
    AVPacket* pkt = av_packet_alloc();

    AVPixelFormat out_pixel_fmt = (USE_COLOR) ? AV_PIX_FMT_RGB24 : AV_PIX_FMT_GRAY8;

    SwsContext* sws = sws_getContext(codecCtx->width, codecCtx->height, codecCtx->pix_fmt, WIDTH, HEIGHT,
                                     out_pixel_fmt, SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!sws) {
        std::cerr << "ERROR could not get sws ctx\n";
        return 1;
    }

    out->format = out_pixel_fmt;
    out->width = WIDTH;
    out->height = HEIGHT;

    if (av_frame_get_buffer(out, 32) < 0) {
        std::cerr << "ERROR could not get framve buffer";
        return 1;
    }

    // double fps = av_q2d(stream->avg_frame_rate);
    // double frame_delay_ms = 1000.0 / fps;

    size_t buf_size = USE_COLOR ? HEIGHT * WIDTH * 50 + HEIGHT + 1 : HEIGHT * (WIDTH + 1) + 1;
    char* ascii_buffer = new char[buf_size];

    while (av_read_frame(fmtCtx, pkt) >= 0) {
        if (pkt->stream_index != vStreamIndex) {
            av_packet_unref(pkt);
            continue;
        }

        avcodec_send_packet(codecCtx, pkt);
        while (avcodec_receive_frame(codecCtx, frame) == 0) {
            sws_scale(sws, frame->data, frame->linesize, 0, frame->height, out->data, out->linesize);

            if (USE_COLOR) {
                frame_to_color_ASCII(out, HEIGHT, WIDTH, ascii_buffer);
            } else {
                frame_to_ASCII(out, HEIGHT, WIDTH, ascii_buffer);
            }

            std::printf("\033[H%s", ascii_buffer);
            // fflush(stdout);
            // NOTE: Not accurate
            // usleep((useconds_t)(frame_delay_ms * 1000));

            usleep((useconds_t)(1e6 / FPS));
        }

        av_packet_unref(pkt);
    }

    // Free
    delete[] ascii_buffer;
    av_frame_free(&out);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    sws_freeContext(sws);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&fmtCtx);
    std::printf("End\n");

    return 0;
}
