#pragma once
#include "defs.h"
struct spinlock
{
    uint32 locked;
    int count;
    struct cpu* p;
};
