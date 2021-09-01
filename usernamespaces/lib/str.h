#ifndef _STR_H
#define _STR_H

#include <stdio.h>
#include <stdlib.h>

#define FMT_SIZED_BUFFER(fmt, arg) malloc(snprintf(0,0,(fmt),(arg))+1)

#endif
