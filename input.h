
typedef enum
{
    CPU_SKILL_BEGINNER,
    CPU_SKILL_ADVANCED,
    CPU_SKILL_EXPERT
} CpuSkillLevel;

// Bit values returned from read_joystick.
#define JOYSTICK_LEFT   (0x01 << 0)
#define JOYSTICK_RIGHT  (0x01 << 1)
#define JOYSTICK_UP     (0x01 << 2)
#define JOYSTICK_DOWN   (0x01 << 3)
#define JOYSTICK_BUTTON (0x01 << 4)

extern uint8_t read_joystick();
extern uint8_t read_joystick_debounced();

extern void print_joystick_value(uint8_t value);

extern uint8_t read_num_players();
extern bool read_cpu_is_x();
extern CpuSkillLevel read_cpu_skill_level();
