#ifndef __APP_MAIN_H__
#define __APP_MAIN_H__

struct sensor_msg {
  char DeviceClass[5];
  char IdDevice[13];  // device mac address
  char MsgType[5];
  char MsgContent[64];
};

#endif  // __APP_MAIN_H__