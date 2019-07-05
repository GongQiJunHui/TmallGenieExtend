/*
 * control_main.c
 *
 *  Created on: 2019��4��13��
 *      Author: gongjunhui
 */


#include "control_main.h"
#include "control_bl.h"
#include "key_handle.h"
#include <string.h>
#include "gpio.h"
#include "st7735.h"
#include "qn8027.h"
#include "image.h"	//ͼƬ����
#include "iwdg.h"


_audio_mode_em  em_audio_mode; //��Ƶ����ģʽ
_work_mode_em   em_work_mode;  //������ʾģʽ

_device_switch_em  em_FM_switch;//FM�㲥����
_device_switch_em  em_BL_switch;//��������
//static _device_switch_em  em_ADC_switch;//���뿪��
static _device_switch_em  em_play_switch;//���ǲ��ſ���

static  uint8_t  first_power_on_flag;//�����ϵ��־λ
static  uint8_t  uc_lcd_init_state;//����Һ����ʼ���ж�

static  uint16_t  uc_lcd_init_count;//����Һ����ʼ������
//static  uint8_t  lcd_display_refresh_flag; //��ʾ��־

static  uint8_t  uc_fm_channel_old;  //fm����Ƶ�� ��ʾ�� 76Mhz - 108 Mhz ֻȡ����
static  _bl_status_em  em_bl_state_old;//����ģ��״̬

static  uint8_t   falsh_write_flag;  //flash����д��־
static  uint16_t  falsh_write_count; //flash����д����
static  uint16_t  menu_exit_count;       //�˵��˳�����
static  uint16_t  dormancy_exit_count;   //���߼���

