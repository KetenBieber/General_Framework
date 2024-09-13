#pragma once

#include "logtest.h"
#include "bsp_log.h"

int LogTestFunc(void)
{
    int b = 0;
    PrintLog("LogTestFunc\n");
    return 0;
}

int LogTestFunc1(void)
{
    int a = 2;
    PrintLog("LogTestFunc1\n");
    return 0;
}

int LogTestFunc2(void)
{
    int c = 3;
    PrintLog("LogTestFunc2\n");
    return 0;
}