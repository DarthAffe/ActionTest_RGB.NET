#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#include "bsp/board.h"
#include "tusb.h"

// This code allows to use the Rasperry Pi PICO as HIDWS2812USBDevice.

#define VERSION 1

//#### CONFIGURATION ####

#define CHANNELS 8 // change this only if you add or remove channels in the implementation-part. To disable channels set them to 0 leds.

// no more than 255 leds per channel
#define LEDS_CHANNEL_1 3
#define LEDS_CHANNEL_2 0
#define LEDS_CHANNEL_3 0
#define LEDS_CHANNEL_4 0
#define LEDS_CHANNEL_5 0
#define LEDS_CHANNEL_6 0
#define LEDS_CHANNEL_7 0
#define LEDS_CHANNEL_8 0

#define PIN_CHANNEL_1 8
#define PIN_CHANNEL_2 9
#define PIN_CHANNEL_3 10
#define PIN_CHANNEL_4 11
#define PIN_CHANNEL_5 12
#define PIN_CHANNEL_6 13
#define PIN_CHANNEL_7 14
#define PIN_CHANNEL_8 15

//#######################

#define OFFSET_MULTIPLIER 60

#define BUFFER_SIZE_CHANNEL_1 LEDS_CHANNEL_1 * 3
#define BUFFER_SIZE_CHANNEL_2 LEDS_CHANNEL_2 * 3
#define BUFFER_SIZE_CHANNEL_3 LEDS_CHANNEL_3 * 3
#define BUFFER_SIZE_CHANNEL_4 LEDS_CHANNEL_4 * 3
#define BUFFER_SIZE_CHANNEL_5 LEDS_CHANNEL_5 * 3
#define BUFFER_SIZE_CHANNEL_6 LEDS_CHANNEL_6 * 3
#define BUFFER_SIZE_CHANNEL_7 LEDS_CHANNEL_7 * 3
#define BUFFER_SIZE_CHANNEL_8 LEDS_CHANNEL_8 * 3

const PIO CHANNEL_PIO[CHANNELS] = {pio0, pio0, pio0, pio0, pio1, pio1, pio1, pio1};
const uint CHANNEL_SM[CHANNELS] = {0, 1, 2, 3, 0, 1, 2, 3};

const int pins[CHANNELS] = {PIN_CHANNEL_1, PIN_CHANNEL_2, PIN_CHANNEL_3, PIN_CHANNEL_4, PIN_CHANNEL_5, PIN_CHANNEL_6, PIN_CHANNEL_7, PIN_CHANNEL_8};
const int led_counts[CHANNELS] = {LEDS_CHANNEL_1, LEDS_CHANNEL_2, LEDS_CHANNEL_3, LEDS_CHANNEL_4, LEDS_CHANNEL_5, LEDS_CHANNEL_6, LEDS_CHANNEL_7, LEDS_CHANNEL_8};
const int buffer_sizes[CHANNELS] = {BUFFER_SIZE_CHANNEL_1, BUFFER_SIZE_CHANNEL_2, BUFFER_SIZE_CHANNEL_3, BUFFER_SIZE_CHANNEL_4, BUFFER_SIZE_CHANNEL_5, BUFFER_SIZE_CHANNEL_6, BUFFER_SIZE_CHANNEL_7, BUFFER_SIZE_CHANNEL_8};

uint8_t buffer_channel_1[BUFFER_SIZE_CHANNEL_1];
uint8_t buffer_channel_2[BUFFER_SIZE_CHANNEL_2];
uint8_t buffer_channel_3[BUFFER_SIZE_CHANNEL_3];
uint8_t buffer_channel_4[BUFFER_SIZE_CHANNEL_4];
uint8_t buffer_channel_5[BUFFER_SIZE_CHANNEL_5];
uint8_t buffer_channel_6[BUFFER_SIZE_CHANNEL_6];
uint8_t buffer_channel_7[BUFFER_SIZE_CHANNEL_7];
uint8_t buffer_channel_8[BUFFER_SIZE_CHANNEL_8];

