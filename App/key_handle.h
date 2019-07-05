/*
 * key_handle.h
 *
 *  Created on: 2019��5��1��
 *      Author: gongjunhui
 */

#ifndef KEY_HANDLE_H_
#define KEY_HANDLE_H_


typedef enum{       //������ֵ
	KEY_NONE              ,  //û�м�ֵ
    KEY_FM_SHORT_PRESS    ,  //FM�� �̰�
	KEY_FM_LONG_PRESS     ,  //FM�� ����
	KEY_BL_SHORT_PRESS    ,  //BL�� �̰�
	KEY_BL_LONG_PRESS     ,  //BL�� ����
	KEY_MAIN_SHORT_PRESS  ,  //���� �̰�
	KEY_MAIN_LONG_PRESS   ,  //���� ����
}_key_value_em;


extern volatile  _key_value_em  uckey_value;   /* ������ֵ  */

extern void ISR_key_handle_function(void);

#endif /* KEY_HANDLE_H_ */
