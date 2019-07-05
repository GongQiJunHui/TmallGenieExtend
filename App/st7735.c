/*
 * st7735.c
 *
 *  Created on: 2019��4��13��
 *      Author: gongjunhui
 */

#include "st7735.h"
//#include "GB2424.h"	//24*24������ģ
#include "GB1616.h"	//16*16������ģ
#include "GB1212.h"	//12*12������ģ
#include "image.h"	//ͼƬ����

/******************************************************************************
      ����˵����LCD��������д�뺯��
      ������ݣ�dat  Ҫд��Ĵ�������
      ����ֵ��  ��
******************************************************************************/
static void LCD_Writ_Bus(uint8_t dat)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		OLED_SCLK_Clr();
		if(dat&0x80)
		   OLED_SDIN_Set();
		else
		   OLED_SDIN_Clr();
		OLED_SCLK_Set();
		dat<<=1;
	}
}
/******************************************************************************
      ����˵����LCDд������
      ������ݣ�dat д�������
      ����ֵ��  ��
******************************************************************************/
static void LCD_WR_REG(uint8_t dat)
{
	OLED_DC_Clr();//д����
	LCD_Writ_Bus(dat);
}
/******************************************************************************
      ����˵����LCDд������
      ������ݣ�dat д�������
      ����ֵ��  ��
******************************************************************************/
static void LCD_WR_DATA8(uint8_t dat)
{
	OLED_DC_Set();//д����
	LCD_Writ_Bus(dat);
}
/******************************************************************************
      ����˵����LCDд������
      ������ݣ�dat д�������
      ����ֵ��  ��
******************************************************************************/
static void LCD_WR_DATA(uint16_t dat)
{
	OLED_DC_Set();//д����
	LCD_Writ_Bus(dat>>8);
	LCD_Writ_Bus(dat);
}
/******************************************************************************
      ����˵����LCDд������
      ������ݣ�dat д�������
      ����ֵ��  ��
******************************************************************************/
static void LCD_WR_DATA_FAST(uint8_t dat)
{
	uint8_t i;
	OLED_DC_Set();//д����
	OLED_SDIN_Clr(); //��8λȫд0
	for(i=0;i<8;i++){
		OLED_SCLK_Clr();
		OLED_SCLK_Set();
	}

	for(i=0;i<8;i++)
	{
		OLED_SCLK_Clr();
		if(dat&0x80)
			OLED_SDIN_Set();
		else
			OLED_SDIN_Clr();
		OLED_SCLK_Set();
		dat<<=1;
	}
}
/******************************************************************************
      ����˵����������ʼ�ͽ�����ַ
      ������ݣ�Xstart,Xend �����е���ʼ�ͽ�����ַ
         Ystart,Yend �����е���ʼ�ͽ�����ַ
      ����ֵ��  ��
******************************************************************************/
static void LCD_Address_Set(uint8_t Xstart,uint8_t Ystart,uint8_t Xend,uint8_t Yend)
{
#if  USE_HORIZONTAL==0
	LCD_WR_REG(0x2a);//�е�ַ����
	LCD_WR_DATA(Xstart+26);
	LCD_WR_DATA(Xend+26);
	LCD_WR_REG(0x2b);//�е�ַ����
	LCD_WR_DATA(Ystart+1);
	LCD_WR_DATA(Yend+1);
	LCD_WR_REG(0x2c);//������д

#elif USE_HORIZONTAL==1
	LCD_WR_REG(0x2a);//�е�ַ����
	LCD_WR_DATA(Xstart+26);
	LCD_WR_DATA(Xend+26);
	LCD_WR_REG(0x2b);//�е�ַ����
	LCD_WR_DATA(Ystart+1);
	LCD_WR_DATA(Yend+1);
	LCD_WR_REG(0x2c);//������д

#elif USE_HORIZONTAL==2
	LCD_WR_REG(0x2a);//�е�ַ����
	LCD_WR_DATA(Xstart+1);
	LCD_WR_DATA(Xend+1);
	LCD_WR_REG(0x2b);//�е�ַ����
	LCD_WR_DATA(Ystart+26);
	LCD_WR_DATA(Yend+26);
	LCD_WR_REG(0x2c);//������д

#else
	LCD_WR_REG(0x2a);//�е�ַ����
	LCD_WR_DATA(Xstart+1);
	LCD_WR_DATA(Xend+1);
	LCD_WR_REG(0x2b);//�е�ַ����
	LCD_WR_DATA(y1+26);
	LCD_WR_DATA(Yend+26);
	LCD_WR_REG(0x2c);//������д

#endif
}

