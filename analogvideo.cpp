#include <TFT_eSPI.h>
#include <vector>
#include <algorithm>
#include "analogvideo.h"


/* glitch effect functions */
void glitch_vert_scroll(TFT_eSPI *tft, uint16_t *fb, int v_offset) {
    int h = tft->height();
    int w = tft->width();
    uint16_t offset = v_offset % h;
    uint16_t fb_temp[h * w];
    
    glitch_chrom_noise(fb_temp, fb, w, h, 0.22);
    tft->pushImage(0, 0, w, offset, fb_temp + w*(h - offset));
    tft->pushImage(0, offset, w, h - offset, fb_temp);
}



void glitch_hori_scroll(TFT_eSPI *tft, uint16_t *fb, int h_offset) {
    int h = tft->height();
    int w = tft->width();
    uint16_t offset = h_offset % w;

    tft->pushImage(0, 0, w, h, fb + (h - offset));
}



void glitch_hori_warp(TFT_eSPI *tft, uint16_t *fb) {
    int h = tft->height();
    int w = tft->width();
    int glitched_w,glitched_hsync;
    int offset_x, offset_y, block, scanline = 0;

    glitch_chrom_noise(tft, fb, 0.22);
    while (scanline < h) { 
        glitched_w = w + random(w/16);
        glitched_hsync = 10 - random(31);
        block = random(8) + 1;
        
        tft->pushImage(glitched_hsync, scanline, glitched_w, block, fb + (w * scanline));
        scanline += block;
    }
}



void glitch_power_off(TFT_eSPI *tft, uint16_t *fb) {
    Serial.println("power_off");
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
        tft->pushImage(0, 0, w, start, scratch);
        tft->pushImage(0, start, w*collapse, (h/collapse), fb);
        tft->pushImage(0, start + (h/collapse), w, start, scratch);
        tft->pushImage(0, h/2 - 1, w, 1, whiteline);
    }
    tft->pushImage(0, 0, w, (h/2) - 1, scratch);
    tft->pushImage(0, h/2 - 1, w, 1, whiteline);
    tft->pushImage(0, h/2, w, (h/2) - 1, scratch);
    delay(100);
    for (collapse = 1; collapse < (w); collapse ++) {
        start = ((float)w/2.0) - w*(0.5/(float)collapse);
        tft->pushImage(0, h/2 - 1, start, 1, scratch);
        tft->pushImage((w/2) + (w/2) - start, h/2 - 1, start, 1, scratch);
        delay(5);
    }
    free(scratch);
    free(whiteline);
}


/* this function will apply noise and push to tft */
void glitch_chrom_noise(TFT_eSPI *tft, uint16_t *fb, float chroma_noise) {
    int h = tft->height();
    int w = tft->width();
    yiqcolori_t yiq;
    
    uint16_t fb_temp[h * w];

    memcpy(fb_temp, fb, h*w*sizeof(fb_temp[0]));

    int noise_y, noise_u, noise_v = 0;
    for (int noise_pixels = 0; noise_pixels < 4096; noise_pixels++) {
        int pixelnum = random(h*w);     
        
        yiq = rgb2yiq_i(fb_temp[pixelnum]);
        noise_u = (random(255) - 128) * chroma_noise;
        noise_v = (random(255) - 128) * chroma_noise;
        noise_y = (random(255) - 32) * (chroma_noise/3);

        yiq.y = min(max(yiq.y + noise_y, 0), 255);
        yiq.i = min(max(yiq.i + noise_u, -127), 127);
        yiq.q = min(max(yiq.q + noise_v, -127), 127);
    
        fb_temp[pixelnum] = yiq2rgb565_i(yiq);
    }
    tft->pushImage(0, 0, w, h, fb_temp);
}


/* this function will not push to TFT, and is used when combining glitches */
void glitch_chrom_noise(uint16_t *fb_temp, uint16_t *fb, int w, int h, float chroma_noise) {
    int noise_y, noise_u, noise_v = 0;
    yiqcolori_t yiq;

    memcpy(fb_temp, fb, h*w*sizeof(fb_temp[0]));
    
    for (int noise_pixels = 0; noise_pixels < 4096; noise_pixels++) {
        int pixelnum = random(h*w);     
        
        yiq = rgb2yiq_i(fb_temp[pixelnum]);
        noise_u = (random(255) - 128) * chroma_noise;
        noise_v = (random(255) - 128) * chroma_noise;
        noise_y = (random(255) - 32) * (chroma_noise/3);

        yiq.y = min(max(yiq.y + noise_y, 0), 255);
        yiq.i = min(max(yiq.i + noise_u, -127), 127);
        yiq.q = min(max(yiq.q + noise_v, -127), 127);
    
        fb_temp[pixelnum] = yiq2rgb565_i(yiq);
    }
}

