#include "main.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_attr.h"

#include "esp_idf_version.h"
//#include "gpio.h"
#include "driver/gpio.h"
#include "soc/gpio_reg.h"

#include "benchmarker.h"
DEFINE_BENCHMARK(intBM, "", 2000, false);
void printBM(void) { BMARK_PRINT_RESULTS(&intBM); }

uint32_t int_counter = 0;
IRAM_ATTR
static void gpio_isr_handler(void* arg)
{
	if (int_counter > 0)
	{
		BMARK_SUSPEND(&intBM);
		BMARK_RUN(&intBM);
	} else {
		BMARK_RUN(&intBM);
	}
    int_counter++;

#ifndef USE_GPIO_ISR_SERVICE
	uint32_t gpio_intr_status = READ_PERI_REG(GPIO_STATUS_REG);   //read status to get interrupt status for GPIO0-31
	uint32_t gpio_intr_status_h = READ_PERI_REG(GPIO_STATUS1_REG);//read status1 to get interrupt status for GPIO32-39
	SET_PERI_REG_MASK(GPIO_STATUS_W1TC_REG, gpio_intr_status);    //Clear intr for gpio0-gpio31
	SET_PERI_REG_MASK(GPIO_STATUS1_W1TC_REG, gpio_intr_status_h); //Clear intr for gpio32-39
#endif
}

void start_gpio_int(void)
{
	esp_err_t ret = ESP_OK;
	gpio_config_t io_conf;
	//interrupt of rising edge
	io_conf.intr_type = GPIO_INTR_POSEDGE;//GPIO_INTR_POSEDGE;//GPIO_INTR_NEGEDGE;//GPIO_INTR_ANYEDGE;//
	io_conf.pin_bit_mask = ((uint64_t)1 << INT_GPIO_PIN);
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	ret |= gpio_config(&io_conf);
#ifdef USE_GPIO_ISR_SERVICE
	ret |= gpio_install_isr_service(ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3);
	ret |= gpio_isr_handler_add(INT_GPIO_PIN, gpio_isr_handler, NULL);
#else
	ret |= gpio_intr_enable(INT_GPIO_PIN);
	ret |= gpio_set_intr_type(INT_GPIO_PIN, GPIO_INTR_POSEDGE);
	ret |= gpio_isr_register(gpio_isr_handler, NULL, ESP_INTR_FLAG_IRAM, NULL);
#endif
	if (ret == ESP_OK)
	{
		ESP_LOGI(__func__, "OK");
	} else
	{
		ESP_LOGE(__func__, "ERROR %d", ret);
	}
}
