#pragma once

THIRD_PARTY_INCLUDES_START

#ifdef _WIN64
#include "Windows/AllowWindowsPlatformTypes.h"
#include "MQTTClient.h"
#include "Windows/HideWindowsPlatformTypes.h"

#else
#include "MQTTClient.h"
#endif

THIRD_PARTY_INCLUDES_END