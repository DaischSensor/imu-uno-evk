#include "zephyr_systick.h"
#include <zephyr/kernel.h>

static uint64_t cycles_per_sec = 0;

void TickInit(void)
{
    cycles_per_sec = sys_clock_hw_cycles_per_sec();
}

uint64_t GetTickUs(void)
{
    uint64_t cyc = k_cycle_get_64();

    return (cyc / cycles_per_sec) * 1000000ULL +
           (cyc % cycles_per_sec) * 1000000ULL / cycles_per_sec;
}
