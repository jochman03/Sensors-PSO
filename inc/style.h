#ifndef STYLE_H
#define STYLE_H

#include <raylib.h>

#define BTN_FONT_SIZE 18
#define BTN_HEIGHT 24

typedef struct {
    Color background;
    Color menu;
    Color text;
    Color input_text;
    Color input_background;
    Color input_clicked_background;
    Color accent;
    Color menu_darker;
    Color input_text_blocked;
} style_t;

extern style_t dark_mode;
extern style_t light_mode;

extern style_t* style;

#endif // STYLE_H