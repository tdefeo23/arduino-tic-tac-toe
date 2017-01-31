/*************************************************** 
 
 ****************************************************/

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

#include <TimerOne.h>

#include "display.h"
#include "input.h"
#include "SimpleMusicPlayer.h"


// Pin assgnments
#define PIN_JOYSTICK_X      A1
#define PIN_JOYSTICK_Y      A0
#define PIN_JOYSTICK_BUTTON A2
#define PIN_CPU_SKILL_BIT_0  3
#define PIN_CPU_SKILL_BIT_1  4
#define PIN_X_CPU_OR_HUMAN   5
#define PIN_O_CPU_OR_HUMAN   6
#define PIN_AUDIO_OUT        8

Adafruit_LEDBackpack matrix = Adafruit_LEDBackpack();

typedef enum
{
  CPU_SKILL_BEGINNER,
  CPU_SKILL_ADVANCED,
  CPU_SKILL_EXPERT
} CpuSkillLevel;

typedef enum
{
  TIE,
  CPU,
  PLAYER_1,
  PLAYER_2,
} Winner;

typedef enum
{
  INVALID,
  NEW_PLAYER_CONFIGURATION,
  NEW_GAME,
  CPU_X_TURN,
  CPU_O_TURN,
  HUMAN_X_TURN,
  HUMAN_O_TURN,
  SHOW_WINNER,
  GAME_OVER
} GameState;

static const char *game_state_names[] =
{
  "INVALID",
  "NEW_PLAYER_CONFIGURATION",
  "NEW_GAME",
  "CPU_X_TURN",
  "CPU_O_TURN",
  "HUMAN_X_TURN",
  "HUMAN_O_TURN",
  "SHOW_WINNER",
  "GAME_OVER"
};

#define CURSOR_FLASH_DELAY 300

static GameState game_state = NEW_PLAYER_CONFIGURATION;
static GameState last_game_state = INVALID;
static CharacterType tic_tac_toe_game_grid[3 * 3];
static int total_move_count = 0;
static uint32_t total_x_wins = 98;
static uint32_t total_o_wins = 98;
static uint32_t total_ties = 98;
static int8_t cursor_row = 0;
static int8_t cursor_column = 0;

static bool player_configuration_changed = false;
static uint8_t number_of_players = 1;
static bool cpu_is_x = false;
static CpuSkillLevel cpu_skill_level = CPU_SKILL_BEGINNER;

SimpleMusicPlayer musicPlayer(PIN_AUDIO_OUT);

/*const SimpleMusicPlayer::Note charge_melody[] = 
{
  {NOTE_C4, SIXTEENTH_NOTE},
  {NOTE_E4, SIXTEENTH_NOTE},
  {NOTE_G4, SIXTEENTH_NOTE},
  {NOTE_G4, EIGHTH_NOTE},
  {NOTE_E4, SIXTEENTH_NOTE},
  {NOTE_G4, QUARTER_NOTE},
  {NOTE_END}
};*/

const int chargeMelody[] PROGMEM = {NOTE_C4,     NOTE_E4,     NOTE_G4,     NOTE_G4,      NOTE_E4,     NOTE_G4,  -1};
const int chargeTempo[] PROGMEM  = {SIXTEENTH_NOTE, SIXTEENTH_NOTE, SIXTEENTH_NOTE, EIGHTH_NOTE, SIXTEENTH_NOTE, QUARTER_NOTE};

const int imperialMelody[] PROGMEM = {NOTE_A3,      NOTE_A3,      NOTE_A3,      NOTE_F3,                    NOTE_C4,        NOTE_A3,      NOTE_F3,                    NOTE_C4,        NOTE_A3, -1};
const int imperialTempo[] PROGMEM  = {QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, EIGHTH_NOTE+SIXTEENTH_NOTE, SIXTEENTH_NOTE, QUARTER_NOTE, EIGHTH_NOTE+SIXTEENTH_NOTE, SIXTEENTH_NOTE, HALF_NOTE};

const int starwarsMelody[] PROGMEM = {NOTE_F4, NOTE_F4, NOTE_F4, NOTE_AS4, NOTE_F5, NOTE_DS5, NOTE_D5, NOTE_C5, NOTE_AS5, NOTE_F5, NOTE_DS5, NOTE_D5, NOTE_C5, NOTE_AS5, NOTE_F5, NOTE_DS5, NOTE_D5, NOTE_DS5, NOTE_C5, -1};
const int starwarsTempo[] PROGMEM  = {
  SIXTEENTH_NOTE, 
  SIXTEENTH_NOTE, 
  SIXTEENTH_NOTE, 
  QUARTER_NOTE + SIXTEENTH_NOTE,
  QUARTER_NOTE,
  SIXTEENTH_NOTE, 
  SIXTEENTH_NOTE, 
  SIXTEENTH_NOTE, 
  QUARTER_NOTE + SIXTEENTH_NOTE,
  QUARTER_NOTE,
  SIXTEENTH_NOTE, 
  SIXTEENTH_NOTE, 
  SIXTEENTH_NOTE, 
  QUARTER_NOTE + SIXTEENTH_NOTE,
  QUARTER_NOTE,
  SIXTEENTH_NOTE, 
  SIXTEENTH_NOTE, 
  SIXTEENTH_NOTE, 
  QUARTER_NOTE + SIXTEENTH_NOTE,
};


