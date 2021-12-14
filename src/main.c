#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

#include "lcd_1602_i2c.h"

// Number of laps stored
#define MAX_LAPS 99

// Start/Stop (pause) button
#define BUTTON_1 13
// Lap/Reset button
#define BUTTON_2 14
// Show next lap button
#define BUTTON_3 15

bool running = false;
bool do_clear_screen = false;
time_t measured_time = 0;
int laps = 0;
int shown_lap = 0;
struct repeating_timer timer;

char message_time[MAX_CHARS + 1] = "0:00:00";
char messages_lap[MAX_LAPS + 1][MAX_CHARS + 1];

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    if (!running)
    {
        return 0;
    }

    measured_time += 1;
    int minutes = measured_time / 6000;
    int seconds = (measured_time % 6000) / 100;
    int centi_seconds = measured_time % 100;
    sprintf(message_time, "%d:%0.2d:%0.2d", minutes, seconds, centi_seconds);
    return -10000;
}

unsigned button_1_last_press = 0;
unsigned button_2_last_press = 0;
unsigned button_3_last_press = 0;
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_1)
    {
        if (button_1_last_press + 200 > millis())
        {
            return;
        }
        else
        {
            button_1_last_press = millis();
        }

        if (running)
        {
            running = false;
        }
        else
        {
            add_alarm_in_ms(10, alarm_callback, NULL, false);
            shown_lap = laps;
            running = true;
        }
    }
    else if (gpio == BUTTON_2)
    {
        if (button_2_last_press + 200 > millis())
        {
            return;
        }
        else
        {
            button_2_last_press = millis();
        }

        if (running)
        {
            if (laps < MAX_LAPS)
            {
                // Increment both variables
                shown_lap = ++laps;
                sprintf(messages_lap[shown_lap], "Lap %d: %s", shown_lap, message_time);
            }
        }
        else
        {
            measured_time = 0;
            sprintf(message_time, "%d:%0.2d:%0.2d", 0, 0, 0);
            for (int i = 0; i < laps; i++)
            {
                sprintf(messages_lap[i], "");
            }
            shown_lap = laps = 0;
            do_clear_screen = true;
        }
    }
    else if (gpio == BUTTON_3)
    {
        if (button_3_last_press + 200 > millis())
        {
            return;
        }
        else
        {
            button_3_last_press = millis();
        }

        if (!running)
        {
            shown_lap++;
            if (shown_lap > laps)
            {
                shown_lap = 1;
            }
            do_clear_screen = true;
        }
    }
}

void init_button(int button)
{
    gpio_init(button);
    gpio_set_dir(button, GPIO_IN);
    gpio_pull_up(button);
    gpio_set_irq_enabled_with_callback(button, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}

int main() {
#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning i2c/lcd_1602_i2c example requires a board with I2C pins
#else
    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    lcd_init();

    // Init status LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // Init button
    init_button(BUTTON_1);
    init_button(BUTTON_2);
    init_button(BUTTON_3);

    while (1) {
        // Print message
        lcd_set_cursor(0, MAX_CHARS - strlen(message_time));
        lcd_string(message_time);
        lcd_set_cursor(1, MAX_CHARS - strlen(messages_lap[shown_lap]));
        lcd_string(messages_lap[shown_lap]);

        if (running)
        {
            gpio_put(PICO_DEFAULT_LED_PIN, true);
            sleep_ms(10);
            gpio_put(PICO_DEFAULT_LED_PIN, false);
        }
        else
        {
            sleep_ms(10);
        }

        if (do_clear_screen)
        {
            lcd_clear();
            do_clear_screen = false;
        }
    }

    return 0;
#endif
}
