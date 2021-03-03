#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

// This code allows to use the Rasperry Pi PICO as ArduinoWS2812USBDevice.

#define VERSION 1

//#### CONFIGURATION ####

#define CHANNELS 8 // change this only if you add or remove channels in the implementation-part. To disable channels set them to 0 leds.

// no more than 255 leds per channel
#define LEDS_CHANNEL_1 255
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

#define SERIAL_PROMPT '>'

//#######################

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
uint8_t data_channel_1[BUFFER_SIZE_CHANNEL_1];
uint8_t data_channel_2[BUFFER_SIZE_CHANNEL_2];
uint8_t data_channel_3[BUFFER_SIZE_CHANNEL_3];
uint8_t data_channel_4[BUFFER_SIZE_CHANNEL_4];
uint8_t data_channel_5[BUFFER_SIZE_CHANNEL_5];
uint8_t data_channel_6[BUFFER_SIZE_CHANNEL_6];
uint8_t data_channel_7[BUFFER_SIZE_CHANNEL_7];
uint8_t data_channel_8[BUFFER_SIZE_CHANNEL_8];

uint8_t *buffers[] = {(uint8_t *)buffer_channel_1, (uint8_t *)buffer_channel_2, (uint8_t *)buffer_channel_3, (uint8_t *)buffer_channel_4, (uint8_t *)buffer_channel_5, (uint8_t *)buffer_channel_6, (uint8_t *)buffer_channel_7, (uint8_t *)buffer_channel_8};
uint8_t *channels[] = {(uint8_t *)data_channel_1, (uint8_t *)data_channel_2, (uint8_t *)data_channel_3, (uint8_t *)data_channel_4, (uint8_t *)data_channel_5, (uint8_t *)data_channel_6, (uint8_t *)data_channel_7, (uint8_t *)data_channel_8};

bool buffer_dirty[CHANNELS];
bool data_dirty[CHANNELS];

static inline void send(uint8_t data)
{
  printf("%c", data);
}

static inline void prompt()
{
  send(SERIAL_PROMPT);
}

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

void update_buffer(int channel)
{
  int length = buffer_sizes[channel];
  uint8_t *buffer = buffers[channel];

  for (int i = 0; i < length; i++)
  {
    int read = getchar_timeout_us(100000);
    if (read < 0)
    {
      __builtin_memset(buffer, 0, (uint)length);
      break;
    }
    buffer[i] = (uint8_t)read;
  }

  buffer_dirty[channel] = true;
}

void update_channel(int channel)
{
  PIO pio = CHANNEL_PIO[channel];
  uint sm = CHANNEL_SM[channel];
  uint8_t *data = channels[channel];
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

  data_dirty[channel] = false;
}

void update_leds()
{
  for (int i = 0; i < CHANNELS; i++)
  {
    if (data_dirty[i])
    {
      update_channel(i);
    }
  }

  sleep_us(400);
}

void stage_channel(int channel)
{
  int length = buffer_sizes[channel];
  uint8_t *buffer = buffers[channel];
  uint8_t *data = channels[channel];

  __builtin_memcpy(data, buffer, length);
  data_dirty[channel] = buffer_dirty[channel];
}

void stage_data()
{
  for (int i = 0; i < CHANNELS; i++)
  {
    if (buffer_dirty[i])
    {
      stage_channel(i);
    }
  }
}

void setup()
{
  stdio_init_all();

  for (int i = 0; i < CHANNELS; i++)
  {
    if (led_counts[i] > 0)
    {
      init_channel(i);

      int length = buffer_sizes[i];
      uint8_t *buffer = buffers[i];
      uint8_t *data = channels[i];

      __builtin_memset(buffer, 0, (uint)length);
      __builtin_memset(data, 0, (uint)length);
      buffer_dirty[i] = true;
    }
  }

  prompt();
}

void loop_core1()
{
  stage_data();
  update_leds();
  multicore_fifo_push_blocking(2);
  while (1)
  {
    multicore_fifo_pop_blocking();
    stage_data();
    multicore_fifo_push_blocking(1);
    update_leds();
    multicore_fifo_push_blocking(2);
  }
}

void loop()
{
  int command = getchar_timeout_us(1000);
  if (command < 0)
  {
    return;
  }

  int request = command & 0x0F;
  int channel = (command >> 4) & 0x0F;

  if (channel == 0) // Device-commands
  {
    if (request == 0x01)
    {
      send(CHANNELS);
      prompt();
    }
    else if (request == 0x02)
    {
      multicore_fifo_pop_blocking();
      multicore_fifo_push_blocking(1);
      multicore_fifo_pop_blocking();
      prompt();
    }
    else if (request == 0x0E)
    {
      send(VERSION);
      prompt();
    }
    else if (request == 0x0F)
    {
      prompt();
    }
  }
  else if (channel <= CHANNELS)
  {
    channel--;
    if (request == 0x01)
    {
      send(led_counts[channel]);
      prompt();
    }
    else if (request == 0x02)
    {
      update_buffer(channel);
      prompt();
    }
  }
}

int main()
{
  setup();

  multicore_launch_core1(loop_core1);

  while (1)
  {
    loop();
  }
}
