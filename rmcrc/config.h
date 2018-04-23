#ifndef __CONFIG_H
#define __CONFIG_H
#include <stdio.h>
#include <string.h>
#include "msg_lcd.h"

#define CONFIG_PATH 128

int save_param(void);
int read_param(void);

#endif