/*
 * @Author: OULIHONG 1756950720@qq.com
 * @Date: 2024-03-07 14:11:03
 * @LastEditors: OULIHONG 1756950720@qq.com
 * @LastEditTime: 2024-03-07 14:11:03
 * @FilePath: \蓝牙拓展模块\apps\spp_and_le\examples\hpy_ble_module\hardware_drive\uart_drive.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */

#pragma once
#include <cpu.h>

#define MASTER_TX_IO IO_PORT_DP
#define MASTER_RX_IO IO_PORT_DM

#define SLAVE_TX_IO IO_PORTA_08
#define SLAVE_RX_IO IO_PORTA_07

void uart_master_send(u8 *buf, u8 len);

void uart_slave_send(u8 *buf, u8 len);


void uart_callback_register(void (*callback)(unsigned char *, unsigned char), unsigned char uart_index);

void uart_init_master(u32 baud);

void uart_init_slave(u32 baud);
