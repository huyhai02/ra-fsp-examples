/***********************************************************************************************************************
 * File Name    : system_config.h
 * Description  : Consolidated system configuration header file containing API definitions, 
 *              : data structures, enumerations, and configuration macros for GUIX-based GUI system
 **********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2023 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include "gx_api.h"

/***********************************************************************************************************************
 * System Configuration Macros
 **********************************************************************************************************************/
/* Temperature Settings */
#define TEMP_MAX_C                      (30.0f)
#define TEMP_MIN_C                      (15.0f)
#define TEMP_INC_C                      (0.6f)

/* Brightness Settings */
#define PWM_BRIGHTNESS_CONTROL          (1)
#define BRIGHTNESS_DIMMING              (1)
#define BRIGHTNESS_MAX                  (100)
#define BRIGHTNESS_MIN                  (10)
#define BRIGHTNESS_INC                  (5)

/* System Timing */
#define SPLASH_TIMEOUT                  (160)

/***********************************************************************************************************************
 * API Macros
 **********************************************************************************************************************/
/* Calculate the temperature from ADC TSN value
 * Refer to section 55.3.1 of the RA8D2 Hardware User Manual for further info
 * Formula: T = ((Vs - V1) / Slope) + 95
 * Where: V1 = 3.3 * CAL95 / 4096, Vs = 3.3 * ADC / 4096, Slope = 2.7 mV/°C
 * CAL95 = 1289 (from R_TSN_CAL->TSCDR)
 */
#define ADCTEMP_AS_C(a)             ((((float)(a)) * 0.29839409722f) - 289.747169907f)
#define ADCTEMP_AS_F(a)             ((((float)(a)) * 0.537109375f) - 489.544705833f)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
struct tm
{
  int   tm_sec;
  int   tm_min;
  int   tm_hour;
  int   tm_mday;
  int   tm_mon;
  int   tm_year;
  int   tm_wday;
  int   tm_yday;
  int   tm_isdst;
};
typedef struct tm rtc_time_t;

typedef struct {
    rtc_time_t     current_time;
    uint16_t       temperature;
} shared_data_t;

typedef enum e_sf_message_event_class
{
    SF_MESSAGE_EVENT_CLASS_AUDIO, /* Audio Playback */
    SF_MESSAGE_EVENT_CLASS_TOUCH, /* Touch */
    SF_MESSAGE_EVENT_CLASS_SYSTEM, /* System */
    SF_MESSAGE_EVENT_CLASS_VOLUME, /* Volume */
    SF_MESSAGE_EVENT_CLASS_TIME, /* Time */
    SF_MESSAGE_EVENT_CLASS_TEMPERATURE, /* Temperature */
    SF_MESSAGE_EVENT_CLASS_DISPLAY, /* Display */
} sf_message_event_class_t;

typedef enum e_sf_message_event
{
    SF_MESSAGE_EVENT_UNUSED, /* Unused */
    SF_MESSAGE_EVENT_NEW_DATA, /* New Data */
    SF_MESSAGE_EVENT_AUDIO_START, /* Audio Playback Start */
    SF_MESSAGE_EVENT_AUDIO_STOP, /* Audio Playback Stop */
    SF_MESSAGE_EVENT_AUDIO_PAUSE, /* Audio Playback Pause */
    SF_MESSAGE_EVENT_AUDIO_RESUME, /* Audio Playback Resume */
    SF_MESSAGE_EVENT_REFRESH, /* Refresh */
    SF_MESSAGE_EVENT_TEMPERATURE_INCREMENT, /* Temperature Increment */
    SF_MESSAGE_EVENT_TEMPERATURE_DECREMENT, /* Temperature Decrement */
    SF_MESSAGE_EVENT_HOUR_MODE_TOGGLE, /* Hour Mode Toggle */
    SF_MESSAGE_EVENT_AM_PM_TOGGLE, /* AM PM Toggle */
    SF_MESSAGE_EVENT_SET_TIME, /* Set Time */
    SF_MESSAGE_EVENT_SET_DATE, /* Set Date */
    SF_MESSAGE_EVENT_FAN_TOGGLE, /* Fan Toggle */
    SF_MESSAGE_EVENT_SYSTEM_MODE_TOGGLE, /* System Mode Toggle */
    SF_MESSAGE_EVENT_TEMPERATURE_UNIT_C, /* Temperature Unit C */
    SF_MESSAGE_EVENT_TEMPERATURE_UNIT_F, /* Temperature Unit F */
    SF_MESSAGE_EVENT_BRIGHTNESS_INCREMENT, /* Brightness Increment */
    SF_MESSAGE_EVENT_BRIGHTNESS_DECREMENT, /* Brightness Decrement */
    SF_MESSAGE_EVENT_VOLUME_INCREMENT, /* Volume Increment */
    SF_MESSAGE_EVENT_VOLUME_DECREMENT, /* Volume Decrement */
    SF_MESSAGE_EVENT_UPDATE_TEMPERATURE, /* Update Temperature */
    SF_MESSAGE_EVENT_UPDATE_TIME, /* Update Time */
    SF_MESSAGE_EVENT_UPDATE_DATE, /* Update Date */
} sf_message_event_t;

