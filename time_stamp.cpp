//
// Created by kaylor on 22-7-5.
//

#include "time_stamp.h"
#include "sys/time.h"

timestamp_t TimeStamp::now_with_milliseconds()
{
  struct timeval tv;
  gettimeofday(&tv,0);
  timestamp_t time=(timestamp_t)tv.tv_sec*1000+tv.tv_usec/1000;
  return time;
}

timestamp_t TimeStamp::now_with_microseconds()
{
  struct timeval tv;
  gettimeofday(&tv,0);
  timestamp_t time=(timestamp_t)tv.tv_sec*1000000+tv.tv_usec;
  return time;
}