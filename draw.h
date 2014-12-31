#include <Arduino.h>

typedef enum
{
    ADD = 0,
    OVER,
    SUB
} CompositOp;


typedef struct
{
    uint8_t x;
    uint8_t y;
}LEDPoint;

typedef enum
{
    NONE = 0,
    HI,
    BLUSH,
    CHOMP,
    COOL,
    TV
}Program;

#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00
#define PINK   0xFF1088
#define ORANGE 0xE05800
#define WHITE  0xFFFFFF
#define BLACK  0x000000


