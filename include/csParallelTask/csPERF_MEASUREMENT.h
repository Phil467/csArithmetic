#pragma once

#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_CSPARALLEL_DLL
    #define CSPARALLEL_API __declspec(dllexport)
  #else
    #define CSPARALLEL_API __declspec(dllimport)
  #endif
#else
  #ifdef BUILDING_CSPARALLEL_DLL
    #define CSPARALLEL_API __attribute__ ((visibility ("default")))
  #else
    #define CSPARALLEL_API
  #endif
#endif

#ifndef CSPERF_MEASUREMENT_H
#define CSPERF_MEASUREMENT_H

#include <chrono>
#include <iostream>

#define CSTIME_UNIT_HOUR            0
#define CSTIME_UNIT_MINUTE          1
#define CSTIME_UNIT_SECOND          2
#define CSTIME_UNIT_MILLISECOND     3
#define CSTIME_UNIT_MICROSECOND     4
#define CSTIME_UNIT_NANOSECOND      5

class CSPARALLEL_API csPERF_MEASUREMENT
{
  private:
  std::chrono::time_point<std::chrono::high_resolution_clock>
  strt, stp;
  size_t ellapsed;
  int unit = CSTIME_UNIT_MICROSECOND;
  char*unitName = " microseconds\0";

  public:
/**
 * @brief TODO: Constructor.
 * @param unit TODO: Time unit.
 */
  csPERF_MEASUREMENT(int unit = CSTIME_UNIT_MICROSECOND);
/**
 * @brief TODO: Sets up the time unit.
 * @param unit TODO: Time unit.
 */
  void setTimeUnit(int unit);
/**
 * @brief TODO: Starts measuring execution time.
 */
  void start();
/**
 * @brief TODO: Stop measuring execution time.
 */
  void stop();
/**
 * @brief TODO: Print the execution time for a code block
 * @param title TODO: Output Title.
 */
  void printReport(const char* title);
/**
 * @brief TODO: Returns execution time for a code block.
 * @return size_t TODO: Execution time.
 */
  size_t getEllapsedTime();
};

#endif // CSPERF_MEASUREMENT_H
