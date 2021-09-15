#ifndef __TrackerDisplay_h_
/* COLOR DEFINITIONS */

class TrackerDisplay {
    public:
        TrackerDisplay();

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
        void refresh();

        /* graphics functions */
        void fill_screen(int color);
        void put_record(int x, int y, int color);
        void draw_pixel(int x, int y, int color);
        void draw_hline(int x, int y, int l, int color);
        void draw_vline(int x, int y, int l, int color);
        void tri(int x0, int y0, int x1, int y1, int x2, int y2, int color);
        void rect(int x, int y, int w, int h, int color);
        void tri_fill(int x0, int y0, int x1, int y1, int x2, int y2, int color);
        void rect_fill(int x, int y, int w, int h, int color);

        void draw_arc(int16_t x, int16_t y, int16_t r, float rs, float re, int color);
        
        
        /* text functions */        
        void set_cursor(int x, int y);
        void set_text_size(char size);
        void set_text_color(int color);
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

        static const int M314_BLACK  = 0x0000;
        static const int M314_BLUE   = 0x03EC;
        static const int M314_LTBLUE = 0x03EC;
        static const int M314_ORANGE = 0xFC00;
        static const int M314_RED    = 0xF800;
        static const int M314_WHITE  = 0xFFFF;



    private:
        static const int WIDTH = 160;
        static const int HEIGHT = 128;
        static const int COLOR_DEPTH = 16;

};

#endif /*\ #ifndef __TrackerDisplay_h_ \*/
