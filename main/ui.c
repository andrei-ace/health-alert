#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "core2forAWS.h"
#include "ui.h"

#define MAX_TEXTAREA_LENGTH 1024

static lv_obj_t *active_screen;
static lv_obj_t *out_txtarea;
static lv_obj_t *wifi_label;

static lv_obj_t *infected_btn;
static lv_obj_t *infected_btn_label;

static char *TAG = "UI";

static void ui_textarea_prune(size_t new_text_length)
{
    const char *current_text = lv_textarea_get_text(out_txtarea);
    size_t current_text_len = strlen(current_text);
    if (current_text_len + new_text_length >= MAX_TEXTAREA_LENGTH)
    {
        for (int i = 0; i < new_text_length; i++)
        {
            lv_textarea_set_cursor_pos(out_txtarea, 0);
            lv_textarea_del_char_forward(out_txtarea);
        }
        lv_textarea_set_cursor_pos(out_txtarea, LV_TEXTAREA_CURSOR_LAST);
    }
}

void ui_textarea_add(char *baseTxt, char *param, size_t paramLen)
{
    if (baseTxt != NULL)
    {
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
        if (param != NULL && paramLen != 0)
        {
            size_t baseTxtLen = strlen(baseTxt);
            ui_textarea_prune(paramLen);
            size_t bufLen = baseTxtLen + paramLen;
            char buf[(int)bufLen];
            sprintf(buf, baseTxt, param);
            lv_textarea_add_text(out_txtarea, buf);
        }
        else
        {
            lv_textarea_add_text(out_txtarea, baseTxt);
        }
        xSemaphoreGive(xGuiSemaphore);
    }
    else
    {
        ESP_LOGE(TAG, "Textarea baseTxt is NULL!");
    }
}

void ui_wifi_label_update(bool state)
{
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    if (state == false)
    {
        lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    }
    else
    {
        char buffer[25];
        sprintf(buffer, "#0000ff %s #", LV_SYMBOL_WIFI);
        lv_label_set_text(wifi_label, buffer);
    }
    xSemaphoreGive(xGuiSemaphore);
}

void ui_init()
{
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    active_screen = lv_scr_act();

    infected_btn = lv_btn_create(active_screen, NULL);
    lv_obj_align(infected_btn, NULL, LV_ALIGN_CENTER, 0, -40);
    infected_btn_label = lv_label_create(btn1, NULL);
    lv_label_set_text(infected_btn_label, "Infected");

    // wifi_label = lv_label_create(active_screen, NULL);
    // lv_obj_align(wifi_label, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 6);
    // lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    // lv_label_set_recolor(wifi_label, true);

    // out_txtarea = lv_textarea_create(active_screen, NULL);
    // lv_obj_set_size(out_txtarea, 300, 180);
    // lv_obj_align(out_txtarea, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -12);
    // lv_textarea_set_max_length(out_txtarea, MAX_TEXTAREA_LENGTH);
    // lv_textarea_set_text_sel(out_txtarea, false);
    // lv_textarea_set_cursor_hidden(out_txtarea, true);
    // lv_textarea_set_text(out_txtarea, "Starting Cloud Connected Blinky\n");
    xSemaphoreGive(xGuiSemaphore);
}