uint16_t yiq2rgb565(yiqcolor_t yiq) {
    char hibyte,lobyte;
    uint16_t rgb565;
    float r,g,b;

    color_t rgb;

    r = 255.0 * max((1.000 * yiq.y + 0.956 * yiq.i + 0.621 * yiq.q), 0.0);
    g = 255.0 * max((1.000 * yiq.y + -0.272 * yiq.i + -0.647 * yiq.q), 0.0);
    b = 255.0 * max((1.000 * yiq.y + -1.106 * yiq.i + 1.703 * yiq.q), 0.0);
    rgb.ry =min((int)r, 255);
    rgb.gi =min((int)g, 255);
    rgb.bq =min((int)b, 255);

    rgb565 = (((rgb.ry & 0xF8) <<8) + ((rgb.gi & 0xFC ) <<3) + (rgb.bq >>3));
    /* swap endianness */
    hibyte = (rgb565 & 0xff00) >> 8;
    lobyte = (rgb565 & 0xff);
    rgb565 = lobyte << 8 | hibyte;    
    
    return (rgb565);
}

yiqcolor_t rgb2yiq(uint16_t rgb) {
    char r, g, b;
    yiqcolor_t yiq;
    char hibyte,lobyte;

    /* swap endianness */
    hibyte = (rgb & 0xff00) >> 8;
    lobyte = (rgb & 0xff);
    rgb = lobyte << 8 | hibyte;

    r = (rgb & 0xF800) >> 8; /* rrrrr... ........ -> rrrrr000 */
    g = (rgb & 0x07E0) >> 3; /* .....ggg ggg..... -> gggggg00 */
    b = (rgb & 0x1F) << 3;   /* ........ ...bbbbb -> bbbbb000 */
    //Serial.printf("%i %i %i ", r, g, b);

    yiq.y = (0.299900 * r/255 + 0.587000 * g/255 + 0.114000 * b/255);
    yiq.i = (0.595716 * r/255 - 0.274453 * g/255 - 0.321264 * b/255);
    yiq.q = (0.211456 * r/255 - 0.522591 * g/255 + 0.311350 * b/255);

    return yiq;
}

yiqcolori_t rgb2yiq_i(uint16_t rgb) {
    char r, g, b;
    yiqcolori_t yiq;

    /* swap endianness */
    rgb = byteswap(rgb);
    //Serial.printf("rgb565:%x > ", rgb);

    r = (rgb & 0xF800) >> 8; /* rrrrr... ........ -> rrrrr000 */
    g = (rgb & 0x07E0) >> 3; /* .....ggg ggg..... -> gggggg00 */
    b = (rgb & 0x1F) << 3;   /* ........ ...bbbbb -> bbbbb000 */
    //Serial.printf("rgb888:%i,%i,%i > ", r, g, b);

    /* using integer math for speed */
    /*
    yiq.y =min(max((  76*r/255 + 150*g/255 +  37*b/255), 0), 255);
    yiq.i =min(max(( 276*r/255 - 127*g/255 - 149*b/255), -127), 127);
    yiq.q =min(max(( 127*r/255 - 313*g/255 + 186*b/255), -127), 127);
    */
    
    yiq.y =min(max(( 76*r/248 + 150*g/252 +  37*b/248), 0), 255);
    yiq.i =min(max(( 76*r/248 -  35*g/252 -  41*b/248), -127), 127);
    yiq.q =min(max(( 27*r/248 -  66*g/252 +  40*b/248), -127), 127);    

    //Serial.printf("yiq: %i,%i,%i>", yiq.y, yiq.i, yiq.q);
    
    return yiq;
}

uint16_t yiq2rgb565_i(yiqcolori_t yiq) {
    uint16_t rgb565;
    //char r,g,b;

    color_t rgb;
    //Serial.printf("yiq:%i %i %i>", yiq.y, yiq.i, yiq.q);
    
    /* FIX SIGNED VS UNSIGNED! 
    r = max(yiq.y + (244 * yiq.i/255) + (158 * yiq.q/255),0);
    g = max(yiq.y - (69 * yiq.i/255) - (165 * yiq.q/255),0);
    b = max(yiq.y - (282 * yiq.i/255) + (434 * yiq.q/255),0);
    Serial.printf("rgb:%i %i %i>", r, g, b);
    */

    // i = -0.5957 ... 0.5957 [127/213]
    // q = -0.5226 ... 0.5226 [127/243]

    rgb.ry = min(max((yiq.y + (255 * yiq.i / 223) + (255 * yiq.q / 391)), 0), 255);
    rgb.gi = min(max((yiq.y - (255 * yiq.i / 783) - (255 * yiq.q / 375)), 0), 255);
    rgb.bq = min(max((yiq.y - (255 * yiq.i / 192) + (255 * yiq.q / 143)), 0), 255);
    
    rgb565 = (((rgb.ry & 0xF8) <<8) + ((rgb.gi & 0xFC ) <<3) + (rgb.bq >>3));
    //Serial.println(rgb565, HEX);
    rgb565 = byteswap(rgb565);    

    //Serial.println(rgb565, HEX);
    return (rgb565);
}

uint16_t byteswap(uint16_t input) {
    char hibyte,lobyte;
    
    hibyte = (input & 0xff00) >> 8;
    lobyte = (input & 0xff);
    return(lobyte << 8 | hibyte); 
}
