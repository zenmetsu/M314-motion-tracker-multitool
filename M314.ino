#include <SPI.h>
#include <TFT_eSPI.h>
#include <interrupt.h>
#include <stdio.h>
#include <stdlib.h>

#include "ui.h"
#include "framebuffer.h"
#include "coordinates.h"
#include "RPLidar.h"
#include "Picopixel.h"
#include "profiling.h"



/* pin definitions */
#define TFT_DC D11
#define TFT_CS D10
#define TFT_RS -1
#define ZOOM_IN D5
#define ZOOM_OUT D6
#define RPLIDAR_MOTOR A1      /* PWM pin for LIDAR motor */



/* TYPE DEFINITIONS */
typedef struct
  { int x, y
  ; bool live,drawn
  ;
  }  reflection;



/* GLOBAL DEFINITIONS */
#define PROFILING 1      /* turn profiling on and off */
#define PROFILING_MAIN 1 /* this needs to be true in at least ONE .c, .cpp, or .ino file in your sketch */
#define MAXPROF 8        /* override the number of bins */



/* GLOBAL CONSTANTS */
const int MIN_CYCLE_TIME_MS = 0; /* minimum time in milliseconds between new scan events */



/* GLOBAL VARIABLES */
bool g_debugging = false;
float g_zoom = 5;
int g_spindle_dutycycle = 255;
unsigned long g_last_scan_timestamp = 0;
float g_last_scan_duration_ms = 0.0;
float g_last_range = 0.0;
char g_new_scan = 0;
Scene g_display;
reflection g_scanlog[1024];

/* These are used to get information about static SRAM and flash memory sizes */
extern "C" char __data_start[];    /* start of SRAM data */
extern "C" char _end[];            /* end of SRAM data (used to check amount of SRAM this program's variables use) */
extern "C" char __data_load_end[]; /* end of FLASH (used to check amount of Flash this program's code and data uses) */

enum ui_modes{system_menu, sonar, mapping, thermal};
enum ui_modes g_ui_mode;
int  g_ui_color;


/* profiler timekeeping */
volatile unsigned int int_counter;
volatile unsigned char seconds, minutes;
unsigned int tcnt2; /* used to store timer value */

                       
/* create driver instances */
RPLidar lidar;
HardwareSerial Serial1(D0, D1);
TFT_eSPI tft = TFT_eSPI(); 

/* debugging macros */
#define DL(x) Serial.print(x)
#define DLn(x) Serial.println(x)
#define DV(m, v) do{Serial.print(m);Serial.print(v);Serial.print(" ");}while(0)
#define DVn(m, v) do{Serial.print(m);Serial.println(v);}while(0)



/* ONE-TIME EXECUTION BLOCK */
                        
void setup() {
    /* set pin modes */
    pinMode(RPLIDAR_MOTOR, OUTPUT);
    pinMode(ZOOM_IN, INPUT_PULLDOWN);
    pinMode(ZOOM_OUT, INPUT_PULLDOWN);

    /* delay to give time to see initial serial messages */
    delay(200);
    Serial.begin(115200);
    Serial.println();
  
    /* bind the RPLIDAR driver to the arduino hardware serial */
    lidar.begin(Serial1);
    analogWrite(RPLIDAR_MOTOR, 0);
  
    /* initialize the display */
    //tft.initR(INITR_BLACKTAB);
    tft.init();
    tft.fillScreen(M314_BLACK);
    tft.setRotation(3);

    /* do some initialization */
    clearFramebuffer(3);
    go_sonar_mode();
}



/* MAIN EXECUTION BLOCK */

void loop() {
    /* poll the LIDAR unit */ 
    pollLIDAR();
    if ( g_new_scan > 0 ) {
        print_sonar_metrics();
        drawFramebuffer();
        clearFramebuffer(1);
        g_new_scan = 0;
    }
    
    if (digitalRead(ZOOM_IN) == HIGH) {
        g_zoom = g_zoom * 1.001;
    }
    if (digitalRead(ZOOM_OUT) == HIGH) {
        g_zoom = g_zoom / 1.001;
    }
}



