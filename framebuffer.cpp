#include "framebuffer.h"

Framebuffer::Framebuffer() {
  erase();
}

void Framebuffer::erase() {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        pixels_[i] = 0x00;
    }
}

void Framebuffer::put(int x, int y, char color) {
    pixels_[(WIDTH * y) + x] = color;
}

char Framebuffer::get(int x, int y) {
    return pixels_[(WIDTH * y) + x];
}

const char* Framebuffer::getPixels() {
    return pixels_;
}
