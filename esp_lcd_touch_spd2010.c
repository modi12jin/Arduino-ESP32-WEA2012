/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch.h"

static const char *TAG = "lcd_panel_touch_spd2010";

typedef struct stTPSta_H
{
    uint8_t none0;
    uint8_t none1;
    uint8_t none2;
    uint8_t CPU_RUN;
    uint8_t TINT_Low;
    uint8_t TIC_IN_CPU;
    uint8_t TIC_IN_BIOS;
    uint8_t TIC_BUSY;
} TPStaH_t;

typedef struct stTPSta_L
{
    uint8_t PT_Exist;
    uint8_t Gesture;
    uint8_t Key;
    uint8_t Aux;
    uint8_t Keep;
    uint8_t RawOrPT;
    uint8_t none6;
    uint8_t none7;
} TPStaL_t;

typedef struct stTP_SL
{
    struct stTPSta_L Status_L;
    struct stTPSta_H Status_H;
    uint16_t ReadLen;
} TP_SL_t;

typedef struct stTP_Report
{
    uint8_t id;
    uint16_t x;
    uint16_t y;
    uint8_t weight;
} TP_Report_t;

typedef struct stTP_Tocuh
{
    struct stTP_Report rpt[10];
    uint8_t touch_num;
    uint8_t pack_code;
    uint8_t down;
    uint8_t up;
    uint8_t gesture;
    uint16_t down_x;
    uint16_t down_y;
    uint16_t up_x;
    uint16_t up_y;
} TP_Touch_t;

typedef struct stTP_HDP_STA
{
    uint8_t Sta;
    uint16_t Next_Packet_Len;
} TP_HDP_STA_t;

static esp_err_t read_data(esp_lcd_touch_handle_t tp);
static bool get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num);
static esp_err_t del(esp_lcd_touch_handle_t tp);
static esp_err_t reset(esp_lcd_touch_handle_t tp);

static esp_err_t Write_TP_Point_Mode_Cmd(esp_lcd_touch_handle_t tp);
static esp_err_t Write_TP_Start_Cmd(esp_lcd_touch_handle_t tp);
static esp_err_t Write_TP_CPU_Start_Cmd(esp_lcd_touch_handle_t tp);
static esp_err_t Write_TP_Clear_INT_Cmd(esp_lcd_touch_handle_t tp);
static esp_err_t Read_TP_Status_Length(esp_lcd_touch_handle_t tp, struct stTP_SL *TP_Sta_Len);
static esp_err_t Read_TP_HDP(esp_lcd_touch_handle_t tp, struct stTP_SL *TP_Sta_Len, struct stTP_Tocuh *Touch);
static esp_err_t Read_TP_HDP_STA(esp_lcd_touch_handle_t tp, struct stTP_HDP_STA *TP_Hdp_Sta);
static esp_err_t Read_FW_Version(esp_lcd_touch_handle_t tp);
static esp_err_t TP_Read_Data(esp_lcd_touch_handle_t tp, struct stTP_Tocuh *Touch);

esp_err_t lcd_panel_touch_new_i2c_spd2010(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *tp)
{
    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_ARG, TAG, "Invalid io");
    ESP_RETURN_ON_FALSE(config, ESP_ERR_INVALID_ARG, TAG, "Invalid config");
    ESP_RETURN_ON_FALSE(tp, ESP_ERR_INVALID_ARG, TAG, "Invalid touch handle");

    /* Prepare main structure */
    esp_err_t ret = ESP_OK;
    esp_lcd_touch_handle_t spd2010 = calloc(1, sizeof(esp_lcd_touch_t));
    ESP_GOTO_ON_FALSE(spd2010, ESP_ERR_NO_MEM, err, TAG, "Touch handle malloc failed");

    /* Communication interface */
    spd2010->io = io;
    /* Only supported callbacks are set */
    spd2010->read_data = read_data;
    spd2010->get_xy = get_xy;
    spd2010->del = del;
    /* Mutex */
    spd2010->data.lock.owner = portMUX_FREE_VAL;
    /* Save config */
    memcpy(&spd2010->config, config, sizeof(esp_lcd_touch_config_t));

    /* Prepare pin for touch interrupt */
    if (spd2010->config.int_gpio_num != GPIO_NUM_NC) {
        const gpio_config_t int_gpio_config = {
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = BIT64(spd2010->config.int_gpio_num)
        };
        ESP_GOTO_ON_ERROR(gpio_config(&int_gpio_config), err, TAG, "GPIO intr config failed");
    }
    /* Prepare pin for touch controller reset */
    if (spd2010->config.rst_gpio_num != GPIO_NUM_NC) {
        const gpio_config_t rst_gpio_config = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = BIT64(spd2010->config.rst_gpio_num)
        };
        ESP_GOTO_ON_ERROR(gpio_config(&rst_gpio_config), err, TAG, "GPIO reset config failed");
    }
    /* Reset controller */
    ESP_GOTO_ON_ERROR(reset(spd2010), err, TAG, "Reset failed");
    ESP_GOTO_ON_ERROR(Read_FW_Version(spd2010), err, TAG, "Read version failed");

    *tp = spd2010;

    return ESP_OK;