/**********************************************
��������Lcd��ѡ����
���ܣ�ѡ��Lcd��ָ���ľ�������
ע�⣺xStart�� yStart������Ļ����ת���ı䣬λ���Ǿ��ο���ĸ���
��ڲ�����        xStart x�������ʼ��
          ySrart y�������ֹ��
          xLong Ҫѡ�����ε�x���򳤶�
          yLong  Ҫѡ�����ε�y���򳤶�
����ֵ����
***********************************************/
void lcd_color_box(uint8_t xStart,uint8_t yStart,uint8_t xLong,uint8_t yLong,uint16_t color)
{
	uint8_t i,j;
	OLED_CS_Clr(); //Ƭѡ��Ч
	LCD_Address_Set(xStart,yStart,xStart+xLong,yStart+yLong);
	OLED_DC_Set(); //д����
	for(i=0;i<xLong+1;i++)
	{
		for(j=0;j<yLong+1;j++)
		{
			LCD_WR_DATA(color);
		}
	}
	OLED_CS_Set();    //Ƭѡ��Ч
}



/**********************************************
Lcd��㺯��
***********************************************/
static void DrawPixel(uint8_t x, uint8_t y, uint16_t color)
{
	//OLED_CS_Clr(); //Ƭѡ��Ч
	LCD_WR_REG(0x2a);//�е�ַ����
	LCD_WR_DATA_FAST(x+1);
	LCD_WR_REG(0x2b);//�е�ַ����
	LCD_WR_DATA_FAST(y+26);
	LCD_WR_REG(0x2c);//������д
	LCD_WR_DATA(color);
	//OLED_CS_Set();    //Ƭѡ��Ч
}

/******************************************************
* ��������Display_GB1212
* ����  ����ʾ12*12�ĺ���
* ����  : λ�á����ݡ�������ɫ
* ���  ����
* ˵��  : ��Ҫ��GB1212.h�����Ҫ��ʾ������ �������ⲿflash�ֿ�
* ����  ��V0.00   2018-05-27
*********************************************************/
void display_gb1212(uint8_t x0, uint8_t y0,const char *s, uint16_t fColor,uint16_t bColor)
{
    unsigned int font_data=0;
    unsigned short Num,h=0,l=0,i=0;
    while(*s != '\0')//����ַ���δ������ ����ѭ��
    {
    	OLED_CS_Clr(); //Ƭѡ��Ч
        if( *s >=0xa1){  //GB2312���뺺�ֵ� ��ʾ
          //--------�Ա��Ƿ����ֿ���--------------------------
           for(Num = 0; Num < len_code12x12; Num++)
           {
             //�ж��Ƿ��Ƿ�Ϊ�����ֽ�
             if((GB2312Code12x12[Num].Head[0] == *s) && (GB2312Code12x12[Num].Head[1] == *(s+1))){

                 for(h=0;h<12;h++){ //����
                     //-----------��¼����---------------------------
                     font_data = (GB2312Code12x12[Num].Infor[2*h]<<8)|(GB2312Code12x12[Num].Infor[2*h+1]);
                     //���
                     for(i=0;i<12;i++){
                         if(font_data&0x8000){
                             DrawPixel(x0+i+l,y0+h,fColor);
                         }else{
                        	 DrawPixel(x0+i+l,y0+h,bColor);
                         }
                         font_data<<=1;
                     }
                 }
                 break;
             }
           }
           l+=12;//ת�Ƶ���һ����ʾλ��
           s+=2;//ת�Ƶ���һ���ַ�
        }else{             //ASCII�ı�ʾ
        //--------�Ա��Ƿ����ֿ���--------------------------
           for(Num = 0; Num < len_code6x12; Num++)
           {
             //�ж��Ƿ��Ƿ�Ϊ�����ֽ�
             if(GB2312Code6x12[Num].Head[0] == *s){

                 for(h=0;h<12;h++){ //����
                     //-----------��¼����---------------------------
                     font_data = GB2312Code6x12[Num].Infor[h];
                     //���
                     for(i=0;i<8;i++){
                         if(font_data&0x80){
                             DrawPixel(x0+i+l,y0+h,fColor);
                         }//else{
                        //	 DrawPixel(x0+i+l,y0+h,bColor);
                         //}
                         font_data<<=1;
                     }
                 }
                 break;
             }
           }
           l+=7;//ת�Ƶ���һ����ʾλ��
           s+=1;//ת�Ƶ���һ���ַ�
        }
    }
    OLED_CS_Set();    //Ƭѡ��Ч
}