/*******************************************
 * ��������flash_write_handle
 * ����  ��  flashд����
 * ����  ��  ��
 * ���  ��  ��
 * ����  ��  V0.00  2019-5-2
********************************************/
static void flash_write_handle(void)
{
   FLASH_EraseInitTypeDef f;
   uint32_t PageError = 0;//����PageError
   uint32_t ui_data = 0;//��Ҫ���������
   uint32_t ui_dataq = 0;

   //byte1 byte2 byte3 byte4
   //byte4:bit7-bit4: 0001��Ƶ����ģʽ  0000��Ƶ����ģʽ
   //byte4:bit3-bit0: 0001���ǲ��ſ�      0000���ǲ��Ź�
   //byte3:bit7-bit4: 0001������             0000������
   //byte3:bit3-bit0: 0001FM��               0000FM��
   //byte2:FMƵ��

   HAL_FLASH_Unlock();//1������FLASH
   //2������FLASH
   //��ʼ��FLASH_EraseInitTypeDef
   f.TypeErase = FLASH_TYPEERASE_PAGES;/*!<Pages erase only*/
   f.PageAddress = 0x08007C00;//��ʼ��ַ
   f.NbPages = 1;//����1��ҳ
   //���ò�������
   HAL_FLASHEx_Erase(&f, &PageError);
   ui_data = 0;
   if(em_audio_mode == AUDIO_DECODING){//��Ƶ����ģʽ ��Ƶ����
	   ui_data |= 0x10;
   }
   //if(em_play_switch == SWITCH_OPEN){//���ǲ��ſ���
	//   ui_data |= 0x01;
   //}
   if(em_BL_switch == SWITCH_OPEN){   //��������
   	   ui_data |= 0x1000;
   }
   if(em_FM_switch == SWITCH_OPEN){   //FM����
      ui_data |= 0x0100;
   }
   ui_dataq = uc_fm_channel;
   ui_data |= ((ui_dataq<<16)&0x00ff0000);//�㲥Ƶ��
   //3����FLASH��д
   HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x08007C00, ui_data);
   //4����סFLASH
   HAL_FLASH_Lock();
}
/*******************************************
 * ��������flash_read_handle
 * ����  ��  flash������
 * ����  ��  ��
 * ���  ��  ��
 * ����  ��  V0.00  2019-5-2
********************************************/
static void flash_read_handle(void)
{
   uint32_t ui_data = *(__IO uint32_t*)(0x08007C00);
   //byte4:bit3-bit0: 0001���ǲ��ſ�      0000���ǲ��Ź�
//   if((ui_data&0x0000000F)== 0x00000001){
//	   em_play_switch = SWITCH_OPEN;
//   }else{
//	   em_play_switch = SWITCH_CLOSE;
//   }
   //byte4:bit7-bit4: 0001��Ƶ����ģʽ  0000��Ƶ����ģʽ
   if((ui_data&0x000000F0)== 0x00000010){
	   em_audio_mode = AUDIO_DECODING;
   }else{
	   em_audio_mode = AUDIO_INPUT;
   }
   //byte3:bit7-bit4: 0001������             0000������
   if((ui_data&0x0000F000)== 0x00001000){
	   em_BL_switch = SWITCH_OPEN;
   }else{
	   em_BL_switch = SWITCH_CLOSE;
   }
   //byte3:bit3-bit0: 0001FM��               0000FM��
   if((ui_data&0x00000F00)== 0x00000100){
   	   em_FM_switch = SWITCH_OPEN;
   }else{
   	   em_FM_switch = SWITCH_CLOSE;
   }
   //byte2:FMƵ��
   uc_fm_channel = (ui_data>>16)&0xff;
   if(uc_fm_channel>108){
	   uc_fm_channel=108;
   }else if(uc_fm_channel<76){
	   uc_fm_channel=76;
   }

}
/*******************************************
 * ��������work_mode_screen_init
 * ����  ��  ��Ļ��ʼ��
 * ����  ��  ��
 * ���  ��  ��
 * ����  ��  V0.00  2019-4-13
********************************************/
static void work_mode_screen_init(void)
{
	uint8_t disp_buff[5];
	if(uc_lcd_init_state == 0){
		OLED_RST_Clr();
		uc_lcd_init_count++;
		if(uc_lcd_init_count>20){
			uc_lcd_init_count = 0;
			uc_lcd_init_state = 1;
		}
		return;
	}else if(uc_lcd_init_state == 1){
		OLED_RST_Set();
		uc_lcd_init_count++;
		if(uc_lcd_init_count>30){
			uc_lcd_init_count = 0;
			uc_lcd_init_state = 2;
		}
		return;
	}
	lcd_init();
	MX_IWDG_Refresh();
	if(first_power_on_flag == 1){
		first_power_on_flag = 0;
		em_work_mode = WORK_MODE_POWER_ON; //����logo
	}else{
		em_work_mode = WORK_MODE_NORMAL;   //����ģʽ
		uckey_value = KEY_NONE;   //û�м�ֵ
		//disp_clear_screen(BLACK);
		disp_rectangle(39,0,1,80,LGRAY);//
		disp_rectangle(120,0,1,80,LGRAY);
		disp_rectangle(40,0,80,80,DCYAN);

		if(em_FM_switch == SWITCH_OPEN){//FM�㲥����
			disp_image_3232(128,45,gImage_FM,GRAYBLUE,BLACK);
			if(uc_fm_channel>99){//��λ��
				disp_buff[0] = (uc_fm_channel/100)+0x30;
				disp_buff[1] = (uc_fm_channel/10%10)+0x30;
				disp_buff[2] = (uc_fm_channel%10)+0x30;
				disp_buff[3] = 0;
				display_gb1212(134,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
			}else{ //��λ��
				disp_buff[0] = (uc_fm_channel/10%10)+0x30;
				disp_buff[1] = (uc_fm_channel%10)+0x30;
				disp_buff[2] = 0;
				display_gb1212(141,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
			}
		}else{
			disp_image_3232(128,45,gImage_FM,GRAY,BLACK);
		}
		if(em_BL_switch == SWITCH_OPEN){//��������
			if(em_bl_state == BL_STATUS_LINK_OK){ //���ӳɹ�
				disp_image_3232(2,45,gImage_BL_ON,GRAYBLUE,BLACK);
			}else{
				disp_image_3232(2,45,gImage_BL_OFF,GRAYBLUE,BLACK);
			}
		}else{
			disp_image_3232(2,45,gImage_BL_OFF,GRAY,BLACK);
		}
		if(em_play_switch == SWITCH_OPEN){  //���ǲ��ſ���
			disp_image_3232(2,5,gImage_audio,GRAYBLUE,BLACK);
		}else{
			disp_image_3232(2,5,gImage_audio,GRAY,BLACK);
		}
		if(em_audio_mode == AUDIO_DECODING){  //��Ƶ����ģʽ ��Ƶ����
			disp_image_3232(128,5,gImage_headset,GRAYBLUE,BLACK);
			display_gb1616(64,2,"ģʽ",BLUE,DCYAN);//LGRAY
			disp_image_8040(40,20,gImage_music,MAGENTA,DCYAN);//LIGHTGREEN
			display_gb1212(56,67,"���ǽ���",GREEN,DCYAN);//RED
		}else{
			disp_image_3232(128,5,gImage_headset,GRAY,BLACK);
			display_gb1616(64,2,"ģʽ",BLUE,DCYAN);//LGRAY
			disp_image_8040(40,20,gImage_35mm,YELLOW,DCYAN);//LIGHTGREEN
			display_gb1212(56,67,"��Ƶ����",GREEN,DCYAN);//RED
		}
	}
}
/*******************************************
 * ��������work_mode_power_on
 * ����  ��  ��Ļlogo��ʾ
 * ����  ��  ��
 * ���  ��  ��
 * ����  ��  V0.00  2019-4-13
********************************************/
static void work_mode_power_on(void)
{
	uint8_t disp_buff[5];
	uc_lcd_init_count++;
	if(uc_lcd_init_count==1){
		//disp_clear_screen(BLACK);
		disp_image_10080(31,0,gImage_logo,BLUE,BLACK);
		//display_gb2424(8,47,"mastergong.cn",BLUE);
	}else if(uc_lcd_init_count==(1000/10)){
		display_gb1616(20,30,"�������Ҹ�����",RED,BLACK);
	}else if(uc_lcd_init_count==(3000/10)){
		display_gb1616(132,60,"1.0",GREEN,BLACK);
	}else if(uc_lcd_init_count==(5000/10)){
		disp_clear_screen(BLACK);
	}else if(uc_lcd_init_count>(5000/10)){
		uc_lcd_init_count =0 ;
//		disp_QRcode(BLUE,WHITE);
//		display_gb1616(12,20,"��",RED);
//		display_gb1616(12,50,"װ",RED);
//		display_gb1616(134,20,"��",RED);
//		display_gb1616(134,50,"��",RED);
		em_work_mode = WORK_MODE_NORMAL;   //����ģʽ
		uckey_value = KEY_NONE;   //û�м�ֵ

		//disp_clear_screen(BLACK);
		disp_rectangle(39,0,1,80,LGRAY);//
		disp_rectangle(120,0,1,80,LGRAY);
		disp_rectangle(40,0,80,80,DCYAN);

		if(em_FM_switch == SWITCH_OPEN){//FM�㲥����
			disp_image_3232(128,45,gImage_FM,GRAYBLUE,BLACK);
			if(uc_fm_channel>99){//��λ��
				disp_buff[0] = (uc_fm_channel/100)+0x30;
				disp_buff[1] = (uc_fm_channel/10%10)+0x30;
				disp_buff[2] = (uc_fm_channel%10)+0x30;
				disp_buff[3] = 0;
				display_gb1212(134,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
			}else{ //��λ��
				disp_buff[0] = (uc_fm_channel/10%10)+0x30;
				disp_buff[1] = (uc_fm_channel%10)+0x30;
				disp_buff[2] = 0;
				display_gb1212(141,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
			}
		}else{
			disp_image_3232(128,45,gImage_FM,GRAY,BLACK);
		}
		if(em_BL_switch == SWITCH_OPEN){//��������
			if(em_bl_state == BL_STATUS_LINK_OK){ //���ӳɹ�
				 disp_image_3232(2,45,gImage_BL_ON,GRAYBLUE,BLACK);
			}else{
				 disp_image_3232(2,45,gImage_BL_OFF,GRAYBLUE,BLACK);
			}
		}else{
			disp_image_3232(2,45,gImage_BL_OFF,GRAY,BLACK);
		}
		if(em_play_switch == SWITCH_OPEN){  //���ǲ��ſ���
			disp_image_3232(2,5,gImage_audio,GRAYBLUE,BLACK);
		}else{
			disp_image_3232(2,5,gImage_audio,GRAY,BLACK);
		}
		if(em_audio_mode == AUDIO_DECODING){  //��Ƶ����ģʽ ��Ƶ����
			disp_image_3232(128,5,gImage_headset,GRAYBLUE,BLACK);
			display_gb1616(64,2,"ģʽ",BLUE,DCYAN);//LGRAY
			disp_image_8040(40,20,gImage_music,MAGENTA,DCYAN);//LIGHTGREEN
			display_gb1212(56,67,"���ǽ���",GREEN,DCYAN);//RED
		}else{
			disp_image_3232(128,5,gImage_headset,GRAY,BLACK);
			display_gb1616(64,2,"ģʽ",BLUE,DCYAN);//LGRAY
			disp_image_8040(40,20,gImage_35mm,YELLOW,DCYAN);//LIGHTGREEN
			display_gb1212(56,67,"��Ƶ����",GREEN,DCYAN);//RED
		}
	}



}
/*******************************************
 * ��������work_mode_screen_close
 * ����  ��  ��Ļ�ر�
 * ����  ��  ��
 * ���  ��  ��
 * ����  ��  V0.00  2019-4-13
********************************************/
static void work_mode_screen_close(void)
{
  LCD_POWER_OFF;
  LCD_SCL_CLR;
  LCD_SDA_CLR;
  LCD_RESET_CLR;
  LCD_RS_CLR;
  LCD_CS_CLR;
  //----------��������---------------
  if(uckey_value != KEY_NONE){  //�а���
	  uckey_value = KEY_NONE;
	  em_work_mode = WORK_MODE_SCREEN_INIT;  //Һ����ʼ��
	  LCD_POWER_ON;
	  LCD_SCL_SET;
	  LCD_SDA_SET;
	  LCD_RESET_SET;
	  LCD_RS_SET;
	  LCD_CS_SET;
  }
}
/*******************************************
 * ��������work_mode_normal
 * ����  ��  ����ģʽ
 * ����  ��  ��
 * ���  ��  ��
 * ����  ��  V0.00  2019-4-13
********************************************/
static void work_mode_normal(void)
{
  uint8_t disp_buff[5];

  task_qn8027_function();

  //--------�����ж�-------
  dormancy_exit_count++;   //���߼���
  if(dormancy_exit_count >= (5*60*1000/10)){//5min
	  dormancy_exit_count=0; //���߼���
	  em_work_mode = WORK_MODE_SCREEN_CLOSE; //Ϣ��
  }
  //------- �����ж� ------------
  if(em_play_switch == SWITCH_OPEN){//���ǲ��ſ���
	  DAC_MTUE_OFF;
  }else{
	  DAC_MTUE_ON;
  }

  //-------Flash �洢���ֲ���------------
  if(falsh_write_flag == 0x5A){ //flash����д��־
	  falsh_write_count++;
	  if(falsh_write_count >= (5*60*1000/10)){//5min
		  falsh_write_flag = 0;
		  flash_write_handle();//flashд����
	  }
  }else{
	  falsh_write_count=0;
  }
  //----------���벿�ֵ�Դ����----------------
  if(em_audio_mode == AUDIO_DECODING){ //��Ƶ����
	  DAC_POWER_ON;
  }else{             //��Ƶ����
	  DAC_POWER_OFF;
  }
  //------------������־ʵʱˢ��----------------
  if(em_bl_state_old != em_bl_state){
	  if(em_bl_state == BL_STATUS_LINK_OK){ //���ӳɹ�
		  disp_image_3232(2,45,gImage_BL_ON,GRAYBLUE,BLACK);
	  }else{
	  	  disp_image_3232(2,45,gImage_BL_OFF,GRAYBLUE,BLACK);
	  }
      em_bl_state_old = em_bl_state;//��¼����״̬
  }
  //----------��������---------------
  if(uckey_value == KEY_FM_SHORT_PRESS){  //FM�� �̰�
	  uckey_value = KEY_NONE;
	  dormancy_exit_count=0; //���߼���
	  falsh_write_flag = 0x5A; //flash����д��־
	  if(em_FM_switch == SWITCH_OPEN){//FM�㲥����
		  em_FM_switch = SWITCH_CLOSE;
		  disp_image_3232(128,45,gImage_FM,GRAY,BLACK);
	  }else{
		  em_FM_switch = SWITCH_OPEN;
		  disp_image_3232(128,45,gImage_FM,GRAYBLUE,BLACK);
		  if(uc_fm_channel>99){//��λ��
			 disp_buff[0] = (uc_fm_channel/100)+0x30;
			 disp_buff[1] = (uc_fm_channel/10%10)+0x30;
			 disp_buff[2] = (uc_fm_channel%10)+0x30;
			 disp_buff[3] = 0;
			 display_gb1212(134,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
		  }else{ //��λ��
			 disp_buff[0] = (uc_fm_channel/10%10)+0x30;
			 disp_buff[1] = (uc_fm_channel%10)+0x30;
			 disp_buff[2] = 0;
			 display_gb1212(141,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
		  }

	  }
  }else if(uckey_value == KEY_FM_LONG_PRESS){  //FM�� ����
	  uckey_value = KEY_NONE;
	  dormancy_exit_count=0; //���߼���
	  menu_exit_count = 0;   //�˵��˳�����
	  em_work_mode = WORK_MODE_FM;  //FM����
	  em_FM_switch = SWITCH_OPEN;
	  disp_clear_screen(BLACK);
	  uc_fm_channel_old = uc_fm_channel;//��¼FM��Ƶֵ
	  display_gb1616(55,2,"FM��Ƶ",RED,BLACK);//LGRAY
	  display_gb1212(4,65,"��",GREEN,BLACK);
	  display_gb1212(144,65,"��",GREEN,BLACK);
	  display_gb1212(66,65,"ȷ��",GREEN,BLACK);
	  if(uc_fm_channel_old < 76){//����
		  uc_fm_channel_old=76;
	  }
	  if(uc_fm_channel_old>99){//��λ��
		  disp_buff[0] = (uc_fm_channel_old/100)+0x30;
		  disp_buff[1] = (uc_fm_channel_old/10%10)+0x30;
		  disp_buff[2] = (uc_fm_channel_old%10)+0x30;
		  disp_buff[3] = 0;
	  }else{ //��λ��
		  disp_buff[0] = ' ';
		  disp_buff[1] = (uc_fm_channel_old/10%10)+0x30;
		  disp_buff[2] = (uc_fm_channel_old%10)+0x30;
		  disp_buff[3] = 0;
	  }
	  display_gb1616(42,30,(const char*)disp_buff,YELLOW,BLACK);//������ʾ
	  display_gb1616(88,30,"MHz",BLUE,BLACK);//������ʾ

  }else if(uckey_value == KEY_BL_SHORT_PRESS){  //������ �̰�
	  uckey_value = KEY_NONE;
	  dormancy_exit_count=0; //���߼���
	  falsh_write_flag = 0x5A; //flash����д��־
	  if(em_BL_switch == SWITCH_OPEN){//��������
		  em_BL_switch = SWITCH_CLOSE;
		  disp_image_3232(2,45,gImage_BL_OFF,GRAY,BLACK);
	  }else{
		  em_BL_switch = SWITCH_OPEN;
		  disp_image_3232(2,45,gImage_BL_OFF,GRAYBLUE,BLACK);
	  }
  }else if(uckey_value == KEY_BL_LONG_PRESS){  //������ ����
	  uckey_value = KEY_NONE;
	  dormancy_exit_count=0; //���߼���
	  menu_exit_count = 0;   //�˵��˳�����
	  em_work_mode = WORK_MODE_BL;   //��������
	  em_BL_switch = SWITCH_OPEN;
	  disp_clear_screen(BLACK);
	  display_gb1616(50,2,"��������",RED,BLACK);//LGRAY
	  display_gb1212(4,65,"��λ",GREEN,BLACK);
	  display_gb1212(135,65,"����",GREEN,BLACK);
	  display_gb1212(66,65,"����",GREEN,BLACK);

	  if(em_bl_state == BL_STATUS_RESET){ //��������
	  	  display_gb1212(45,35,"���������",YELLOW,BLACK);

	  }else if(em_bl_state == BL_STATUS_RESTART){ //������
		  display_gb1212(45,35,"ղ������ղ",YELLOW,BLACK);

	  }else if(em_bl_state == BL_STATUS_LINKING){ //������
		  display_gb1212(45,35,"ղ������ղ",YELLOW,BLACK);//

	  }else{//���ӳɹ�
		  display_gb1212(45,35,"ղ���ӳɹ�",BLUE,BLACK);//
	  }
	  em_bl_state_old = em_bl_state;//��¼����״̬

  }else if(uckey_value == KEY_MAIN_SHORT_PRESS){  //���� �̰�
	  uckey_value = KEY_NONE;
	  dormancy_exit_count=0; //���߼���
	  //falsh_write_flag = 0x5A; //flash����д��־
	  if(em_play_switch == SWITCH_OPEN){  //���ǲ��ſ���
		  em_play_switch = SWITCH_CLOSE;
		  disp_image_3232(2,5,gImage_audio,GRAY,BLACK);
	  }else{
		  em_play_switch = SWITCH_OPEN;
		  disp_image_3232(2,5,gImage_audio,GRAYBLUE,BLACK);
	  }
  }else if(uckey_value == KEY_MAIN_LONG_PRESS){   //���� ����
	  uckey_value = KEY_NONE;
	  dormancy_exit_count=0; //���߼���
	  falsh_write_flag = 0x5A; //flash����д��־
	  if(em_audio_mode == AUDIO_DECODING){  //��Ƶ����ģʽ ��Ƶ����
		  em_audio_mode = AUDIO_INPUT;//��Ƶ����
		  disp_image_3232(128,5,gImage_headset,GRAY,BLACK);
		  display_gb1616(64,2,"ģʽ",BLUE,DCYAN);
		  disp_image_8040(40,20,gImage_35mm,YELLOW,DCYAN);//LIGHTGREEN
		  display_gb1212(56,67,"��Ƶ����",GREEN,DCYAN);//RED
	  }else{
		  em_audio_mode = AUDIO_DECODING;////��Ƶ����
		  disp_image_3232(128,5,gImage_headset,GRAYBLUE,BLACK);
		  display_gb1616(64,2,"ģʽ",BLUE,DCYAN);//LGRAY
		  disp_image_8040(40,20,gImage_music,MAGENTA,DCYAN);//LIGHTGREEN
		  display_gb1212(56,67,"���ǽ���",GREEN,DCYAN);//RED
	  }
  }
}
/*******************************************
 * ��������work_mode_bl_set
 * ����  ��  ��������
 * ����  ��  ��
 * ���  ��  ��
 * ����  ��  V0.00  2019-5-1
********************************************/
static void work_mode_bl_set(void)
{
	uint8_t disp_buff[5];
    if(em_bl_state_old != em_bl_state){
    	if(em_bl_state == BL_STATUS_RESET){ //��������
    		display_gb1212(45,35,"���������",YELLOW,BLACK);

    	}else if(em_bl_state == BL_STATUS_RESTART){ //������
    		display_gb1212(45,35,"ղ������ղ",YELLOW,BLACK);

    	}else if(em_bl_state == BL_STATUS_LINKING){ //������
    		display_gb1212(45,35,"ղ������ղ",YELLOW,BLACK);//

    	}else{//���ӳɹ�
    		display_gb1212(45,35,"ղ���ӳɹ�",BLUE,BLACK);//
    	}
    	em_bl_state_old = em_bl_state;//��¼����״̬
    }

	//----------��������---------------
	if(uckey_value == KEY_FM_SHORT_PRESS){  //FM�� �̰� +
	    uckey_value = KEY_NONE;
	    menu_exit_count = 0;   //�˵��˳�����
	    bl_binding();

    }else if(uckey_value == KEY_BL_SHORT_PRESS){  //������ �̰� -
	    uckey_value = KEY_NONE;
	    menu_exit_count = 0;   //�˵��˳�����
	    bl_bind_init();

    }else if(uckey_value == KEY_MAIN_SHORT_PRESS){  //���� �̰�
	    uckey_value = KEY_NONE;
	    menu_exit_count = 0;   //�˵��˳�����
	    em_work_mode = WORK_MODE_NORMAL; //����ģʽ
	 	//д��FLASH����
        //disp_clear_screen(BLACK);
	 	disp_rectangle(39,0,1,80,LGRAY);//
	 	disp_rectangle(120,0,1,80,LGRAY);
	 	disp_rectangle(40,0,80,80,DCYAN);
	 	if(em_FM_switch == SWITCH_OPEN){//FM�㲥����
	 	     disp_image_3232(128,45,gImage_FM,GRAYBLUE,BLACK);
	 	     if(uc_fm_channel>99){//��λ��
	 	   	    disp_buff[0] = (uc_fm_channel/100)+0x30;
	 	   	    disp_buff[1] = (uc_fm_channel/10%10)+0x30;
	 	   	    disp_buff[2] = (uc_fm_channel%10)+0x30;
	 	   	    disp_buff[3] = 0;
	 	   	    display_gb1212(134,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
	 	     }else{ //��λ��
	 	   	    disp_buff[0] = (uc_fm_channel/10%10)+0x30;
	 	   	    disp_buff[1] = (uc_fm_channel%10)+0x30;
	 	   	    disp_buff[2] = 0;
	 	   	    display_gb1212(141,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
	 	     }
	 	}else{
	 	   	 disp_image_3232(128,45,gImage_FM,GRAY,BLACK);
	 	}
	 	if(em_BL_switch == SWITCH_OPEN){//��������
	 		if(em_bl_state == BL_STATUS_LINK_OK){ //���ӳɹ�
	 			disp_image_3232(2,45,gImage_BL_ON,GRAYBLUE,BLACK);
	 		}else{
	 			disp_image_3232(2,45,gImage_BL_OFF,GRAYBLUE,BLACK);
	 		}

	 	}else{
	 	    disp_image_3232(2,45,gImage_BL_OFF,GRAY,BLACK);
	 	}
	 	if(em_play_switch == SWITCH_OPEN){  //���ǲ��ſ���
	 	    disp_image_3232(2,5,gImage_audio,GRAYBLUE,BLACK);
	 	}else{
	 	   	disp_image_3232(2,5,gImage_audio,GRAY,BLACK);
	 	}
	 	if(em_audio_mode == AUDIO_DECODING){  //��Ƶ����ģʽ ��Ƶ����
	 	   	disp_image_3232(128,5,gImage_headset,GRAYBLUE,BLACK);
	 	   	display_gb1616(64,2,"ģʽ",BLUE,DCYAN);//LGRAY
	 	   	disp_image_8040(40,20,gImage_music,MAGENTA,DCYAN);//LIGHTGREEN
	 	   	display_gb1212(56,67,"���ǽ���",GREEN,DCYAN);//RED
	 	}else{
	 	   	disp_image_3232(128,5,gImage_headset,GRAY,BLACK);
	 	   	display_gb1616(64,2,"ģʽ",BLUE,DCYAN);//LGRAY
	 	   	disp_image_8040(40,20,gImage_35mm,YELLOW,DCYAN);//LIGHTGREEN
	 	   	display_gb1212(56,67,"��Ƶ����",GREEN,DCYAN);//RED
	 	}

	}else if(uckey_value == KEY_BL_LONG_PRESS){   //������ ����
		uckey_value = KEY_NONE;
		menu_exit_count = 0;   //�˵��˳�����
    }else if(uckey_value == KEY_FM_LONG_PRESS){   //FM�� ����
		uckey_value = KEY_NONE;
		menu_exit_count = 0;   //�˵��˳�����
	}else if(uckey_value == KEY_MAIN_LONG_PRESS){ //���� ����
		uckey_value = KEY_NONE;
		menu_exit_count = 0;   //�˵��˳�����
	}
	menu_exit_count++;   //�˵��˳�����
	if(menu_exit_count > (30*1000/10)){//30S�޲����Զ��˳�
		em_work_mode = WORK_MODE_NORMAL; //����ģʽ

		//disp_clear_screen(BLACK);
	    disp_rectangle(39,0,1,80,LGRAY);//
	    disp_rectangle(120,0,1,80,LGRAY);
		disp_rectangle(40,0,80,80,DCYAN);
		if(em_FM_switch == SWITCH_OPEN){//FM�㲥����
			 disp_image_3232(128,45,gImage_FM,GRAYBLUE,BLACK);
			 if(uc_fm_channel>99){//��λ��
			 	disp_buff[0] = (uc_fm_channel/100)+0x30;
			 	disp_buff[1] = (uc_fm_channel/10%10)+0x30;
			 	disp_buff[2] = (uc_fm_channel%10)+0x30;
			 	disp_buff[3] = 0;
			 	display_gb1212(134,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
			 }else{ //��λ��
			 	disp_buff[0] = (uc_fm_channel/10%10)+0x30;
			 	disp_buff[1] = (uc_fm_channel%10)+0x30;
			 	disp_buff[2] = 0;
			 	display_gb1212(141,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
			 }
		}else{
			 	disp_image_3232(128,45,gImage_FM,GRAY,BLACK);
	    }
	    if(em_BL_switch == SWITCH_OPEN){//��������
			 if(em_bl_state == BL_STATUS_LINK_OK){ //���ӳɹ�
			 	disp_image_3232(2,45,gImage_BL_ON,GRAYBLUE,BLACK);
			 }else{
			 	disp_image_3232(2,45,gImage_BL_OFF,GRAYBLUE,BLACK);
			 }
		}else{
			 disp_image_3232(2,45,gImage_BL_OFF,GRAY,BLACK);
		}
		if(em_play_switch == SWITCH_OPEN){  //���ǲ��ſ���
			 disp_image_3232(2,5,gImage_audio,GRAYBLUE,BLACK);
		}else{
			 disp_image_3232(2,5,gImage_audio,GRAY,BLACK);
		}
	    if(em_audio_mode == AUDIO_DECODING){  //��Ƶ����ģʽ ��Ƶ����
			 disp_image_3232(128,5,gImage_headset,GRAYBLUE,BLACK);
			 display_gb1616(64,2,"ģʽ",BLUE,DCYAN);//LGRAY
			 disp_image_8040(40,20,gImage_music,MAGENTA,DCYAN);//LIGHTGREEN
			 display_gb1212(56,67,"���ǽ���",GREEN,DCYAN);//RED
	    }else{
			 disp_image_3232(128,5,gImage_headset,GRAY,BLACK);
			 display_gb1616(64,2,"ģʽ",BLUE,DCYAN);//LGRAY
			 disp_image_8040(40,20,gImage_35mm,YELLOW,DCYAN);//LIGHTGREEN
			 display_gb1212(56,67,"��Ƶ����",GREEN,DCYAN);//RED
		}
	}
}
/*******************************************
 * ��������work_mode_fm_set
 * ����  ��  FM����
 * ����  ��  ��
 * ���  ��  ��
 * ����  ��  V0.00  2019-5-1
********************************************/
static void work_mode_fm_set(void)
{
	uint8_t disp_buff[5];
	//----------��������---------------
	if(uckey_value == KEY_FM_SHORT_PRESS){  //FM�� �̰� +
	   uckey_value = KEY_NONE;
	   menu_exit_count = 0;   //�˵��˳�����
	   uc_fm_channel_old++;
	   if(uc_fm_channel_old > 108){
		   uc_fm_channel_old = 76;
	   }
	   if(uc_fm_channel_old>99){//��λ��
	  	   disp_buff[0] = (uc_fm_channel_old/100)+0x30;
	  	   disp_buff[1] = (uc_fm_channel_old/10%10)+0x30;
	  	   disp_buff[2] = (uc_fm_channel_old%10)+0x30;
	  	   disp_buff[3] = 0;
	  }else{ //��λ��
	  	   disp_buff[0] = ' ';
	  	   disp_buff[1] = (uc_fm_channel_old/10%10)+0x30;
	  	   disp_buff[2] = (uc_fm_channel_old%10)+0x30;
	  	   disp_buff[3] = 0;
	  }
	  display_gb1616(42,30,(const char*)disp_buff,YELLOW,BLACK);//������ʾ

	}else if(uckey_value == KEY_BL_SHORT_PRESS){  //������ �̰� -
	   uckey_value = KEY_NONE;
	   menu_exit_count = 0;   //�˵��˳�����
	   uc_fm_channel_old--;
	   if(uc_fm_channel_old < 76){
	   	   uc_fm_channel_old = 108;
	   }
	   if(uc_fm_channel_old>99){//��λ��
	  	   disp_buff[0] = (uc_fm_channel_old/100)+0x30;
	  	   disp_buff[1] = (uc_fm_channel_old/10%10)+0x30;
	  	   disp_buff[2] = (uc_fm_channel_old%10)+0x30;
	  	   disp_buff[3] = 0;
	  }else{ //��λ��
	  	   disp_buff[0] = ' ';
	  	   disp_buff[1] = (uc_fm_channel_old/10%10)+0x30;
	  	   disp_buff[2] = (uc_fm_channel_old%10)+0x30;
	  	   disp_buff[3] = 0;
	  }
	  display_gb1616(42,30,(const char*)disp_buff,YELLOW,BLACK);//������ʾ

	}else if(uckey_value == KEY_MAIN_SHORT_PRESS){  //���� �̰�
	   uckey_value = KEY_NONE;
	   menu_exit_count = 0;   //�˵��˳�����
	   em_work_mode = WORK_MODE_NORMAL; //����ģʽ
	   if(uc_fm_channel != uc_fm_channel_old){
	      uc_fm_channel = uc_fm_channel_old;
	      falsh_write_flag = 0x5A; //flash����д��־
	      qn8027_fm_channel_set_function();//Ƶ������
	   }
	   //disp_clear_screen(BLACK);
	   disp_rectangle(39,0,1,80,LGRAY);//
	   disp_rectangle(120,0,1,80,LGRAY);
	   disp_rectangle(40,0,80,80,DCYAN);
	   if(em_FM_switch == SWITCH_OPEN){//FM�㲥����
	   		disp_image_3232(128,45,gImage_FM,GRAYBLUE,BLACK);
	   		if(uc_fm_channel>99){//��λ��
	   			disp_buff[0] = (uc_fm_channel/100)+0x30;
	   			disp_buff[1] = (uc_fm_channel/10%10)+0x30;
	   			disp_buff[2] = (uc_fm_channel%10)+0x30;
	   			disp_buff[3] = 0;
	   			display_gb1212(134,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
	   		}else{ //��λ��
	   			disp_buff[0] = (uc_fm_channel/10%10)+0x30;
	   			disp_buff[1] = (uc_fm_channel%10)+0x30;
	   			disp_buff[2] = 0;
	   			display_gb1212(141,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
	   	   }
	   }else{
	   		 disp_image_3232(128,45,gImage_FM,GRAY,BLACK);
	   }
	   if(em_BL_switch == SWITCH_OPEN){//��������
		   if(em_bl_state == BL_STATUS_LINK_OK){ //���ӳɹ�
		   	   disp_image_3232(2,45,gImage_BL_ON,GRAYBLUE,BLACK);
		   }else{
		   	   disp_image_3232(2,45,gImage_BL_OFF,GRAYBLUE,BLACK);
		   }
	   }else{
	   		 disp_image_3232(2,45,gImage_BL_OFF,GRAY,BLACK);
	   }
	   if(em_play_switch == SWITCH_OPEN){  //���ǲ��ſ���
	   		 disp_image_3232(2,5,gImage_audio,GRAYBLUE,BLACK);
	   }else{
	   		 disp_image_3232(2,5,gImage_audio,GRAY,BLACK);
	   }
	   if(em_audio_mode == AUDIO_DECODING){  //��Ƶ����ģʽ ��Ƶ����
	   		 disp_image_3232(128,5,gImage_headset,GRAYBLUE,BLACK);
	   		 display_gb1616(64,2,"ģʽ",BLUE,DCYAN);//LGRAY
	   		 disp_image_8040(40,20,gImage_music,MAGENTA,DCYAN);//LIGHTGREEN
	   		 display_gb1212(56,67,"���ǽ���",GREEN,DCYAN);//RED
	   }else{
	   		 disp_image_3232(128,5,gImage_headset,GRAY,BLACK);
	   		 display_gb1616(64,2,"ģʽ",BLUE,DCYAN);//LGRAY
	   		 disp_image_8040(40,20,gImage_35mm,YELLOW,DCYAN);//LIGHTGREEN
	   		 display_gb1212(56,67,"��Ƶ����",GREEN,DCYAN);//RED
	   }


	}else if(uckey_value == KEY_BL_LONG_PRESS){   //������ ����
		  uckey_value = KEY_NONE;
		  menu_exit_count = 0;   //�˵��˳�����
	}else if(uckey_value == KEY_FM_LONG_PRESS){   //FM�� ����
		  uckey_value = KEY_NONE;
		  menu_exit_count = 0;   //�˵��˳�����
	}else if(uckey_value == KEY_MAIN_LONG_PRESS){ //���� ����
		  uckey_value = KEY_NONE;
		  menu_exit_count = 0;   //�˵��˳�����
	}
	menu_exit_count++;   //�˵��˳�����
	if(menu_exit_count > (30*1000/10)){//30S�޲����Զ��˳�
		em_work_mode = WORK_MODE_NORMAL; //����ģʽ
		//disp_clear_screen(BLACK);
	    disp_rectangle(39,0,1,80,LGRAY);//
	    disp_rectangle(120,0,1,80,LGRAY);
		disp_rectangle(40,0,80,80,DCYAN);
		if(em_FM_switch == SWITCH_OPEN){//FM�㲥����
			 disp_image_3232(128,45,gImage_FM,GRAYBLUE,BLACK);
			 if(uc_fm_channel>99){//��λ��
			 	disp_buff[0] = (uc_fm_channel/100)+0x30;
			 	disp_buff[1] = (uc_fm_channel/10%10)+0x30;
			 	disp_buff[2] = (uc_fm_channel%10)+0x30;
			 	disp_buff[3] = 0;
			 	display_gb1212(134,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
			 }else{ //��λ��
			 	disp_buff[0] = (uc_fm_channel/10%10)+0x30;
			 	disp_buff[1] = (uc_fm_channel%10)+0x30;
			 	disp_buff[2] = 0;
			 	display_gb1212(141,58,(const char*)disp_buff,BLUE,BLACK);//������ʾ
			 }
		}else{
			 	disp_image_3232(128,45,gImage_FM,GRAY,BLACK);
	    }
	    if(em_BL_switch == SWITCH_OPEN){//��������
			 if(em_bl_state == BL_STATUS_LINK_OK){ //���ӳɹ�
			 	disp_image_3232(2,45,gImage_BL_ON,GRAYBLUE,BLACK);
			 }else{
			 	disp_image_3232(2,45,gImage_BL_OFF,GRAYBLUE,BLACK);
			 }
		}else{
			 disp_image_3232(2,45,gImage_BL_OFF,GRAY,BLACK);
		}
		if(em_play_switch == SWITCH_OPEN){  //���ǲ��ſ���
			 disp_image_3232(2,5,gImage_audio,GRAYBLUE,BLACK);
		}else{
			 disp_image_3232(2,5,gImage_audio,GRAY,BLACK);
		}
	    if(em_audio_mode == AUDIO_DECODING){  //��Ƶ����ģʽ ��Ƶ����
			 disp_image_3232(128,5,gImage_headset,GRAYBLUE,BLACK);
			 display_gb1616(64,2,"ģʽ",BLUE,DCYAN);//LGRAY
			 disp_image_8040(40,20,gImage_music,MAGENTA,DCYAN);//LIGHTGREEN
			 display_gb1212(56,67,"���ǽ���",GREEN,DCYAN);//RED
	    }else{
			 disp_image_3232(128,5,gImage_headset,GRAY,BLACK);
			 display_gb1616(64,2,"ģʽ",BLUE,DCYAN);//LGRAY
			 disp_image_8040(40,20,gImage_35mm,YELLOW,DCYAN);//LIGHTGREEN
			 display_gb1212(56,67,"��Ƶ����",GREEN,DCYAN);//RED
		}
	}

}
/*******************************************
 * ��������work_mode_normal
 * ����  ��  ����ģʽ
 * ����  ��  ��
 * ���  ��  ��
 * ����  ��  V0.00  2019-4-13
********************************************/
//static void work_mode_select(void)
//{
//
//
//}

/*******************************************
 * ��������task_control_power_on_function
 * ����  ��  ������  �ϵ��ʼ��
 * ����  ����
 * ���  ����
 * ����  ��V0.00  2019-4-13
 ********************************************/
void task_control_power_on_function(void)
{
	flash_read_handle();
	first_power_on_flag = 1;//�����ϵ��־λ
	uc_lcd_init_state = 0;//����Һ����ʼ���ж�
	uc_lcd_init_count = 0;//����Һ����ʼ������
	em_work_mode = WORK_MODE_SCREEN_INIT;//Һ����ʼ��
	LCD_POWER_ON;
	bl_data_handle_init();
	qn8027_init_function();
}

/*******************************************
 * ��������task_control_main_function
 * ����  ��  ��������
 * ����  ��  ��
 * ���  ��  ��
 * ����  ��  V0.00  2019-4-13
********************************************/
void task_control_main_function(void)
{
	bl_control_function();

	switch(em_work_mode){
	    case WORK_MODE_SCREEN_INIT:    //Һ����ʼ��
	    	work_mode_screen_init();
		    break;
	   	case WORK_MODE_POWER_ON:        //����logo
	   		work_mode_power_on();
	   	    break;
	   	case WORK_MODE_SCREEN_CLOSE:   //Ϣ��
	   		work_mode_screen_close();
	   	   	break;
	   	case WORK_MODE_NORMAL:   	   //����ģʽ
	   		work_mode_normal();
	   		break;
	   	case WORK_MODE_BL:   	       //��������
	   		work_mode_bl_set();
	   		break;
	   	case WORK_MODE_FM:   	       //FM����
	   		work_mode_fm_set();
	   		break;
	   //	case WORK_MODE_SELECT:         //ģʽѡ��
	   //		work_mode_select();
	   //		break;
	   	   default:
	   	       break;
	}

}