uint8_t *buffers[] = {(uint8_t *)buffer_channel_1, (uint8_t *)buffer_channel_2, (uint8_t *)buffer_channel_3, (uint8_t *)buffer_channel_4, (uint8_t *)buffer_channel_5, (uint8_t *)buffer_channel_6, (uint8_t *)buffer_channel_7, (uint8_t *)buffer_channel_8};
uint8_t  sendBuffer[64];

static inline void put_pixel(PIO pio, int sm, uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | ((uint32_t)(b));
}

void init_channel(int channel)
{
    PIO pio = CHANNEL_PIO[channel];
    uint sm = CHANNEL_SM[channel];
    int pin = pins[channel];

    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, pin, 800000, false);
}

void update_channel(int channel)
{
    PIO pio = CHANNEL_PIO[channel];
    uint sm = CHANNEL_SM[channel];
    uint8_t *data = buffers[channel];
    int count = led_counts[channel];

    for (int i = count - 1; i >= 0; i--)
    {
        int offset = i * 3;
        uint8_t r = data[offset];
        uint8_t g = data[offset + 1];
        uint8_t b = data[offset + 2];
        uint32_t pixel = urgb_u32(r, g, b);
        put_pixel(pio, sm, pixel);
    }
}

void stage_channel_update(uint8_t channel, uint8_t const *buffer, uint16_t length)
{
    uint8_t *data = buffers[channel];
    bool update = buffer[0] > 0;
    int offset = buffer[1] * OFFSET_MULTIPLIER;
    int dataLength = length - 2;
    if ((offset + dataLength) > buffer_sizes[channel])
    {
        dataLength = buffer_sizes[channel] - offset;
        if (dataLength <= 0)
        {
            return;
        }
    }
    multicore_fifo_pop_blocking();
    __builtin_memcpy(data + offset, buffer + 2, dataLength);
    if (update)
    {
        multicore_fifo_push_blocking(channel);
    }
    else
    {
        multicore_fifo_push_blocking(0xFF);
    }
}

void send_info(uint8_t command, uint8_t info)
{
    sendBuffer[0] = info;
    tud_hid_report(0, sendBuffer, 64);
}

void process_command(uint8_t command, uint8_t const *data, uint16_t length)
{
    int request = command & 0x0F;
    int channel = (command >> 4) & 0x0F;

    if (channel == 0) // Device-commands
    {
        if (request == 0x01)
        {
            send_info(command, CHANNELS);
        }
        else if (request == 0x0F)
        {
            send_info(command, VERSION);
        }
    }
    else if (channel <= CHANNELS)
    {
        channel--;
        if (request == 0x01)
        {
            send_info(command, led_counts[channel]);
        }
        else if (request == 0x02)
        {
            stage_channel_update(channel, data, length);
        }
    }
}

void reset()
{
    for (int i = 0; i < CHANNELS; i++)
    {
        if (led_counts[i] > 0)
        {
            uint8_t *data = buffers[i];
            int length = buffer_sizes[i];

            multicore_fifo_pop_blocking();
            __builtin_memset(data, 0, length);
            multicore_fifo_push_blocking(i);
        }
    }
}

void setup()
{
    for (int i = 0; i < CHANNELS; i++)
    {
        if (led_counts[i] > 0)
        {
            init_channel(i);
        }
    }
}

void loop_core1()
{
    multicore_fifo_push_blocking(0);
    while (1)
    {
        uint32_t channel = multicore_fifo_pop_blocking();
        if (channel < CHANNELS)
        {
            update_channel(channel);
        }
        multicore_fifo_push_blocking(channel);
    }
}

uint16_t tud_hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

void tud_hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    (void)report_type;

    process_command(buffer[0], buffer + 1, bufsize - 1);
}

void tud_umount_cb(void)
{
    reset();
}

int main(void)
{
    setup();

    multicore_launch_core1(loop_core1);

    tusb_init();

    while (1)
    {
        tud_task();
    }

    return 0;
}
