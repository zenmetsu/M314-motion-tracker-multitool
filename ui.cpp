#include <TFT_eSPI.h>
#include "ui.h"
#include "Picopixel.h"


static constexpr int RGB332_TO_565_LUT[256] = {
    0x0000, 0x000a, 0x0015, 0x001f, 0x0120, 0x012a, 0x0135, 0x013f, 
    0x0240, 0x024a, 0x0255, 0x025f, 0x0360, 0x036a, 0x0375, 0x037f, 
    0x0480, 0x048a, 0x0495, 0x049f, 0x05a0, 0x05aa, 0x05b5, 0x05bf, 
    0x06c0, 0x06ca, 0x06d5, 0x06df, 0x07e0, 0x07ea, 0x07f5, 0x07ff, 
    0x2000, 0x200a, 0x2015, 0x201f, 0x2120, 0x212a, 0x2135, 0x213f, 
    0x2240, 0x224a, 0x2255, 0x225f, 0x2360, 0x236a, 0x2375, 0x237f, 
    0x2480, 0x248a, 0x2495, 0x249f, 0x25a0, 0x25aa, 0x25b5, 0x25bf, 
    0x26c0, 0x26ca, 0x26d5, 0x26df, 0x27e0, 0x27ea, 0x27f5, 0x27ff, 
    0x4800, 0x480a, 0x4815, 0x481f, 0x4920, 0x492a, 0x4935, 0x493f, 
    0x4a40, 0x4a4a, 0x4a55, 0x4a5f, 0x4b60, 0x4b6a, 0x4b75, 0x4b7f, 
    0x4c80, 0x4c8a, 0x4c95, 0x4c9f, 0x4da0, 0x4daa, 0x4db5, 0x4dbf, 
    0x4ec0, 0x4eca, 0x4ed5, 0x4edf, 0x4fe0, 0x4fea, 0x4ff5, 0x4fff, 
    0x6800, 0x680a, 0x6815, 0x681f, 0x6920, 0x692a, 0x6935, 0x693f, 
    0x6a40, 0x6a4a, 0x6a55, 0x6a5f, 0x6b60, 0x6b6a, 0x6b75, 0x6b7f, 
    0x6c80, 0x6c8a, 0x6c95, 0x6c9f, 0x6da0, 0x6daa, 0x6db5, 0x6dbf, 
    0x6ec0, 0x6eca, 0x6ed5, 0x6edf, 0x6fe0, 0x6fea, 0x6ff5, 0x6fff, 
    0x9000, 0x900a, 0x9015, 0x901f, 0x9120, 0x912a, 0x9135, 0x913f, 
    0x9240, 0x924a, 0x9255, 0x925f, 0x9360, 0x936a, 0x9375, 0x937f, 
    0x9480, 0x948a, 0x9495, 0x949f, 0x95a0, 0x95aa, 0x95b5, 0x95bf, 
    0x96c0, 0x96ca, 0x96d5, 0x96df, 0x97e0, 0x97ea, 0x97f5, 0x97ff, 
    0xb000, 0xb00a, 0xb015, 0xb01f, 0xb120, 0xb12a, 0xb135, 0xb13f, 
    0xb240, 0xb24a, 0xb255, 0xb25f, 0xb360, 0xb36a, 0xb375, 0xb37f, 
    0xb480, 0xb48a, 0xb495, 0xb49f, 0xb5a0, 0xb5aa, 0xb5b5, 0xb5bf, 
    0xb6c0, 0xb6ca, 0xb6d5, 0xb6df, 0xb7e0, 0xb7ea, 0xb7f5, 0xb7ff, 
    0xd800, 0xd80a, 0xd815, 0xd81f, 0xd920, 0xd92a, 0xd935, 0xd93f, 
    0xda40, 0xda4a, 0xda55, 0xda5f, 0xdb60, 0xdb6a, 0xdb75, 0xdb7f, 
    0xdc80, 0xdc8a, 0xdc95, 0xdc9f, 0xdda0, 0xddaa, 0xddb5, 0xddbf, 
    0xdec0, 0xdeca, 0xded5, 0xdedf, 0xdfe0, 0xdfea, 0xdff5, 0xdfff, 
    0xf800, 0xf80a, 0xf815, 0xf81f, 0xf920, 0xf92a, 0xf935, 0xf93f, 
    0xfa40, 0xfa4a, 0xfa55, 0xfa5f, 0xfb60, 0xfb6a, 0xfb75, 0xfb7f, 
    0xfc80, 0xfc8a, 0xfc95, 0xfc9f, 0xfda0, 0xfdaa, 0xfdb5, 0xfdbf, 
    0xfec0, 0xfeca, 0xfed5, 0xfedf, 0xffe0, 0xffea, 0xfff5, 0xffff 
};


