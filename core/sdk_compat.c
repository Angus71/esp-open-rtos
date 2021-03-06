/*
 * Wrappers for functions called by binary espressif libraries
 *
 * Part of esp-open-rtos
 * Copyright (C) 2015 Superhouse Automation Pty Ltd
 * BSD Licensed as described in the file LICENSE
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <common_macros.h>

void IRAM *zalloc(size_t nbytes)
{
    return calloc(1, nbytes);
}
