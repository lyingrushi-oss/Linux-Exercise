#include "crc.h"

int crc_calculate(int value)
{
    return value ^ 0x55;
}