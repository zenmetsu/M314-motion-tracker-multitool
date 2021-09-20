#include <TFT_eSPI.h>
#include "ui.h"
#include "analogvideo.h"
#include "Picopixel.h"


TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);  /* sprite class, used as framebuffer */
uint16_t* sprPtr0;                    /* pointer to sprite */
uint16_t* sprPtr1;                    /* pointer to sprite */


/* these hold timestamps to end given effects */
int expire_vert_scroll = 0; 
int expire_hori_scroll = 0;
int expire_hori_warp   = 0;
int expire_ringing     = 0;
int expire_chrom_dly   = 0;
int expire_chrom_noise = 0;

/* these hold parameters for analog glitches */
int glitch_vert,glitch_hori,glitch_vert_speed = 0;
int chrom_noise = 0;
int ring_level = 0;


TrackerDisplay::TrackerDisplay() {
}

/* hardware control */
void TrackerDisplay::init() {
    tft.init();
    tft.initDMA();
    tft.fillScreen(M314_BLACK);
    tft.setRotation(3);
    img.setColorDepth(COLOR_DEPTH);

    /* create sprite */
    sprPtr0 = (uint16_t*)img.createSprite(WIDTH, HEIGHT);

    /* hold CS low for maximum effect */
    tft.startWrite();
    
    //scan_index = 0;
    ui_current_zoom = 5.0;

    trigger_power_on();
    
    draw_test_card();
    tft.pushImageDMA(0, 0, tft.width(), tft.height(), sprPtr0);
    delay(1000);    
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
    img.fillSprite(M314_BLACK);
}



void TrackerDisplay::refresh() {

    /* push framebuffer using DMA */
    //tft.pushImage(0, 0, tft.width(), tft.height(), sprPtr0);
    //tft.pushImageDMA(0, 0, tft.width(), tft.height(), sprPtr1);

    /* draw a test pattern while debugging analog video routines */
    draw_test_card();

    /* trigger horizontal warp */
    if (random(1000) > 990) {
        expire_hori_warp = millis() + random(100);
    }

    /* trigger vertical scroll */
    if ((millis() > expire_vert_scroll) && (random(1000) > 998)) {
        expire_vert_scroll = millis() + random(400);
        glitch_vert = 0;
        glitch_vert_speed = random(16);
    }    

    /* do horizontal warp if true */
    if ( millis() < expire_hori_warp) {
        glitch_hori_warp(&tft, sprPtr0);

    /* do vertical scroll if true */
    } else if ( millis() < expire_vert_scroll) {
        glitch_vert+=glitch_vert_speed;
        glitch_vert_scroll(&tft, sprPtr0, glitch_vert);

    /* else be boring */
    } else {
        tft.pushImageDMA(0, 0, tft.width(), tft.height(), sprPtr0);
    }

    /* simulate a power off every minute */
    if ((millis() % 60000) < 100) {
        glitch_power_off(&tft, sprPtr0);
    }

    /* and a power on 3 seconds later */
    if ((millis()-3000 % 60000) < 200){
        trigger_power_on();
    }
}

/* graphics effects */
void TrackerDisplay::trigger_power_on() {
    draw_test_card();
    for (glitch_vert = 0;glitch_vert<60;glitch_vert++) {
        glitch_vert_scroll(&tft, sprPtr0, glitch_vert);
        glitch_vert += ((millis() % 18) / 16) + (32 * (tft.height() - (glitch_vert % tft.height())) / tft.height());

        glitch_hori_scroll(&tft, sprPtr0, glitch_vert);
        glitch_vert += 2;      
    }    
}



/* graphics functions */
void TrackerDisplay::put_record(int x, int y, int color) {
    img.drawPixel(x, y, color);
}



void TrackerDisplay::draw_arc(int16_t x, int16_t y, int16_t r, float rs, float re, int color) { 
    const float degrad = 0.01745329252;
    int col,row;
    int16_t _width = tft.width(), _height = tft.height();
      
    for (float i = rs*degrad; i < re*degrad; i += (0.50/r)) {
        col = x + cos(i) * r;
        row = y + sin(i) * r;
        img.drawPixel(col, row, color);
    }
}



/* ui functions */
void TrackerDisplay::update_scantime() {
  time_last_scan_ms = millis();
}
        
        
        
void TrackerDisplay::update_zoom(float ratio) {
  ui_current_zoom *= ratio;
  //framebuffer_clear();
}


