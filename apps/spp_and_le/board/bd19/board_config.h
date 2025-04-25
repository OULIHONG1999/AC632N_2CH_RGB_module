#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#ifndef   PWR_NO_CHANGE
//提供给预编译使用，定值同 power_interface.h
#define   PWR_NO_CHANGE             0
#define   PWR_LDO33                 1
#define   PWR_LDO15                 2
#define   PWR_DCDC15                3
#define   PWR_DCDC15_FOR_CHARGE     4
#endif


/*
 *  板级配置选择
 */
#define CONFIG_BOARD_AC632N_DEMO

#include "board_ac632n_demo_cfg.h"

#endif