const int shaveMelody[] PROGMEM = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3,0, NOTE_B3, NOTE_C4, -1};
const int shaveTempo[] PROGMEM = {QUARTER_NOTE, SIXTEENTH_NOTE, SIXTEENTH_NOTE, SIXTEENTH_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE, QUARTER_NOTE};

const int cursor_move_note[] PROGMEM = {NOTE_C4, -1};
const int cursor_move_tempo[] PROGMEM = {(SIXTEENTH_NOTE / 2)};

const int cursor_select_x_note[] PROGMEM = {NOTE_G4, -1};
const int cursor_select_x_tempo[] PROGMEM = {SIXTEENTH_NOTE};

const int cursor_select_o_note[] PROGMEM = {NOTE_F4, -1};
const int cursor_select_o_tempo[] PROGMEM = {SIXTEENTH_NOTE};



CharacterType get_tic_tac_toe_grid_value(uint8_t row, uint8_t col)
{
  CharacterType result = CHARACTER_BLANK;
  if ((row < 3) && (col < 3))
  {
    result = tic_tac_toe_game_grid[(row * 3) + col];
  }

  return result;
}


void draw_tic_tac_toe_game_grid()
{
  for (uint8_t row = 0; row < 3; row++)
  {
    for (uint8_t col = 0; col < 3; col++)
    {
      draw_character(get_tic_tac_toe_grid_value(row, col), row, col);
    }
  }
  matrix.writeDisplay();
}

void draw_pl1()
{
  draw_character(CHARACTER_P, 3, 1);
  draw_character(CHARACTER_L, 3, 2);
  draw_character(CHARACTER_1, 3, 3);
}

void draw_pl2()
{
  draw_character(CHARACTER_P, 3, 1);
  draw_character(CHARACTER_L, 3, 2);
  draw_character(CHARACTER_2, 3, 3);
}

void draw_cpu()
{
  draw_character(CHARACTER_C, 3, 1);
  draw_character(CHARACTER_P, 3, 2);
  draw_character(CHARACTER_U, 3, 3);
}

void draw_tie()
{
  draw_character(CHARACTER_T, 3, 1);
  draw_character(CHARACTER_I, 3, 2);
  draw_character(CHARACTER_E, 3, 3);
}

void draw_blanks()
{
  draw_character(CHARACTER_BLANK, 3, 1);
  draw_character(CHARACTER_BLANK, 3, 2);
  draw_character(CHARACTER_BLANK, 3, 3);
}

void draw_score(uint8_t score)
{
  static CharacterType digit_character[10] = {CHARACTER_0, CHARACTER_1, CHARACTER_2, CHARACTER_3, CHARACTER_4, CHARACTER_5, CHARACTER_6, CHARACTER_7, CHARACTER_8, CHARACTER_9};
  draw_character(CHARACTER_HYPHEN, 3, 1);
  uint8_t tens = score / 10;
  uint8_t ones = score % 10;

  draw_character(/*(tens == 0) ? CHARACTER_BLANK :*/ digit_character[tens], 3, 2);
  draw_character(digit_character[ones], 3, 3);
}


void print_tic_tac_toe_game_grid()
{
  for (uint8_t row = 0; row < 3; row++)
  {
    for (uint8_t col = 0; col < 3; col++)
    {
      switch(get_tic_tac_toe_grid_value(row, col))
      {
        case CHARACTER_X:
          Serial.print("X");
          break;
        case CHARACTER_O:
          Serial.print("O");
          break;
        default:
          Serial.print(" ");
          break;
      }      
      if (col < 2)
      {
        Serial.print("|");
      }
    }
    if (row < 2)
    {
      Serial.println();
      Serial.print("-----");
    }
    Serial.println();
  }
}


static uint8_t win_patterns[8][3] = 
{
  {0,1,2},
  {3,4,5},
  {6,7,8},
  {0,3,6},
  {1,4,7},
  {2,5,8},
  {0,4,8},
  {2,4,6}
};
    
int8_t get_win_pattern_index() 
{    
    int8_t result = -1;
    
    for(uint8_t index = 0; index < 8; index++) 
    {  
      if ((tic_tac_toe_game_grid[win_patterns[index][0]] != CHARACTER_BLANK) &&
          (tic_tac_toe_game_grid[win_patterns[index][0]] == tic_tac_toe_game_grid[win_patterns[index][1]]) &&
          (tic_tac_toe_game_grid[win_patterns[index][0]] == tic_tac_toe_game_grid[win_patterns[index][2]]))
      {
        result = index;
        break;
      }
    }
        
    return result;
}

uint8_t check_win()
{  
  uint8_t result = CHARACTER_BLANK;
  int8_t index = get_win_pattern_index();
  if (index != -1)
  {
    result = tic_tac_toe_game_grid[win_patterns[index][0]];
  }

  //Serial.print("check_win() returning: "); Serial.println(result);
  return result;  
}


