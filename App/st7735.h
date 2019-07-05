/*
 * st7735.h
 *
 *  Created on: 2019��4��13��
 *      Author: gongjunhui
 */

#ifndef ST7735_H_
#define ST7735_H_

#include "main.h"

#define USE_HORIZONTAL 2  //���ú�������������ʾ 0��1Ϊ���� 2��3Ϊ����

#if  USE_HORIZONTAL==0
#define LCD_W 80
#define LCD_H 160
#define HORIZONTAL_DATA  0x08

#elif USE_HORIZONTAL==1
#define LCD_W 80
#define LCD_H 160
#define HORIZONTAL_DATA  0xC8

#elif USE_HORIZONTAL==2
#define LCD_W 160
#define LCD_H 80
#define HORIZONTAL_DATA  0x78

#else
#define LCD_W 160
#define LCD_H 80
#define HORIZONTAL_DATA  0xA8

#endif



//-----------------OLED�˿ڶ���----------------
#define OLED_SCLK_Clr() LCD_SCL_CLR//CLK
#define OLED_SCLK_Set() LCD_SCL_SET
#define OLED_SDIN_Clr() LCD_SDA_CLR//DIN
#define OLED_SDIN_Set() LCD_SDA_SET
#define OLED_RST_Clr()  LCD_RESET_CLR//RES
#define OLED_RST_Set()  LCD_RESET_SET
#define OLED_DC_Clr()   LCD_RS_CLR//DC
#define OLED_DC_Set()   LCD_RS_SET
#define OLED_CS_Clr()   LCD_CS_CLR//CS
#define OLED_CS_Set()   LCD_CS_SET


//������ɫ
#define   BLACK                0x0000                // ��ɫ��    0,   0,   0 //
#define   BLUE                 0x001F                // ��ɫ��    0,   0, 255 //
#define   GREEN                0x07E0                // ��ɫ��    0, 255,   0 //
#define   CYAN                 0x07FF                // ��ɫ��    0, 255, 255 //
#define   RED                  0xF800                // ��ɫ��  255,   0,   0 //
#define   MAGENTA              0xF81F                // Ʒ�죺  255,   0, 255 //
#define   YELLOW               0xFFE0                // ��ɫ��  255, 255, 0   //
#define   WHITE                0xFFFF                // ��ɫ��  255, 255, 255 //
#define   NAVY                 0x000F                // ����ɫ��  0,   0, 128 //
#define   DGREEN               0x03E0                // ����ɫ��  0, 128,   0 //
#define   DCYAN                0x03EF                // ����ɫ��  0, 128, 128 //
#define   MAROON               0x7800                // ���ɫ��128,   0,   0 //
#define   PURPLE               0x780F                // ��ɫ��  128,   0, 128 //
#define   OLIVE                0x7BE0                // ����̣�128, 128,   0 //
#define   LGRAY                0xC618                // �Ұ�ɫ��192, 192, 192 //
#define   DGRAY                0x7BEF                // ���ɫ��128, 128, 128 //
#define   BROWN 			   0XBC40                // ��ɫ
#define   BRRED 			   0XFC07                // �غ�ɫ
#define   GRAY  			   0X8430                //��ɫ
//GUI��ɫ
#define   DARKBLUE      	  0X01CF	    //����ɫ
#define   LIGHTBLUE      	  0X7D7C	    //ǳ��ɫ
#define   GRAYBLUE       	  0X5458        //����ɫ
//������ɫΪPANEL����ɫ
#define   LIGHTGREEN     	  0X841F        //ǳ��ɫ
#define   LGRAYBLUE           0XA651        //ǳ����ɫ(�м����ɫ)
#define   LBBLUE              0X2B12        //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)













extern void lcd_init(void); //LCD��ʼ������
extern void disp_clear_screen(uint16_t color);//LCD��������
//--------24*24�ֿ���ʾ-------------
//extern void display_gb2424(uint8_t x0, uint8_t y0,const char *s, uint16_t fColor);
//--------16*16�ֿ���ʾ-------------
extern void display_gb1616(uint8_t x0, uint8_t y0,const char *s, uint16_t fColor,uint16_t bColor);
//--------12*12�ֿ���ʾ-------------
extern void display_gb1212(uint8_t x0, uint8_t y0,const char *s, uint16_t fColor,uint16_t bColor);

//extern void disp_QRcode(uint16_t color,uint16_t Bcolor);//��ά����ʾ����
extern void disp_image_3232(uint8_t x,uint8_t y,const uint8_t* buff, uint16_t color,uint16_t Bcolor);
extern void disp_image_8040(uint8_t x,uint8_t y,const uint8_t* buff, uint16_t color,uint16_t Bcolor);
extern void disp_image_10080(uint8_t x,uint8_t y,const uint8_t* buff, uint16_t color,uint16_t Bcolor);
extern void disp_rectangle(uint8_t x,uint8_t y,uint8_t x_len,uint8_t y_len, uint16_t color);

#endif /* ST7735_H_ */


