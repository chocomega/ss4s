#pragma once

#ifndef SS4S_MODAPI_H

#include <stdbool.h>
#include <stddef.h>

#include "ss4s/player.h"
#include "ss4s/audio.h"
#include "ss4s/video.h"

typedef struct SS4S_Config {
    const char *audioDriver;
    const char *videoDriver;
} SS4S_Config;

int SS4S_Init(int argc, char *argv[], const SS4S_Config *config);

int SS4S_PostInit(int argc, char *argv[]);

void SS4S_Quit();

const char *SS4S_GetAudioModuleName();

const char *SS4S_GetVideoModuleName();

bool SS4S_IsDriverAvailable(const char *driverName);

#endif // SS4S_MODAPI_H