int minimax(uint8_t player, int *best_move, uint8_t *move_depth, uint8_t *end_move_depth) 
{
  int move = -1;
  int score = -2;
  uint8_t selected_end_move_depth = 0;
  uint8_t max = 0;
  uint8_t min = 10;

  //Serial.print("minimax depth: "); Serial.println((*move_depth));
  //Serial.println(player);
  if (best_move != NULL)
  {
    *best_move = -1;
  }
  
  CharacterType winner = (CharacterType) check_win();
  if (winner != CHARACTER_BLANK)
  {
    *end_move_depth = *move_depth;
    return (winner == player) ? 1 : -1;
  }
  
  for(uint8_t index = 0; index < 9; ++index) 
  {
    if(tic_tac_toe_game_grid[index] == CHARACTER_BLANK) 
    {
      uint8_t local_end_move_depth = 0;
      
      tic_tac_toe_game_grid[index] = (CharacterType) player;  
      (*move_depth)++;  
      int thisScore = -minimax((player == CHARACTER_X) ? CHARACTER_O : CHARACTER_X, NULL, move_depth, &local_end_move_depth);
      (*move_depth)--;    
      tic_tac_toe_game_grid[index] = CHARACTER_BLANK;
      
      //if (best_move != NULL)
      //{
      //  Serial.print("(Pos: "); Serial.print(index); Serial.print(" Score: "); Serial.print(thisScore); Serial.print(" Move depth: "); Serial.print(local_end_move_depth); Serial.print(")");        
      //}
      
      if ((thisScore > score) || 
          ((thisScore == score) && (thisScore > 0) && (local_end_move_depth < min)) ||
          ((thisScore == score) && (thisScore < 0) && (local_end_move_depth > max)))
      {
        max = local_end_move_depth;
        min = local_end_move_depth;
        //min_decisions = local_decision_count;
        //max_decisions = local_decision_count;
        //if (decision_count != NULL)
        //{
        //  *decision_count = *decision_count + local_decision_count;
        //}
        score = thisScore;
        move = index;
        selected_end_move_depth = local_end_move_depth;
      } 
    }
  }
  
  //if (best_move != NULL)
  //{
  //  Serial.println();
  //  Serial.print("Chose: "); Serial.println(move);
  //}
    
  if (move == -1) 
  {
    // All moves made, this is a tie.
    *end_move_depth = *move_depth;
    return 0;
  }
  
  if (best_move != NULL)
  {
    *best_move = move;
    if (score == -1)
    {
      // If we are going to loose, at least make a blocking move if possible to prolong the game as much as possible.
      // TPD - FIND BLOCK HERE.     
    }
    Serial.print("Best move: "); Serial.print(move); Serial.print(" Move depth: "); Serial.print(*move_depth); Serial.print(" Score: "); Serial.println(score);
  }
  
  *end_move_depth = selected_end_move_depth;
  return score;
}

int compute_best_move(uint8_t player, int move_count)
{
  static uint8_t corners_table[4] = {0, 2, 6, 8};
  int move = -1;
  
  //Serial.print("computer move :");
  //Serial.println((player == CHARACTER_X) ? "X" : "O");
  if (move_count == 0)
  {
    move = random(9);
  }
  else if ((move_count == 1) || (cpu_skill_level == CPU_SKILL_BEGINNER))
  {
    int dumb_move = 0;//random(2);
    if ((cpu_skill_level == CPU_SKILL_BEGINNER) || ((cpu_skill_level == CPU_SKILL_ADVANCED) && (dumb_move == 0)))
    {
      if ((cpu_skill_level == CPU_SKILL_ADVANCED) && (dumb_move == 0))
      {
        Serial.println("CPU_SKILL_ADVANCED, DUMB MOVE.");
      }
      
      // Dumb CPU.
      move = random(9);
      while(tic_tac_toe_game_grid[move] != CHARACTER_BLANK) 
      {
        move = (move + 1) % 9;
      }
    }
    else
    {
      // Smart CPU.
      // Find opponent's opening move.
      if(tic_tac_toe_game_grid[4] != CHARACTER_BLANK) 
      {
        // Opponent chose center, so choose one of the four corners;
        move = corners_table[random(4)];
      }
      else
      {
        // Opponent did not choose center, so take it.
        move = 4;
      }
    }
  }
  else
  {
    int dumb_move = 0;//random(2);
    if ((move_count == 2) && (cpu_skill_level == CPU_SKILL_ADVANCED) && (dumb_move == 0))
    {     
      Serial.println("CPU_SKILL_ADVANCED, DUMB MOVE.");
      
      // Dumb CPU.
      move = random(9);
      while(tic_tac_toe_game_grid[move] != CHARACTER_BLANK) 
      {
        move = (move + 1) % 9;
      }
    }
    else
    {
      uint8_t decision_count = 0;
      uint8_t depth = 0, max_depth = 0;
      minimax(player, &move, &depth, &max_depth);
    }
  }

  return move;
}

