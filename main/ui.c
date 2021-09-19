#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "core2forAWS.h"
#include "ui.h"

#include "bl.h"

#define MAX_TEXTAREA_LENGTH 1024

static lv_obj_t *active_screen;
static lv_obj_t *out_txtarea;
static lv_obj_t *wifi_label;

static lv_obj_t *infected_btn;
static lv_obj_t *infected_btn_label;

static lv_obj_t *check_btn;
static lv_obj_t *check_btn_label;

static char *TAG = "UI";

extern QueueHandle_t msg_queue_infected;
extern QueueHandle_t msg_queue_check;

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

void event_handler_infected_btn(lv_obj_t *obj, lv_event_t event)
{
    char msg[100];
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        lv_state_t state = lv_obj_get_state(obj, LV_BTN_PART_MAIN);
        if (state & LV_STATE_CHECKED)
        {
            bl_get_simple_message(&msg);
            xQueueSend(msg_queue_infected, msg, 0);
        }
    }
}

void event_handler_check_btn(lv_obj_t *obj, lv_event_t event)
{
    char msg[100];
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        lv_state_t state = lv_obj_get_state(obj, LV_BTN_PART_MAIN);
        if (state & LV_STATE_CHECKED)
        {
            bl_get_simple_message(&msg);
            xQueueSend(msg_queue_check, msg, 0);
        }
    }
}

void reset_infected_btn()
{
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    lv_btn_toggle(infected_btn);
    xSemaphoreGive(xGuiSemaphore);
}
void reset_check_btn()
{
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    lv_btn_toggle(infected_btn);
    xSemaphoreGive(xGuiSemaphore);
}

void ui_init()
{
    xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
    active_screen = lv_scr_act();

    infected_btn = lv_btn_create(active_screen, NULL);
    lv_obj_align(infected_btn, NULL, LV_ALIGN_CENTER, -70, -80);
    infected_btn_label = lv_label_create(infected_btn, NULL);
    lv_label_set_text(infected_btn_label, "Infected");
    lv_obj_set_event_cb(infected_btn, event_handler_infected_btn);
    lv_btn_set_checkable(infected_btn, true);
    lv_btn_set_fit2(infected_btn, LV_FIT_NONE, LV_FIT_TIGHT);

    check_btn = lv_btn_create(active_screen, NULL);
    lv_obj_align(check_btn, NULL, LV_ALIGN_CENTER, 70, -80);
    check_btn_label = lv_label_create(check_btn, NULL);
    lv_label_set_text(check_btn_label, "Check");
    lv_obj_set_event_cb(check_btn, event_handler_check_btn);
    lv_btn_set_checkable(check_btn, true);
    lv_btn_set_fit2(check_btn, LV_FIT_NONE, LV_FIT_TIGHT);

    wifi_label = lv_label_create(active_screen, NULL);
    lv_obj_align(wifi_label, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 6);
    lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    lv_label_set_recolor(wifi_label, true);

    out_txtarea = lv_textarea_create(active_screen, NULL);
    lv_obj_set_size(out_txtarea, 300, 140);
    lv_obj_align(out_txtarea, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -12);
    lv_textarea_set_max_length(out_txtarea, MAX_TEXTAREA_LENGTH);
    lv_textarea_set_text_sel(out_txtarea, false);
    lv_textarea_set_cursor_hidden(out_txtarea, true);
    lv_textarea_set_text(out_txtarea, "Starting Health Alarm\n");
    xSemaphoreGive(xGuiSemaphore);
}