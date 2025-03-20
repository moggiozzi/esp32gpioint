#include <string.h>
#include "benchmarker.h"

/**
 * @brief Инициализация структуры состояния измерителя. Предпочтительно использовать макро
 *        DEFINE_BENCHMARK вместо этой функции.
 *
 * @param pBenchmark: указатель на инициализируемую структуру.
 * @param name: указатель на строку-идентификатор данного экземпляра измерителя.
 * @param n: размер выборки (число прогонов), по которому будет производится усреднение времени выполнения.
 * @param autoPrintResults: флаг необходимости вывода результатов в конце каждого цикла усреднения
 */
BM_FUNS_ATTR
void bmark_init(BENCHMARK_DATA_t* pBenchmark, char const* name, uint16_t n, bool autoPrintResults)
{
  memset(pBenchmark, 0, sizeof(BENCHMARK_DATA_t));
  pBenchmark->minExecTime = UINT32_MAX;
  pBenchmark->name = name;
  pBenchmark->n = n;
  pBenchmark->autoPrintResults = autoPrintResults;
}


/**
 * @brief Вывод в лог результатов работы измерителя. Эта функция будет вызываться автоматически в конце каждого <br>
 *        цикла усреднения, если bmark_init() была вызвана с autoPrintResult = true.
 *
 * @param pBenchmark: указатель на структуру состояния данного экземпляра измерителя.
 */

void bmark_printResults(BENCHMARK_DATA_t* pBenchmark)
{
  const char defName[] = "Anonimous";
  const char *pName = defName;
  if (pBenchmark->name != NULL)
  {
    pName = pBenchmark->name;
  }
  DBG("%s benchmark results: min = %u, max = %u, total = %u", pName,
                                                               pBenchmark->minExecTime,
                                                               pBenchmark->maxExecTime,
                                                               pBenchmark->totalExecTime);

  if (pBenchmark->n) {
    DBG(", average = %u\r\n", pBenchmark->averageExecTime);
  } else {
	DBG("\r\n");
  }
}


/**
 * @brief Приостановка измерителя. Эта функция вызывается непосредственно после окончания тестируемой процедуры. <br>
 *        Производит обновление внутренних переменных измерителя и, при необходимости, выдачу в лог результатов.
 *
 * @param pBenchmark: указатель на структуру состояния данного экземпляра измерителя.
 */
BM_FUNS_ATTR
void bmark_suspend(BENCHMARK_DATA_t* pBenchmark)
{
 uint32_t endTime = BMARK_GET_CURRENT_TICKS_COUNT();
 uint32_t currentExecTime = endTime - pBenchmark->startTime;
//  int32_t currentExecTime = endTime - pBenchmark->startTime;
//  if (currentExecTime < 0)
//  {
//    DBG("%s: Error: Negative execution time !!\r\n", __func__);
//    return;
//  }

  if (currentExecTime < pBenchmark->minExecTime)
  {
    pBenchmark->minExecTime = currentExecTime;
  }

  if (currentExecTime > pBenchmark->maxExecTime)
  {
    pBenchmark->maxExecTime = currentExecTime;
  }

  if (pBenchmark->n)
  {
    pBenchmark->accumulator += currentExecTime;
    pBenchmark->iteration++;
    if (pBenchmark->iteration >= pBenchmark->n)
    {
      pBenchmark->totalExecTime += pBenchmark->accumulator;
      // рассчитываем среднее
      pBenchmark->averageExecTime = pBenchmark->accumulator / pBenchmark->iteration;
      pBenchmark->accumulator = 0;
      pBenchmark->iteration = 0;
      // выводим результаты если требуется
      if (pBenchmark->autoPrintResults == true)
      {
        bmark_printResults(pBenchmark);
      }
      pBenchmark->minExecTime = UINT32_MAX;;
      pBenchmark->maxExecTime = 0;
    }
  }
  else
  {
    pBenchmark->totalExecTime += currentExecTime;
  }
}