bool move_cursor(uint8_t direction)
{
  bool result = false;
  uint8_t new_cursor_row = cursor_row;
  uint8_t new_cursor_column = cursor_column;
  uint8_t row_index, col_index;
  
  switch (direction)
  {
    case JOYSTICK_UP:
      if (new_cursor_row > 0)
      {
        for (col_index = 0; col_index < 3; col_index++)
        {
          new_cursor_row = cursor_row;        
          for (row_index = 0; row_index < 3; row_index++)
          {
            if (new_cursor_row > 0)
            {          
              new_cursor_row--;
            }
  
            if ((new_cursor_column != cursor_column) || (new_cursor_row != cursor_row))
            {
              //Serial.print("UP Checking row: "); Serial.print(new_cursor_row); Serial.print(" col: "); Serial.println(new_cursor_column);
              if (get_tic_tac_toe_grid_value(new_cursor_row, new_cursor_column) == CHARACTER_BLANK)
              {
                result = true;
                break;
              }
            }
          }
  
          if (result == true)
          {
            break;
          }
  
          new_cursor_column++;
          if (new_cursor_column > 2)
          {
            new_cursor_column = 0;
          }
        }
      }
      
      break;
      
    case JOYSTICK_DOWN:
      if (new_cursor_row < 2)
      {
        for (col_index = 0; col_index < 3; col_index++)
        {
          new_cursor_row = cursor_row;        
          for (row_index = 0; row_index < 3; row_index++)
          {
            if (new_cursor_row < 2)
            {          
              new_cursor_row++;
            }
  
            if ((new_cursor_column != cursor_column) || (new_cursor_row != cursor_row))
            {
              //Serial.print("DOWN Checking row: "); Serial.print(new_cursor_row); Serial.print(" col: "); Serial.println(new_cursor_column);
              if (get_tic_tac_toe_grid_value(new_cursor_row, new_cursor_column) == CHARACTER_BLANK)
              {
                result = true;
                break;
              }
            }
          }
  
          if (result == true)
          {
            break;
          }
            
          if (new_cursor_column == 0)
          {
            new_cursor_column = 2;
          }
          else
          {
            new_cursor_column--;          
          }
        }
      }
      break;
      
    case JOYSTICK_LEFT:    
      if ((new_cursor_row < 3) && (new_cursor_column > 0))
      {
        for (row_index = 0; row_index < 3; row_index++)
        {
          new_cursor_column = cursor_column;        
          for (col_index = 0; col_index < 3; col_index++)
          {
            if (new_cursor_column > 0)
            {          
              new_cursor_column--;
            }

            if ((new_cursor_column != cursor_column) || (new_cursor_row != cursor_row))
            {
              //Serial.print("LEFT Checking row: "); Serial.print(new_cursor_row); Serial.print(" col: "); Serial.println(new_cursor_column);
              if (get_tic_tac_toe_grid_value(new_cursor_row, new_cursor_column) == CHARACTER_BLANK)
              {
                result = true;
                break;
              }
            }
          }
  
          if (result == true)
          {
            break;
          }

          if (new_cursor_row == 0)
          {
            new_cursor_row = 2;
          }
          else
          {
            new_cursor_row--;          
          }
        }
      }
      break;
      
    case JOYSTICK_RIGHT:  
      if ((new_cursor_row < 3) && (new_cursor_column < 2))
      {
        for (row_index = 0; row_index < 3; row_index++)
        {
          new_cursor_column = cursor_column;        
          for (col_index = 0; col_index < 3; col_index++)
          {
            if (new_cursor_column < 2)
            {          
              new_cursor_column++;
            }

            if ((new_cursor_column != cursor_column) || (new_cursor_row != cursor_row))
            {
              //Serial.print("RIGHT Checking row: "); Serial.print(new_cursor_row); Serial.print(" col: "); Serial.println(new_cursor_column);
              if (get_tic_tac_toe_grid_value(new_cursor_row, new_cursor_column) == CHARACTER_BLANK)
              {
                result = true;
                break;
              }
            }
          }
  
          if (result == true)
          {
            break;
          }
  
          new_cursor_row++;
          if (new_cursor_row > 2)
          {
            new_cursor_row = 0;
          }
        }
      }
      break;
  }
  
  if (result == true)
  {
    cursor_row = new_cursor_row;
    cursor_column = new_cursor_column;

    musicPlayer.playTune(cursor_move_note, cursor_move_tempo);
  }
  return result;
}

uint8_t compute_cpu_joystick_direction(bool vertical, uint8_t target_row, uint8_t target_column)
{
  uint8_t result = 0;

  if ((vertical == true) && (cursor_row == target_row))
  {
    vertical = false;
  }
  else if ((vertical == false) && (cursor_column == target_column))
  {
    vertical = true;
  }

  if ((cursor_row == target_row) && (cursor_column == target_column))
  {
    result = JOYSTICK_BUTTON;
  }
  else if (vertical == true)
  {
    if (cursor_row < target_row)
    {
      result = JOYSTICK_DOWN; 
    }
    else if (cursor_row > target_row)
    {
      result = JOYSTICK_UP;
    }      
  }
  else
  {
    if (cursor_column < target_column)
    {
      result = JOYSTICK_RIGHT; 
    }
    else if (cursor_column > target_column)
    {
      result = JOYSTICK_LEFT;
    }
  }

  return result;
}

