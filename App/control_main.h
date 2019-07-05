/*
 * control_main.h
 *
 *  Created on: 2019��4��13��
 *      Author: gongjunhui
 */

#ifndef CONTROL_MAIN_H_
#define CONTROL_MAIN_H_


#include "main.h"

#define   COMM_UART_ID        (huart1)     // ͨѶ uart id


typedef enum {           //��������
    SWITCH_OPEN         ,  //��
	SWITCH_CLOSE        ,  //�ر�
}_device_switch_em;

typedef enum {           //��Ƶ����ģʽ
    AUDIO_DECODING      ,  //��Ƶ����
	AUDIO_INPUT         ,  //��Ƶ����
}_audio_mode_em;


typedef enum {           //����ģʽ ��ʾģʽ
	WORK_MODE_SCREEN_INIT      ,  //Һ����ʼ��
	WORK_MODE_POWER_ON     	   ,  //����logo
    WORK_MODE_SCREEN_CLOSE     ,  //Ϣ��
	WORK_MODE_NORMAL           ,  //����ģʽ
	//WORK_MODE_MENU             ,  //�˵�
	//WORK_MODE_SELECT     	   ,  //ģʽѡ��
	//WORK_MODE_COURSE     	   ,  //�̳�
	WORK_MODE_BL     	       ,  //��������
	WORK_MODE_FM     	       ,  //FM����
}_work_mode_em;

//extern void send_read_motor_data(_read_parameter_st  st_read_data);
extern _device_switch_em  em_BL_switch;//��������
extern _device_switch_em  em_FM_switch;//FM�㲥����

extern void task_control_power_on_function(void);
extern void task_control_main_function(void);

#endif /* CONTROL_MAIN_H_ */
