#include <cvirte.h>		
#include <userint.h>
#include "VoiceControl.h"
#include "windows.h"
#include <ansi_c.h>
#include <mmsystem.h>

static int panelHandle;
typedef int (*USARTDLLFunction)(char *in,char *out);   //���庯��ָ�룬ע�⺯��ԭ�ͣ��βΡ�
//����dll�еĺ���ָ��
USARTDLLFunction USART_OpenDevice,USART_Init,USART_ReadData,USART_WriteData,USART_USB_EnumBoard,USART_SelectDevice;
USARTDLLFunction GPIO_SelectDevice,GPIO_Init,GPIO_WriteData,GPIO_ReadData;
//���庯���������ݻ�����
char inData[10240],outData[102400];
int deviceNum=0;//�豸����

int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	//����dll�ļ�����ȡ����ָ��
    HMODULE hinstLib = LoadLibrary("Ginkgo_Driver.dll");    //װ�ض�̬���ӿ�Ginkgo_Driver.dll
    if (hinstLib != NULL)                                  	//�ɹ�װ�ض�̬���ӿ�Ginkgo_Driver.dll
    {
         USART_OpenDevice = (USARTDLLFunction)GetProcAddress(hinstLib, "VTIF_OpenDevice");    //ȡ����ָ���ַ 
		 USART_ReadData = (USARTDLLFunction)GetProcAddress(hinstLib, "VTIF_USART_ReadData");    //ȡ����ָ���ַ   
		 USART_WriteData = (USARTDLLFunction)GetProcAddress(hinstLib, "VTIF_USART_WriteData");    //ȡ����ָ���ַ    
		 USART_Init = (USARTDLLFunction)GetProcAddress(hinstLib, "VTIF_USART_Init");    //ȡ����ָ���ַ   
		 USART_SelectDevice = (USARTDLLFunction)GetProcAddress(hinstLib, "VTIF_USART_SelectDevice");    //ȡ����ָ���ַ   
		 USART_USB_EnumBoard = (USARTDLLFunction)GetProcAddress(hinstLib, "VTIF_USB_EnumBoard");    //ȡ����ָ���ַ   
		 GPIO_SelectDevice = (USARTDLLFunction)GetProcAddress(hinstLib, "VTIF_GPIO_SelectDevice");
		 GPIO_Init = (USARTDLLFunction)GetProcAddress(hinstLib, "VTIF_GPIO_Init");
		 GPIO_WriteData = (USARTDLLFunction)GetProcAddress(hinstLib, "VTIF_GPIO_WriteData"); 
		 GPIO_ReadData = (USARTDLLFunction)GetProcAddress(hinstLib, "VTIF_GPIO_ReadData"); 
	}else{
		MessagePopup("��ʾ","���ض�̬��ʧ�ܣ�");
	}
	if ((panelHandle = LoadPanel (0, "VoiceControl.uir", PANEL)) < 0)
		return -1;
	DisplayPanel (panelHandle);
	RunUserInterface ();
	DiscardPanel (panelHandle);
	return 0;
}

int CVICALLBACK RingDeviceCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			break;
	}
	return 0;
}

int CVICALLBACK InitDeviceCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char strTemp[100]={0};
	int deviceIndex=0;
	int ret;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlIndex(panelHandle,PANEL_RING_DEVICE,&deviceIndex);
			sprintf(strTemp,"%d",deviceIndex);
			ret = USART_OpenDevice(strTemp,outData);
			if(ret){
				MessagePopup("����","���豸ʧ�ܣ�"); 
				return ret;
			}
			ret = USART_SelectDevice(strTemp,outData);
			if(ret){
				MessagePopup("����","ѡ���豸ʧ�ܣ�"); 
				return ret;
			}
			GPIO_SelectDevice(strTemp,outData);   
			sprintf(strTemp,"1|9600|0|0|0|232"); 
		    ret = USART_Init(strTemp,outData);
		    if(ret!=0){
				MessagePopup("����","��ʼ���豸ʧ�ܣ�"); 
		        return ret;
		    }else{
				MessagePopup("��ʾ","��ʼ���豸�ɹ���"); 
			}
			GPIO_Init("0x0001|0x0000",outData);
			break;
	}
	return 0;
}

