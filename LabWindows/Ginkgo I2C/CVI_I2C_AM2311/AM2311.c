#include <ansi_c.h>
#include <cvirte.h>		
#include <userint.h>
#include "AM2311.h"
#include "windows.h"

static int panelHandle;
//���庯��ָ�룬ע�⺯��ԭ�ͣ��βΡ�
typedef int (*I2CDLLFunction)(char *in,char *out);   
//����dll�еĺ���ָ��
I2CDLLFunction I2C_OpenDevice,I2C_Init,I2C_WriteData,I2C_ReadData,I2C_SelectDevice,I2C_USB_EnumBoard;
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
        //ȡdll�еĺ���ָ���ַ   
		I2C_OpenDevice = (I2CDLLFunction)GetProcAddress(hinstLib, "VTIF_OpenDevice");    
		I2C_ReadData = (I2CDLLFunction)GetProcAddress(hinstLib, "VTIF_I2C_ReadData");    //ȡ����ָ���ַ  
		I2C_WriteData = (I2CDLLFunction)GetProcAddress(hinstLib, "VTIF_I2C_WriteData"); 
		I2C_Init = (I2CDLLFunction)GetProcAddress(hinstLib, "VTIF_I2C_Init");    //ȡ����ָ���ַ   
		I2C_SelectDevice = (I2CDLLFunction)GetProcAddress(hinstLib, "VTIF_I2C_SelectDevice");    //ȡ����ָ���ַ   
		I2C_USB_EnumBoard = (I2CDLLFunction)GetProcAddress(hinstLib, "VTIF_USB_EnumBoard");    //ȡ����ָ���ַ    
	}else{
		MessagePopup("��ʾ","���ض�̬��ʧ�ܣ�");
	}
	
	if ((panelHandle = LoadPanel (0, "AM2311.uir", PANEL)) < 0)
		return -1;
	DisplayPanel (panelHandle);
	RunUserInterface ();
	DiscardPanel (panelHandle);
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
			ret = I2C_OpenDevice(strTemp,outData);
			if(ret){
				MessagePopup("����","���豸ʧ�ܣ�"); 
				return ret;
			}
			ret = I2C_SelectDevice(strTemp,outData);
			if(ret){
				MessagePopup("����","ѡ���豸ʧ�ܣ�"); 
				return ret;
			}
			sprintf(strTemp,"0x01|100000|2|0|7|0"); 
		    ret = I2C_Init(strTemp,outData);
		    if(ret!=0){
				MessagePopup("����","��ʼ���豸ʧ�ܣ�"); 
		        return ret;
		    }else{
				MessagePopup("��ʾ","��ʼ���豸�ɹ���"); 
			}
			break;
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
			vt_board_num=I2C_USB_EnumBoard((char*)"10",outData);
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

int CVICALLBACK MeasureCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int ret,i;
	char *strTemp[64]={NULL};
	float temperature,humidity;
	switch (event)
	{
		case EVENT_TIMER_TICK:
			// Wake up AM2311 sensor
			ret = I2C_WriteData("0xB8|0x00|0x00|0x00",outData); 
			//Send out read temperature and huminity command
			ret = I2C_WriteData("0xB8|0x00|0x00|0x03|0x00|0x04",outData);
			if(ret){
				SetCtrlAttribute(panelHandle,PANEL_TIMER_START,ATTR_ENABLED,0);
				SetCtrlAttribute(panelHandle,PANEL_COMMANDBUTTON_START,ATTR_LABEL_TEXT,"__StartMeasure");  
				MessagePopup("����","�����ȡ��ʪ������ʧ�ܣ�");  
				return ret;
			}
			// Read out temperature and huminity
			ret = I2C_ReadData("0xB8|0x00|0x00|0x08",outData);
			if(ret){
				SetCtrlAttribute(panelHandle,PANEL_TIMER_START,ATTR_ENABLED,0);
				SetCtrlAttribute(panelHandle,PANEL_COMMANDBUTTON_START,ATTR_LABEL_TEXT,"__StartMeasure");  
				MessagePopup("����","��ȡ��ʪ������ʧ�ܣ�"); 
				return ret;
			}
			//�����ص������ַ����е��¶Ⱥ�ʪ��������ȡ����
			i=0;
	        strTemp[i]= (char *)strtok(outData,"|");
	        while(strTemp[i++]!=NULL) {
	            strTemp[i]=(char *)strtok(NULL,"|");
	        }
			humidity = ((strtol(strTemp[2],NULL,0)<<8)|(strtol(strTemp[3],NULL,0)))/10.0;
			temperature = ((strtol(strTemp[4],NULL,0)<<8)|(strtol(strTemp[5],NULL,0)))/10.0;  
			//��ʾ����ֵ
			SetCtrlVal(panelHandle,PANEL_NUMERICMETER_TMP,temperature);
			SetCtrlVal(panelHandle,PANEL_NUMERICMETER_HUM,humidity); 
			break;
	}
	return 0;
}
