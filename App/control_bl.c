/*
 * control_bl.c
 *
 *  Created on: 2019��5��2��
 *      Author: gongjunhui
 */
#include "control_main.h"
#include "control_bl.h"
#include "main.h"
#include "usart.h"
#include <string.h>



//-----------���ݽ���----------
static  uint8_t  Host_Rec_buff[200];    /* ���ջ��� */
static  uint8_t  Host_Rec_byte[2];      /* ����byte */
static  uint16_t  Rec_Count = 0;  //���ռ���
static  volatile  uint8_t  Rec_OK_Flag = 0;               /* ������ɱ�־  */
static  volatile  uint32_t  frame_interval_time=0;        /* ֡�������  */
static  volatile  uint32_t  frame_interval_time_flag=0;   /* ֡���������־  */

static uint8_t bl_init_flag;    //��ʼ��
static uint8_t bl_restart_flag; //������־
static uint8_t bl_delay_count;  //��ʱ����

_bl_status_em  em_bl_state;//����ģ��״̬

const char*  str_delvmlink= "AT+DELVMLINK";

/*******************************************
 * ��������received_data_Reset
 * ����  ��  ���ݽ������㸴λ
 * ����  �� ��
 * ���  �� ��
 * ��������V0.00  2019-5-2
********************************************/
static void received_data_Reset(void)
{
	memset(Host_Rec_buff, 0, sizeof(Host_Rec_buff));   //��ջ�����
    Rec_OK_Flag =0;
	Rec_Count=0;
	//Data_Length =0;
	frame_interval_time=0;        /* ֡�������  */
	frame_interval_time_flag=0;   /* ֡���������־  */

	//HAL_UART_Receive_IT(&MESH_UART_ID,(uint8_t *)Host_Rec_byte,1);
}


/*******************************************
 * ��������received_data_processing
 * ����  �� �յ������ݴ���
 * ����  �� ��
 * ���  �� ��
 * ��������V0.00  2019-5-2
********************************************/
static void  received_data_processing(void)
{

    if(Rec_OK_Flag == 0){
	    return;
    }
    if(strstr((char const*)Host_Rec_buff,"CONNECTED") != NULL){
    	em_bl_state = BL_STATUS_LINK_OK;  //���ӳɹ�

    }else if(strstr((char const*)Host_Rec_buff,"POWER ON") != NULL){
        em_bl_state = BL_STATUS_LINKING;  //������

    }else if(strstr((char const*)Host_Rec_buff,"DISCONNECT") != NULL){
        em_bl_state = BL_STATUS_LINKING;  //������

    }else{

    }

    received_data_Reset();
}


/*******************************************
 * ��������bl_data_handle_init
 * ����  ��
 * ����  �� ��
 * ���  �� ��
 * ��������V0.00  2019-5-2
********************************************/
void bl_data_handle_init(void)
{
	received_data_Reset();
	while(HAL_OK != HAL_UART_Receive_IT(&huart1,(uint8_t *)Host_Rec_byte,1));

}
/*******************************************
 * ��������bl_control_function
 * ����  ��  ����������
 * ����  ����
 * ���  ����
 * ��������V0.00  2019-5-2
 ********************************************/
void bl_control_function(void)
{
   if(em_BL_switch == SWITCH_OPEN){//��
	  BL_POWER_ON;
   }else{         //�ر�
	  BL_POWER_OFF;
   }
   received_data_processing(); //�յ��������ݵĴ���

   if(bl_init_flag == 0x5A){
	   bl_restart_flag=0;
	   bl_delay_count++;
	   if(bl_delay_count == 1){
		   HAL_UART_Transmit_IT(&huart1,(uint8_t*)str_delvmlink,strlen(str_delvmlink));
	   }else if(bl_delay_count <= (500/10)){
		   em_BL_switch = SWITCH_OPEN;
	   }else if(bl_delay_count <= (1500/10)){
		   em_BL_switch = SWITCH_CLOSE;
	   }else{
		   bl_delay_count = 0;
		   bl_init_flag = 0;
		   em_BL_switch = SWITCH_OPEN;
	   }
   }

   if(bl_restart_flag == 0x5A){
	   bl_delay_count++;
	   if(bl_delay_count <= (1000/10)){
		   em_BL_switch = SWITCH_CLOSE;
	   }else{
		   bl_delay_count = 0;
		   bl_restart_flag = 0;
		   em_BL_switch = SWITCH_OPEN;
	   }
   }


}

/*******************************************
 * ��������bl_bind_init
 * ����  �� ����ģ�����°�
 * ����  �� ��
 * ���  �� ��
 * ��������V0.00  2019-5-2
********************************************/
void bl_bind_init(void)
{
	bl_init_flag = 0x5A;    //��ʼ��
	bl_delay_count = 0;     //��ʱ����
	em_bl_state = BL_STATUS_RESET;    //����
}
/*******************************************
 * ��������bl_binding
 * ����  �� ����ģ���
 * ����  �� ��
 * ���  �� ��
 * ��������V0.00  2019-5-2
********************************************/
void bl_binding(void)
{
   bl_restart_flag = 0x5A;//������־
   bl_delay_count = 0;  //��ʱ����
   em_bl_state = BL_STATUS_RESTART;  //��������
}
/********************************
 * ������: UART_RxTime
 * ˵��:
 ********************************/
void UART_RxTime(void)
{
	if(frame_interval_time_flag == 1){
		frame_interval_time ++ ;        /* ֡�������  */
		if(frame_interval_time>=30){    //30ms֡���
			Rec_OK_Flag = 1;
			frame_interval_time_flag=0;   /* ֡���������־  */
		}
	}
}

/********************************
 * �������� UART_RxCallback
 * ˵��:  ͨѶģ�� �����жϻص�����
 ********************************/
static void UART_RxCallback(void)
{

	frame_interval_time=0;        /* ֡�������  */
	frame_interval_time_flag=1;   /* ֡���������־  */

	Host_Rec_buff[Rec_Count] = Host_Rec_byte[0];
	if(Rec_OK_Flag == 0){
	    Rec_Count++;
	}
	if(Rec_Count > 198){
		Rec_OK_Flag = 1;
		frame_interval_time_flag=0;   /* ֡���������־  */
	}

	while(HAL_OK != HAL_UART_Receive_IT(&huart1,(uint8_t *)Host_Rec_byte,1));
}



/********************************
 * ��������HAL_UART_RxCpltCallback
 * ˵��: �����жϻص�����
 ********************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{

	if(UartHandle == &huart1){
		UART_RxCallback();
	}
}

