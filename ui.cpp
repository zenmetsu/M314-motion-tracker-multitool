#include <TFT_eSPI.h>
#include "ui.h"
#include "analogvideo.h"
#include "Picopixel.h"
#include "Font4x7Fixed.h"
#include "Orbitron_Medium_20.h"
#include "Orbitron_Medium_8.h"
#include "Orbitron_Medium_7.h"
#include "coordinates.h"


TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);  /* sprite class, used as framebuffer */
//TFT_eSprite fbb = TFT_eSprite(&tft);  /* framebuffer backup */

uint16_t* sprPtr0;                    /* pointer to sprite */
//uint16_t* fbbPtr; 


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

const char * SysSubmenu[] = { "CALIBRATE", "EQUALIZE", "TRACK" };
const char * SpectralSubmenu[] = { "BIO", "INERT" };
const char * IFFSubmenu[] = { "SEARCH", "TEST", "ENGAGED", "INTER-TE" };
const char * TargetSubmenu[] = { "SOFT", "MEDIUM", "HARD" };

TrackerDisplay::TrackerDisplay() {
}

/* hardware control */
void TrackerDisplay::init() {
    tft.init();
    tft.initDMA();
    tft.fillScreen(M314_BLACK);
    tft.setRotation(3);
    img.setColorDepth(COLOR_DEPTH);
//    fbb.setColorDepth(COLOR_DEPTH);

    /* create sprite */
    sprPtr0 = (uint16_t*)img.createSprite(WIDTH, HEIGHT);
//    fbbPtr = (uint16_t*)img.createSprite(WIDTH, HEIGHT);

    /* hold CS low for maximum effect */
    tft.startWrite();
    
    //scan_index = 0;
    ui_current_zoom = 5.0;

    /* clear background */
    img.fillRect(0, 0, img.width(), img.height(), M314_BLACK);
    tft.pushImage(0, 0, tft.width(), tft.height(), sprPtr0);

    trigger_power_on();
    
    img.fillRect(0, 0, img.width(), img.height(), M314_BLACK);
    tft.pushImage(0, 0, tft.width(), tft.height(), sprPtr0);
    
    hold(80);   
    draw_company_logo();
    boot_up();
    hold(1800);
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
    
    /* trigger horizontal warp */
    if ((millis() > expire_hori_warp) && (random(1000) > 940))  {
        expire_hori_warp = millis() + random(40);
    }

    /* trigger vertical scroll */
    if ((millis() > expire_vert_scroll) && (random(1000) > 988)) {
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
        uint16_t fb_temp[HEIGHT * WIDTH];
        memcpy(fb_temp, sprPtr0, HEIGHT*WIDTH*sizeof(fb_temp[0]));
        glitch_ringing(fb_temp, WIDTH, HEIGHT, 64, 3);

        glitch_chrom_noise(&tft, fb_temp, analogRead(A4)*analogRead(A5)/300000.0);
    }

    /* simulate a power off every minute for debugging*/
    if ((millis() % 60000) < 100) {
        glitch_power_off(&tft, sprPtr0);
    }

    /* and a power on 3 seconds later */
    if ((24000 < millis()) && (millis()-3000 % 60000) < 200){
        trigger_power_on();
    }
}



void TrackerDisplay::hold(int duration) {
  for (int count = 0; count < duration / 3; count++) {
    refresh();
  }
}


