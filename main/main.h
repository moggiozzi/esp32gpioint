#include "esp_idf_version.h"
#if ESP_IDF_VERSION_MAJOR < 5
#define IDF_42 // use 4.2 API, else 5.4 API used
#endif

#define INT_GPIO_PIN               34 // GPIO for interrupt
#define PWM_PULSE_GPIO             32 // GPIO for PWM signal line

#define PWM_TIMEBASE_RESOLUTION_HZ 1000000  // 1MHz, 1us per tick
#define PWM_TIMEBASE_PERIOD        500      // 500us

//#define USE_GPIO_ISR_SERVICE // gpio_install_isr_service() API performance is worse than gpio_isr_register() API

void start_spp(void);
void start_pwm(void);
void start_gpio_int(void);

void printBM(void); // print benchmark measures