err:
    if (spd2010) {
        del(spd2010);
    }
    ESP_LOGE(TAG, "Initialization failed!");
    return ret;
}

static esp_err_t read_data(esp_lcd_touch_handle_t tp)
{
    uint8_t touch_cnt = 0;

    struct stTP_Tocuh Touch = {0};
    ESP_RETURN_ON_ERROR(TP_Read_Data(tp, &Touch), TAG, "read data failed");

    portENTER_CRITICAL(&tp->data.lock);
    /* Expect Number of touched points */
    touch_cnt = (Touch.touch_num > CONFIG_ESP_LCD_TOUCH_MAX_POINTS ? CONFIG_ESP_LCD_TOUCH_MAX_POINTS : Touch.touch_num);
    tp->data.points = touch_cnt;

    /* Fill all coordinates */
    for (int i = 0; i < touch_cnt; i++) {
        tp->data.coords[i].x = Touch.rpt[i].x;
        tp->data.coords[i].y = Touch.rpt[i].y;
        tp->data.coords[i].strength = Touch.rpt[i].weight;
    }
    portEXIT_CRITICAL(&tp->data.lock);

    return ESP_OK;
}

static bool get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    portENTER_CRITICAL(&tp->data.lock);
    /* Count of points */
    *point_num = (tp->data.points > max_point_num ? max_point_num : tp->data.points);
    for (size_t i = 0; i < *point_num; i++) {
        x[i] = tp->data.coords[i].x;
        y[i] = tp->data.coords[i].y;

        if (strength) {
            strength[i] = tp->data.coords[i].strength;
        }
    }
    /* Invalidate */
    tp->data.points = 0;
    portEXIT_CRITICAL(&tp->data.lock);

    return (*point_num > 0);
}

static esp_err_t del(esp_lcd_touch_handle_t tp)
{
    /* Reset GPIO pin settings */
    if (tp->config.int_gpio_num != GPIO_NUM_NC) {
        gpio_reset_pin(tp->config.int_gpio_num);
    }
    if (tp->config.rst_gpio_num != GPIO_NUM_NC) {
        gpio_reset_pin(tp->config.rst_gpio_num);
    }
    /* Release memory */
    free(tp);

    return ESP_OK;
}

