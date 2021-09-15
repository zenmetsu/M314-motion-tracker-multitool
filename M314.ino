#include <SPI.h>
#include <interrupt.h>
#include <stdio.h>
#include <stdlib.h>

#include "ui.h"
#include "framebuffer.h"
#include "coordinates.h"
#include "RPLidar.h"
#include "profiling.h"

#define ZOOM_IN D5
#define ZOOM_OUT D6
#define RPLIDAR_MOTOR A1      /* PWM pin for LIDAR motor */



/* GLOBAL DEFINITIONS */
#define PROFILING 1      /* turn profiling on and off */
#define PROFILING_MAIN 1 /* this needs to be true in at least ONE .c, .cpp, or .ino file in your sketch */
#define MAXPROF 8        /* override the number of bins */

/*
#define BLACK  0x00
#define BLUE   0x0D
#define LTBLUE 0x0D
#define ORANGE 0xF0
#define RED    0xE0
#define WHITE  0xFF
*/


/* GLOBAL CONSTANTS */
const int MIN_CYCLE_TIME_MS = 100; /* minimum time in milliseconds between new scan events */



/* GLOBAL VARIABLES */
bool g_debugging = false;
int g_spindle_dutycycle = 255;

/* These are used to get information about static SRAM and flash memory sizes */
extern "C" char __data_start[];    /* start of SRAM data */
extern "C" char _end[];            /* end of SRAM data (used to check amount of SRAM this program's variables use) */
extern "C" char __data_load_end[]; /* end of FLASH (used to check amount of Flash this program's code and data uses) */

//enum ui_modes{system_menu, sonar, mapping, thermal};
TrackerDisplay::ui_modes ui_mode;

/* profiler timekeeping */
volatile unsigned int int_counter;
volatile unsigned char seconds, minutes;
unsigned int tcnt2; /* used to store timer value */

                       
/* create driver instances */
RPLidar lidar;
HardwareSerial Serial1(D0, D1);
TrackerDisplay display;


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
    display.init();
    display.fill_screen(TrackerDisplay::M314_BLACK);
    display.set_rotation(3);

    /* do some initialization */
    display.framebuffer_clear();  /* clear ui elements */
    display.clear_scanlog();
    ui_mode = TrackerDisplay::sonar;
    display.switch_mode(ui_mode);
}



/* MAIN EXECUTION BLOCK */

void loop() {
    /* poll the LIDAR unit */ 
    pollLIDAR();

    if (digitalRead(ZOOM_IN) == HIGH) {
        display.update_zoom(1.0005);
        display.blank_field();
    }
    if (digitalRead(ZOOM_OUT) == HIGH) {
        display.update_zoom(1/1.0005);
        display.blank_field();
    }
}



/* GRAPHICS FUNCTIONS */


/* DATA CONTROL FUNCTIONS */



/* HARDWARE CONTROL FUNCTIONS */

void pollLIDAR() {
    int x, y, ox, oy, w = display.get_width(), h = display.get_height();
    const float degrad = 0.01745329252;
    Coordinates point = Coordinates();
    float range = 0.001 * (200.0/display.ui_current_zoom) * (h / 2);
    float start_time = micros();
    float bearing,reach;
    //Serial.print("pollLIDAR ");

    if ( IS_OK(lidar.waitPoint()) ) {
        //Serial.print("O");
        float distance = lidar.getCurrentPoint().distance; /* distance value in mm */
        float angle    = lidar.getCurrentPoint().angle;    /* angle value in degrees */
        bool  startBit = lidar.getCurrentPoint().startBit; /* is point first in new scan */
        byte  quality  = lidar.getCurrentPoint().quality;  /* quality of current point */

        bearing = angle;
        reach = distance; 
        /* did we start a new scan? */
        if (startBit) {
            //Serial.print("N");
            if ((millis() - display.time_last_scan_ms) > MIN_CYCLE_TIME_MS) {
                //Serial.print("D");
                display.blank_field();
                display.scan_index = 0;
                display.update_metrics();
            }        
        } 
        
        point.fromPolar(display.ui_current_zoom*distance/200.0,angle*degrad);
        x = (int16_t)((w/2) - point.getY());
        y = (int16_t)((h/2) - point.getX());
        display.scan_index++;
        /* check if point is onscreen */
        if ( (x>=0) && (y>=0) && (x<w) && (y<h) ) {
                display.put_record(x, y, TrackerDisplay::M314_WHITE);
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
    /*
    Serial.print(" time(uS): ");
    Serial.print( micros() - start_time );
    Serial.print(" #");
    Serial.print(g_dbg_scan_records);
    Serial.print(" ");
    Serial.print(reach,0);
    Serial.print("@");
    Serial.print(bearing);
    Serial.println("deg");    
    */
}



/* INITIALIZING FUNCTIONS */
