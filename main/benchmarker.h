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

// *************** ����� ������ �������� ������� (���������� 1 �� ���� ���������) *****************
// 1.������������ ��� �������������� ��������.
//#define BMARK_GET_CURRENT_TICKS_COUNT()  hal_getSysTimer()

// 2.������������ ��� �������������� ��������
// (��-�� ��������� ��������� �������� ������ ������������ �� �������� ����� 10 ���).
// ���������: ��� ���������� ����������� (-O0) �� ��������� LPC2468-72MHz-GCC, ���������� �� flash
// ����������� "��������� �������" ���������� ���������� 6 ���.
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
 * @brief ��������� ��������� ����������
 */
typedef struct BENCHMARK_DATA
{
  const char *name;        // ���������  �� ������, ���������������� ���� (�������� ��� ������ � ������������).
  unsigned int minExecTime;       // ����������� ����� ���������� ����������� ���������.
  unsigned int maxExecTime;       // ������������ ����� ���������� ����������� ���������.
  unsigned int totalExecTime;     // ����� ����������� ����� ���������� ����������� ��������� � ������ �����.
  unsigned int averageExecTime;   // ������� ����� ���������� ����������� ���������. ����������� ������ n ��������.
  unsigned int startTime;         // ���������� ��� ���������� ������� ������ ���������� ����������� ���������.
  unsigned int accumulator;       // ����������� ����� ���������� ����������� ��������� � ���������� ������� �verageExecTime.
  unsigned int iteration;         // ������� ��������.
  unsigned int n;                 // ���������� ��������, �� ������� ������������ ����������.
  bool   autoPrintResults;  // ���� ���������� ������ ����������� � ��� ������ n ��������.
} BENCHMARK_DATA_t;


#if defined(ENABLE_BENCHMARK)
#include <stdio.h>

/**
 * @brief ����������� � ������������� ��������� ��������� ����������. 	<br>
 *		  �������������:                                                  <br>
 *                                                                      <br>
 *		  <������� �������>                                               <br>
 *      ...                                                             <br>
 *		  DEFINE_BENCHMARK(bmarkName, "charName", n, autoPrintResults);   <br>
 *      ...                                                             <br>
 *		  <������� �������>                                               <br>
 *		  ...                                                             <br>
 *		  BMARK_RUN(&bmarkName);                                          <br>
 *		  <����������� ���>                                               <br>
 *		  BMARK_SUSPEND(&bmarkName);                                      <br>
 *		  ...
 * @param bmarkName: ��� ���������������� ��������� ���������.
 * @param "charName": ������-������������� ������� ���������� ����������.
 * @param n (uint16): ������ ������� (����� ��������), �� �������� ����� ������������ ���������� ������� ����������.
 * @param autoPrintResults (bool): ���� ������������� ������ ����������� � ����� ������� ����� ����������.
 */
#define DEFINE_BENCHMARK(bmarkName, charName, n, autoPrintResults) 	\
	static const char bmarkName##Str[] = charName;	\
	static BENCHMARK_DATA_t bmarkName = {bmarkName##Str, UINT32_MAX, 0, 0, 0, 0, 0, 0, (n), (autoPrintResults)}

/**
 * @brief ������������� ��������� ��������� ����������. ��������������� ������������ �����
 *        DEFINE_BENCHMARK ������ ���� �������.
 *
 * @param pBenchmark: ��������� �� ���������������� ���������.
 * @param name: ��������� �� ������-������������� ������� ���������� ����������.
 * @param n: ������ ������� (����� ��������), �� �������� ����� ������������ ���������� ������� ����������.
 * @param autoPrintResults: ���� ������������� ������ ����������� � ����� ������� ����� ����������
 */
#define BENCHMARK_INIT(bmarkName, charName, n, autoPrintResults) bmark_init((bmarkName), (charName), (n), (autoPrintResults))
void bmark_init(BENCHMARK_DATA_t* pBenchmark, char const* name, uint16_t n, bool autoPrintResults);

/**
 * @brief ������ ����������. ��� ������� ���������� ��������������� ����� ������� ����������� ���������.
 *
 * @param pBenchmark: ��������� �� ��������� ��������� ������� ���������� ����������.
 */
#define  BMARK_RUN(pBenchmark) bmark_run((pBenchmark))
static inline void bmark_run(BENCHMARK_DATA_t* pBenchmark)
{
  pBenchmark->startTime = BMARK_GET_CURRENT_TICKS_COUNT();
}

/**
 * @brief ������������ ����������. ��� ������� ���������� ��������������� ����� ��������� ����������� ���������. <br>
 *        ���������� ���������� ���������� ���������� ���������� �, ��� �������������, ������ � ��� �����������. <br>
 *        ����� ���������� ������ ����� BMARK_RUN().
 *
 * @param pBenchmark: ��������� �� ��������� ��������� ������� ���������� ����������.
 */
#define  BMARK_SUSPEND(pBenchmark) bmark_suspend((pBenchmark))
void bmark_suspend(BENCHMARK_DATA_t* pBenchmark);

/**
 * @brief ����� � ��� ����������� ������ ����������. ��� ������� ����� ���������� ������������� � ����� ������� <br>
 *        ����� ����������, ���� bmark_init() ���� ������� � autoPrintResult = true.
 *
 * @param pBenchmark: ��������� �� ��������� ��������� ������� ���������� ����������.
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
