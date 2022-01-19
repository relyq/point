#ifndef __HTTP_OTA_H__
#define __HTTP_OTA_H__

void simple_ota_example_task(void *pvParameter);
void perform_ota_update(void);
void get_sha256_of_partitions(void);
// only for debugging purposes
void print_running_partition(void);
#endif  // __HTTP_OTA_H__