static esp_err_t reset(esp_lcd_touch_handle_t tp)
{
    if (tp->config.rst_gpio_num != GPIO_NUM_NC) {
        ESP_RETURN_ON_ERROR(gpio_set_level(tp->config.rst_gpio_num, tp->config.levels.reset), TAG, "GPIO set level failed");
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP_RETURN_ON_ERROR(gpio_set_level(tp->config.rst_gpio_num, !tp->config.levels.reset), TAG, "GPIO set level failed");
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    return ESP_OK;
}

#define I2C_Write(data_p, len)      ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(tp->io, 0, data_p, len), TAG, "Tx failed");
#define I2C_Read(data_p, len)       ESP_RETURN_ON_ERROR(esp_lcd_panel_io_rx_param(tp->io, 0, data_p, len), TAG, "Rx failed");

/*******************************/
/* Write_TP_Point_Mode_Cmd     */
/*******************************/
static esp_err_t Write_TP_Point_Mode_Cmd(esp_lcd_touch_handle_t tp)
{
    uint8_t sample_data[4];

    sample_data[0] = 0x50;
    sample_data[1] = 0x00;
    sample_data[2] = 0x00;
    sample_data[3] = 0x00;

    I2C_Write(&sample_data[0], sizeof(sample_data));
    esp_rom_delay_us(200);
    return ESP_OK;
}

/*******************************/
/* Write_TP_Start_Cmd          */
/*******************************/
static esp_err_t Write_TP_Start_Cmd(esp_lcd_touch_handle_t tp)
{
    uint8_t sample_data[4];

    sample_data[0] = 0x46;
    sample_data[1] = 0x00;
    sample_data[2] = 0x00;
    sample_data[3] = 0x00;

    I2C_Write(&sample_data[0], sizeof(sample_data));
    esp_rom_delay_us(200);
    return ESP_OK;
}

/*******************************/
/* Write_TP_CPU_Start_Cmd      */
/*******************************/
static esp_err_t Write_TP_CPU_Start_Cmd(esp_lcd_touch_handle_t tp)
{
    uint8_t sample_data[4];

    sample_data[0] = 0x04;
    sample_data[1] = 0x00;
    sample_data[2] = 0x01;
    sample_data[3] = 0x00;

    I2C_Write(&sample_data[0], sizeof(sample_data));
    esp_rom_delay_us(200);
    return ESP_OK;
}

/*******************************/
/* Write_TP_Clear_INT_Cmd      */
/*******************************/
static esp_err_t Write_TP_Clear_INT_Cmd(esp_lcd_touch_handle_t tp)
{
    uint8_t sample_data[4];

    sample_data[0] = 0x02;
    sample_data[1] = 0x00;
    sample_data[2] = 0x01;
    sample_data[3] = 0x00;

    I2C_Write(&sample_data[0], sizeof(sample_data));
    esp_rom_delay_us(200);
    return ESP_OK;
}

/*******************************/
/* Read_TP_Status_Length       */
/*******************************/
static esp_err_t Read_TP_Status_Length(esp_lcd_touch_handle_t tp, struct stTP_SL *TP_Sta_Len)
{
    uint8_t sample_data[4];

    sample_data[0] = 0x20;
    sample_data[1] = 0x00;

    I2C_Write(&sample_data[0], 2);
    esp_rom_delay_us(200);
    I2C_Read(&sample_data[0], sizeof(sample_data));
    esp_rom_delay_us(200);
    TP_Sta_Len->Status_L.PT_Exist = (sample_data[0] & 0x01);
    TP_Sta_Len->Status_L.Gesture = (sample_data[0] & 0x02);
    TP_Sta_Len->Status_H.TIC_BUSY = ((sample_data[1] & 0x80) >> 7);
    TP_Sta_Len->Status_H.TIC_IN_BIOS = ((sample_data[1] & 0x40) >> 6);
    TP_Sta_Len->Status_H.TIC_IN_CPU = ((sample_data[1] & 0x20) >> 5);
    TP_Sta_Len->Status_H.TINT_Low = ((sample_data[1] & 0x10) >> 4);
    TP_Sta_Len->Status_H.CPU_RUN = ((sample_data[1] & 0x08) >> 3);
    TP_Sta_Len->Status_L.Aux = ((sample_data[0] & 0x08)); //aux, cytang

    TP_Sta_Len->ReadLen = (sample_data[3] << 8 | sample_data[2]);
    return ESP_OK;
}

/*******************************/
/* Read_TP_HDP                 */
/*******************************/
static esp_err_t Read_TP_HDP(esp_lcd_touch_handle_t tp, struct stTP_SL *TP_Sta_Len, struct stTP_Tocuh *Touch)
{
    uint8_t sample_data[4+(10*6)]; // 4 Bytes Header + 10 Finger * 6 Bytes
    uint8_t i, offset;
    uint8_t check_id;

    sample_data[0] = 0x00;
    sample_data[1] = 0x03;

    I2C_Write(&sample_data[0], 2);
    esp_rom_delay_us(200);
    I2C_Read(&sample_data[0], TP_Sta_Len->ReadLen);
    esp_rom_delay_us(200);

    check_id = sample_data[4];

    if ((check_id <= 0x0A) && TP_Sta_Len->Status_L.PT_Exist) {
        Touch->touch_num = ((TP_Sta_Len->ReadLen - 4)/6);
        Touch->gesture = 0x00;

        for (i = 0; i < Touch->touch_num; i++) {
            offset = i*6;
            Touch->rpt[i].id = sample_data[4 + offset];
            Touch->rpt[i].x = (((sample_data[7 + offset] & 0xF0) << 4)| sample_data[5 + offset]);
            Touch->rpt[i].y = (((sample_data[7 + offset] & 0x0F) << 8)| sample_data[6 + offset]);
            Touch->rpt[i].weight = sample_data[8 + offset];
        }

        /* For slide gesture recognize */
        if ((Touch->rpt[0].weight != 0) && (Touch->down != 1)) {
            Touch->down = 1;
            Touch->up = 0 ;
            Touch->down_x = Touch->rpt[0].x;
            Touch->down_y = Touch->rpt[0].y;
        } else if ((Touch->rpt[0].weight == 0) && (Touch->down == 1)) {
            Touch->up = 1;
            Touch->down = 0;
            Touch->up_x = Touch->rpt[0].x;
            Touch->up_y = Touch->rpt[0].y;
        }

        /* Dump Log */
        for (uint8_t finger_num = 0; finger_num < Touch->touch_num; finger_num++) {
            ESP_LOGD(TAG, "ID[%d], X[%d], Y[%d], Weight[%d]\n",
                         Touch->rpt[finger_num].id,
                         Touch->rpt[finger_num].x,
                         Touch->rpt[finger_num].y,
                         Touch->rpt[finger_num].weight);
        }
    } else if ((check_id == 0xF6) && TP_Sta_Len->Status_L.Gesture) {
        Touch->touch_num = 0x00;
        Touch->up = 0;
        Touch->down = 0;
        Touch->gesture = sample_data[6] & 0x07;
        ESP_LOGD(TAG, "Gesture : 0x%02x\n", Touch->gesture);
    } else {
        Touch->touch_num = 0x00;
        Touch->gesture = 0x00;
    }
    return ESP_OK;
}

/*******************************/
/* Read_TP_HDP_STA             */
/*******************************/
static esp_err_t Read_TP_HDP_STA(esp_lcd_touch_handle_t tp, struct stTP_HDP_STA *TP_Hdp_Sta)
{
    uint8_t sample_data[8];

    sample_data[0] = 0xFC;
    sample_data[1] = 0x02;

    I2C_Write(&sample_data[0], 2);
    esp_rom_delay_us(200);
    I2C_Read(&sample_data[0], sizeof(sample_data));
    esp_rom_delay_us(200);

    TP_Hdp_Sta->Sta = sample_data[5];
    TP_Hdp_Sta->Next_Packet_Len = (sample_data[2] | sample_data[3] << 8);
    return ESP_OK;
}

/*******************************/
/* Read_HDP_REMAIN_DATA        */
/*******************************/
static esp_err_t Read_HDP_REMAIN_DATA(esp_lcd_touch_handle_t tp, struct stTP_HDP_STA *TP_Hdp_Sta)
{
    uint8_t sample_data[32];

    sample_data[0] = 0x00;
    sample_data[1] = 0x03;

    I2C_Write(&sample_data[0], 2);
    esp_rom_delay_us(200);
    I2C_Read(&sample_data[0], TP_Hdp_Sta->Next_Packet_Len);
    esp_rom_delay_us(200);
    return ESP_OK;
}

/*******************************/
/* Read_FW_Version             */
/*******************************/
static esp_err_t Read_FW_Version(esp_lcd_touch_handle_t tp)
{
    uint8_t sample_data[18];
	uint16_t DVer;
	uint32_t Dummy, PID, ICName_H, ICName_L;

    sample_data[0] = 0x26;
    sample_data[1] = 0x00;

    I2C_Write(&sample_data[0], 2);
    esp_rom_delay_us(200);
    I2C_Read(&sample_data[0], 18);
    esp_rom_delay_us(200);

	Dummy = ((sample_data[0] << 24) | (sample_data[1] << 16) | (sample_data[3] << 8) | (sample_data[0]));
	DVer = ((sample_data[5] << 8) | (sample_data[4]));
	PID = ((sample_data[9] << 24) | (sample_data[8] << 16) | (sample_data[7] << 8) | (sample_data[6]));
	ICName_L = ((sample_data[13] << 24) | (sample_data[12] << 16) | (sample_data[11] << 8) | (sample_data[10]));    // "2010"
	ICName_H = ((sample_data[17] << 24) | (sample_data[16] << 16) | (sample_data[15] << 8) | (sample_data[14]));    // "SPD"

    ESP_LOGI(TAG, "Dummy[%"PRIu32"], DVer[%"PRIu16"], PID[%"PRIu32"], Name[%"PRIu32"-%"PRIu32"]", Dummy, DVer, PID, ICName_H, ICName_L);

    return ESP_OK;
}

/*******************************/
/* TP_Read_Data                */
/*******************************/
static esp_err_t TP_Read_Data(esp_lcd_touch_handle_t tp, struct stTP_Tocuh *Touch)
{
    struct stTP_SL TP_Sta_Len = {0};
    struct stTP_HDP_STA TP_Hdp_Sta = {0};

    ESP_RETURN_ON_ERROR(Read_TP_Status_Length(tp, &TP_Sta_Len), TAG, "Read status length failed");

    if (TP_Sta_Len.Status_H.TIC_IN_BIOS) {
        /* Write Clear TINT Command */
        ESP_RETURN_ON_ERROR(Write_TP_Clear_INT_Cmd(tp), TAG, "Write clear int cmd failed");

        /* Write CPU Start Command */
        ESP_RETURN_ON_ERROR(Write_TP_CPU_Start_Cmd(tp), TAG, "Write cpu start cmd failed");

    } else if (TP_Sta_Len.Status_H.TIC_IN_CPU) {
        /* Write Touch Change Command */
        ESP_RETURN_ON_ERROR(Write_TP_Point_Mode_Cmd(tp), TAG, "Write point mode cmd failed");

        /* Write Touch Start Command */
        ESP_RETURN_ON_ERROR(Write_TP_Start_Cmd(tp), TAG, "Write start cmd failed");

        /* Write Clear TINT Command */
        ESP_RETURN_ON_ERROR(Write_TP_Clear_INT_Cmd(tp), TAG, "Write clear int cmd failed");

    } else if (TP_Sta_Len.Status_H.CPU_RUN && TP_Sta_Len.ReadLen == 0) {
        ESP_RETURN_ON_ERROR(Write_TP_Clear_INT_Cmd(tp), TAG, "Write clear int cmd failed");
    } else if (TP_Sta_Len.Status_L.PT_Exist || TP_Sta_Len.Status_L.Gesture) {
        /* Read HDP */
        ESP_RETURN_ON_ERROR(Read_TP_HDP(tp, &TP_Sta_Len, Touch), TAG, "Read hdp failed");

HDP_DONE_CHECK:
        /* Read HDP Status */
        ESP_RETURN_ON_ERROR(Read_TP_HDP_STA(tp, &TP_Hdp_Sta), TAG, "Read hdp sta failed");

        if (TP_Hdp_Sta.Sta == 0x82)
        {
            /* Clear INT */
            ESP_RETURN_ON_ERROR(Write_TP_Clear_INT_Cmd(tp), TAG, "Write clear int cmd failed");
        }
        else if (TP_Hdp_Sta.Sta == 0x00)
        {
            /* Read HDP Remain Data */
            ESP_RETURN_ON_ERROR(Read_HDP_REMAIN_DATA(tp, &TP_Hdp_Sta), TAG, "Read hdp remain data failed");
            goto HDP_DONE_CHECK;
        }
    } else if (TP_Sta_Len.Status_H.CPU_RUN && TP_Sta_Len.Status_L.Aux) {
        ESP_RETURN_ON_ERROR(Write_TP_Clear_INT_Cmd(tp), TAG, "Write clear int cmd failed");
    }

    return ESP_OK;
}
