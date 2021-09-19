#include <TFT_eSPI.h>

void glitch_vert_scroll(TFT_eSPI *tft, uint16_t *fb, int v_offset) {
    int h = tft->height();
    int w = tft->width();
    uint16_t offset = v_offset % h;

    tft->pushImageDMA(0, 0, w, offset, fb + w*(h - offset));
    tft->pushImageDMA(0, offset, w, h - offset, fb);
}

void glitch_hori_scroll(TFT_eSPI *tft, uint16_t *fb, int h_offset) {
    int h = tft->height();
    int w = tft->width();
    uint16_t offset = h_offset % w;

    tft->pushImageDMA(0, 0, w, h, fb + (h - offset));
}

void glitch_hori_warp(TFT_eSPI *tft, uint16_t *fb) {
    int h = tft->height();
    int w = tft->width();
    int offset_x, offset_y, block, scanline = 0;
    

    while (scanline < h) {
        offset_y = random(3) - 1;
        block = random(8);
        tft->pushImageDMA(offset_x, scanline, w*2 + offset_y, block, fb + 1 - offset_y + w*(scanline));
        offset_x -= block*(offset_y) % w;
        scanline += block;
    }

    
}

void glitch_power_off(TFT_eSPI *tft, uint16_t *fb) {
    int h = tft->height();
    int w = tft->width();
    
    uint16_t collapse;
    uint16_t* scratch = NULL;
    uint16_t* whiteline = NULL;
    scratch = (uint16_t*) calloc(h*w, sizeof(uint16_t));
    whiteline = (uint16_t*) calloc(w, sizeof(uint16_t));
    int start;

    for (int i=0; i<w; i++){
        whiteline[i]=0xFFFF;
    }
    for (collapse = 1; collapse < (h / 4); collapse++) {
        start = ((float)h/2.0) - h*(0.5/(float)collapse);
        tft->pushImageDMA(0, 0, w, start, scratch);
        tft->pushImageDMA(0, start, w*collapse, (h/collapse), fb);
        tft->pushImageDMA(0, start + (h/collapse), w, start, scratch);
        tft->pushImageDMA(0, h/2 - 1, w, 1, whiteline);
    }
    tft->pushImageDMA(0, 0, w, (h/2) - 1, scratch);
    tft->pushImageDMA(0, h/2 - 1, w, 1, whiteline);
    tft->pushImageDMA(0, h/2, w, (h/2) - 1, scratch);
    delay(100);
    for (collapse = 1; collapse < (w); collapse ++) {
        start = ((float)w/2.0) - w*(0.5/(float)collapse);
        tft->pushImageDMA(0, h/2 - 1, start, 1, scratch);
        tft->pushImageDMA((w/2) + (w/2) - start, h/2 - 1, start, 1, scratch);
        delay(5);
    }
    free(scratch);
    free(whiteline);
}