void cpu_move(uint8_t player, int move_count) 
{
  bool done = false;  
  uint16_t flash_timer = 0;
  bool flash_state = false;
  bool vertical = true;
  bool move_made = false;
  int cursor_move_count = 0;

  int move = compute_best_move(player, move_count);
  uint8_t target_row = (move / 3);
  uint8_t target_column = move % 3;
  
  cursor_row = 3;
  cursor_column = 0;

  uint16_t current_move_delay = 0;
  uint16_t max_move_delay = 1000 + random(1000);
      
  while (done == false)
  {
    bool refresh_display = false;
    bool cursor_moved = false;
    uint8_t last_cursor_row = cursor_row;
    uint8_t last_cursor_column = cursor_column;    
    uint8_t joyval = 0;

    if (current_move_delay >= max_move_delay)
    {
      joyval = compute_cpu_joystick_direction(vertical, target_row, target_column);//read_joystick_debounced();
      vertical = !vertical;

      current_move_delay = 0;
      max_move_delay = 250 + random(750);
    }
  
    if ((joyval & JOYSTICK_BUTTON) != 0)
    {
      if (cursor_row < 3)
      {
        move_made = true;
        done = true;
        tic_tac_toe_game_grid[(cursor_row * 3) + cursor_column] = (CharacterType) player; 
      }
    }
    else if ((joyval & JOYSTICK_UP) != 0)
    {
      cursor_moved = move_cursor(JOYSTICK_UP);      
    }
    else if ((joyval & JOYSTICK_DOWN) != 0)
    {
      cursor_moved = move_cursor(JOYSTICK_DOWN);      
    }
    else if ((joyval & JOYSTICK_LEFT) != 0)
    {
      cursor_moved = move_cursor(JOYSTICK_LEFT);      
    }
    else if ((joyval & JOYSTICK_RIGHT) != 0)
    {
      cursor_moved = move_cursor(JOYSTICK_RIGHT);      
    }

    if (done == false)
    {
      if (cursor_moved == true)
      {
        //Serial.print("New row: "); Serial.print(cursor_row); Serial.print(" Now col: "); Serial.println(cursor_column);
        draw_character(CHARACTER_BLANK, last_cursor_row, last_cursor_column);
        flash_timer = 0;
        flash_state = true;
        refresh_display = true;
        cursor_move_count++;

        if (cursor_move_count > 5)
        {
          Serial.println("Stuck, reversing.");
          vertical = !vertical;
          cursor_move_count = 0;
        }
      }
  
      flash_timer++;
      if (flash_timer >= CURSOR_FLASH_DELAY)
      {
        flash_timer = 0;
        flash_state = !flash_state;
        refresh_display = true;     
      }
  
      if (refresh_display == true)
      {
        draw_character((flash_state == true) ? player : CHARACTER_BLANK, cursor_row, cursor_column);
        matrix.writeDisplay();      
      }
      
      delay(1);
      current_move_delay++;
    }

    if (check_player_configuration_changed() == true)
    {
      done = true;
    }
  }

  if (move_made == true)
  {
    if (player == CHARACTER_X)
    {
      musicPlayer.playTune(cursor_select_x_note, cursor_select_x_tempo);
    }
    else
    {
      musicPlayer.playTune(cursor_select_o_note, cursor_select_o_tempo);
    }
  }
  //Serial.println("Grid after:");  
  //print_tic_tac_toe_game_grid();
  //Serial.println("========================");
}

void human_move(uint8_t player)
{
  bool done = false;
  uint16_t flash_timer = 0;
  bool flash_state = false;
  bool move_made = false;

  cursor_row = 3;
  cursor_column = 0;
      
  while (done == false)
  {
    bool refresh_display = false;
    bool cursor_moved = false;
    uint8_t last_cursor_row = cursor_row;
    uint8_t last_cursor_column = cursor_column;
    uint8_t joyval = read_joystick_debounced();
  
    if ((joyval & JOYSTICK_BUTTON) != 0)
    {
      if (cursor_row < 3)
      {
        move_made = true;
        done = true;
        tic_tac_toe_game_grid[(cursor_row * 3) + cursor_column] = (CharacterType) player; 
      }
    }
    else if ((joyval & JOYSTICK_UP) != 0)
    {
      cursor_moved = move_cursor(JOYSTICK_UP);      
    }
    else if ((joyval & JOYSTICK_DOWN) != 0)
    {
      cursor_moved = move_cursor(JOYSTICK_DOWN);      
    }
    else if ((joyval & JOYSTICK_LEFT) != 0)
    {
      cursor_moved = move_cursor(JOYSTICK_LEFT);      
    }
    else if ((joyval & JOYSTICK_RIGHT) != 0)
    {
      cursor_moved = move_cursor(JOYSTICK_RIGHT);      
    }

    if (done == false)
    {
      if (cursor_moved == true)
      {
        Serial.print("New row: "); Serial.print(cursor_row); Serial.print(" Now col: "); Serial.println(cursor_column);
        draw_character(CHARACTER_BLANK, last_cursor_row, last_cursor_column);
        flash_timer = 0;
        flash_state = true;
        refresh_display = true;
      }
  
      flash_timer++;
      if (flash_timer >= CURSOR_FLASH_DELAY)
      {
        flash_timer = 0;
        flash_state = !flash_state;
        refresh_display = true;     
      }
  
      if (refresh_display == true)
      {
        draw_character((flash_state == true) ? player : CHARACTER_BLANK, cursor_row, cursor_column);
        matrix.writeDisplay();      
      }
      
      delay(1);
    }

    if (check_player_configuration_changed() == true)
    {
      done = true;
    }
  }

  if (move_made == true)
  {
    if (player == CHARACTER_X)
    {
      musicPlayer.playTune(cursor_select_x_note, cursor_select_x_tempo);
    }
    else
    {
      musicPlayer.playTune(cursor_select_o_note, cursor_select_o_tempo);
    }
  }
}

