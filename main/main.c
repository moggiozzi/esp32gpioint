#include "main.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_idf_version.h"

#define BUILD_DATE_TIME (__DATE__" "__TIME__)
void app_main(void)
{
	ESP_LOGI("", "\r\nApp build %s \nESP_IDF_VER %s\r\n", BUILD_DATE_TIME, esp_get_idf_version());

	start_spp();
    start_gpio_int();
    start_pwm();
    while(1) {
    	vTaskDelay(pdMS_TO_TICKS(1000));
    	printBM();
    }
}