/* GRAPHICS FUNCTIONS */

void clearFramebuffer(char which) {
    g_display.erase(which);
}



void drawFramebuffer() {
    int x, y, w = tft.width(), h = tft.height();
    int deltas = 0;

    long start = millis();
    g_display.flip();
    for (x=0; x < w; x++) {
        for (y=0; y < h; y++) {
           if ( !(g_display.compare(x, y)) ) {
               tft.drawPixel(x, y, RGB332_TO_565_LUT[g_display.get(x, y)]);
               deltas++;
               g_display.flip();
               pollLIDAR();
               g_display.flip();
           }
        }
    }    
}


void fillFramebuffer() {
  
}


void drawField() {
    int x, y, w = tft.width(), h = tft.height();
    
    /* define center of display */
    y = h/2;
    x = w/2;

    /* draw crosshairs */
    tft.drawFastHLine(0, y, w, M314_ORANGE);
    tft.drawFastVLine(x, 0, h, M314_ORANGE);
}



void draw_sonar_compass() {
    int x, y, w = tft.width(), h = tft.height();
    
    /* define origin for scanner */
    y = h - 31;
    x = w/2;

    /* draw static compass arcs */
    drawArc(x, y, 31, 180, 360, M314_LTBLUE);
    drawArc(x, y, 32, 180, 360, M314_LTBLUE);
    pollLIDAR();
    drawArc(x, y, 76, 180, 360, M314_LTBLUE);
    drawArc(x, y, 77, 180, 360, M314_LTBLUE);    
}



void draw_sonar_ui() {
    int x, y, w = tft.width(), h = tft.height();
    
    /* define origin for scanner */
    y = h - 31;
    x = w/2;

    draw_sonar_compass();

    /* draw bottom field */
    tft.fillRect(0, y-1, x-32, 31, M314_BLUE);
    tft.fillRect(x+32, y-1, x-32, 31, M314_BLUE);
    tft.fillRect(0, h-11, w, h, M314_BLUE);
    tft.fillTriangle(x-32, y-1, x-32, h, x-20, h, M314_BLUE);
    tft.fillTriangle(x+32, y-1, x+32, h, x+20, h, M314_BLUE);
    pollLIDAR();

    /* add text to bottom field */
    tft.setTextSize(1);
    tft.setTextColor(M314_WHITE);
    tft.setCursor(3, 103);
    //tft.setFont(&Picopixel);
    tft.print("FEMS 5.547.52");
    tft.setCursor(112, 109);
    tft.print("CX. 54/");
    //tft.setFont();
}



void drawArc(int16_t x, int16_t y, int16_t r, float rs, float re, uint16_t color) { 
    const float degrad = 0.01745329252;
    int16_t _width = tft.width(), _height = tft.height();
      
    for (float i = rs*degrad; i < re*degrad; i += (5.0/r)) {
        tft.drawPixel(x + cos(i) * r, y + sin(i) * r, color);
    }
}



void print_sonar_scan_freq() {
    /* set background color to erase prior value */
    tft.setTextColor(M314_BLUE);
    tft.setTextSize(1);   
    //tft.setFont(&Picopixel);

    /* perform an erase and draw cycle */
    for (int draw_cycle = 0; draw_cycle < 2; draw_cycle++) {
        tft.setCursor(112, 109);
        tft.print("CX. 54/");
        tft.print(g_last_scan_duration_ms, 2);
        tft.print("Hz");
        
        /* change color and update global vars */
        if ( !(draw_cycle) ) {
            pollLIDAR();
            tft.setTextColor(M314_WHITE);
            g_last_scan_duration_ms = 1000.0 / (float)(millis() - g_last_scan_timestamp);
            g_last_scan_timestamp = millis();
        }
    }
}