TFT_eSPI tft = TFT_eSPI();

TrackerDisplay::TrackerDisplay() {
  framebuffer_clear();
}

/* hardware control */
void TrackerDisplay::init() {
    tft.init();
    scan_index = 0;
    ui_current_zoom = 5.0;
}



int TrackerDisplay::get_height() {
    return tft.height();
}



int TrackerDisplay::get_width() {
    return tft.width();
}



void TrackerDisplay::set_rotation(char rotations) {
    tft.setRotation(rotations);
}



/* framebuffer control */
void TrackerDisplay::framebuffer_clear() {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        pixels_[i] = 0x00;
    }
}



void TrackerDisplay::framebuffer_put(int x, int y, char color) {
    pixels_[(WIDTH * y) + x] = color;
}



char TrackerDisplay::framebuffer_get(int x, int y) {
    return pixels_[(WIDTH * y) + x];
}



const char* TrackerDisplay::getPixels() {
    return pixels_;
}



/* graphics functions */
void TrackerDisplay::fill_screen(int color) {
    tft.fillScreen(RGB332_TO_565_LUT[color]);
}



void TrackerDisplay::put_record(int x, int y, char color) {
    int i = scanlog[scan_index].x;
    int j = scanlog[scan_index].y;
    
    tft.drawPixel(i, j, RGB332_TO_565_LUT[ framebuffer[i][j] ]);
    scanlog[scan_index].x = x;
    scanlog[scan_index].y = y;
    scanlog[scan_index].live = M314_BLUE;
    tft.drawPixel(x, y, RGB332_TO_565_LUT[M314_WHITE]);
}




void TrackerDisplay::draw_pixel(int x, int y, char color) {
    tft.drawPixel(x, y, RGB332_TO_565_LUT[color]);
}



void TrackerDisplay::draw_arc(int16_t x, int16_t y, int16_t r, float rs, float re, char color) { 
    const float degrad = 0.01745329252;
    int col,row;
    int16_t _width = tft.width(), _height = tft.height();
      
    for (float i = rs*degrad; i < re*degrad; i += (5.0/r)) {
        col = x + cos(i) * r;
        row = y + sin(i) * r;
        tft.drawPixel(col, row, RGB332_TO_565_LUT[color]);
        framebuffer[(char)col][(char)row] = color;
    }
}



/* ui functions */
void TrackerDisplay::blank_field() {
    for (int idx = scan_index; idx < sizeof(scanlog)/sizeof(scanlog[0]); idx++) {
        if ( scanlog[idx].live ) {
            int i = scanlog[idx].x;
            int j = scanlog[idx].y;
            tft.drawPixel(i, j, RGB332_TO_565_LUT[ framebuffer[i][j] ]);          
            scanlog[idx].x = 0;
            scanlog[idx].y = 0;
            scanlog[idx].live = 0x00;
        }
    }
}



void TrackerDisplay::clear_scanlog() {
  for (int idx = 0; idx < sizeof(scanlog)/sizeof(scanlog[0]); idx++) {
    scanlog[idx].x = 0;
    scanlog[idx].y = 0;
    scanlog[idx].live  = 0;
    scanlog[idx].drawn = 0;
  }
}



void TrackerDisplay::update_scantime() {
  time_last_scan_ms = millis();
}
        
        
        
void TrackerDisplay::update_zoom(float ratio) {
  ui_current_zoom *= ratio;
  framebuffer_clear();
}