void show_scores()
{
  typedef enum {BLANK_START, X_PLAYER, X_SCORE, BLANK_X_O, O_PLAYER, O_SCORE, BLANK_O_TIE, TIE, TIE_SCORE, BLANK_TIE_X} ScoreDisplayState;
  ScoreDisplayState score_display_state = BLANK_START;
  uint16_t elapsed = 500;
  uint16_t display_delay = 750;
  bool done = false;
  
  while (done == false)
  {
    if (check_player_configuration_changed() == true)
    {
      done = true;
      continue;
    }
    
    if ((read_joystick() & JOYSTICK_BUTTON) != 0x00)
    {
      done = true;
      continue;
    }
    
    if (elapsed >= display_delay)
    {
      switch (score_display_state)
      {
        case BLANK_START:
          draw_character(CHARACTER_BLANK, 3, 0);
          draw_blanks();
          score_display_state = X_PLAYER;
          display_delay = 250;
          break;
          
        case X_PLAYER:
          draw_character(CHARACTER_X, 3, 0);
          if (number_of_players == 0)
          {
            draw_cpu();  
          }
          else if (number_of_players == 1)
          {
            if (cpu_is_x == true)
            {
              draw_cpu();
            }
            else
            {
              draw_pl1();
            }
          }
          else
          {
            draw_pl1();
          }
          
          score_display_state = X_SCORE;
          display_delay = 750;
        break;

        case X_SCORE:
          draw_character(CHARACTER_X, 3, 0);          
          draw_score(total_x_wins);
          score_display_state = BLANK_X_O;
          display_delay = 750;
          break;

        case BLANK_X_O:
          draw_character(CHARACTER_BLANK, 3, 0);
          draw_blanks();
          score_display_state = O_PLAYER;
          display_delay = 250;
          break;
          
        case O_PLAYER:
          draw_character(CHARACTER_O, 3, 0);
          if (number_of_players == 0)
          {
            draw_cpu();  
          }
          else if (number_of_players == 1)
          {
            if (cpu_is_x == true)
            {
              draw_pl1();
            }
            else
            {
              draw_cpu();
            }
          }
          else
          {
            draw_pl2();
          }
          
          score_display_state = O_SCORE;
          display_delay = 750;
          break;

        case O_SCORE:
          draw_character(CHARACTER_O, 3, 0);          
          draw_score(total_o_wins);
          score_display_state = BLANK_O_TIE;
          display_delay = 750;
          break;

        case BLANK_O_TIE:
          draw_character(CHARACTER_BLANK, 3, 0);
          draw_blanks();
          score_display_state = TIE;
          display_delay = 250;
          break;
        
        case TIE:
          draw_character(CHARACTER_TIE, 3, 0);          
          draw_tie();
          score_display_state = TIE_SCORE;
          display_delay = 750;
          break;
          
        case TIE_SCORE:
          draw_character(CHARACTER_TIE, 3, 0);          
          draw_score(total_ties);
          score_display_state = BLANK_TIE_X;
          display_delay = 750;
          break;

        case BLANK_TIE_X:
          draw_character(CHARACTER_BLANK, 3, 0);
          draw_blanks();
          score_display_state = X_PLAYER;
          display_delay = 500;

          if (number_of_players == 0)
          {
            done = true;
          }
          break;
      }
      
      matrix.writeDisplay();
      elapsed = 0;
    }
    else
    {
      delay(1);
      elapsed++;
    }
  }
}

void show_player_configuration()
{
  typedef enum {BLANK_START, X_PLAYER, BLANK_X_O, O_PLAYER, BLANK_O_X_PLAYER} ScoreDisplayState;
  ScoreDisplayState score_display_state = BLANK_START;
  uint16_t elapsed = 500;
  uint16_t display_delay = 750;
  bool done = false;
  
  while (done == false)
  {
    if (check_player_configuration_changed() == true)
    {
      done = true;
      continue;
    }
    
    if ((read_joystick() & JOYSTICK_BUTTON) != 0x00)
    {
      done = true;
      continue;
    }
    
    if (elapsed >= display_delay)
    {
      switch (score_display_state)
      {
        case BLANK_START:
          draw_character(CHARACTER_BLANK, 3, 0);
          draw_blanks();
          score_display_state = X_PLAYER;
          display_delay = 250;
          break;
          
        case X_PLAYER:
          draw_character(CHARACTER_X, 3, 0);
          if (number_of_players == 0)
          {
            draw_cpu();  
          }
          else if (number_of_players == 1)
          {
            if (cpu_is_x == true)
            {
              draw_cpu();
            }
            else
            {
              draw_pl1();
            }
          }
          else
          {
            draw_pl1();
          }
          
          score_display_state = BLANK_X_O;
          display_delay = 750;
        break;

        case BLANK_X_O:
          draw_character(CHARACTER_BLANK, 3, 0);
          draw_blanks();
          score_display_state = O_PLAYER;
          display_delay = 250;
          break;
          
        case O_PLAYER:
          draw_character(CHARACTER_O, 3, 0);
          if (number_of_players == 0)
          {
            draw_cpu();  
          }
          else if (number_of_players == 1)
          {
            if (cpu_is_x == true)
            {
              draw_pl1();
            }
            else
            {
              draw_cpu();
            }
          }
          else
          {
            draw_pl2();
          }
          
          score_display_state = BLANK_O_X_PLAYER;
          display_delay = 750;
          break;
          
        case BLANK_O_X_PLAYER:
          draw_character(CHARACTER_BLANK, 3, 0);
          draw_blanks();
          score_display_state = X_PLAYER;
          display_delay = 250;
          break;

          if (number_of_players == 0)
          {
            done = true;
          }
          break;
      }
      
      matrix.writeDisplay();
      elapsed = 0;
    }
    else
    {
      delay(1);
      elapsed++;
    }
  }
}