/** Conversion of system events to GUIX events */
typedef enum
{
    /** Some classes do not use the event enumeration. */
    GXEVENT_MSG_UNUSED = GX_FIRST_APP_EVENT,

    /** System data refresh. */
    GXEVENT_MSG_REFRESH_SYSTEM_DATA,

    /** Temperature set point adjustment. */
    GXEVENT_MSG_TEMPERATURE_INCREMENT,
    GXEVENT_MSG_TEMPERATURE_DECREMENT,

    /** Time setting. */
    GXEVENT_MSG_HOUR_MODE_TOGGLE,
    GXEVENT_MSG_AM_PM_TOGGLE,
    GXEVENT_MSG_TIME_UPDATE,
    GXEVENT_MSG_DATE_UPDATE,

    /** System settings. */
    GXEVENT_MSG_FAN_TOGGLE,
    GXEVENT_MSG_SYSTEM_MODE_TOGGLE,

    /** Temperature unit settings. */
    GXEVENT_MSG_TEMPERATURE_UNIT_C,
    GXEVENT_MSG_TEMPERATURE_UNIT_F,

    /** Display Settings. */
    GXEVENT_MSG_BRIGHTNESS_INCREMENT,
    GXEVENT_MSG_BRIGHTNESS_DECREMENT,

    /** Volume Settings. */
    GXEVENT_MSG_VOLUME_INCREMENT,
    GXEVENT_MSG_VOLUME_DECREMENT,

    /** Temperature data. */
    GXEVENT_MSG_UPDATE_TEMPERATURE,

    /** Time data. */
    GXEVENT_MSG_UPDATE_TIME,
} gx_event_message_t;

typedef enum
{
    SYSTEM_TEMP_UNITS_C,
    SYSTEM_TEMP_UNITS_F,
} system_temp_units_t;

typedef enum
{
    SYSTEM_HOUR_MODE_24  = 0,
    SYSTEM_HOUR_MODE_12  = 1,
} system_hour_mode_t;

typedef enum
{
    SYSTEM_FAN_MODE_AUTO   = 0,
    SYSTEM_FAN_MODE_ON     = 1,
} system_fan_mode_t;

typedef enum
{
    SYSTEM_MODE_OFF = 0,
    SYSTEM_MODE_COOL,
    SYSTEM_MODE_HEAT,
    SYSTEM_MODE_MAX
} system_mode_t;

typedef struct st_time_payload
{
    rtc_time_t  time;
} time_payload_t;

/** Message header definition  */
typedef struct st_message_header
{
    union
    {
        uint32_t  event;
        struct
        {
            uint32_t  class_code     : 8;      /* Event class code */
            uint32_t  class_instance : 8;      /* Event class instance number */
            uint32_t  code           : 16;     /* Event code */
        }  event_b;
    };
} system_msg_header_t;

typedef struct st_system_state
{
    rtc_time_t          time;
    system_hour_mode_t  hour_mode;
    system_mode_t       mode;
    system_fan_mode_t   fan_mode;
    system_temp_units_t temp_units;
    float               temp_c;         /* In Celsius */
    float               target_temp;    /* In system_state_t::temp_units */
    uint8_t             brightness;
    bool                fan_on;
} system_state_t;

typedef struct st_system_payload
{
    GX_EVENT        gx_event;
    system_state_t  state;
} system_payload_t;

typedef union u_system_msg_payload
{
    time_payload_t time_payload;
} system_msg_payload_t;

typedef struct u_system_msg
{
    system_msg_header_t msg_id;
    system_msg_payload_t msg_payload;
}system_msg_t; /*system_payload_t*/

#endif /* SYSTEM_CONFIG_H */