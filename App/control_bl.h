/*
 * control_bl.h
 *
 *  Created on: 2019��5��2��
 *      Author: gongjunhui
 */

#ifndef CONTROL_BL_H_
#define CONTROL_BL_H_

typedef enum {             //����ģ��״̬
	BL_STATUS_RESET       ,  //�����ָ���
	BL_STATUS_RESTART     ,  //������
    BL_STATUS_LINKING     ,  //������
	BL_STATUS_LINK_OK     ,  //���ӳɹ�
}_bl_status_em;


extern _bl_status_em  em_bl_state;//����ģ��״̬

extern void UART_RxTime(void);
extern void bl_control_function(void);
extern void bl_data_handle_init(void);
extern void bl_bind_init(void);
extern void bl_binding(void);

#endif /* CONTROL_BL_H_ */
