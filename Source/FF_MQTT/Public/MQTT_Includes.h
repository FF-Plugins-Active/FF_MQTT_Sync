#pragma once

// UE Includes.
#include "JsonObjectWrapper.h"
#include "JsonUtilities.h"

// Custom Includes.
#include "MQTT_Enums.h"

THIRD_PARTY_INCLUDES_START

#ifdef _WIN64
#include "Windows/AllowWindowsPlatformTypes.h"
#include "MQTTClient.h"
#include "MQTTAsync.h"
#include "Windows/HideWindowsPlatformTypes.h"

#else
#include "MQTTClient.h"
#include "MQTTAsync.h"
#endif

#include <string>

THIRD_PARTY_INCLUDES_END