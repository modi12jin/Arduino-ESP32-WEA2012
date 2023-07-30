#ifndef _SPD2010_LCD_H
#define _SPD2010_LCD_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "Arduino.h"

#define LCD_SPI_HOST SPI2_HOST
#define LCD_H_RES 412
#define LCD_V_RES 412
#define SEND_BUF_SIZE (0x4000)  //(LCD_H_RES * LCD_V_RES + 8) / 10

static spi_device_handle_t spi_dev;

class SPD2010_LCD
{
public:
    SPD2010_LCD(int8_t sck_pin, int8_t qspi_0_pin, int8_t qspi_1_pin, int8_t qspi_2_pin,
                int8_t qspi_3_pin, int8_t cs_pin, int8_t lcd_rst_pin = -1);

    void begin();
    void lcd_address_set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void lcd_PushColors(uint16_t x,uint16_t y,uint16_t width,uint16_t high,uint16_t *data);
    void lcd_fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color);

private:
    int8_t _lcd_rst, _sck, _qspi_0, _qspi_1, _qspi_2, _qspi_3, _cs;

    void lcd_write_cmd(uint32_t cmd, uint8_t *data, uint32_t len);
};

typedef struct
{
    uint8_t cmd;
    uint8_t data[4];
    uint8_t len;
} lcd_cmd_data_t;

