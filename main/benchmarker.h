#ifndef BENCHMARKER_H_
#define BENCHMARKER_H_
/* ------------------------------------------------------------------------------------------------ */
#include <stdbool.h>
#include <inttypes.h>

#define ENABLE_BENCHMARK

#ifdef ENABLE_BENCHMARK
#include "esp_log.h"
#include "esp_attr.h"
#define DBG(fmt,...) ESP_LOGI("BM", fmt, ##__VA_ARGS__)
#define BM_FUNS_ATTR IRAM_ATTR
#else
#define DBG(...)
#define BM_FUNS_ATTR
#endif //defined (DEBUG)

// *************** макро чтения счетчика времени (определить 1 из трех вариантов) *****************
// 1.Использовать для миллисекундной точности.
//#define BMARK_GET_CURRENT_TICKS_COUNT()  hal_getSysTimer()

// 2.Использовать для микросекундной точности
// (из-за возросших накладных расходов нельзя рассчитывать на точночть лучше 10 мкс).
// Уточнение: при выключеной оптимизации (-O0) на платформе LPC2468-72MHz-GCC, исполнение из flash
// собственные "накладные расходы" измерителя составляют 6 мкС.
#define BMARK_GET_CURRENT_TICKS_COUNT()   bmark_get_uSeconds()
#include "esp_timer.h" //esp_timer_get_time()
static inline uint32_t bmark_get_uSeconds()
{
	return (uint32_t)esp_timer_get_time();
}
// 3.
//#define BMARK_GET_CURRENT_TICKS_COUNT()   (DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk, DWT->CYCCNT)

// **************************************************************

/**
 * @brief Структура состояния измерителя
 */
typedef struct BENCHMARK_DATA
{
  const char *name;        // указатель  на строку, идентифицирующую тест (выдается лог вместе с результатами).
  unsigned int minExecTime;       // минимальное время выполнения тестируемой опрерации.
  unsigned int maxExecTime;       // максимальное время выполнения тестируемой опрерации.
  unsigned int totalExecTime;     // общее накопленное время выполнения тестируемой опрерации с начала теста.
  unsigned int averageExecTime;   // среднее время выполнения тестируемой опрерации. Обновляется каждые n прогонов.
  unsigned int startTime;         // переменная для сохранения времени начала выполнения тестируемой процедуры.
  unsigned int accumulator;       // накопленное время выполнения тестируемой опрерации с последнего расчета аverageExecTime.
  unsigned int iteration;         // счетчик прогонов.
  unsigned int n;                 // количество прогонов, по которым производится усреднение.
  bool   autoPrintResults;  // флаг разрешения вывода результатов в лог каждые n прогонов.
} BENCHMARK_DATA_t;


#if defined(ENABLE_BENCHMARK)
#include <stdio.h>

/**
 * @brief Определение и инициализация структуры состояния измерителя. 	<br>
 *		  Использование:                                                  <br>
 *                                                                      <br>
 *		  <внешний уровень>                                               <br>
 *      ...                                                             <br>
 *		  DEFINE_BENCHMARK(bmarkName, "charName", n, autoPrintResults);   <br>
 *      ...                                                             <br>
 *		  <уровень функции>                                               <br>
 *		  ...                                                             <br>
 *		  BMARK_RUN(&bmarkName);                                          <br>
 *		  <тестируемый код>                                               <br>
 *		  BMARK_SUSPEND(&bmarkName);                                      <br>
 *		  ...
 * @param bmarkName: имя инициализируемой структуры состояния.
 * @param "charName": строка-идентификатор данного экземпляра измерителя.
 * @param n (uint16): размер выборки (число прогонов), по которому будет производится усреднение времени выполнения.
 * @param autoPrintResults (bool): флаг необходимости вывода результатов в конце каждого цикла усреднения.
 */
#define DEFINE_BENCHMARK(bmarkName, charName, n, autoPrintResults) 	\
	static const char bmarkName##Str[] = charName;	\
	static BENCHMARK_DATA_t bmarkName = {bmarkName##Str, UINT32_MAX, 0, 0, 0, 0, 0, 0, (n), (autoPrintResults)}

/**
 * @brief Инициализация структуры состояния измерителя. Предпочтительно использовать макро
 *        DEFINE_BENCHMARK вместо этой функции.
 *
 * @param pBenchmark: указатель на инициализируемую структуру.
 * @param name: указатель на строку-идентификатор данного экземпляра измерителя.
 * @param n: размер выборки (число прогонов), по которому будет производится усреднение времени выполнения.
 * @param autoPrintResults: флаг необходимости вывода результатов в конце каждого цикла усреднения
 */
#define BENCHMARK_INIT(bmarkName, charName, n, autoPrintResults) bmark_init((bmarkName), (charName), (n), (autoPrintResults))
void bmark_init(BENCHMARK_DATA_t* pBenchmark, char const* name, uint16_t n, bool autoPrintResults);

/**
 * @brief Запуск измерителя. Эта функция вызывается непосредственно перед началом тестируемой процедуры.
 *
 * @param pBenchmark: указатель на структуру состояния данного экземпляра измерителя.
 */
#define  BMARK_RUN(pBenchmark) bmark_run((pBenchmark))
static inline void bmark_run(BENCHMARK_DATA_t* pBenchmark)
{
  pBenchmark->startTime = BMARK_GET_CURRENT_TICKS_COUNT();
}

/**
 * @brief Приостановка измерителя. Эта функция вызывается непосредственно после окончания тестируемой процедуры. <br>
 *        Производит обновления внутренних переменных измерителя и, при необходимости, выдачу в лог результатов. <br>
 *        Может вызываться только после BMARK_RUN().
 *
 * @param pBenchmark: указатель на структуру состояния данного экземпляра измерителя.
 */
#define  BMARK_SUSPEND(pBenchmark) bmark_suspend((pBenchmark))
void bmark_suspend(BENCHMARK_DATA_t* pBenchmark);

/**
 * @brief Вывод в лог результатов работы измерителя. Эта функция может вызываться автоматически в конце каждого <br>
 *        цикла усреднения, если bmark_init() была вызвана с autoPrintResult = true.
 *
 * @param pBenchmark: указатель на структуру состояния данного экземпляра измерителя.
 */

#define  BMARK_PRINT_RESULTS(pBenchmark) bmark_printResults((pBenchmark))
void bmark_printResults(BENCHMARK_DATA_t* pBenchmark);



#else //defined(ENABLE_BENCHMARK)

#define DEFINE_BENCHMARK(bmarkName, charName, n, autoPrintResults)
#define BENCHMARK_INIT(bmarkName, charName, n, autoPrintResults)
#define BMARK_RUN(pBenchmark)
#define BMARK_SUSPEND(pBenchmark)
#define BMARK_PRINT_RESULTS(pBenchmark)

#endif //defined(ENABLE_BENCHMARK)

#endif  /* BENCHMARKER_H_ */