void show_winner(CharacterType winning_character)
{
  Winner winner;
  int8_t win_index;  
  
  Serial.print("Winner: "); 
  switch (winning_character)
  {
    case CHARACTER_BLANK:
      Serial.print("TIE");
      musicPlayer.playTune(shaveMelody, shaveTempo);
      total_ties++; 
      total_ties %= 100;         
      winner = TIE;
      //draw_character(CHARACTER_TIE, 3, 0);
      break;
    case CHARACTER_X:
      Serial.print("X");
      musicPlayer.playTune(starwarsMelody, starwarsTempo);
      total_x_wins++;
      total_x_wins %= 100;
      if ((number_of_players == 0) || ((number_of_players == 1) && (cpu_is_x == true)))
      {
        winner = CPU;
      }
      else
      {
        winner = PLAYER_1;
      }
      break;
    case CHARACTER_O:
      musicPlayer.playTune(imperialMelody, imperialTempo);
      Serial.print("O");
      total_o_wins++;
      total_o_wins %= 100;
      if ((number_of_players == 0) || ((number_of_players == 1) && (cpu_is_x == false)))
      {
        winner = CPU;
      }
      else if ((number_of_players < 2) && (cpu_is_x == true))
      {
        winner = PLAYER_1;
      }
      else
      {
        winner = PLAYER_2;
      }

      break;        
  }

  Serial.print(" Totals: X: "); Serial.print(total_x_wins); Serial.print(" O: "); Serial.print(total_o_wins); Serial.print(" Ties: "); Serial.println(total_ties);      

  draw_character(CHARACTER_BLANK, 3, 0);

  win_index = get_win_pattern_index();      
  for (uint8_t count = 0; count < 6; count++)
  {          
      draw_blanks();
      if (winner != TIE)
      {
        tic_tac_toe_game_grid[win_patterns[win_index][0]] = CHARACTER_BLANK;
        tic_tac_toe_game_grid[win_patterns[win_index][1]] = CHARACTER_BLANK;
        tic_tac_toe_game_grid[win_patterns[win_index][2]] = CHARACTER_BLANK;
      }
      draw_tic_tac_toe_game_grid();
      delay(250);

      switch (winner)
      {
        case TIE:
          draw_tie();
          break;
        case CPU:
          draw_cpu();
          break;
        case PLAYER_1:
          draw_pl1();
          break;
        case PLAYER_2:
          draw_pl2();
          break;
      }
      if (winner != TIE)
      {
        tic_tac_toe_game_grid[win_patterns[win_index][0]] = winning_character;
        tic_tac_toe_game_grid[win_patterns[win_index][1]] = winning_character;
        tic_tac_toe_game_grid[win_patterns[win_index][2]] = winning_character;
      }
      draw_tic_tac_toe_game_grid();
      delay(250);
  }

  delay(250);
}


static unsigned int int_count = 0;
void timer_interrupt() 
{
   musicPlayer.updateMusic(1);
   int_count++;
}

void setup() 
{
  Serial.begin(9600);
  Serial.println("Tic Tac Toe");
  
  matrix.begin(0x70);  // pass in the address

  pinMode(PIN_JOYSTICK_X, INPUT);
  pinMode(PIN_JOYSTICK_Y, INPUT);
  pinMode(PIN_JOYSTICK_BUTTON, INPUT_PULLUP);
  pinMode(PIN_CPU_SKILL_BIT_0, INPUT_PULLUP);
  pinMode(PIN_CPU_SKILL_BIT_1, INPUT_PULLUP);
  pinMode(PIN_X_CPU_OR_HUMAN, INPUT_PULLUP);  
  pinMode(PIN_O_CPU_OR_HUMAN, INPUT_PULLUP);  
  pinMode(PIN_AUDIO_OUT, OUTPUT);
  
  // 1K Interrupt.
  Timer1.initialize(1000);         
  Timer1.attachInterrupt(timer_interrupt);   

  //musicPlayer.playTune(shaveMelody, shaveTempo);
  //musicPlayer.playTune(chargeMelody, chargeTempo);
  //musicPlayer.playTune(starwarsMelody, starwarsTempo);
  //musicPlayer.playTune(imperialMelody, imperialTempo);
  

  
  //tone(8, 1000, 10000);
  //delay(1000);
  //noTone(8);
}

