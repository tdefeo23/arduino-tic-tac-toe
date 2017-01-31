
#include "Arduino.h"
#include "input.h"
#include "pin_assignments.h"

uint8_t read_joystick()
{
    uint8_t result = 0;

    int x = analogRead(PIN_JOYSTICK_X);
    int y = analogRead(PIN_JOYSTICK_Y);
    int button = digitalRead(PIN_JOYSTICK_BUTTON);

    if (x > 1000)
    {
        result |= JOYSTICK_RIGHT;
    }
    else if (x < 10)
    {
        result |= JOYSTICK_LEFT;
    }

    if (y > 1000)
    {
        result |= JOYSTICK_DOWN;
    }
    else if (y < 10)
    {
        result |= JOYSTICK_UP;
    }

    if (button == 0)
    {
        result |= JOYSTICK_BUTTON;
    }

    return result;
}

uint8_t read_joystick_debounced()
{
    static uint8_t last_joyval = 0;
    static uint8_t joystick_debounce = 0;

    uint8_t result = 0;
    uint8_t joyval = read_joystick();

    if ((last_joyval != 0) && (joyval != 0))
    {
        joystick_debounce = 0;
        return result;
    }

    joystick_debounce++;
    if (joystick_debounce >= 16)
    {
        if (joyval != 0)
        {
            joystick_debounce = 0;
            last_joyval = joyval;
            result = joyval;
        }
        else
        {
            joystick_debounce = 16;
            last_joyval = 0;
        }
    }

    return result;
}

void print_joystick_value(uint8_t value)
{
    Serial.print("Joystick: ");

    if (value == 0)
    {
        Serial.println("NONE");
    }
    else
    {
        if (value & JOYSTICK_LEFT)
        {
            Serial.print("LEFT ");
        }
        if (value & JOYSTICK_RIGHT)
        {
            Serial.print("RIGHT ");
        }
        if (value & JOYSTICK_UP)
        {
            Serial.print("UP ");
        }
        if (value & JOYSTICK_DOWN)
        {
            Serial.print("DOWN ");
        }
        if (value & JOYSTICK_BUTTON)
        {
            Serial.print("BUTTON ");
        }
        Serial.println();
    }
}

uint8_t read_num_players()
{
    uint8_t result = 0;

    int x_cpu_or_human = digitalRead(PIN_X_CPU_OR_HUMAN);
    int o_cpu_or_human = digitalRead(PIN_O_CPU_OR_HUMAN);

    if (x_cpu_or_human == 0)
    {
        result++;
    }

    if (o_cpu_or_human == 0)
    {
        result++;
    }

    return result;
}

bool read_cpu_is_x()
{
    return (digitalRead(PIN_X_CPU_OR_HUMAN) == 1);
}

CpuSkillLevel read_cpu_skill_level()
{
    CpuSkillLevel result = CPU_SKILL_BEGINNER;
    int skill_bit_0 = digitalRead(PIN_CPU_SKILL_BIT_0);
    int skill_bit_1 = digitalRead(PIN_CPU_SKILL_BIT_1);

    //Serial.print(skill_bit_0); Serial.print(" "); Serial.println(skill_bit_1);
    if ((skill_bit_0 == 1) && (skill_bit_1 == 1))
    {
        result = CPU_SKILL_ADVANCED;
    }
    else if (skill_bit_0 == 1)
    {
        result = CPU_SKILL_BEGINNER;
    }
    else
    {
        result = CPU_SKILL_EXPERT;
    }

    return result;
}

