#include "WEA2012_LCD.h"

WEA2012_LCD::WEA2012_LCD(int8_t sck_pin, int8_t qspi_0_pin, int8_t qspi_1_pin, int8_t qspi_2_pin,
                         int8_t qspi_3_pin, int8_t cs_pin, int8_t lcd_rst_pin)
{
    _sck = sck_pin;
    _qspi_0 = qspi_0_pin;
    _qspi_1 = qspi_1_pin;
    _qspi_2 = qspi_2_pin;
    _qspi_3 = qspi_3_pin;
    _cs = cs_pin;
    _lcd_rst = lcd_rst_pin;
}

void WEA2012_LCD::begin()
{
    // 复位引脚配置
    if (_lcd_rst != -1)
    {
        pinMode(_lcd_rst, OUTPUT);
        digitalWrite(_lcd_rst, LOW);
        delay(100);
        digitalWrite(_lcd_rst, HIGH);
        delay(100);
    }
    pinMode(_cs, OUTPUT);

    spi_bus_config_t spi_config = {
        .data0_io_num = _qspi_0,
        .data1_io_num = _qspi_1,
        .sclk_io_num = _sck,
        .data2_io_num = _qspi_2,
        .data3_io_num = _qspi_3,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * sizeof(uint16_t),
        .flags = SPICOMMON_BUSFLAG_QUAD,
    };

    spi_bus_initialize(LCD_SPI_HOST, &spi_config, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t dev_config = {
        .command_bits = 8,
        .address_bits = 24,
        .mode = 0,
        .clock_speed_hz = SPI_MASTER_FREQ_80M,
        .spics_io_num = -1,
        // .spics_io_num = _cs,
        .flags = SPI_DEVICE_HALFDUPLEX,
        .queue_size = 17,
    };
    spi_bus_add_device((spi_host_device_t)LCD_SPI_HOST, &dev_config, &spi_dev);

    // 初始化屏幕
    const lcd_cmd_data_t *lcd_init = lcd_cmd_data;
    for (int i = 0; i < (sizeof(lcd_cmd_data) / sizeof(lcd_cmd_data_t)); i++)
    {
        if (lcd_init[i].len == 0xff)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        else
        {
            lcd_write_cmd(lcd_init[i].cmd,
                          (uint8_t *)lcd_init[i].data,
                          lcd_init[i].len);
        }
    }
}

void WEA2012_LCD::lcd_write_cmd(uint32_t cmd, uint8_t *data, uint32_t len)
{
    digitalWrite(_cs, 0);
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    t.flags = SPI_TRANS_MULTILINE_CMD | SPI_TRANS_MULTILINE_ADDR;
    t.cmd = 0x02;
    t.addr = cmd << 8;
    t.rx_buffer = NULL;
    t.rxlength = 0;
    if (len != 0)
    {
        t.tx_buffer = data;
        t.length = 8 * len;
    }
    else
    {
        t.tx_buffer = NULL;
        t.length = 0;
    }

    spi_device_polling_transmit(spi_dev, &t);
    digitalWrite(_cs, 1);
}

void WEA2012_LCD::lcd_address_set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    lcd_cmd_data_t t[3] = {
        {0x2a, {x1 >> 8, x1, x2 >> 8, x2}, 0x04},
        {0x2b, {y1 >> 8, y1, y2 >> 8, y2}, 0x04},
        {0x2c, {0x00}, 0x00},
    };

    for (uint32_t i = 0; i < 3; i++)
    {
        lcd_write_cmd(t[i].cmd, (uint8_t *)t[i].data, t[i].len);
    }
}

void WEA2012_LCD::lcd_PushColors(uint16_t x,
                                 uint16_t y,
                                 uint16_t width,
                                 uint16_t high,
                                 uint16_t *data)
{
    bool first_send = 1;
    size_t len = width * high;
    uint16_t *p = (uint16_t *)data;

    lcd_address_set(x, y, x + width - 1, y + high - 1);
    digitalWrite(_cs, 0);
    do
    {
        size_t chunk_size = len;
        spi_transaction_ext_t t = {0};
        memset(&t, 0, sizeof(t));
        if (first_send)
        {
            t.base.flags = SPI_TRANS_MODE_QIO;
            t.base.cmd = 0x32;
            t.base.addr = 0x002C00;
            first_send = 0;
        }
        else
        {
            t.base.flags = SPI_TRANS_MODE_QIO | SPI_TRANS_VARIABLE_CMD | SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY;
            t.command_bits = 0;
            t.address_bits = 0;
            t.dummy_bits = 0;
        }
        if (chunk_size > SEND_BUF_SIZE)
        {
            chunk_size = SEND_BUF_SIZE;
        }
        t.base.tx_buffer = p;
        t.base.length = chunk_size * 16;

        // spi_device_queue_trans(spi_dev, (spi_transaction_t *)&t, portMAX_DELAY);
        spi_device_polling_transmit(spi_dev, (spi_transaction_t *)&t);
        len -= chunk_size;
        p += chunk_size;
    } while (len > 0);
    digitalWrite(_cs, 1);
}

void WEA2012_LCD::lcd_fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color)
{
    uint16_t w = xend - xsta;
    uint16_t h = yend - ysta;
    uint16_t *color_p = (uint16_t *)heap_caps_malloc(w * h * 2, MALLOC_CAP_8BIT);
    memset(color_p, color, w * h * 2);
    lcd_PushColors(xsta, ysta, w, h, color_p);
    free(color_p);
}