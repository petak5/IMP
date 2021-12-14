/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

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
struct repeating_timer timer;

char messages[MAX_LINES][MAX_CHARS + 1] =
{
    "            0:00",
    "disabled display"
};

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    if (!running)
    {
        return 0;
    }

    gpio_put(PICO_DEFAULT_LED_PIN, true);
    int sum = 0;
    for (int i = 0; i < 1000000; i++)
    {
        sum += i;
    }
    gpio_put(PICO_DEFAULT_LED_PIN, !sum);
    messages[0][MAX_CHARS - 1]++;
    return -1000000;
}

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_1)
    {
        if (!running)
        {
            //add_repeating_timer_ms(-1000, repeating_timer_callback, NULL, &timer);
            add_alarm_in_ms(1000, alarm_callback, NULL, false);
            running = true;
            //sleep_ms(2000);
            //cancel_repeating_timer(&timer);
        }
        //gpio_put(PICO_DEFAULT_LED_PIN, is_led_on);
        //is_led_on = !is_led_on;
        //sleep_ms(1);
        //gpio_put(PICO_DEFAULT_LED_PIN, false);
    }
    else if (gpio == BUTTON_2)
    {
        if (running)
        {
            running = false;
            //gpio_put(PICO_DEFAULT_LED_PIN, false);
        }
    }
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

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // Button 1
    gpio_init(BUTTON_1);
    gpio_set_dir(BUTTON_1, GPIO_IN);
    gpio_pull_up(BUTTON_1);

    // Button 2
    gpio_init(BUTTON_2);
    gpio_set_dir(BUTTON_2, GPIO_IN);
    gpio_pull_up(BUTTON_2);

    gpio_set_irq_enabled_with_callback(BUTTON_1, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_2, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);



    //add_alarm_in_ms(1000, alarm_callback, NULL, false);
    // while(1)
    // {
    //     tight_loop_contents();
    // }

    lcd_set_cursor(0, 0);
    lcd_string(messages[0]);

    while (1) {
        // Print message
        lcd_set_cursor(0, 0);
        lcd_string(messages[0]);
        //gpio_put(PICO_DEFAULT_LED_PIN, running);

        //messages[0][MAX_CHARS]++;
        //lcd_clear();

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
    }

    return 0;
#endif
}
