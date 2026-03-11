/*
 * veeprom.c
 *
 *  Created on: 25JUN2023
 *      Author: VIn D'Agostino, D'Agostino Industries Group, Inc.
 */

#include "hal_data.h"
#include "veeprom.h"
#include "project.h"

extern uint8_t callback_called;
extern uint16_t onTimeSetting;
extern uint16_t offTimeSetting;
extern uint8_t enableBlueLED;
uint16_t  *p_rec1_in_flash;
uint8_t *p_b_led_feat_in_flash;
uint8_t * p_refData;
uint32_t lengthInBytes;
extern uint8_t callback_called;
extern uint8_t writeReference;
extern uint8_t writeParameters;
extern uint8_t refData[REF_DATA_LENGTH];

uint8_t writeState;
fsp_err_t err;


void OpenAndRetrieveVEEPROM_Values(void)
{
    // ***** Manage the VEEPROM Info on startup *****
    callback_called = false;
    err = RM_VEE_FLASH_Open(&g_vee0_ctrl, &g_vee0_cfg);
    if (err != FSP_SUCCESS)
    {
        while(1);
    }

    // Grab the onTime value stored in VEEPROM
    lengthInBytes = 2;
    callback_called = false;
    err = RM_VEE_FLASH_RecordPtrGet(&g_vee0_ctrl, ON_TIME_ID, (uint8_t **)&p_rec1_in_flash, &lengthInBytes);
    if (err == FSP_SUCCESS)
    {
        onTimeSetting = *p_rec1_in_flash;
    }
    else if (err != FSP_ERR_NOT_FOUND)
    {
        while(1);
    }

    // Grab the offTime value stored in VEEPROM
    lengthInBytes = 2;
    callback_called = false;
    err = RM_VEE_FLASH_RecordPtrGet(&g_vee0_ctrl, OFF_TIME_ID, (uint8_t **)&p_rec1_in_flash, &lengthInBytes);
    if (err == FSP_SUCCESS)
    {
        offTimeSetting = *p_rec1_in_flash;
    }
    else if (err != FSP_ERR_NOT_FOUND)
    {
        while(1);
    }

    // Grab the pointer to the Blue LED enabled feature status value stored in VEEPROM
    // ...the code will use this value while it is in flash
    lengthInBytes = 1;
    callback_called = false;
    err = RM_VEE_FLASH_RecordPtrGet(&g_vee0_ctrl, BLUE_LED_FEATURE_ID, (uint8_t **)&p_b_led_feat_in_flash, &lengthInBytes);
    if (err == FSP_SUCCESS)
    {
        enableBlueLED = *p_b_led_feat_in_flash;
    }
    else if (err != FSP_ERR_NOT_FOUND)
    {
        while(1);
    }

    err = RM_VEE_FLASH_RefDataPtrGet(&g_vee0_ctrl, &p_refData);
    if (err == FSP_SUCCESS)
    {
        strcpy((char *)&refData, (char *)p_refData);

    }
    else if (err != FSP_ERR_NOT_FOUND) {
        while(1);
    }

}


void WriteVEEPROM_Parameters(void)
{
    callback_called = false;

    switch (writeState++)
    {
        case 0:
            err = RM_VEE_FLASH_RecordWrite(&g_vee0_ctrl, ON_TIME_ID, (uint8_t *)&onTimeSetting, sizeof(onTimeSetting));
            break;
        case 1:
            err = RM_VEE_FLASH_RecordWrite(&g_vee0_ctrl, OFF_TIME_ID, (uint8_t *)&offTimeSetting, sizeof(onTimeSetting));
            break;
        case 2:
            err = RM_VEE_FLASH_RecordWrite(&g_vee0_ctrl, BLUE_LED_FEATURE_ID, (uint8_t *)&enableBlueLED, sizeof(onTimeSetting));
            break;
        case 3:
            writeParameters = false;
            writeState = 0;
            break;
    }

}


void WriteVEEPROM_Reference(void)
{
    err = RM_VEE_FLASH_RefDataWrite(&g_vee0_ctrl, (uint8_t *)&refData);

    writeReference = false;

}
