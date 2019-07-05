/*
 * GB1212.h
 *
 *  Created on: 2018��5��27��
 *      Author: gongjunhui
 */

#ifndef GB1212_H_
#define GB1212_H_
#include "main.h"
//---------------------------------------------------------------------------------
//------------------------------ ASCII�ֿ� ---------------------------------------
//--------------------------------------------------------------------------------
typedef struct{
		unsigned char Head[1];         /* ���� */
		unsigned char Infor[12];       /* ������Ϣ */
}_GB2312Type6x12;


// ------------------  ������ģ�����ݽṹ���� ------------------------ //
typedef struct{                // ������ģ���ݽṹ
      unsigned char   Head[2];               // ������������
      unsigned char   Infor[24];             // ����������
}_GB2312Type12x12;

extern const _GB2312Type6x12  GB2312Code6x12[];
extern const _GB2312Type12x12 GB2312Code12x12[];
extern const uint16_t len_code6x12;
extern const uint16_t len_code12x12;

#endif /* LCD_1_44_GB1212_H_ */
