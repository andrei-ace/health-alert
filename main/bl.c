/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/****************************************************************************
*
* This file is for Classic Bluetooth device and service discovery Demo.
*
****************************************************************************/

#include <stdint.h>
#include <string.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "bl.h"
#include "set.h"

#define GAP_TAG "GAP"

typedef enum
{
    APP_GAP_STATE_IDLE = 0,
    APP_GAP_STATE_DEVICE_DISCOVERING,
    APP_GAP_STATE_DEVICE_DISCOVER_COMPLETE,
    APP_GAP_STATE_SERVICE_DISCOVERING,
    APP_GAP_STATE_SERVICE_DISCOVER_COMPLETE,
} app_gap_state_t;

typedef struct
{
    bool dev_found;
    uint8_t bdname_len;
    uint8_t eir_len;
    uint8_t rssi;
    uint32_t cod;
    uint8_t eir[ESP_BT_GAP_EIR_DATA_LEN];
    uint8_t bdname[ESP_BT_GAP_MAX_BDNAME_LEN + 1];
    esp_bd_addr_t bda;
    app_gap_state_t state;
} app_gap_cb_t;

static app_gap_cb_t m_dev_info;

static simple_set address_set;

extern QueueHandle_t msg_queue;

static uint8_t *own_address;

static char *
bda2str(esp_bd_addr_t bda, char *str, size_t size)
{
    if (bda == NULL || str == NULL || size < ADDR_LEN)
    {
        return NULL;
    }

    uint8_t *p = bda;
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            p[0], p[1], p[2], p[3], p[4], p[5]);
    return str;
}

static void update_device_info(esp_bt_gap_cb_param_t *param)
{
    char bda_str[18];
    char *address = bda2str(param->disc_res.bda, bda_str, ADDR_LEN);
    ESP_LOGI(GAP_TAG, "Device found: %s", address);
    if (set_length(&address_set) <= MAX_ADDR_LEN &&
        set_contains(&address_set, address) == SET_FALSE)
    {
        set_add(&address_set, address);
    }
}

static void bt_app_gap_init(void)
{
    app_gap_cb_t *p_dev = &m_dev_info;
    memset(p_dev, 0, sizeof(app_gap_cb_t));

    /* start to discover nearby Bluetooth devices */
    p_dev->state = APP_GAP_STATE_DEVICE_DISCOVERING;
}

static void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    app_gap_cb_t *p_dev = &m_dev_info;
    char bda_str[ADDR_LEN];

    switch (event)
    {
    case ESP_BT_GAP_DISC_RES_EVT:
    {
        update_device_info(param);
        break;
    }
    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
    {
        if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED)
        {
            ESP_LOGI(GAP_TAG, "Device discovery stopped.");
        }
        else if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED)
        {
            ESP_LOGI(GAP_TAG, "Discovery started.");
        }
        break;
    }
    case ESP_BT_GAP_RMT_SRVC_REC_EVT:
    default:
    {
        ESP_LOGI(GAP_TAG, "event: %d", event);
        break;
    }
    }
    return;
}

static void bt_app_gap_start_up(void)
{
    char *dev_name = "ESP_GAP";
    char bda_str[ADDR_LEN];
    esp_bt_dev_set_device_name(dev_name);

    /* register GAP callback function */
    esp_bt_gap_register_callback(bt_app_gap_cb);

    /* set discoverable and connectable mode, wait to be connected */
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    /* inititialize device information and status */
    bt_app_gap_init();

    esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
}

void bl_get_simple_message(char *msg_address)
{
    char bda_str[ADDR_LEN];
    char *own_address_str = bda2str(own_address, bda_str, ADDR_LEN);
    sprintf(msg_address, "{\"own\":\"%s\"}", own_address_str);
    // sprintf(msg_address, "{\"own\":\"xxx\"}");
}

void initialise_bl(void)
{
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK)
    {
        ESP_LOGE(GAP_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK)
    {
        ESP_LOGE(GAP_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK)
    {
        ESP_LOGE(GAP_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
}

void start_bl(void)
{
    esp_err_t ret;
    if ((ret = esp_bluedroid_enable()) != ESP_OK)
    {
        ESP_LOGE(GAP_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    own_address = esp_bt_dev_get_address();

    set_init(&address_set);

    bt_app_gap_start_up();
}

char **get_addresses(uint64_t *size)
{
    return set_to_array(&address_set, size);
}

void stop_bl(void)
{
    ESP_LOGI(GAP_TAG, "Cancel device discovery ...");
    esp_bt_gap_cancel_discovery();

    uint64_t addr_size;
    char **address = get_addresses(&addr_size);
    char values[MSG_QUEUE_SIZE];
    char bda_str[ADDR_LEN];
    char *own_address_str = bda2str(own_address, bda_str, ADDR_LEN);
    sprintf(values, "{\"own\":\"%s\",\"seen\":[", own_address_str);
    for (int i = 0; i < addr_size; i++)
    {
        strcat(values, "\"");
        strcat(values, address[i]);
        strcat(values, "\"");
        if (i != addr_size - 1)
        {
            strcat(values, ",");
        }
    }
    strcat(values, "]}");
    if (addr_size > 0)
    {
        ESP_LOGI(GAP_TAG, "Sent to queue: %s", values);
        xQueueSend(msg_queue, values, 0);
    }
    free(address);

    set_destroy(&address_set);

    esp_err_t ret;
    if ((ret = esp_bluedroid_disable()) != ESP_OK)
    {
        ESP_LOGE(GAP_TAG, "%s disable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
}
