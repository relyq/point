#ifndef __APP_MAIN_H__
#define __APP_MAIN_H__

struct sensor_msg {
  char DeviceClass[5];
  char IdDevice[9];
  char MsgType[5];
  char MsgContent[64];
};

#endif  // __APP_MAIN_H__