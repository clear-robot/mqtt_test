//
// Created by kaylor on 22-7-5.
//

#ifndef MQTT__TIME_STAMP_H_
#define MQTT__TIME_STAMP_H_
typedef long long timestamp_t;
class TimeStamp {
 public:
  static timestamp_t now_with_milliseconds();
  static timestamp_t now_with_microseconds();
};

#endif //MQTT__TIME_STAMP_H_
