
#include "Arduino.h"
#include "Adafruit_LEDBackpack.h"
#include "display.h"

#define TOP_X_BITS        0x0c00
#define BOTTOM_X_BITS     0x0003
#define TOP_O_BITS        0x03C0
#define BOTTOM_O_BITS     0x003C
#define TOP_TIE_BITS      0x0800
#define BOTTOM_TIE_BITS   0x0001
#define TOP_BLANK_BITS    0x0000
#define BOTTOM_BLANK_BITS 0x0000
#define ZERO_BITS         0x007E
#define ONE_BITS          0x0022
#define TWO_BITS          0x0057
#define THREE_BITS        0x0067
#define FOUR_BITS         0x002B
#define FIVE_BITS         0x006D
#define SIX_BITS          0x007D
#define SEVEN_BITS        0x0026
#define EIGHT_BITS        0x007F
#define NINE_BITS         0x006F
#define C_BITS            0x005C
#define E_BITS            0x005D
#define I_BITS            0x0018
#define L_BITS            0x0058
#define P_BITS            0x001F
#define T_BITS            0x0059
#define U_BITS            0x007A
#define HYPHEN_BITS       0x0001
#define EQUALS_BITS       0x0041

#define BOTTOM_MASK 0x003F
#define TOP_MASK    0x0FC0

static uint16_t character_bitmap_table[NUM_CHARACTERS][2] ={
    {BOTTOM_X_BITS, TOP_X_BITS},        // CHARACTER_X
    {BOTTOM_O_BITS, TOP_O_BITS},        // CHARACTER_Y
    {BOTTOM_TIE_BITS, TOP_TIE_BITS},    // CHARACTER_TIE
    {BOTTOM_BLANK_BITS, TOP_BLANK_BITS},// CHARACTER_BLANK
    {ZERO_BITS, ZERO_BITS},             // CHARACTER_0
    {ONE_BITS, ONE_BITS},               // CHARACTER_1
    {TWO_BITS, TWO_BITS},               // CHARACTER_2
    {THREE_BITS, THREE_BITS},           // CHARACTER_3
    {FOUR_BITS, FOUR_BITS},             // CHARACTER_4
    {FIVE_BITS, FIVE_BITS},             // CHARACTER_5
    {SIX_BITS, SIX_BITS},               // CHARACTER_6
    {SEVEN_BITS, SEVEN_BITS},           // CHARACTER_7
    {EIGHT_BITS, EIGHT_BITS},           // CHARACTER_8
    {NINE_BITS, NINE_BITS},             // CHARACTER_9
    {C_BITS, C_BITS},                   // CHARACTER_C
    {E_BITS, E_BITS},                   // CHARACTER_E
    {I_BITS, I_BITS},                   // CHARACTER_I
    {L_BITS, L_BITS},                   // CHARACTER_L
    {P_BITS, P_BITS},                   // CHARACTER_P
    {T_BITS, T_BITS},                   // CHARACTER_T
    {U_BITS, U_BITS},                   // CHARACTER_U
    {HYPHEN_BITS, HYPHEN_BITS},         // CHARACTER_HYPHEN
    {EQUALS_BITS, EQUALS_BITS},         // CHARACTER_EQUALS
};

typedef enum
{
    POSITION_BOTTOM,
    POSITION_TOP,
    POSITION_BOTH,
    POSITION_INVALID
} CharacterPosition;

static CharacterPosition matrix_position_table[NUM_DISPLAY_ROWS][NUM_DISPLAY_COLUMNS] ={
    {POSITION_TOP, POSITION_TOP, POSITION_TOP, POSITION_INVALID},
    {POSITION_BOTTOM, POSITION_BOTTOM, POSITION_BOTTOM, POSITION_INVALID},
    {POSITION_TOP, POSITION_TOP, POSITION_BOTTOM, POSITION_INVALID},
    {POSITION_BOTTOM, POSITION_BOTH, POSITION_BOTH, POSITION_BOTH}
};

static uint8_t matrix_row_table[NUM_DISPLAY_ROWS][NUM_DISPLAY_COLUMNS] ={
    {2, 3, 4, -1},
    {2, 3, 4, -1},
    {0, 1, 1, -1},
    {0, 7, 6, 5}
};

CharacterType digit_character[10] = {CHARACTER_0, CHARACTER_1, CHARACTER_2, CHARACTER_3, CHARACTER_4, CHARACTER_5, CHARACTER_6, CHARACTER_7, CHARACTER_8, CHARACTER_9};

void draw_character(Adafruit_LEDBackpack &matrix, CharacterType character, uint8_t row, uint8_t col)
{
    if ((row < NUM_DISPLAY_ROWS) && (col < NUM_DISPLAY_COLUMNS))
    {
        uint8_t position = matrix_position_table[row][col];
        if (position != POSITION_INVALID)
        {
            uint16_t character_bits = character_bitmap_table[character][(position == POSITION_BOTH) ? 0 : position];
            uint16_t character_mask;
            if (position == POSITION_BOTH)
            {
                character_mask = TOP_MASK | BOTTOM_MASK;
            }
            else
            {
                character_mask = (position == POSITION_TOP) ? TOP_MASK : BOTTOM_MASK;
            }
            matrix.displaybuffer[matrix_row_table[row][col]] &= ~character_mask;
            matrix.displaybuffer[matrix_row_table[row][col]] |= (character_bits & character_mask);
        }
    }
}

void draw_random(Adafruit_LEDBackpack &matrix, uint8_t row, uint8_t col)
{
    if ((row < NUM_DISPLAY_ROWS) && (col < NUM_DISPLAY_COLUMNS))
    {
        uint8_t position = matrix_position_table[row][col];
        if (position != POSITION_INVALID)
        {
            uint16_t character_bits = random(65536);
            uint16_t character_mask;
            if (position == POSITION_BOTH)
            {
                character_mask = TOP_MASK | BOTTOM_MASK;
            }
            else
            {
                character_mask = (position == POSITION_TOP) ? TOP_MASK : BOTTOM_MASK;
            }
            matrix.displaybuffer[matrix_row_table[row][col]] &= ~character_mask;
            matrix.displaybuffer[matrix_row_table[row][col]] |= (character_bits & character_mask);
        }
    }
}