bool check_player_configuration_changed()
{
  bool result = false;
  uint8_t new_number_of_players = read_num_players();
  bool new_cpu_is_x = read_cpu_is_x();
  CpuSkillLevel new_cpu_skill_level = read_cpu_skill_level();

  if (number_of_players != new_number_of_players)
  {
    number_of_players = new_number_of_players;
    result = true;
  }
  
  if (cpu_is_x != new_cpu_is_x)
  {
    cpu_is_x = new_cpu_is_x;
    if (number_of_players == 1)
    {
      result = true;
    }
  }

  if (cpu_skill_level != new_cpu_skill_level)
  {
    cpu_skill_level = new_cpu_skill_level;
    if (number_of_players < 2)
    {
      result = true;
    }
    Serial.print("CPU SKILL LEVEL: "); Serial.println(cpu_skill_level);
  }

  if (result == true)
  {
    player_configuration_changed = true;
  }

  return result;
}

void loop() {
  CharacterType winning_character;  

  if (last_game_state != game_state)
  {
    Serial.print("New game state: ");
    Serial.println(game_state_names[game_state]);
    last_game_state = game_state;
  }
  switch (game_state)
  {
    case NEW_PLAYER_CONFIGURATION:
      total_x_wins = 0;
      total_o_wins = 0;
      total_ties = 0;
      for (int index = 0; index < 9; index++)
      {
        tic_tac_toe_game_grid[index] = CHARACTER_BLANK;
      }
      draw_blanks();
      draw_character(CHARACTER_BLANK, 3, 0);
      draw_tic_tac_toe_game_grid();
      delay(250);

      player_configuration_changed = false;
      show_player_configuration();
      
      game_state = NEW_GAME;
      break;
      
    case NEW_GAME:
      Serial.print("Seeding random with: "); Serial.println(int_count);
      randomSeed(int_count);
      
      total_move_count = 0;

      Serial.print("New game, num players: "); Serial.println(number_of_players);
      if (number_of_players == 1)
      {
        Serial.print("CPU is: "); Serial.println((cpu_is_x == true) ? "X" : "O");
      }

      draw_blanks();
      draw_character(CHARACTER_BLANK, 3, 0);
      for (int index = 0; index < 9; index++)
      {
        tic_tac_toe_game_grid[index] = CHARACTER_BLANK;
      }
      
      draw_tic_tac_toe_game_grid();

      switch (number_of_players)
      {
        case 2:
          game_state = HUMAN_X_TURN;  
          break;

        case 1:
          if (cpu_is_x == true)
          {
            game_state = CPU_X_TURN;
          }
          else
          {
            game_state = HUMAN_X_TURN;
          }
          break;
          
        case 0:
        default:
          game_state = CPU_X_TURN;  
          break;
      }
      break;
      
    case CPU_X_TURN:
      draw_cpu();      
      cpu_move(CHARACTER_X, total_move_count); 
      draw_tic_tac_toe_game_grid();
      total_move_count++;
      //Serial.print("Moves: "); Serial.print(total_move_count); Serial.print(" Check win: "); Serial.println(check_win());

      if ((total_move_count >= 9) || (check_win() != CHARACTER_BLANK))
      {
        game_state = SHOW_WINNER;
      }
      else
      {
        game_state = (number_of_players == 0) ? CPU_O_TURN : HUMAN_O_TURN;
      }
      //delay(500);
      break;
      
    case CPU_O_TURN:
      draw_cpu();
      cpu_move(CHARACTER_O, total_move_count); 
      draw_tic_tac_toe_game_grid();
      total_move_count++;
      //Serial.print("Moves: "); Serial.print(total_move_count); Serial.print(" Check win: "); Serial.println(check_win());      
      if ((total_move_count >= 9) || (check_win() != CHARACTER_BLANK))
      {
        game_state = SHOW_WINNER;
      }
      else
      {
        game_state = (number_of_players == 0) ? CPU_X_TURN : HUMAN_X_TURN;
      }
      break;

    case HUMAN_X_TURN:
      draw_pl1();
      human_move(CHARACTER_X);
      draw_tic_tac_toe_game_grid();
      total_move_count++;
      if ((total_move_count >= 9) || (check_win() != CHARACTER_BLANK))
      {
        game_state = SHOW_WINNER;
      }
      else
      {
        game_state = (number_of_players == 1) ? CPU_O_TURN : HUMAN_O_TURN;
      }
      break;

    case HUMAN_O_TURN:
      if (number_of_players == 1)
      {
        draw_pl1();
      }
      else
      {
        draw_pl2();
      }
      human_move(CHARACTER_O);
      draw_tic_tac_toe_game_grid();
      total_move_count++;
      if ((total_move_count >= 9) || (check_win() != CHARACTER_BLANK))
      {
        game_state = SHOW_WINNER;
      }
      else
      {
        game_state = (number_of_players == 1) ? CPU_X_TURN : HUMAN_X_TURN;
      }
      break;
    
    case SHOW_WINNER:
      delay(250);
      winning_character = (CharacterType) check_win();
      show_winner(winning_character);
      game_state = GAME_OVER;
      break;

    case GAME_OVER:
      show_scores();
      game_state = NEW_GAME;      
      break;
      
    default:
      delay(100);
  }

   if (player_configuration_changed == true)
   {
    game_state = NEW_PLAYER_CONFIGURATION;
   }
}
