#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

#include "lcd_1602_i2c.h"

#define BUTTON_1 14
#define BUTTON_2 15

bool running = false;
bool do_clear_screen = false;
time_t measured_time = 0;
struct repeating_timer timer;

char messages[MAX_LINES][MAX_CHARS + 1] =
{
    "0:00:00",
    ""
};

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    if (!running)
    {
        return 0;
    }

    measured_time += 1;
    int minutes = measured_time / 360000;
    int seconds = measured_time / 100;
    int deca_seconds = measured_time % 100;
    sprintf(messages[0], "%d:%0.2d:%0.2d", minutes, seconds, deca_seconds);
    return -10000;
}

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_1)
    {
        if (running)
        {
            running = false;
        }
        else
        {
            add_alarm_in_ms(10, alarm_callback, NULL, false);
            running = true;
        }
    }
    else if (gpio == BUTTON_2)
    {
        if (running)
        {
            sprintf(messages[1], "Lap: %s", messages[0]);
        }
        else
        {
            measured_time = 0;
            sprintf(messages[0], "%d:%0.2d:%0.2d", 0, 0, 0);
            sprintf(messages[1], "");
            do_clear_screen = true;
        }
    }
}

void init_buttons()
{
    // Button 1
    gpio_init(BUTTON_1);
    gpio_set_dir(BUTTON_1, GPIO_IN);
    gpio_pull_up(BUTTON_1);
    gpio_set_irq_enabled_with_callback(BUTTON_1, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Button 2
    gpio_init(BUTTON_2);
    gpio_set_dir(BUTTON_2, GPIO_IN);
    gpio_pull_up(BUTTON_2);
    gpio_set_irq_enabled_with_callback(BUTTON_2, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
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

    init_buttons();

    while (1) {
        // Print message
        lcd_set_cursor(0, MAX_CHARS - strlen(messages[0]));
        lcd_string(messages[0]);
        lcd_set_cursor(1, MAX_CHARS - strlen(messages[1]));
        lcd_string(messages[1]);

        if (running)
        {
            gpio_put(PICO_DEFAULT_LED_PIN, true);
            sleep_ms(100);
            gpio_put(PICO_DEFAULT_LED_PIN, false);
        }
        else
        {
            sleep_ms(100);
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