DRAM_ATTR static const lcd_cmd_data_t lcd_cmd_data[] = {
    {0xFF, {0x20, 0x10, 0x10}, 3},
    {0x0C, {0x11}, 1},
    {0x10, {0x02}, 1},
    {0x11, {0x11}, 1},
    {0x15, {0x42}, 1},
    {0x16, {0x11}, 1},
    {0x1A, {0x02}, 1},
    {0x1B, {0x11}, 1},
    {0x61, {0x80}, 1},
    {0x62, {0x80}, 1},
    {0x54, {0x44}, 1},
    {0x58, {0x88}, 1},
    {0x5C, {0xcc}, 1},
    {0xFF, {0x20, 0x10, 0x10}, 3},
    {0x20, {0x80}, 1},
    {0x21, {0x81}, 1},
    {0x22, {0x31}, 1},
    {0x23, {0x20}, 1},
    {0x24, {0x11}, 1},
    {0x25, {0x11}, 1},
    {0x26, {0x12}, 1},
    {0x27, {0x12}, 1},
    {0x30, {0x80}, 1},
    {0x31, {0x81}, 1},
    {0x32, {0x31}, 1},
    {0x33, {0x20}, 1},
    {0x34, {0x11}, 1},
    {0x35, {0x11}, 1},
    {0x36, {0x12}, 1},
    {0x37, {0x12}, 1},
    {0xFF, {0x20, 0x10, 0x10}, 3},
    {0x41, {0x11}, 1},
    {0x42, {0x22}, 1},
    {0x43, {0x33}, 1},
    {0x49, {0x11}, 1},
    {0x4A, {0x22}, 1},
    {0x4B, {0x33}, 1},
    {0xFF, {0x20, 0x10, 0x15}, 3},
    {0x00, {0x00}, 1},
    {0x01, {0x00}, 1},
    {0x02, {0x00}, 1},
    {0x03, {0x00}, 1},
    {0x04, {0x10}, 1},
    {0x05, {0x0C}, 1},
    {0x06, {0x23}, 1},
    {0x07, {0x22}, 1},
    {0x08, {0x21}, 1},
    {0x09, {0x20}, 1},
    {0x0A, {0x33}, 1},
    {0x0B, {0x32}, 1},
    {0x0C, {0x34}, 1},
    {0x0D, {0x35}, 1},
    {0x0E, {0x01}, 1},
    {0x0F, {0x01}, 1},
    {0x20, {0x00}, 1},
    {0x21, {0x00}, 1},
    {0x22, {0x00}, 1},
    {0x23, {0x00}, 1},
    {0x24, {0x0C}, 1},
    {0x25, {0x10}, 1},
    {0x26, {0x20}, 1},
    {0x27, {0x21}, 1},
    {0x28, {0x22}, 1},
    {0x29, {0x23}, 1},
    {0x2A, {0x33}, 1},
    {0x2B, {0x32}, 1},
    {0x2C, {0x34}, 1},
    {0x2D, {0x35}, 1},
    {0x2E, {0x01}, 1},
    {0x2F, {0x01}, 1},
    {0xFF, {0x20, 0x10, 0x16}, 3},
    {0x00, {0x00}, 1},
    {0x01, {0x00}, 1},
    {0x02, {0x00}, 1},
    {0x03, {0x00}, 1},
    {0x04, {0x08}, 1},
    {0x05, {0x04}, 1},
    {0x06, {0x19}, 1},
    {0x07, {0x18}, 1},
    {0x08, {0x17}, 1},
    {0x09, {0x16}, 1},
    {0x0A, {0x33}, 1},
    {0x0B, {0x32}, 1},
    {0x0C, {0x34}, 1},
    {0x0D, {0x35}, 1},
    {0x0E, {0x01}, 1},
    {0x0F, {0x01}, 1},
    {0x20, {0x00}, 1},
    {0x21, {0x00}, 1},
    {0x22, {0x00}, 1},
    {0x23, {0x00}, 1},
    {0x24, {0x04}, 1},
    {0x25, {0x08}, 1},
    {0x26, {0x16}, 1},
    {0x27, {0x17}, 1},
    {0x28, {0x18}, 1},
    {0x29, {0x19}, 1},
    {0x2A, {0x33}, 1},
    {0x2B, {0x32}, 1},
    {0x2C, {0x34}, 1},
    {0x2D, {0x35}, 1},
    {0x2E, {0x01}, 1},
    {0x2F, {0x01}, 1},
    {0xFF, {0x20, 0x10, 0x12}, 3},
    {0x00, {0x99}, 1},
    {0x2A, {0x28}, 1},
    {0x2B, {0x0f}, 1},
    {0x2C, {0x16}, 1},
    {0x2D, {0x28}, 1},
    {0x2E, {0x0f}, 1},
    {0xFF, {0x20, 0x10, 0xA0}, 3},
    {0x08, {0xdc}, 1},
    {0xFF, {0x20, 0x10, 0x45}, 3},
    {0x01, {0x9C}, 1},
    {0x03, {0x9C}, 1},
    {0xFF, {0x20, 0x10, 0x42}, 3},
    {0x05, {0x2c}, 1},
    {0xFF, {0x20, 0x10, 0x11}, 3},
    {0x50, {0x01}, 1},
    {0xFF, {0x20, 0x10, 0x00}, 3},
    {0x2A, {0x00, 0x00, 0x01, 0x9B}, 4},
    {0x2B, {0x00, 0x00, 0x01, 0x9B}, 4},
    {0xFF, {0x20, 0x10, 0x40}, 3},
    {0x86, {0x00}, 1},
    {0xFF, {0x20, 0x10, 0x00}, 3},
    {0xFF, {0x20, 0x10, 0x12}, 3},
    {0x0D, {0x66}, 1},
    {0xFF, {0x20, 0x10, 0x17}, 3},
    {0x39, {0x3c}, 1},
    {0xff, {0x20, 0x10, 0x31}, 3},
    {0x38, {0x03}, 1},
    {0x39, {0xf0}, 1},
    {0x36, {0x03}, 1},
    {0x37, {0xe8}, 1},
    {0x34, {0x03}, 1},
    {0x35, {0xCF}, 1},
    {0x32, {0x03}, 1},
    {0x33, {0xBA}, 1},
    {0x30, {0x03}, 1},
    {0x31, {0xA2}, 1},
    {0x2e, {0x03}, 1},
    {0x2f, {0x95}, 1},
    {0x2c, {0x03}, 1},
    {0x2d, {0x7e}, 1},
    {0x2a, {0x03}, 1},
    {0x2b, {0x62}, 1},
    {0x28, {0x03}, 1},
    {0x29, {0x44}, 1},
    {0x26, {0x02}, 1},
    {0x27, {0xfc}, 1},
    {0x24, {0x02}, 1},
    {0x25, {0xd0}, 1},
    {0x22, {0x02}, 1},
    {0x23, {0x98}, 1},
    {0x20, {0x02}, 1},
    {0x21, {0x6f}, 1},
    {0x1e, {0x02}, 1},
    {0x1f, {0x32}, 1},
    {0x1c, {0x01}, 1},
    {0x1d, {0xf6}, 1},
    {0x1a, {0x01}, 1},
    {0x1b, {0xb8}, 1},
    {0x18, {0x01}, 1},
    {0x19, {0x6E}, 1},
    {0x16, {0x01}, 1},
    {0x17, {0x41}, 1},
    {0x14, {0x00}, 1},
    {0x15, {0xfd}, 1},
    {0x12, {0x00}, 1},
    {0x13, {0xCf}, 1},
    {0x10, {0x00}, 1},
    {0x11, {0x98}, 1},
    {0x0e, {0x00}, 1},
    {0x0f, {0x89}, 1},
    {0x0c, {0x00}, 1},
    {0x0d, {0x79}, 1},
    {0x0a, {0x00}, 1},
    {0x0b, {0x67}, 1},
    {0x08, {0x00}, 1},
    {0x09, {0x55}, 1},
    {0x06, {0x00}, 1},
    {0x07, {0x3F}, 1},
    {0x04, {0x00}, 1},
    {0x05, {0x28}, 1},
    {0x02, {0x00}, 1},
    {0x03, {0x0E}, 1},
    {0xff, {0x20, 0x10, 0x00}, 3},
    {0xff, {0x20, 0x10, 0x32}, 3},
    {0x38, {0x03}, 1},
    {0x39, {0xf0}, 1},
    {0x36, {0x03}, 1},
    {0x37, {0xe8}, 1},
    {0x34, {0x03}, 1},
    {0x35, {0xCF}, 1},
    {0x32, {0x03}, 1},
    {0x33, {0xBA}, 1},
    {0x30, {0x03}, 1},
    {0x31, {0xA2}, 1},
    {0x2e, {0x03}, 1},
    {0x2f, {0x95}, 1},
    {0x2c, {0x03}, 1},
    {0x2d, {0x7e}, 1},
    {0x2a, {0x03}, 1},
    {0x2b, {0x62}, 1},
    {0x28, {0x03}, 1},
    {0x29, {0x44}, 1},
    {0x26, {0x02}, 1},
    {0x27, {0xfc}, 1},
    {0x24, {0x02}, 1},
    {0x25, {0xd0}, 1},
    {0x22, {0x02}, 1},
    {0x23, {0x98}, 1},
    {0x20, {0x02}, 1},
    {0x21, {0x6f}, 1},
    {0x1e, {0x02}, 1},
    {0x1f, {0x32}, 1},
    {0x1c, {0x01}, 1},
    {0x1d, {0xf6}, 1},
    {0x1a, {0x01}, 1},
    {0x1b, {0xb8}, 1},
    {0x18, {0x01}, 1},
    {0x19, {0x6E}, 1},
    {0x16, {0x01}, 1},
    {0x17, {0x41}, 1},
    {0x14, {0x00}, 1},
    {0x15, {0xfd}, 1},
    {0x12, {0x00}, 1},
    {0x13, {0xCf}, 1},
    {0x10, {0x00}, 1},
    {0x11, {0x98}, 1},
    {0x0e, {0x00}, 1},
    {0x0f, {0x89}, 1},
    {0x0c, {0x00}, 1},
    {0x0d, {0x79}, 1},
    {0x0a, {0x00}, 1},
    {0x0b, {0x67}, 1},
    {0x08, {0x00}, 1},
    {0x09, {0x55}, 1},
    {0x06, {0x00}, 1},
    {0x07, {0x3F}, 1},
    {0x04, {0x00}, 1},
    {0x05, {0x28}, 1},
    {0x02, {0x00}, 1},
    {0x03, {0x0E}, 1},
    {0xff, {0x20, 0x10, 0x00}, 3},
    {0xFF, {0x20, 0x10, 0x11}, 3},
    {0x60, {0x01}, 1},
    {0x65, {0x03}, 1},
    {0x66, {0x38}, 1},
    {0x67, {0x04}, 1},
    {0x68, {0x34}, 1},
    {0x69, {0x03}, 1},
    {0x61, {0x03}, 1},
    {0x62, {0x38}, 1},
    {0x63, {0x04}, 1},
    {0x64, {0x34}, 1},
    {0x0A, {0x11}, 1},
    {0x0B, {0x20}, 1},
    {0x0c, {0x20}, 1},
    {0x55, {0x06}, 1},
    {0xFF, {0x20, 0x10, 0x42}, 3},
    {0x05, {0x3D}, 1},
    {0x06, {0x03}, 1},
    {0xFF, {0x20, 0x10, 0x00}, 3},
    {0xFF, {0x20, 0x10, 0x12}, 3},
    {0x1F, {0xDC}, 1},
    {0xff, {0x20, 0x10, 0x17}, 3},
    {0x11, {0xAA}, 1},
    {0x16, {0x12}, 1},
    {0x0B, {0xC3}, 1},
    {0x10, {0x0E}, 1},
    {0x14, {0xAA}, 1},
    {0x18, {0xA0}, 1},
    {0x1A, {0x80}, 1},
    {0x1F, {0x80}, 1},
    {0xff, {0x20, 0x10, 0x11}, 3},
    {0x30, {0xEE}, 1},
    {0xff, {0x20, 0x10, 0x12}, 3},
    {0x15, {0x0F}, 1},
    {0xff, {0x20, 0x10, 0x2D}, 3},
    {0x01, {0x3E}, 1},
    {0xff, {0x20, 0x10, 0x40}, 3},
    {0x83, {0xC4}, 1},
    {0xFF, {0x20, 0x10, 0x12}, 3},
    {0x00, {0xCC}, 1},
    {0x36, {0xA0}, 1},
    {0x2A, {0x2D}, 1},
    {0x2B, {0x1e}, 1},
    {0x2C, {0x26}, 1},
    {0x2D, {0x2D}, 1},
    {0x2E, {0x1e}, 1},
    {0x1F, {0xE6}, 1},
    {0xFF, {0x20, 0x10, 0xA0}, 3},
    {0x08, {0xE6}, 1},
    {0xFF, {0x20, 0x10, 0x12}, 3},
    {0x10, {0x0F}, 1},
    {0xFF, {0x20, 0x10, 0x18}, 3},
    {0x01, {0x01}, 1},
    {0x00, {0x1E}, 1},
    {0xFF, {0x20, 0x10, 0x43}, 3},
    {0x03, {0x04}, 1},
    {0xFF, {0x20, 0x10, 0x18}, 3},
    {0x3A, {0x01}, 1},

    /* TP Enable */
    {0xFF, {0x20, 0x10, 0x50}, 3},
    {0x05, {0x08}, 1}, // 0x00 关闭AUTO RUN
    {0xFF, {0x20, 0x10, 0x00}, 3},
    {0xFF, {0x20, 0x10, 0x50}, 3},
    {0x00, {0xA6}, 1},
    {0x01, {0xA6}, 1},
    {0xFF, {0x20, 0x10, 0x00}, 3},
    {0xFF, {0x20, 0x10, 0x50}, 3},
    {0x08, {0x55}, 1},
    {0xFF, {0x20, 0x10, 0x00}, 3},

    {0xFF, {0x20, 0x10, 0x10}, 3},
    {0x0B, {0x43}, 1},
    {0x0C, {0x12}, 1},
    {0x10, {0x01}, 1},
    {0x11, {0x12}, 1},
    {0x15, {0x00}, 1},
    {0x16, {0x00}, 1},
    {0x1A, {0x00}, 1},
    {0x1B, {0x00}, 1},
    {0x61, {0x00}, 1},
    {0x62, {0x00}, 1},
    {0x51, {0x11}, 1},
    {0x55, {0x55}, 1},
    {0x58, {0x00}, 1},
    {0x5C, {0x00}, 1},
    {0xFF, {0x20, 0x10, 0x10}, 3},
    {0x20, {0x81}, 1},
    {0x21, {0x82}, 1},
    {0x22, {0x72}, 1},
    {0x30, {0x00}, 1},
    {0x31, {0x00}, 1},
    {0x32, {0x00}, 1},
    {0xFF, {0x20, 0x10, 0x10}, 3},
    {0x44, {0x44}, 1},
    {0x45, {0x55}, 1},
    {0x46, {0x66}, 1},
    {0x47, {0x77}, 1},
    {0x49, {0x00}, 1},
    {0x4A, {0x00}, 1},
    {0x4B, {0x00}, 1},
    {0xFF, {0x20, 0x10, 0x17}, 3},
    {0x37, {0x00}, 1},
    {0xFF, {0x20, 0x10, 0x15}, 3},
    {0x04, {0x08}, 1},
    {0x05, {0x04}, 1},
    {0x06, {0x1C}, 1},
    {0x07, {0x1A}, 1},
    {0x08, {0x18}, 1},
    {0x09, {0x16}, 1},
    {0x24, {0x05}, 1},
    {0x25, {0x09}, 1},
    {0x26, {0x17}, 1},
    {0x27, {0x19}, 1},
    {0x28, {0x1B}, 1},
    {0x29, {0x1D}, 1},
    {0xFF, {0x20, 0x10, 0x16}, 3},
    {0x04, {0x09}, 1},
    {0x05, {0x05}, 1},
    {0x06, {0x1D}, 1},
    {0x07, {0x1B}, 1},
    {0x08, {0x19}, 1},
    {0x09, {0x17}, 1},
    {0x24, {0x04}, 1},
    {0x25, {0x08}, 1},
    {0x26, {0x16}, 1},
    {0x27, {0x18}, 1},
    {0x28, {0x1A}, 1},
    {0x29, {0x1C}, 1},
    {0xFF, {0x20, 0x10, 0x18}, 3},
    {0x1F, {0x02}, 1},
    {0xFF, {0x20, 0x10, 0x11}, 3},
    {0x15, {0x99}, 1},
    {0x16, {0x99}, 1},
    {0x1C, {0x88}, 1},
    {0x1D, {0x88}, 1},
    {0x1E, {0x88}, 1},
    {0x13, {0xf0}, 1},
    {0x14, {0x34}, 1},
    {0xFF, {0x20, 0x10, 0x12}, 3},
    {0x12, {0x89}, 1},
    {0x06, {0x06}, 1},
    {0x18, {0x00}, 1},
    {0xFF, {0x20, 0x10, 0x11}, 3},
    {0x0A, {0x00}, 1},
    {0x0B, {0xF0}, 1},
    {0x0c, {0xF0}, 1},
    {0x6A, {0x10}, 1},
    {0xFF, {0x20, 0x10, 0x00}, 3},
    {0xFF, {0x20, 0x10, 0x11}, 3},
    {0x08, {0x70}, 1},
    {0x09, {0x00}, 1},
    {0xFF, {0x20, 0x10, 0x00}, 3},
    {0x35, {0x00}, 1},
    {0x3A, {0x05}, 1}, // #05=RGB16BIT,07=RGB24BIT
    {0xFF, {0x20, 0x10, 0x12}, 3},
    {0x21, {0x70}, 1}, // #VCOM
    {0xFF, {0x20, 0x10, 0x2D}, 3},
    {0x02, {0x00}, 1},
    {0xFF, {0x20, 0x10, 0x00}, 3},
    {0x11, {0x00}, 0},
    {0x00, {0x00}, 0xff}, // 100ms
    {0x29, {0x00}, 0},
    {0x00, {0x00}, 0xff}, // 100ms
};

#endif
