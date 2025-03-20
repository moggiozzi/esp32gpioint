#include <string.h>
#include "benchmarker.h"

/**
 * @brief ������������� ��������� ��������� ����������. ��������������� ������������ �����
 *        DEFINE_BENCHMARK ������ ���� �������.
 *
 * @param pBenchmark: ��������� �� ���������������� ���������.
 * @param name: ��������� �� ������-������������� ������� ���������� ����������.
 * @param n: ������ ������� (����� ��������), �� �������� ����� ������������ ���������� ������� ����������.
 * @param autoPrintResults: ���� ������������� ������ ����������� � ����� ������� ����� ����������
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
 * @brief ����� � ��� ����������� ������ ����������. ��� ������� ����� ���������� ������������� � ����� ������� <br>
 *        ����� ����������, ���� bmark_init() ���� ������� � autoPrintResult = true.
 *
 * @param pBenchmark: ��������� �� ��������� ��������� ������� ���������� ����������.
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
 * @brief ������������ ����������. ��� ������� ���������� ��������������� ����� ��������� ����������� ���������. <br>
 *        ���������� ���������� ���������� ���������� ���������� �, ��� �������������, ������ � ��� �����������.
 *
 * @param pBenchmark: ��������� �� ��������� ��������� ������� ���������� ����������.
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
      // ������������ �������
      pBenchmark->averageExecTime = pBenchmark->accumulator / pBenchmark->iteration;
      pBenchmark->accumulator = 0;
      pBenchmark->iteration = 0;
      // ������� ���������� ���� ���������
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

