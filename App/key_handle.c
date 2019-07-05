/*
 * key_handle.c
 *
 *  Created on: 2019��5��1��
 *      Author: gongjunhui
 */
#include "main.h"
#include "key_handle.h"

volatile  _key_value_em  uckey_value;   /* ������ֵ  */

//��������
static  uint8_t key_fm_count;
static  uint8_t key_bl_count;
static  uint8_t key_main_count;
//�������������־
static  uint8_t key_fm_en_flag;
static  uint8_t key_bl_en_flag;
static  uint8_t key_main_en_flag;

/*******************************************
 * ��������ISR_key_handle_function
 * ����  ��  ���������� ���ж��е���  10msһ��
 * ����  ��  ��
 * ���  ��  ��
 * ����  ��  V0.00  2019-5-1
********************************************/
void ISR_key_handle_function(void)
{
   if(uckey_value != KEY_NONE){ //�����ڼ�ֵ���ڼ���µļ�ֵ
//	   key_fm_count = 0;
//	   key_bl_count = 0;
//	   key_main_count = 0;
	   return;
   }
   //----------�������ж�----------------
   if(KEY_BL==GPIO_PIN_RESET){
	   if(key_bl_en_flag==1){
		   return;
	   }
	   key_bl_count++;
	   if(key_bl_count>(2000/10)){//2S
	   	   uckey_value = KEY_BL_LONG_PRESS;   //BL�� ����
	   	   key_bl_en_flag=1;
	   	   key_bl_count=0;
	   }
   }else{
	   if(key_bl_count>(50/10)){
		   uckey_value = KEY_BL_SHORT_PRESS;  //BL�� �̰�
	   }
	   key_bl_count=0;
	   key_bl_en_flag = 0;
   }
   //----------FM���ж�----------------
   if(KEY_FM==GPIO_PIN_RESET){
	   if(key_fm_en_flag==1){
	   		return;
	   }
   	   key_fm_count++;
   	   if(key_fm_count>(2000/10)){//2S
   	   	   uckey_value = KEY_FM_LONG_PRESS;   //BL�� ����
   	   	   key_fm_en_flag=1;
   	   	   key_fm_count=0;
   	   }
   }else{
   	   if(key_fm_count>(50/10)){
   		   uckey_value = KEY_FM_SHORT_PRESS;  //BL�� �̰�
   	   }
   	   key_fm_count=0;
   	   key_fm_en_flag = 0;
   }
   //----------�����ж�----------------
   if(KEY_MAIN==GPIO_PIN_RESET){
	   if(key_main_en_flag==1){
	  	  return;
	   }
   	   key_main_count++;
   	   if(key_main_count>(2000/10)){//2S
   	   	   uckey_value = KEY_MAIN_LONG_PRESS;   //���� ����
   	   	   key_main_en_flag=1;
   	   	   key_main_count=0;
   	   }
   }else{
   	   if(key_main_count>(50/10)){
   		   uckey_value = KEY_MAIN_SHORT_PRESS;  //���� �̰�
   	   }
   	   key_main_count=0;
   	   key_main_en_flag = 0;
   }

}


