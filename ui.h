#ifndef __TrackerDisplay_h_
/* COLOR DEFINITIONS */

class TrackerDisplay {
    public:
        TrackerDisplay();

        typedef struct { 
            char x, y, live, drawn; 
        }  reflection;

        reflection scanlog[1024];

        int scan_index;

        uint8_t framebuffer[160][128];

        enum ui_modes{system_menu, sonar, mapping, thermal};
        enum ui_modes ui_mode;
        
        float time_last_scan_ms;
        float scan_frequency;
        float ui_current_zoom, last_range;

        



        /* hardware control */
        void init();
        int get_height();
        int get_width();
        void set_rotation(char rotations);     

        /* framebuffer control */
        void framebuffer_clear();
        void framebuffer_put(int x, int y, char color);
        char framebuffer_get(int x, int y);
        const char* getPixels();

        /* graphics functions */
        void fill_screen(int color);
        void put_record(int x, int y, char color);
        void draw_pixel(int x, int y, char color);
        void draw_hline(int x, int y, int l, char color);
        void draw_vline(int x, int y, int l, char color);
        void tri(int x0, int y0, int x1, int y1, int x2, int y2, char color);
        void rect(int x, int y, int w, int h, char color);
        void tri_fill(int x0, int y0, int x1, int y1, int x2, int y2, char color);
        void rect_fill(int x, int y, int w, int h, char color);

        void draw_arc(int16_t x, int16_t y, int16_t r, float rs, float re, char color);
        
        
        /* text functions */        
        void set_cursor(int x, int y);
        void set_text_size(char size);
        void set_text_color(char color);
        void print(const char* text);

        /* ui functions */
        void blank_field();
        void clear_scanlog();

        void update_scantime();
        void update_zoom(float ratio);
        
        void draw_sonar_ui();
        void draw_sonar_compass();
        void print_sonar_scan_freq();
        void print_sonar_scan_range();

        void update_metrics();

        void draw_lasernav_ui();

        void switch_mode(ui_modes mode);

        static const char M314_BLACK  = 0x00;
        static const char M314_BLUE   = 0x0D;
        static const char M314_LTBLUE = 0x0D;
        static const char M314_ORANGE = 0xF0;
        static const char M314_RED    = 0xE0;
        static const char M314_WHITE  = 0xFF;

    private:
        static const int WIDTH = 160;
        static const int HEIGHT = 128;

        char pixels_[WIDTH * HEIGHT];
};

#endif /*\ #ifndef __TrackerDisplay_h_ \*/