/******************************************************
* ��������Display_GB1616
* ����  ����ʾ16*16�ĺ��� �� 8*16��ASCII
* ����  : λ�á����ݡ�������ɫ
* ���  ����
* ˵��  : ��Ҫ��GB1616.h�����Ҫ��ʾ������ �������ⲿflash�ֿ�
* ����  ��V0.02   2017-05-24
*********************************************************/
void display_gb1616(uint8_t x0, uint8_t y0,const char *s, uint16_t fColor,uint16_t bColor)
{
    unsigned int font_data=0;
    unsigned short Num,h=0,l=0,i=0;
    while(*s != '\0')//����ַ���δ������ ����ѭ��
    {
    	OLED_CS_Clr(); //Ƭѡ��Ч
        if( *s >=0xa1){  //GB2312���뺺�ֵ� ��ʾ
          //--------�Ա��Ƿ����ֿ���--------------------------
           for(Num = 0; Num < len_code16x16; Num++)
           {
             //�ж��Ƿ��Ƿ�Ϊ�����ֽ�
             if((GB2312Code16x16[Num].Head[0] == *s) && (GB2312Code16x16[Num].Head[1] == *(s+1))){

                 for(h=0;h<16;h++){ //����
                     //-----------��¼����---------------------------
                     font_data = (GB2312Code16x16[Num].Infor[2*h]<<8)|(GB2312Code16x16[Num].Infor[2*h+1]);
                     //���
                     for(i=0;i<16;i++){
                         if(font_data&0x8000){
                                 DrawPixel(x0+i+l,y0+h,fColor);
                         }else{
                                 DrawPixel(x0+i+l,y0+h,bColor);
                         }
                         font_data<<=1;
                     }
                 }
                 break;
             }
           }
           l+=17;//ת�Ƶ���һ����ʾλ��
           s+=2;//ת�Ƶ���һ���ַ�
        }else{             //ASCII�ı�ʾ
          //--------�Ա��Ƿ����ֿ���--------------------------
           for(Num = 0; Num < len_code8x16; Num++)
           {
             //�ж��Ƿ��Ƿ�Ϊ�����ֽ�
             if(GB2312Code8x16[Num].Head[0] == *s){

                 for(h=0;h<16;h++){ //����
                     //-----------��¼����---------------------------
                     font_data = GB2312Code8x16[Num].Infor[h];
                     //���
                     for(i=0;i<8;i++){
                         if(font_data&0x80){
                                 DrawPixel(x0+i+l,y0+h,fColor);
                         }else{
                                 DrawPixel(x0+i+l,y0+h,bColor);
                         }
                         font_data<<=1;
                     }
                 }
                 break;
             }
           }
           l+=9;//ת�Ƶ���һ����ʾλ��
           s+=1;//ת�Ƶ���һ���ַ�
        }
    }
    OLED_CS_Set();    //Ƭѡ��Ч
}
///******************************************************
//* ��������Display_GB2424
//* ����  ����ʾ24*24�ĺ��� �� 12*24��ASCII
//* ����  : λ�á����ݡ�������ɫ
//* ���  ����
//* ˵��  : ��Ҫ��GB2424.h�����Ҫ��ʾ������ �������ⲿflash�ֿ�
//* ����  ��V0.02   2019-04-30
//*********************************************************/
//void display_gb2424(uint8_t x0, uint8_t y0,const char *s, uint16_t fColor)
//{
//    unsigned int font_data=0;
//    unsigned short Num,h=0,l=0,i=0;
//    while(*s != '\0')//����ַ���δ������ ����ѭ��
//    {
//    	OLED_CS_Clr(); //Ƭѡ��Ч
//        if( *s >=0xa1){  //GB2312���뺺�ֵ� ��ʾ
//          //--------�Ա��Ƿ����ֿ���--------------------------
//           for(Num = 0; Num < len_code24x24; Num++)
//           {
//             //�ж��Ƿ��Ƿ�Ϊ�����ֽ�
//             if((GB2312Code24x24[Num].Head[0] == *s) && (GB2312Code24x24[Num].Head[1] == *(s+1))){
//
//                 for(h=0;h<24;h++){ //����
//                     //-----------��¼����---------------------------
//                     font_data = (GB2312Code24x24[Num].Infor[3*h]<<16)|(GB2312Code24x24[Num].Infor[3*h+1]<<8)|(GB2312Code24x24[Num].Infor[3*h+2]);
//                     //���
//                     for(i=0;i<24;i++){
//                         if(font_data&0x800000){
//                                 DrawPixel(x0+i+l,y0+h,fColor);
//                         }//else{
//                          //       DrawPixel(x0+i+l,y0+h,bColor);
//                          //}
//                         font_data<<=1;
//                     }
//                 }
//                 break;
//             }
//           }
//           l+=25;//ת�Ƶ���һ����ʾλ��
//           s+=2;//ת�Ƶ���һ���ַ�
//        }else{             //ASCII�ı�ʾ
//          //--------�Ա��Ƿ����ֿ���--------------------------
//           for(Num = 0; Num < len_code12x24; Num++)
//           {
//             //�ж��Ƿ��Ƿ�Ϊ�����ֽ�
//             if(GB2312Code12x24[Num].Head[0] == *s){
//
//                 for(h=0;h<24;h++){ //����
//                     //-----------��¼����---------------------------
//                     font_data = (GB2312Code12x24[Num].Infor[h*2]<<8)|(GB2312Code12x24[Num].Infor[h*2+1]);
//                     //���
//                     for(i=0;i<16;i++){
//                         if(font_data&0x8000){
//                                 DrawPixel(x0+i+l,y0+h,fColor);
//                         }//else{
//                              //   DrawPixel(x0+i+l,y0+h,bColor);
//                         //}
//                         font_data<<=1;
//                     }
//                 }
//                 break;
//             }
//           }
//           if((*s=='s')||(*s=='r')||(*s=='.')){
//        	   l+=8;//ת�Ƶ���һ����ʾλ��
//           }else{
//        	   l+=12;//ת�Ƶ���һ����ʾλ��
//           }
//
//           s+=1;//ת�Ƶ���һ���ַ�
//        }
//    }
//    OLED_CS_Set();    //Ƭѡ��Ч
//}
/******************************************************************************
      ����˵����LCD��������
      ������ݣ���
      ����ֵ��  ��
******************************************************************************/
void disp_clear_screen(uint16_t color)
{
	uint8_t i,j;
	OLED_CS_Clr(); //Ƭѡ��Ч
	LCD_Address_Set(0,0,LCD_W-1,LCD_H-1);
	OLED_DC_Set(); //д����
	for(i=0;i<LCD_W;i++){
		for(j=0;j<LCD_H;j++){
			LCD_Writ_Bus(color>>8);
			LCD_Writ_Bus(color);;
		}
	}
	OLED_CS_Set();    //Ƭѡ��Ч
}