void print_sonar_scan_range() {
    float range = 0.001 * (200.0/g_zoom) * (tft.height() / 2);
    int last_range_int, last_range_dec;

    if ( range != g_last_range ) {
        int range_int = floor(range);
        int range_dec = floor(100 * (range - range_int));
    
        /* set background color to erase prior value */
        tft.setTextColor(M314_BLACK);
        //tft.setFont();
        
        /* perform an erase and draw cycle */
        for (int draw_cycle = 0; draw_cycle < 2; draw_cycle++) {
            last_range_int = floor(g_last_range);
            last_range_dec = floor(100 * (g_last_range - last_range_int));
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
                pollLIDAR();
                tft.setTextColor(M314_RED);
                g_last_range = range;
            }
        }
    }     
}    



void print_sonar_metrics() {
    print_sonar_scan_range();
    print_sonar_scan_freq();
}



/* DATA CONTROL FUNCTIONS */



/* HARDWARE CONTROL FUNCTIONS */

void pollLIDAR() {
    int x, y, ox, oy, w = tft.width(), h = tft.height();
    const float degrad = 0.01745329252;
    Coordinates point = Coordinates();
    float range = 0.001 * (200.0/g_zoom) * (h / 2);
    float start_time = millis();
    
    if ( IS_OK(lidar.waitPoint()) ) {
        float distance = lidar.getCurrentPoint().distance; /* distance value in mm */
        float angle    = lidar.getCurrentPoint().angle;    /* angle value in degrees */
        bool  startBit = lidar.getCurrentPoint().startBit; /* is point first in new scan */
        byte  quality  = lidar.getCurrentPoint().quality;  /* quality of current point */

        /* did we start a new scan? */
        if (startBit) {
            if ((millis() - g_last_scan_timestamp) > MIN_CYCLE_TIME_MS) {
                g_new_scan++;
            }        
        } 
      
        point.fromPolar(g_zoom*distance/200.0,angle*degrad);
        x = (int16_t)((w/2) - point.getY());
        y = (int16_t)((h/2) - point.getX());
        
        /* check if point is onscreen */
        if ( (x>=0) && (y>=0) && (x<w) && (y<h) ) { 
                g_display.put(x, y, M314_BLUE);
                //g_scanlog[g_scanlog_idx].x = x;
                //g_scanlog[g_scanlog_idx].y = y;
            }
        
        if (g_debugging) {
            /* pad distance with leading zeros */
            if (distance < 10000) Serial.print("0");
            if (distance < 1000) Serial.print("0");
            if (distance < 100) Serial.print("0");
            if (distance < 10) Serial.print("0");
            Serial.print(distance, 2);
            
            Serial.print(" @ ");

            /* pad bearing with leading zeros */
            if (angle < 100) Serial.print("0");
            if (angle < 10) Serial.print("0");
            Serial.println(angle, 2);
        }
    } 
    else { /*\ if (IS_OK(lidar.waitPoint())) \*/
        Serial.println("NOK...");
        
        /* stop the LIDAR motor */
        analogWrite(RPLIDAR_MOTOR, 0);
    
        /* try to detect LIDAR */ 
        rplidar_response_device_info_t info;
        if (IS_OK(lidar.getDeviceInfo(info, 100))) {
            Serial.println("detected...");
            lidar.startScan();
        }
        
        /* start the LIDAR motor */
        analogWrite(RPLIDAR_MOTOR, g_spindle_dutycycle);
        delay(1000);
    }
}



/* INITIALIZING FUNCTIONS */





/* MODE SWITCHING FUNCTIONS */
void go_sonar_mode() {
    g_ui_mode = sonar;
    g_ui_color = M314_BLUE;
    draw_sonar_ui();
    draw_sonar_compass();
    print_sonar_metrics();
    g_last_range = 0.001 * (200.0/g_zoom) * (tft.height() / 2);
}    
