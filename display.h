
typedef enum
{
    CHARACTER_X,
    CHARACTER_O,
    CHARACTER_TIE,
    CHARACTER_BLANK,
    CHARACTER_0,
    CHARACTER_1,
    CHARACTER_2,
    CHARACTER_3,
    CHARACTER_4,
    CHARACTER_5,
    CHARACTER_6,
    CHARACTER_7,
    CHARACTER_8,
    CHARACTER_9,
    CHARACTER_C,
    CHARACTER_E,
    CHARACTER_I,
    CHARACTER_L,
    CHARACTER_P,
    CHARACTER_T,
    CHARACTER_U,
    CHARACTER_HYPHEN,
    CHARACTER_EQUALS,
    NUM_CHARACTERS
} CharacterType;

#define NUM_DISPLAY_ROWS    4
#define NUM_DISPLAY_COLUMNS 4

extern void draw_character(Adafruit_LEDBackpack &matrix, CharacterType character, uint8_t row, uint8_t col);

extern void draw_random(Adafruit_LEDBackpack &matrix, uint8_t row, uint8_t col);