/******************************************************************************
      ����˵������ʾ��ά��
      ������ݣ���
      ����ֵ��  ��
******************************************************************************/
//void disp_QRcode(uint16_t color,uint16_t Bcolor)
//{
//	uint8_t i,j,h;
//	uint8_t font_data;
//	OLED_CS_Clr(); //Ƭѡ��Ч
//	LCD_Address_Set(40,0,120-1,80-1);
//	OLED_DC_Set(); //д����
//	for(h=0;h<80;h++){ //����
//		for(j=0;j<10;j++){
//			  font_data = gImage_drcode[h*10+j];
//			  for(i=0;i<8;i++){
//			    if(font_data&0x80){
//			    	LCD_Writ_Bus(color>>8);
//			    	LCD_Writ_Bus(color);
//			    }else{
//			    	LCD_Writ_Bus(Bcolor>>8);
//			    	LCD_Writ_Bus(Bcolor);
//			    }
//			    font_data<<=1;
//			 }
//	   }
//	}
//	OLED_CS_Set();    //Ƭѡ��Ч
//}

/******************************************************************************
      ����˵������ʾͼ��
      ������ݣ���
      ����ֵ��  ��
******************************************************************************/
void disp_image_3232(uint8_t x,uint8_t y,const uint8_t* buff, uint16_t color,uint16_t Bcolor)
{
	uint8_t i,j,h;
	uint8_t font_data;
	OLED_CS_Clr(); //Ƭѡ��Ч
	LCD_Address_Set(x,y,x+32-1,y+32-1);
	OLED_DC_Set(); //д����
	for(h=0;h<32;h++){ //����
		for(j=0;j<4;j++){
			  font_data = buff[h*4+j];
			  for(i=0;i<8;i++){
			    if(font_data&0x80){
			    	LCD_Writ_Bus(color>>8);
			    	LCD_Writ_Bus(color);
			    }else{
			    	LCD_Writ_Bus(Bcolor>>8);
			    	LCD_Writ_Bus(Bcolor);
			    }
			    font_data<<=1;
			 }
	   }
	}
	OLED_CS_Set();    //Ƭѡ��Ч
}