int CVICALLBACK StartMeasureCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int flag=0;
	switch (event)
	{
		case EVENT_COMMIT:
			if(flag==0){
				SetCtrlAttribute(panelHandle,PANEL_TIMER_START,ATTR_ENABLED,1);
				SetCtrlAttribute(panelHandle,PANEL_COMMANDBUTTON_START,ATTR_LABEL_TEXT,"__StopMeasure");  
				flag = 1;
			}else{
				SetCtrlAttribute(panelHandle,PANEL_TIMER_START,ATTR_ENABLED,0);
				SetCtrlAttribute(panelHandle,PANEL_COMMANDBUTTON_START,ATTR_LABEL_TEXT,"__StartMeasure"); 
				flag = 0;
			}
			break;
	}
	return 0;
}

int CVICALLBACK QuitCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			QuitUserInterface (0);
			break;
	}
	return 0;
}

int CVICALLBACK ScanDeviceCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    char deviceInfo[1024]={0};
    int vt_board_num,i;
    char *strTemp[64]={NULL};
    char getDataTmp[512]={0};
	char deviceName[100]={0};
	switch (event)
	{
		case EVENT_TIMER_TICK:
			vt_board_num=USART_USB_EnumBoard((char*)"10",outData);
	        strcpy(getDataTmp,outData);
	        i=0;
	        strTemp[i]= (char *)strtok(getDataTmp,"|");
	        while(strTemp[i++]!=NULL) {
	            strTemp[i]=(char *)strtok(NULL,"|");
	        }
	        if(vt_board_num!=deviceNum){//����һ���豸
				ClearListCtrl (panelHandle,PANEL_RING_DEVICE);//���֮ǰ���е��豸
	            for(i=0;i<vt_board_num;i++){
	                sprintf(deviceName,"DEVICE %d",(int)strtol(strTemp[i]+2,NULL,0));
					InsertListItem(panelHandle,PANEL_RING_DEVICE,i,deviceName,i);//����һ���豸�б�
	            }
	            if(vt_board_num>0){
	            }else{
	                InsertListItem(panelHandle,PANEL_RING_DEVICE,0,"NO DEVICE",0);//û���豸����
	            }
	        }
	        deviceNum=vt_board_num;
			break;
	}
	return 0;
}
void displayData(char *indata)
{
	int data=0;
	SetCtrlVal(panelHandle,PANEL_COMMAND,indata);
	data = (int)strtol(indata,NULL,0); 
	switch(data)
	{
		case 0x00:SetCtrlVal(panelHandle,PANEL_LED_1,1);break;//����1
		case 0x01:SetCtrlVal(panelHandle,PANEL_LED_2,1);break;//����2  
		case 0x02:SetCtrlVal(panelHandle,PANEL_LED_3,1);break;//����3  
		case 0x03:SetCtrlVal(panelHandle,PANEL_LED_4,1);break;//����4  
		case 0x04:SetCtrlVal(panelHandle,PANEL_LED_5,1);break;//����5  
		case 0x05:SetCtrlVal(panelHandle,PANEL_LED_6,1);break;//����6  
		case 0x06:SetCtrlVal(panelHandle,PANEL_LED_7,1);break;//����7  
		case 0x07:SetCtrlVal(panelHandle,PANEL_LED_8,1);break;//����8  
		case 0x08:SetCtrlVal(panelHandle,PANEL_LED_9,1);break;//����9  
		case 0x09:SetCtrlVal(panelHandle,PANEL_LED_10,1);break;//����10  
		
		case 0x0A:SetCtrlVal(panelHandle,PANEL_LED_1,0);break;//�ص�1
		case 0x0B:SetCtrlVal(panelHandle,PANEL_LED_2,0);break;//�ص�2   
		case 0x0C:SetCtrlVal(panelHandle,PANEL_LED_3,0);break;//�ص�3   
		case 0x0D:SetCtrlVal(panelHandle,PANEL_LED_4,0);break;//�ص�4   
		case 0x0E:SetCtrlVal(panelHandle,PANEL_LED_5,0);break;//�ص�5     
		case 0x0F:SetCtrlVal(panelHandle,PANEL_LED_6,0);break;//�ص�6     
		case 0x10:SetCtrlVal(panelHandle,PANEL_LED_7,0);break;//�ص�7     
		case 0x11:SetCtrlVal(panelHandle,PANEL_LED_8,0);break;//�ص�8     
		case 0x12:SetCtrlVal(panelHandle,PANEL_LED_9,0);break;//�ص�9     
		case 0x13:SetCtrlVal(panelHandle,PANEL_LED_10,0);break;//�ص�10    
		
		case 0x17://ȫ������
			SetCtrlVal(panelHandle,PANEL_LED_1,1);
			SetCtrlVal(panelHandle,PANEL_LED_2,1);  
			SetCtrlVal(panelHandle,PANEL_LED_3,1);  
			SetCtrlVal(panelHandle,PANEL_LED_4,1);  
			SetCtrlVal(panelHandle,PANEL_LED_5,1);  
			SetCtrlVal(panelHandle,PANEL_LED_6,1);  
			SetCtrlVal(panelHandle,PANEL_LED_7,1);  
			SetCtrlVal(panelHandle,PANEL_LED_8,1);  
			SetCtrlVal(panelHandle,PANEL_LED_9,1);  
			SetCtrlVal(panelHandle,PANEL_LED_10,1); 
			break;
		case 0x18://ȫ���ر�
			SetCtrlVal(panelHandle,PANEL_LED_1,0);
			SetCtrlVal(panelHandle,PANEL_LED_2,0);  
			SetCtrlVal(panelHandle,PANEL_LED_3,0);  
			SetCtrlVal(panelHandle,PANEL_LED_4,0);  
			SetCtrlVal(panelHandle,PANEL_LED_5,0);  
			SetCtrlVal(panelHandle,PANEL_LED_6,0);  
			SetCtrlVal(panelHandle,PANEL_LED_7,0);  
			SetCtrlVal(panelHandle,PANEL_LED_8,0);  
			SetCtrlVal(panelHandle,PANEL_LED_9,0);  
			SetCtrlVal(panelHandle,PANEL_LED_10,0); 
			break;
		case 0x16:DisplayImageFile(panelHandle,PANEL_PICTURE,"Desert.jpg");break; //��ʾͼƬ
		case 0x15:sndPlaySound ("Mode_Story.wav", 3);break;//�������� 
		case 0x14:sndPlaySound ("Mode_Song.wav", 3);break;//�������    
		case 0x19:sndPlaySound ("Story1.wav", 3);break;//��һ��	
		case 0x1A:sndPlaySound ("Story2.wav", 3);break;//�ڶ���	
		case 0x1B:sndPlaySound ("Story3.wav", 3);break;//������	
		default:break;
	}
}

int CVICALLBACK MeasureCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char strTemp[100]={0}; 
	int ret,GPIO_Data;
	switch (event)
	{
		case EVENT_TIMER_TICK:
			GPIO_ReadData("0x0001",outData);
			GPIO_Data = (int)strtol(outData,NULL,0); 
			if(GPIO_Data){
				SetCtrlVal(panelHandle,PANEL_LED_BUSY,1);
				return 0;
			}else{
				SetCtrlVal(panelHandle,PANEL_LED_BUSY,0); 
			}
			ret = USART_ReadData("",outData);
			if(ret){
				return ret;
			}else{
				displayData(outData);
			}
			break;
	}
	return 0;
}