/* graphics effects */
void TrackerDisplay::trigger_power_on() {
    Serial.println("power_on");
    draw_test_card();
    for (glitch_vert = 0;glitch_vert<120;glitch_vert++) {
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



/* draw an NTSC test pattern */
/* NTSC = Never Twice Same Color XD */
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

void TrackerDisplay::draw_company_logo() {
    int x, y, w = tft.width(), h = tft.height();
    float rotation_angle_rad;     /* zero is right, PI/2 is down */ 
    int rot_throttle = 30;         /* logo animation delay */
    const float UP_ANGLE = -PI/2;
    const char LOGO_HEIGHT = 32;
    const char LOGO_THICC = 18;
    const int LOGO_TOP = (tft.height() - LOGO_HEIGHT) / 2;
    const int LOGO_BOT = 1 + (tft.height() + LOGO_HEIGHT) / 2;
  
    img.fillSprite(M314_BLACK);
    Coordinates point0, point1, point2, point3, point4, point5 = Coordinates();

    point0.setCartesian((w / 2) - (LOGO_THICC / 2) - 1, LOGO_BOT);
    point1.setCartesian((w / 2) + (LOGO_THICC / 2), LOGO_BOT);

    /* draw the Weyland "W" procedurally... */

    /* draw the initial "V", starting with a vertical rectangle */
    for (rotation_angle_rad = 0; rotation_angle_rad <= (PI/4 + PI/64); rotation_angle_rad += PI/128) {
        /* clear background */
        img.fillRect(0, 0, img.width(), img.height(), M314_BLACK);

        /* draw initial vertical block */
        point2.fromPolar(LOGO_HEIGHT+8,UP_ANGLE + rotation_angle_rad, point0.getX(), point0.getY());
        point3.fromPolar(LOGO_HEIGHT+8,UP_ANGLE + rotation_angle_rad, point1.getX(), point1.getY());
        point4.fromPolar(LOGO_HEIGHT+8,UP_ANGLE - rotation_angle_rad + 0.01, point0.getX(), point0.getY());
        point5.fromPolar(LOGO_HEIGHT+8,UP_ANGLE - rotation_angle_rad + 0.01, point1.getX(), point1.getY());
        
        img.fillTriangle(point0.getX(), point0.getY(), point1.getX(), point1.getY(), point2.getX(), point2.getY(), WY_ORANGE);
        img.fillTriangle(point1.getX(), point1.getY(), point2.getX(), point2.getY(), point3.getX(), point3.getY(), WY_ORANGE);
        img.fillTriangle(point0.getX() + 1, point0.getY(), point1.getX(), point1.getY(), point4.getX() + 1, point4.getY(), WY_ORANGE);
        img.fillTriangle(point1.getX(), point1.getY(), point4.getX(), point4.getY(), point5.getX(), point5.getY(), WY_ORANGE);
        
        /* crop top and bottom */
        img.fillRect(0, 0, w, LOGO_TOP+5, M314_BLACK);
        img.fillRect(0, LOGO_BOT + 2, w, LOGO_TOP, M314_BLACK);

        /* crop sides */
        img.fillRect(0, 0, 46, img.height(), M314_BLACK);
        img.fillRect(img.width() - 46, 0, 46, img.height(), M314_BLACK);

        /* perform an atomic screen update */
        refresh();
        if ( !rotation_angle_rad ) {
            for (int loop = 0; loop < 32; loop++) {
                refresh();
            }
        } else {
            hold(rot_throttle);
            rot_throttle *= 0.92;
        }
    }

    for (int loop = 0; loop < 24; loop++) {
      refresh();
    }
    rot_throttle = 4;

    /* clone the "V" and separate the two clones to form a "W" */
    for (int translation = 0; translation < 29; translation++) {
         /* clear background */
        img.fillRect(0, 0, img.width(), img.height(), M314_BLACK);

        /* right clone */
        img.fillTriangle(point0.getX() + translation, point0.getY(), point1.getX() + translation, point1.getY(), point2.getX() + translation, point2.getY(), WY_ORANGE);
        img.fillTriangle(point1.getX() + translation, point1.getY(), point2.getX() + translation, point2.getY(), point3.getX() + translation, point3.getY(), WY_ORANGE);
        img.fillTriangle(point0.getX() + translation, point0.getY(), point1.getX() + translation, point1.getY(), point4.getX() + translation, point4.getY(), WY_ORANGE);
        img.fillTriangle(point1.getX() + translation, point1.getY(), point4.getX() + translation, point4.getY(), point5.getX() + translation, point5.getY(), WY_ORANGE);
        img.fillTriangle(point4.getX() + translation, point4.getY(), point4.getX() + translation, point4.getY() + 14, point4.getX() + translation + 14, point4.getY(), M314_BLACK);
        img.drawPixel(point0.getX() + translation - 1, point0.getY(), WY_ORANGE);
        /* left clone */
        img.fillTriangle(point0.getX() - translation + 2, point0.getY(), point1.getX() - translation + 2, point1.getY(), point2.getX() - translation + 2, point2.getY(), WY_ORANGE);
        img.fillTriangle(point1.getX() - translation + 2, point1.getY(), point2.getX() - translation + 2, point2.getY(), point3.getX() - translation + 2, point3.getY(), WY_ORANGE);
        img.fillTriangle(point0.getX() - translation + 2, point0.getY(), point1.getX() - translation + 2, point1.getY(), point4.getX() - translation + 2, point4.getY(), WY_ORANGE);
        img.fillTriangle(point1.getX() - translation + 2, point1.getY(), point4.getX() - translation + 2, point4.getY(), point5.getX() - translation + 2, point5.getY(), WY_ORANGE);
        img.drawPixel(point0.getX() - translation + 1, point0.getY(), WY_ORANGE);
        if (22 > translation) {
            img.fillTriangle(point3.getX() - translation + 2, point3.getY(), point3.getX() - translation - 3, point3.getY() + 9, point3.getX() - translation - 14 + 2, point3.getY(), M314_BLACK);
            img.fillTriangle(point0.getX() + translation + 2, point0.getY(), point1.getX() + translation + 2, point1.getY(), point2.getX() + translation + 2, point2.getY(), WY_ORANGE);
            img.fillTriangle(point1.getX() + translation + 2, point1.getY(), point2.getX() + translation + 2, point2.getY(), point3.getX() + translation + 2, point3.getY(), WY_ORANGE);
        } else {
            img.fillTriangle(point5.getX() + 15 + translation, point5.getY(), point5.getX() + 15 + translation, point5.getY() + 14, point5.getX() + 15 + translation - 14, point5.getY(), M314_BLACK);
        }
        /* crop top and bottom */
        img.fillRect(0, 0, w, LOGO_TOP, M314_BLACK);
        img.fillRect(0, LOGO_BOT + 1, w, LOGO_TOP, M314_BLACK);

        /* crop sides */
        img.fillRect(0, 0, (46 - translation), img.height(), M314_BLACK);
        img.fillRect(img.width() - (46 - translation), 0, (46 - translation), img.height(), M314_BLACK); 

        /* perform an atomic screen update */
        refresh();
        hold(rot_throttle);
        rot_throttle *= 1.11;
    }
    for (int loop = 0; loop < 24; loop++) {
      refresh();
    }
    
    rot_throttle = 50;

    /* draw Yutani chevrons */
    for (int offset = -15; offset < 1; offset++) {
        int translation = 28;

        img.fillRect(0, 0, img.width(), img.height(), M314_BLACK);
        img.fillTriangle(80, 69 + offset, 71, 78 + offset, 89, 78 + offset, M314_WHITE);
        img.fillRect(71, 78 + offset, 19, 4, M314_WHITE);

        img.fillTriangle (53, 66 - offset, 44, 57 - offset, 62, 57 - offset, M314_WHITE); 
        img.fillRect(44, 53 - offset, 16, 4, M314_WHITE);
        img.fillTriangle(60, 53 - offset, 60, 56 - offset, 63, 56 - offset, M314_WHITE);

        img.fillTriangle (107, 66 - offset, 98, 57 - offset, 116, 57 - offset, M314_WHITE);
        img.fillRect(101, 53 - offset, 16, 4, M314_WHITE);
        img.fillTriangle(97, 56 - offset, 100, 56 - offset, 100, 53 - offset, M314_WHITE);

        /* redraw Weyland "W" */
        /* right clone */
        img.fillTriangle(point0.getX() + translation, point0.getY(), point1.getX() + translation, point1.getY(), point2.getX() + translation, point2.getY(), WY_ORANGE);
        img.fillTriangle(point1.getX() + translation, point1.getY(), point2.getX() + translation, point2.getY(), point3.getX() + translation, point3.getY(), WY_ORANGE);
        img.fillTriangle(point0.getX() + translation, point0.getY(), point1.getX() + translation, point1.getY(), point4.getX() + translation, point4.getY(), WY_ORANGE);
        img.fillTriangle(point1.getX() + translation, point1.getY(), point4.getX() + translation, point4.getY(), point5.getX() + translation, point5.getY(), WY_ORANGE);
        img.fillTriangle(point4.getX() + translation, point4.getY(), point4.getX() + translation, point4.getY() + 14, point4.getX() + translation + 14, point4.getY(), M314_BLACK);
        img.drawPixel(point0.getX() + translation - 1, point0.getY(), WY_ORANGE);
        /* left clone */
        img.fillTriangle(point0.getX() - translation + 2, point0.getY(), point1.getX() - translation + 2, point1.getY(), point2.getX() - translation + 2, point2.getY(), WY_ORANGE);
        img.fillTriangle(point1.getX() - translation + 2, point1.getY(), point2.getX() - translation + 2, point2.getY(), point3.getX() - translation + 2, point3.getY(), WY_ORANGE);
        img.fillTriangle(point0.getX() - translation + 2, point0.getY(), point1.getX() - translation + 2, point1.getY(), point4.getX() - translation + 2, point4.getY(), WY_ORANGE);
        img.fillTriangle(point1.getX() - translation + 2, point1.getY(), point4.getX() - translation + 2, point4.getY(), point5.getX() - translation + 2, point5.getY(), WY_ORANGE);
        img.drawPixel(point0.getX() - translation + 1, point0.getY(), WY_ORANGE);
        img.fillTriangle(point5.getX() + translation + 5, point5.getY(), point5.getX() + translation + 5, point5.getY() + 4, point5.getX() + translation - 4 + 5, point5.getY(), M314_BLACK);
        /* crop top and bottom */
        img.fillRect(0, 0, w, LOGO_TOP, M314_BLACK);
        img.fillRect(0, LOGO_BOT + 1, w, LOGO_TOP, M314_BLACK);

        /* crop sides */
        img.fillRect(0, 0, (46 - translation), img.height(), M314_BLACK);
        img.fillRect(img.width() - (46 - translation), 0, (46 - translation), img.height(), M314_BLACK);         

        /* perform an atomic screen update */
        refresh();
        hold(rot_throttle);
        rot_throttle *= .80;
    }

    /* add text */
    img.setTextSize(1);
    img.setTextColor(M314_WHITE);
    img.setCursor(13, 49);
    //img.setFreeFont(&Picopixel);
    img.setFreeFont(&Orbitron_Medium_8);
    img.print("WEYLAND-YUTANI CORP"); 

    img.setFreeFont(&Orbitron_Medium_7);
    img.setTextSize(1);
    img.setCursor(14, 90);
    img.print("BUILDING BETTER WORLDS"); 
      
    for (int count = 0; count < 256; count++) {
      refresh();
    }
}



void TrackerDisplay::draw_menu_bg(){
    img.fillRect(0, 0, img.width(), img.height(), M314_BLUE);
    img.fillRect(1, 38, 35, 17, M314_BLACK);
    img.fillRect(39, 38, 39, 17, M314_BLACK);
    img.fillRect(81, 38, 39, 17, M314_BLACK);
    img.fillRect(123, 38, 36, 17, M314_BLACK);

    img.fillRect(1, 58, 35, 33, M314_BLACK);
    img.fillRect(39, 58, 39, 33, M314_BLACK);
    img.fillRect(81, 58, 39, 33, M314_BLACK);
    img.fillRect(123, 58, 36, 33, M314_BLACK);

    img.fillRect(1, 94, 158, 33, M314_BLACK);  
}




void TrackerDisplay::draw_menu_uscm_logo(){
    int xpos, ypos;
    int stars_x[4] = {17, 15, 29, 31};
    int stars_y[4] = {13, 18, 13, 18};  

    /* draw circular field */
    img.fillCircle(23, 18, 17, WY_ORANGE);
    img.fillCircle(23, 18, 15, M314_NAVY);
    img.drawCircle(23, 18, 12, WY_ORANGE);

    /* draw stars */
    for (int star_num=0; star_num < 4; star_num++) {
        xpos = stars_x[star_num];
        ypos = stars_y[star_num];
        img.drawPixel(xpos  , ypos  , M314_LTGREY);
        img.drawPixel(xpos  , ypos-1, M314_LTGREY);
        img.drawPixel(xpos  , ypos+1, M314_LTGREY);
        img.drawPixel(xpos-1, ypos+1, M314_LTGREY);
        img.drawPixel(xpos+1, ypos+1, M314_LTGREY);
        img.drawPixel(xpos-1, ypos+2, M314_LTGREY);
        img.drawPixel(xpos+1, ypos+2, M314_LTGREY);
        img.drawPixel(xpos-2, ypos  , M314_LTGREY);
        img.drawPixel(xpos-1, ypos  , M314_LTGREY);
        img.drawPixel(xpos+1, ypos  , M314_LTGREY);
        img.drawPixel(xpos+2, ypos  , M314_LTGREY);
    }  

    /* draw stripes */
    img.drawFastVLine(20, 7, 15, M314_LTGREY);
    img.drawFastVLine(21, 7, 15, M314_RED);
    img.drawFastVLine(22, 7, 15, M314_LTGREY);
    img.drawFastVLine(23, 7, 15, M314_RED);
    img.drawFastVLine(24, 7, 15, M314_LTGREY);
    img.drawFastVLine(25, 7, 15, M314_RED);
    img.drawFastVLine(26, 7, 15, M314_LTGREY);

    img.drawFastVLine(19, 22, 4, M314_LTGREY);
    img.drawFastVLine(20, 22, 5, M314_RED);
    img.drawFastVLine(21, 22, 8, M314_LTGREY);
    img.fillRect(22, 22, 3, 8, M314_RED);
    img.drawFastVLine(25, 22, 8, M314_LTGREY);
    img.drawFastVLine(26, 22, 5, M314_RED);
    img.drawFastVLine(27, 22, 4, M314_LTGREY);

    img.fillRect(16, 27, 2, 1, M314_LTGREY);
    img.fillRect(18, 27, 2, 2, M314_RED);
    img.fillRect(20, 27, 2, 3, M314_LTGREY);
    img.fillRect(25, 27, 2, 3, M314_LTGREY);
    img.fillRect(27, 27, 2, 2, M314_RED);
    img.fillRect(29, 27, 2, 1, M314_LTGREY);

    img.drawPixel(18, 25, M314_LTGREY);
    img.drawPixel(28, 25, M314_LTGREY);
    img.drawPixel(17, 26, M314_LTGREY);
    img.drawPixel(18, 26, M314_LTGREY);  
    img.drawPixel(28, 26, M314_LTGREY);
    img.drawPixel(29, 26, M314_LTGREY);
    img.drawPixel(19, 26, M314_RED);
    img.drawPixel(27, 26, M314_RED);    
}



void TrackerDisplay::pop_menu_text() {
    int xpos, ypos;

    img.setTextDatum(TC_DATUM);
    img.setTextSize(1);
    img.setTextColor(0xC618);
    img.setFreeFont(&Orbitron_Medium_20);
    img.drawString("U.S.C.M.", 100, 6, 1);

    xpos = 18;
    ypos = 40;
    img.setTextSize(1);
    img.setTextColor(M314_AMBER);
    img.setFreeFont(&Picopixel);
    img.drawString("SYSTEM", xpos, ypos, 1);
    xpos = 59;
    img.drawString("SPECTRAL", xpos, ypos, 1);
    xpos = 100;
    img.drawString("IFF", xpos, ypos, 1); 
    xpos = 141;
    img.drawString("TARGET", xpos, ypos, 1);

    xpos = 18;
    ypos += tft.fontHeight(1);
    img.drawString("MODE", xpos, ypos, 1);
    xpos = 59;
    img.drawString("PROFILE", xpos, ypos, 1);
    xpos = 100;
    img.drawString("PROFILE", xpos, ypos, 1); 
    xpos = 141;
    img.drawString("PROFILE", xpos, ypos, 1);    

    xpos = 18;
    ypos = 60;
    img.drawString("CALIBRATE", xpos, ypos, 1);
    xpos = 59;
    img.drawString("BIO", xpos, ypos, 1);
    xpos = 100;
    img.drawString("SEARCH", xpos, ypos, 1); 
    xpos = 141;
    img.drawString("SOFT", xpos, ypos, 1); 

    xpos = 18;
    ypos += tft.fontHeight(1);
    img.drawString("EQUALIZE", xpos, ypos, 1);
    xpos = 59;
    img.drawString("INERT", xpos, ypos, 1);
    xpos = 100;
    img.drawString("TEST", xpos, ypos, 1); 
    xpos = 141;
    img.drawString("MEDIUM", xpos, ypos, 1); 

    xpos = 18;
    ypos += tft.fontHeight(1);
    img.drawString("TRACK", xpos, ypos, 1);
    xpos = 59;
    img.drawString("", xpos, ypos, 1);
    xpos = 100;
    img.drawString("ENGAGED", xpos, ypos, 1); 
    xpos = 141;
    img.drawString("HARD", xpos, ypos, 1);   

    xpos = 18;
    ypos += tft.fontHeight(1);
    img.drawString("", xpos, ypos, 1);
    xpos = 59;
    img.drawString("", xpos, ypos, 1);
    xpos = 100;
    img.drawString("INTER-TE", xpos, ypos, 1); 
    xpos = 141;
    img.drawString("", xpos, ypos, 1);  


    /*  TODO
     *  add sprite for the lower panel
     *  and add scrolling status update
     *  text as system loads, ending
     *  with system name and operator info
     */
    img.fillRect(1, 94, 158, 33, M314_BLACK);
    img.setFreeFont(&Font4x7Fixed);
    img.setCursor(7,105);
    img.print("USCM - M314 MOBILE TRACKING UNIT");
    
    img.setCursor(7, 114);
    img.setFreeFont(&Picopixel);
    img.print("OPERATOR: HICKS,D. - A27/TQ4.0.48215E9");     
}



void TrackerDisplay::spawn_menu_text() {
    char sel;
    int xpos, ypos;
    int hold1 = 20;
    int hold2 = 6;
    

    img.setTextDatum(TC_DATUM);
    img.setTextSize(1);
    img.setTextColor(0xC618);
    img.setFreeFont(&Orbitron_Medium_20);
    img.drawString("U.S.C.M.", 100, 6, 1);

    img.setTextColor(M314_AMBER);
    img.setFreeFont(&Font4x7Fixed);
    img.setCursor(7,105);
    img.print("AUTOCONFIG  IN  PROGRESS...");

    xpos = 18;
    ypos = 40;
    img.setTextSize(1);
    img.setTextColor(M314_AMBER);
    img.setFreeFont(&Picopixel);
    img.drawString("SYSTEM", xpos, ypos, 1);
    hold(hold1);
    ypos += tft.fontHeight(1);
    img.drawString("MODE", xpos, ypos, 1);
    hold(hold1);
    
    ypos = 40;
    xpos = 59;
    img.drawString("SPECTRAL", xpos, ypos, 1);
    hold(hold1);
    ypos += tft.fontHeight(1);
    img.drawString("PROFILE", xpos, ypos, 1);
    hold(hold1);

    ypos = 40;
    xpos = 100;
    img.drawString("IFF", xpos, ypos, 1);
    hold(hold1);
    ypos += tft.fontHeight(1);
    img.drawString("PROFILE", xpos, ypos, 1);
    hold(hold1);

    ypos = 40;
    xpos = 141;
    img.drawString("TARGET", xpos, ypos, 1);
    hold(hold1);
    ypos += tft.fontHeight(1);
    img.drawString("PROFILE", xpos, ypos, 1);
    hold(hold1);



    /* menu boxes for reference
    img.fillRect(1, 38, 35, 17, M314_BLACK);
    img.fillRect(39, 38, 39, 17, M314_BLACK);
    img.fillRect(81, 38, 39, 17, M314_BLACK);
    img.fillRect(123, 38, 36, 17, M314_BLACK);

    img.fillRect(1, 58, 35, 33, M314_BLACK);
    img.fillRect(39, 58, 39, 33, M314_BLACK);
    img.fillRect(81, 58, 39, 33, M314_BLACK);
    img.fillRect(123, 58, 36, 33, M314_BLACK);
    */

    /* populate system menu */
    img.fillRect(1, 38, 35, 17, M314_AMBER);
    xpos = 18;
    ypos = 40;
    img.setTextColor(M314_BLACK);
    img.drawString("SYSTEM", xpos, ypos, 1);
    ypos += tft.fontHeight(1);
    img.drawString("MODE", xpos, ypos, 1);
    hold(hold1);


    /* system submenu */
    ypos = 60;
    xpos = 18;
    img.setTextColor(M314_AMBER);
    for (char entry = 0; entry < sizeof(SysSubmenu)/sizeof(SysSubmenu[0]); entry++) {
        img.drawString(SysSubmenu[entry], xpos, ypos, 1);
        hold(hold2);
        ypos += tft.fontHeight(1);
    }    

    /* cycle through system menu */
    ypos = 60;
    sel = 2;
    for (char entry = 0; entry < min((char)(sizeof(SysSubmenu)/sizeof(SysSubmenu[0])), (char)(sel+1)); entry++) {
        img.setTextColor(M314_BLACK);
        img.fillRect(1, ypos-2, 35, tft.fontHeight(1), M314_AMBER);
        img.drawString(SysSubmenu[entry], xpos, ypos, 1);
        hold(hold1);
        Serial.printf("%i, %i\r\n", entry, sizeof(SysSubmenu)/sizeof(SysSubmenu[0]));
        if ( entry != sel ) {
            img.setTextColor(M314_AMBER);
            img.fillRect(1, ypos-2, 35, tft.fontHeight(1), M314_BLACK);
            img.drawString(SysSubmenu[entry], xpos, ypos, 1);
            ypos += tft.fontHeight(1);
        }
    }

    /* spectral submenu */
    img.fillRect(1, 38, 35, 17, M314_BLACK);
    xpos = 18;
    ypos = 40;
    img.setTextColor(M314_AMBER);
    img.drawString("SYSTEM", xpos, ypos, 1);
    ypos += tft.fontHeight(1);
    img.drawString("MODE", xpos, ypos, 1);
    img.fillRect(39, 38, 39, 17, M314_AMBER);
    xpos = 59;
    ypos = 40;
    img.setTextColor(M314_BLACK);
    img.drawString("SPECTRAL", xpos, ypos, 1);
    ypos += tft.fontHeight(1);
    img.drawString("MODE", xpos, ypos, 1);    
    
    ypos = 60;
    xpos = 59;
    img.setTextColor(M314_AMBER);
    for (char entry = 0; entry < sizeof(SpectralSubmenu)/sizeof(SpectralSubmenu[0]); entry++) {
        img.drawString(SpectralSubmenu[entry], xpos, ypos, 1);
        hold(hold2);
        ypos += tft.fontHeight(1);
    }

    /* cycle through spectral menu */
    ypos = 60; 
    sel = 0;
    for (char entry = 0; entry < min((char)(sizeof(SpectralSubmenu)/sizeof(SpectralSubmenu[0])), (char)(sel+1)); entry++) {
        img.setTextColor(M314_BLACK);
        img.fillRect(39, ypos-2, 39, tft.fontHeight(1), M314_AMBER);
        img.drawString(SpectralSubmenu[entry], xpos, ypos, 1);
        hold(hold1);
        if ( entry != sel ) {
            img.setTextColor(M314_AMBER);
            img.fillRect(39, ypos-2, 39, tft.fontHeight(1), M314_BLACK);
            img.drawString(SpectralSubmenu[entry], xpos, ypos, 1);
            ypos += tft.fontHeight(1);
        }
    }

    /* IFF submenu */
    img.fillRect(39, 38, 39, 17, M314_BLACK);
    xpos = 59;
    ypos = 40;
    img.setTextColor(M314_AMBER);
    img.drawString("SPECTRAL", xpos, ypos, 1);
    ypos += tft.fontHeight(1);
    img.drawString("MODE", xpos, ypos, 1);
    img.fillRect(81, 38, 39, 17, M314_AMBER);
    xpos = 100;
    ypos = 40;
    img.setTextColor(M314_BLACK);
    img.drawString("IFF", xpos, ypos, 1);
    ypos += tft.fontHeight(1);
    img.drawString("MODE", xpos, ypos, 1); 
    
    ypos = 60;
    xpos = 100;
    img.setTextColor(M314_AMBER);
    for (char entry = 0; entry < sizeof(IFFSubmenu)/sizeof(IFFSubmenu[0]); entry++) {
        img.drawString(IFFSubmenu[entry], xpos, ypos, 1);
        hold(hold2);
        ypos += tft.fontHeight(1);
    }

    /* cycle through IFF menu */
    ypos = 60;
    sel = 0; 
    for (char entry = 0; entry < min((char)(sizeof(IFFSubmenu)/sizeof(IFFSubmenu[0])), (char)(sel+1)); entry++) {
        img.setTextColor(M314_BLACK);
        img.fillRect(81, ypos-2, 39, tft.fontHeight(1), M314_AMBER);
        img.drawString(IFFSubmenu[entry], xpos, ypos, 1);
        hold(hold1);
        if ( entry != sel ) {
            img.setTextColor(M314_AMBER);
            img.fillRect(81, ypos-2, 39, tft.fontHeight(1), M314_BLACK);
            img.drawString(IFFSubmenu[entry], xpos, ypos, 1);
            ypos += tft.fontHeight(1);
        }
    }

    /* target submenu */
    img.fillRect(81, 38, 39, 17, M314_BLACK);
    xpos = 100;
    ypos = 40;
    img.setTextColor(M314_AMBER);
    img.drawString("IFF", xpos, ypos, 1);
    ypos += tft.fontHeight(1);
    img.drawString("MODE", xpos, ypos, 1);
    img.fillRect(123, 38, 36, 17, M314_AMBER);
    xpos = 141;
    ypos = 40;
    img.setTextColor(M314_BLACK);
    img.drawString("TARGET", xpos, ypos, 1);
    ypos += tft.fontHeight(1);
    img.drawString("MODE", xpos, ypos, 1); 
    
    ypos = 60;
    xpos = 140;
    img.setTextColor(M314_AMBER);
    for (char entry = 0; entry < sizeof(TargetSubmenu)/sizeof(TargetSubmenu[0]); entry++) {
        img.drawString(TargetSubmenu[entry], xpos, ypos, 1);
        hold(hold2);
        ypos += tft.fontHeight(1);
    }

    /* cycle through target menu */
    ypos = 60;
    sel = 1;
    for (char entry = 0; entry < min((char)(sizeof(TargetSubmenu)/sizeof(TargetSubmenu[0])), (char)(sel+1)); entry++) {
        img.setTextColor(M314_BLACK);
        img.fillRect(123, ypos-2, 36, tft.fontHeight(1), M314_AMBER);
        img.drawString(TargetSubmenu[entry], xpos, ypos, 1);
        hold(hold1);
        if ( entry != sel ) {
            img.setTextColor(M314_AMBER);
            img.fillRect(123, ypos-2, 36, tft.fontHeight(1), M314_BLACK);
            img.drawString(TargetSubmenu[entry], xpos, ypos, 1);
            ypos += tft.fontHeight(1);
        }
    }      

    
    /*  TODO
     *  add sprite for the lower panel
     *  and add scrolling status update
     *  text as system loads, ending
     *  with system name and operator info
     */
    img.fillRect(1, 94, 158, 33, M314_BLACK);
    img.setTextColor(M314_AMBER);
    img.setFreeFont(&Font4x7Fixed);
    img.setCursor(7,105);
    img.print("USCM - M314 MOBILE TRACKING UNIT");
    
    img.setCursor(7, 114);
    img.setFreeFont(&Picopixel);
    img.print("OPERATOR: HICKS,D. - A27/TQ4.0.48215E9");     
}



void TrackerDisplay::boot_up() {
    int xpos = 18;
    int ypos = 40;
    int stars_x[4] = {17, 15, 29, 31};
    int stars_y[4] = {13, 18, 13, 18};

    /* draw background */
    draw_menu_bg();
    draw_menu_uscm_logo();
    
    spawn_menu_text();   
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