/******************************************************************************
      ����˵������ʾͼ��
      ������ݣ���
      ����ֵ��  ��
******************************************************************************/
void disp_image_8040(uint8_t x,uint8_t y,const uint8_t* buff, uint16_t color,uint16_t Bcolor)
{
	uint8_t i,j,h;
	uint8_t font_data;
	OLED_CS_Clr(); //Ƭѡ��Ч
	LCD_Address_Set(x,y,x+80-1,y+40-1);
	OLED_DC_Set(); //д����
	for(h=0;h<40;h++){ //����
		for(j=0;j<10;j++){
			font_data = buff[h*10+j];
			for(i=0;i<8;i++){
			   if(font_data&0x80){
				   LCD_Writ_Bus(color>>8);
				   LCD_Writ_Bus(color);
		       }else{
				   LCD_Writ_Bus(Bcolor>>8);
				   LCD_Writ_Bus(Bcolor);
			   }
			   font_data<<=1;
	       }
	    }
	}
	OLED_CS_Set();    //Ƭѡ��Ч
}
/******************************************************************************
      ����˵������ʾͼ��
      ������ݣ���
      ����ֵ��  ��
******************************************************************************/
void disp_image_10080(uint8_t x,uint8_t y,const uint8_t* buff, uint16_t color,uint16_t Bcolor)
{
	uint8_t i,j,h;
	uint8_t font_data;
	OLED_CS_Clr(); //Ƭѡ��Ч
	LCD_Address_Set(x,y,x+96-1,y+80-1);
	OLED_DC_Set(); //д����
	for(h=0;h<80;h++){ //����
		for(j=0;j<12;j++){
			font_data = buff[h*12+j];
			for(i=0;i<8;i++){
			   if(font_data&0x80){
				   LCD_Writ_Bus(color>>8);
				   LCD_Writ_Bus(color);
		       }else{
				   LCD_Writ_Bus(Bcolor>>8);
				   LCD_Writ_Bus(Bcolor);
			   }
			   font_data<<=1;
	       }
	    }
	}
	OLED_CS_Set();    //Ƭѡ��Ч
}
/******************************************************************************
      ����˵������ʾ����
      ������ݣ���
      ����ֵ��  ��
******************************************************************************/
void disp_rectangle(uint8_t x,uint8_t y,uint8_t x_len,uint8_t y_len, uint16_t color)
{
	uint8_t j,h;
	OLED_CS_Clr(); //Ƭѡ��Ч
	LCD_Address_Set(x,y,x+x_len-1,y+y_len-1);
	OLED_DC_Set(); //д����
	for(h=0;h<y_len;h++){ //����
		for(j=0;j<x_len;j++){
			LCD_Writ_Bus(color>>8);
			LCD_Writ_Bus(color);
	    }
	}
	OLED_CS_Set();    //Ƭѡ��Ч
}
/******************************************************************************
      ����˵����LCD��ʼ������
      ������ݣ���
      ����ֵ��  ��
******************************************************************************/
void lcd_init(void)
{
    //************* Start Initial Sequence **********//
	OLED_CS_Clr();
    LCD_WR_REG(0x36);
    LCD_WR_DATA8(HORIZONTAL_DATA);
    LCD_WR_REG(0x3A);
    LCD_WR_DATA8(0x05);
    LCD_WR_REG(0xB2);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x33);
    LCD_WR_REG(0xB7);
    LCD_WR_DATA8(0x35);
    LCD_WR_REG(0xBB);
    LCD_WR_DATA8(0x19);
    LCD_WR_REG(0xC0);
    LCD_WR_DATA8(0x2C);
    LCD_WR_REG(0xC2);
    LCD_WR_DATA8(0x01);
    LCD_WR_REG(0xC3);
    LCD_WR_DATA8(0x12);
    LCD_WR_REG(0xC4);
    LCD_WR_DATA8(0x20);
    LCD_WR_REG(0xC6);
    LCD_WR_DATA8(0x0F);
    LCD_WR_REG(0xD0);
    LCD_WR_DATA8(0xA4);
    LCD_WR_DATA8(0xA1);
    LCD_WR_REG(0xE0);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x13);
    LCD_WR_DATA8(0x2B);
    LCD_WR_DATA8(0x3F);
    LCD_WR_DATA8(0x54);
    LCD_WR_DATA8(0x4C);
    LCD_WR_DATA8(0x18);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x0B);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x23);
    LCD_WR_REG(0xE1);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x13);
    LCD_WR_DATA8(0x2C);
    LCD_WR_DATA8(0x3F);
    LCD_WR_DATA8(0x44);
    LCD_WR_DATA8(0x51);
    LCD_WR_DATA8(0x2F);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x20);
    LCD_WR_DATA8(0x23);
    OLED_CS_Set();
    disp_clear_screen(BLACK);
    OLED_CS_Clr();
    LCD_WR_REG(0x21);
    LCD_WR_REG(0x11);
    LCD_WR_REG(0x29);
    OLED_CS_Set();
}
