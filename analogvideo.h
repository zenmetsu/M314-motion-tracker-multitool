typedef struct color {
    unsigned char ry;
    unsigned char gi;
    unsigned char bq;
} color_t;

typedef struct yiqcolor {
    float y;
    float i;
    float q;
} yiqcolor_t;

typedef struct yiqcolori {
    unsigned char y;
    signed char i;
    signed char q;
} yiqcolori_t;

void glitch_vert_scroll(TFT_eSPI *in, uint16_t *fb, int v_offset);

void glitch_hori_scroll(TFT_eSPI *tft, uint16_t *fb, int h_offset);

void glitch_hori_warp(TFT_eSPI *tft, uint16_t *fb);

void glitch_chrom_noise(TFT_eSPI *tft, uint16_t *fb, float chroma_noise);
void glitch_chrom_noise(uint16_t *fb_temp, uint16_t *fb, int w, int h, float chroma_noise);

void glitch_power_off(TFT_eSPI *tft, uint16_t *fb);

yiqcolor_t rgb2yiq(uint16_t rgb);

uint16_t yiq2rgb565(yiqcolor_t yiq);

yiqcolori_t rgb2yiq_i(uint16_t rgb);

uint16_t yiq2rgb565_i(yiqcolori_t yiq);

uint16_t byteswap(uint16_t input);