void TrackerDisplay::draw_test_card() {
  int column,row = 0;
  img.fillRect(0 + (23*column), 0, 23, 85, 0xC618);
  column++;
  img.fillRect(0 + (23*column), 0, 23, 85, 0xC600);
  column++;
  img.fillRect(0 + (23*column), 0, 23, 85, 0x0618);
  column++;
  img.fillRect(0 + (23*column), 0, 23, 85, 0x0600);
  column++;
  img.fillRect(0 + (23*column), 0, 23, 85, 0xC018);
  column++;
  img.fillRect(0 + (23*column), 0, 23, 85, 0xC000);
  column++;
  img.fillRect(0 + (23*column), 0, 22, 85, 0x0018);
  column = 0;
  
  img.fillRect(0 + (23*column), 85, 23, 11, 0x0018);
  column++;
  img.fillRect(0 + (23*column), 85, 23, 11, 0x1082);
  column++;
  img.fillRect(0 + (23*column), 85, 23, 11, 0xC018);
  column++;
  img.fillRect(0 + (23*column), 85, 23, 11, 0x1082);
  column++;
  img.fillRect(0 + (23*column), 85, 23, 11, 0x0618);
  column++;
  img.fillRect(0 + (23*column), 85, 23, 11, 0x1082);
  column++;
  img.fillRect(0 + (23*column), 85, 22, 11, 0xc618);
  column = 0;

  img.fillRect(0 + (29*column), 96, 29, 31, 0x0109);
  column++;
  img.fillRect(0 + (29*column), 96, 29, 31, 0xFFFF);
  column++;
  img.fillRect(0 + (29*column), 96, 29, 31, 0x300D);
  column++;
  img.fillRect(0 + (29*column), 96, 29, 31, 0x1082);
  column++;

  img.fillRect(114 , 96, 8, 31, 0x0841);
  img.fillRect(114 + 8, 96, 8, 31, 0x1082);
  img.fillRect(114 + 16, 96, 7, 31, 0x18E3);
  img.fillRect(114 + 23, 96, 23, 31, 0x1082);
}


void TrackerDisplay::draw_sonar_ui() {
    int x, y, w = tft.width(), h = tft.height();

    /* clear the screen */
    framebuffer_clear();

    /* define origin for scanner */
    y = h - 31;
    x = w/2;

    /* draw compass rings */
    draw_sonar_compass();

    /* draw bottom field */
    img.fillRect(0, y-1, x-32, 31, M314_BLUE);
    img.fillRect(x+30, y-1, x-30, 31, M314_BLUE);
    img.fillRect(0, h-11, w, h, M314_BLUE);
    img.fillTriangle(x-32, y-1, x-32, h, x-20, h, M314_BLUE);
    img.fillTriangle(x+30, y-1, x+32, h, x+20, h, M314_BLUE);

    /* add text to bottom field */
    img.setTextSize(1);
    img.setTextColor(M314_WHITE);
    img.setCursor(3, 109);
    img.setFreeFont(&Picopixel);
    img.print("FEMS 5.547.52");
    img.setCursor(112, 109);
    img.print("CX. 54/");
    img.setFreeFont();

    /* update text metrics */
    update_metrics();
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
    img.setTextColor(M314_BLUE);
    img.setTextSize(1);   
    img.setFreeFont(&Picopixel);

    /* perform an erase and draw cycle */
    for (int draw_cycle = 0; draw_cycle < 2; draw_cycle++) {
        img.setCursor(112, 109);
        img.print("CX. 54/");
        img.print(scan_frequency, 2);
        img.print("Hz");
        
        /* change color and update global vars */
        if ( !(draw_cycle) ) {
            img.setTextColor(M314_WHITE);
            scan_frequency = 1000.0 / (float)(millis() - time_last_scan_ms);
            time_last_scan_ms = millis();
        }
    }
}



void TrackerDisplay::print_sonar_scan_range() {
    float range = 0.001 * (200.0/ui_current_zoom) * (tft.height() / 2);
    int last_range_int, last_range_dec;

        int range_int = floor(range);
        int range_dec = floor(100 * (range - range_int));
    
        img.setTextColor(M314_RED);
        img.setFreeFont();
        last_range_int = floor(last_range);
        last_range_dec = floor(100 * (last_range - last_range_int));
        img.setCursor(56, 100);
        img.setTextSize(2);
            
        /* pad with leading space */
        if (last_range_int < 10) {
            img.print(" ");
        }    
        img.print(last_range_int);
        img.setCursor(76, 92);
        img.print(".");
        img.setTextSize(1);
        img.setCursor(86, 99);
            
        /* pad with leading zero */
        if (last_range_dec < 10) {
            img.print("0");
        }    
        img.print(last_range_dec);
        img.setCursor(86, 103);
        img.setTextSize(2);
        img.print("m");
        last_range = range;   
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
    tft.drawFastHLine(0, y, w, M314_ORANGE);
    tft.drawFastVLine(x, 0, h, M314_ORANGE);
}



void TrackerDisplay::switch_mode(ui_modes mode){
    /*  
    ui_mode = sonar;
    draw_sonar_ui();
    update_metrics();
    last_range = 0.001 * (200.0/ui_current_zoom) * (tft.height() / 2);
    */
    draw_test_card();
}
