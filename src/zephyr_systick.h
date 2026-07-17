#ifndef ZEPHYR_SYSTICK_H_
#define ZEPHYR_SYSTICK_H_

#include <zephyr/kernel.h>

void TickInit(void);
uint64_t GetTickUs(void);

#endif