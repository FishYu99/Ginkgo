/**
  ******************************************************************************
  * @file    ControlCAN.h
  * $Author: wdluo $
  * $Revision: 447 $
  * $Date:: 2013-06-29 18:24:57 +0800 #$
  * @brief   Ginkgo USB-CAN�������ײ�������API��������.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, ViewTool</center>
  *<center><a href="http:\\www.viewtool.com">http://www.viewtool.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
#ifndef _CONTROLCAN_H_
#define _CONTROLCAN_H_

#include <stdint.h>
#include "ErrorType.h"
#ifndef OS_LINUX
#include <Windows.h>
#endif

//���������Ͷ���
#define VCI_USBCAN1		3
#define VCI_USBCAN2		4


//CAN������
#define	ERR_CAN_OVERFLOW			0x0001	//CAN �������ڲ�FIFO���
#define	ERR_CAN_ERRALARM			0x0002	//CAN ���������󱨾�
#define	ERR_CAN_PASSIVE				0x0004	//CAN ��������������
#define	ERR_CAN_LOSE				0x0008	//CAN �������ٲö�ʧ
#define	ERR_CAN_BUSERR				0x0010	//CAN ���������ߴ���
#define	ERR_CAN_BUSOFF				0x0020	//CAN ���������߹ر�

//ͨ�ô�����
#define	ERR_DEVICEOPENED			0x0100	//�豸�Ѿ���
#define	ERR_DEVICEOPEN				0x0200	//���豸����
#define	ERR_DEVICENOTOPEN			0x0400	//�豸û�д�
#define	ERR_BUFFEROVERFLOW			0x0800	//���������
#define	ERR_DEVICENOTEXIST			0x1000	//���豸������
#define	ERR_LOADKERNELDLL			0x2000	//װ�ض�̬��ʧ��
#define ERR_CMDFAILED				0x4000	//ִ������ʧ�ܴ�����
#define	ERR_BUFFERCREATE			0x8000	//�ڴ治��


//�������÷���״ֵ̬
#define	STATUS_OK					1
#define STATUS_ERR					0
	

//1.����ZLGCANϵ�нӿڿ���Ϣ���������͡�
typedef  struct  _VCI_BOARD_INFO{
	USHORT	hw_Version;			//Ӳ���汾�ţ���16 ���Ʊ�ʾ������0x0100 ��ʾV1.00��
	USHORT	fw_Version;			//�̼��汾�ţ���16 ���Ʊ�ʾ��
	USHORT	dr_Version;			//��������汾�ţ���16 ���Ʊ�ʾ��
	USHORT	in_Version;			//�ӿڿ�汾�ţ���16 ���Ʊ�ʾ��
	USHORT	irq_Num;			//�忨��ʹ�õ��жϺš�
	BYTE	can_Num;			//��ʾ�м�·CAN ͨ����
	CHAR	str_Serial_Num[20];	//�˰忨�����кš�
	CHAR	str_hw_Type[40];	//Ӳ�����ͣ����硰USBCAN V1.00����ע�⣺�����ַ�����������\0������
	USHORT	Reserved[4];		//ϵͳ������
} VCI_BOARD_INFO,*PVCI_BOARD_INFO; 

//1.Ginkgoϵ�нӿڿ���Ϣ���������͡�
typedef  struct  _VCI_BOARD_INFO_EX{
	USHORT	dr_Version;			//��������汾�ţ���16 ���Ʊ�ʾ��
	USHORT	in_Version;			//�ӿڿ�汾�ţ���16 ���Ʊ�ʾ��
	BYTE	can_Num;			//��ʾ�м�·CAN ͨ����
	CHAR	str_hw_Name[100];	//Ӳ�����ƣ����硰Ginkgo-CAN-Adaptor����ע�⣺�����ַ�����������\0����
	CHAR	str_fw_Version[100];//�̼��汾�ַ��������硰2.0.0����ע�⣺�����ַ�����������\0����
	CHAR	str_hw_Version[100];//Ӳ���汾�ַ��������硰2.1.0����ע�⣺�����ַ�����������\0����
	CHAR	str_Serial_Num[100];//���������к��ַ���
	USHORT	Reserved[4];		//ϵͳ������
} VCI_BOARD_INFO_EX,*PVCI_BOARD_INFO_EX; 

//2.����CAN��Ϣ֡���������͡�
typedef  struct  _VCI_CAN_OBJ{
	UINT	ID;			//����ID��
	UINT	TimeStamp;	//���յ���Ϣ֡ʱ��ʱ���ʶ����CAN ��������ʼ����ʼ��ʱ��
	BYTE	TimeFlag;	//�Ƿ�ʹ��ʱ���ʶ��Ϊ1 ʱTimeStamp ��Ч��TimeFlag ��TimeStamp ֻ�ڴ�֡Ϊ����֡ʱ�����塣
	BYTE	SendType;	//����֡���ͣ�=0 ʱΪ�������ͣ�=1 ʱΪ���η��ͣ�=2 ʱΪ�Է����գ�=3 ʱΪ�����Է����գ�ֻ�ڴ�
						//֡Ϊ����֡ʱ�����塣�����豸����ΪEG20T-CAN ʱ�����ͷ�ʽ��VCI_InitCan ��ͨ�����ã��˴���
						//������Ч������Ϊ�Է�����ģʽ��EG20T-CAN ���ܴ������Ͻ������ݣ�ֻ���յ��Լ����������ݣ�
	BYTE	RemoteFlag;	//�Ƿ���Զ��֡
	BYTE	ExternFlag;	//�Ƿ�����չ֡
	BYTE	DataLen;	//���ݳ���(<=8)����Data �ĳ��ȡ�
	BYTE	Data[8];	//���ĵ����ݡ�
	BYTE	Reserved[3];//ϵͳ������
}VCI_CAN_OBJ,*PVCI_CAN_OBJ;

//3.����CAN������״̬���������͡�
typedef struct _VCI_CAN_STATUS{
	UCHAR	ErrInterrupt;	//�жϼ�¼���������������
	UCHAR	regMode;		//CAN ������ģʽ�Ĵ�����
	UCHAR	regStatus;		//CAN ������״̬�Ĵ�����
	UCHAR	regALCapture;	//CAN �������ٲö�ʧ�Ĵ�����
	UCHAR	regECCapture;	//CAN ����������Ĵ����� 
	UCHAR	regEWLimit;		//CAN ���������󾯸����ƼĴ�����
	UCHAR	regRECounter;	//CAN ���������մ���Ĵ����� 
	UCHAR	regTECounter;	//CAN ���������ʹ���Ĵ�����
	UINT	regESR;			//CAN ����������״̬�Ĵ�����
	UINT	regTSR;			//CAN ����������״̬�Ĵ���
	UINT	BufferSize;		//CAN ���������ջ�������С
	DWORD	Reserved;
}VCI_CAN_STATUS,*PVCI_CAN_STATUS;

//4.���������Ϣ���������͡�
typedef struct _ERR_INFO{
	UINT	ErrCode;			//������
	BYTE	Passive_ErrData[3];	//�������Ĵ���������������ʱ��ʾΪ��������Ĵ����ʶ���ݡ�
	BYTE	ArLost_ErrData;		//�������Ĵ��������ٲö�ʧ����ʱ��ʾΪ�ٲö�ʧ����Ĵ����ʶ���ݡ�
} VCI_ERR_INFO,*PVCI_ERR_INFO;

//5.�����ʼ��CAN����������
typedef struct _INIT_CONFIG{
	DWORD	AccCode;	//������
	DWORD	AccMask;	//������
	DWORD	Reserved;	//����
	UCHAR	Filter;		//�˲���ʽ,0-˫�˲���1-���˲�
	UCHAR	Timing0;	//��ʱ��0��BTR0����	
	UCHAR	Timing1;	//��ʱ��1��BTR1����	
	UCHAR	Mode;		//ģʽ��
}VCI_INIT_CONFIG,*PVCI_INIT_CONFIG;

//�����ʼ��CAN����������
typedef struct _INIT_CONFIG_EX
{
	//CAN������ = 36MHz/(CAN_BRP)/(CAN_SJW+CAN_BS1+CAN_BS2)
	DWORD	CAN_BRP;	//ȡֵ��Χ1~1024
	UCHAR	CAN_SJW;	//ȡֵ��Χ1~4          
	UCHAR	CAN_BS1;	//ȡֵ��Χ1~16          
	UCHAR	CAN_BS2;	//ȡֵ��Χ1~8          
	UCHAR	CAN_Mode;	//CAN����ģʽ��0-����ģʽ��1-����ģʽ��2-��Ĭģʽ��3-��Ĭ����ģʽ
	UCHAR	CAN_ABOM;	//�Զ����߹���0-��ֹ��1-ʹ��
	UCHAR	CAN_NART;	//�����ط�����0-ʹ�ܱ����ش���1-��ֹ�����ش�
	UCHAR	CAN_RFLM;	//FIFO��������0-�±��ĸ��Ǿɱ��ģ�1-�����±���
	UCHAR	CAN_TXFP;	//�������ȼ�����0-��ʶ��������1-��������˳�����
	UCHAR	CAN_RELAY;	//�Ƿ����м̹��ܣ�0x00-�ر��м̹��ܣ�0x10-CAN1��CAN2�м̣�0x01-CAN2��CAN1�м̣�0x11-˫���м�
	DWORD	Reserved;	//ϵͳ����
}VCI_INIT_CONFIG_EX,*PVCI_INIT_CONFIG_EX;


//6.������CAN �˲���������
typedef struct _VCI_FILTER_CONFIG{
	UCHAR	Enable;			//ʹ�ܸù�������1-ʹ�ܣ�0-��ֹ
	UCHAR	FilterIndex;	//�����������ţ�ȡֵ��ΧΪ0��13
	UCHAR	FilterMode;		//������ģʽ��0-����λģʽ��1-��ʶ���б�ģʽ
	UCHAR	ExtFrame;		//���˵�֡���ͱ�־��Ϊ1 ����Ҫ���˵�Ϊ��չ֡��Ϊ0 ����Ҫ���˵�Ϊ��׼֡��
	DWORD	ID_Std_Ext;		//������ID
	DWORD	ID_IDE;			//������IDE
	DWORD	ID_RTR;			//������RTR
	DWORD	MASK_Std_Ext;	//������ID������ֻ���ڹ�����ģʽΪ����λģʽʱ����
	DWORD	MASK_IDE;		//������IDE������ֻ���ڹ�����ģʽΪ����λģʽʱ����
	DWORD	MASK_RTR;		//������RTR������ֻ���ڹ�����ģʽΪ����λģʽʱ����
	DWORD	Reserved;		//ϵͳ����
} VCI_FILTER_CONFIG,*PVCI_FILTER_CONFIG;

#ifdef __cplusplus
extern "C"
{
#endif

DWORD WINAPI VCI_ScanDevice(UCHAR NeedInit);
DWORD WINAPI VCI_OpenDevice(DWORD DevType,DWORD DevIndex,DWORD Reserved);
DWORD WINAPI VCI_CloseDevice(DWORD DevType,DWORD DevIndex);
DWORD WINAPI VCI_InitCAN(DWORD DevType, DWORD DevIndex, DWORD CANIndex, PVCI_INIT_CONFIG pInitConfig);
DWORD WINAPI VCI_InitCANEx(DWORD DevType, DWORD DevIndex, DWORD CANIndex, PVCI_INIT_CONFIG_EX pInitConfig);

DWORD WINAPI VCI_ReadBoardInfo(DWORD DevType,DWORD DevIndex,PVCI_BOARD_INFO pInfo);
DWORD WINAPI VCI_ReadBoardInfoEx(DWORD DevType, DWORD DevIndex, PVCI_BOARD_INFO_EX pInfo);
DWORD WINAPI VCI_ReadErrInfo(DWORD DevType,DWORD DevIndex,DWORD CANIndex,PVCI_ERR_INFO pErrInfo);
DWORD WINAPI VCI_ReadCANStatus(DWORD DevType,DWORD DevIndex,DWORD CANIndex,PVCI_CAN_STATUS pCANStatus);

DWORD WINAPI VCI_GetReference(DWORD DevType,DWORD DevIndex,DWORD CANIndex,DWORD RefType,PVOID pData);
DWORD WINAPI VCI_SetReference(DWORD DevType,DWORD DevIndex,DWORD CANIndex,DWORD RefType,PVOID pData);
DWORD WINAPI VCI_SetFilter(DWORD DevType,DWORD DevIndex,DWORD CANIndex,PVCI_FILTER_CONFIG pFilter);

ULONG WINAPI VCI_GetReceiveNum(DWORD DevType,DWORD DevIndex,DWORD CANIndex);
DWORD WINAPI VCI_ClearBuffer(DWORD DevType,DWORD DevIndex,DWORD CANIndex);

DWORD WINAPI VCI_StartCAN(DWORD DevType,DWORD DevIndex,DWORD CANIndex);
DWORD WINAPI VCI_ResetCAN(DWORD DevType,DWORD DevIndex,DWORD CANIndex);

ULONG WINAPI VCI_Transmit(DWORD DevType,DWORD DevIndex,DWORD CANIndex,PVCI_CAN_OBJ pSend,ULONG Len);
ULONG WINAPI VCI_Receive(DWORD DevType,DWORD DevIndex,DWORD CANIndex,PVCI_CAN_OBJ pReceive,ULONG Len,INT WaitTime=-1);

#ifdef __cplusplus
}
#endif

#endif