void TrackerDisplay::draw_sonar_ui() {
    int x, y, w = tft.width(),h = tft.height();

    /* define origin for scanner */
    y = h - 31;
    x = w/2;

    draw_sonar_compass();

    /* draw bottom field */
    tft.fillRect(0, y-1, x-32, 31, RGB332_TO_565_LUT[M314_BLUE]);
    tft.fillRect(x+32, y-1, x-32, 31, RGB332_TO_565_LUT[M314_BLUE]);
    tft.fillRect(0, h-11, w, h, RGB332_TO_565_LUT[M314_BLUE]);
    tft.fillTriangle(x-32, y-1, x-32, h, x-20, h, RGB332_TO_565_LUT[M314_BLUE]);
    tft.fillTriangle(x+32, y-1, x+32, h, x+20, h, RGB332_TO_565_LUT[M314_BLUE]);

    /* add text to bottom field */
    tft.setTextSize(1);
    tft.setTextColor(RGB332_TO_565_LUT[M314_WHITE]);
    tft.setCursor(3, 103);
    tft.setFreeFont(&Picopixel);
    tft.print("FEMS 5.547.52");
    tft.setCursor(112, 109);
    tft.print("CX. 54/");
    tft.setFreeFont();
}



void TrackerDisplay::draw_sonar_compass() {
    int x, y, w = tft.width(), h = tft.height();
    
    /* define origin for scanner */
    y = h - 31;
    x = w/2;

    /* draw static compass arcs */
    draw_arc(x, y, 31, 180, 360, M314_LTBLUE);
    draw_arc(x, y, 32, 180, 360, M314_LTBLUE);
    draw_arc(x, y, 76, 180, 360, M314_LTBLUE);
    draw_arc(x, y, 77, 180, 360, M314_LTBLUE);    
}



void TrackerDisplay::print_sonar_scan_freq() {
    /* set background color to erase prior value */
    tft.setTextColor(RGB332_TO_565_LUT[M314_BLUE]);
    tft.setTextSize(1);   
    tft.setFreeFont(&Picopixel);

    /* perform an erase and draw cycle */
    for (int draw_cycle = 0; draw_cycle < 2; draw_cycle++) {
        tft.setCursor(112, 109);
        tft.print("CX. 54/");
        tft.print(scan_frequency, 2);
        tft.print("Hz");
        
        /* change color and update global vars */
        if ( !(draw_cycle) ) {
            tft.setTextColor(RGB332_TO_565_LUT[M314_WHITE]);
            scan_frequency = 1000.0 / (float)(millis() - time_last_scan_ms);
            time_last_scan_ms = millis();
        }
    }
}



void TrackerDisplay::print_sonar_scan_range() {
    float range = 0.001 * (200.0/ui_current_zoom) * (tft.height() / 2);
    int last_range_int, last_range_dec;

    if ( range != last_range ) {
        int range_int = floor(range);
        int range_dec = floor(100 * (range - range_int));
    
        /* set background color to erase prior value */
        tft.setTextColor(M314_BLACK);
        tft.setFreeFont();
        
        /* perform an erase and draw cycle */
        for (int draw_cycle = 0; draw_cycle < 2; draw_cycle++) {
            last_range_int = floor(last_range);
            last_range_dec = floor(100 * (last_range - last_range_int));
            tft.setCursor(56, 100);
            tft.setTextSize(2);
            
            /* pad with leading space */
            if (last_range_int < 10) {
                tft.print(" ");
            }    
            tft.print(last_range_int);
            tft.setCursor(76, 92);
            tft.print(".");
            tft.setTextSize(1);
            tft.setCursor(86, 99);
            
            /* pad with leading zero */
            if (last_range_dec < 10) {
                tft.print("0");
            }    
            tft.print(last_range_dec);
            tft.setCursor(86, 103);
            tft.setTextSize(2);
            tft.print("m");
            
            /* change color and update global vars */
            if ( !(draw_cycle) ) {
                tft.setTextColor(RGB332_TO_565_LUT[M314_RED]);
                last_range = range;
            }
        }
    }     
}  



void TrackerDisplay::update_metrics() {
    print_sonar_scan_range();
    print_sonar_scan_freq();
}



void TrackerDisplay::draw_lasernav_ui() {
    int x, y, w = tft.width(), h = tft.height();
    
    /* define center of display */
    y = h/2;
    x = w/2;

    /* draw crosshairs */
    tft.drawFastHLine(0, y, w, RGB332_TO_565_LUT[M314_ORANGE]);
    tft.drawFastVLine(x, 0, h, RGB332_TO_565_LUT[M314_ORANGE]);
}



void TrackerDisplay::switch_mode(ui_modes mode){    
    ui_mode = sonar;
    draw_sonar_ui();
    draw_sonar_compass();
    update_metrics();
    last_range = 0.001 * (200.0/ui_current_zoom) * (tft.height() / 2);
}
