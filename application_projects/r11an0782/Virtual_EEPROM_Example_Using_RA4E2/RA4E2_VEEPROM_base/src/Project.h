/*
 * Project.h
 *
 *  Created on: Jul 9, 2023
 *      Author: VincentD'Agostino
 */

#ifndef PROJECT_H_
#define PROJECT_H_

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define PROJ_REV "VEEPROM v6.2.0\r\n\r\n"

#define DEFAULT_ON_TIME 40
#define DEFAULT_OFF_TIME 40

#define MAX_TIME 150
#define MIN_TIME 5

#define BLUE_LED_INDEX 0

#define DEFAULT_SER_NO "SNxxxxxx"
#define DEFAULT_MODEL_NAME "Acme Widget 2000"
#define DEFAULT_REF_DATA "SNxxxxxx;Acme Widget 2000"

#define SER_NO_LENGTH 9
#define MODEL_NAME_LENGTH 25
#define REF_DATA_LENGTH 32

enum VEPROM_IDS {
    ON_TIME_ID = 1,
    OFF_TIME_ID,
    BLUE_LED_FEATURE_ID
};

#endif /* PROJECT_H_ */
