#include <cvirte.h>		
#include <userint.h>
#include "EnvironmentalMonitoring.h"
#include "windows.h"
#include <ansi_c.h>

static int panelHandle;
typedef int (*ADCDLLFunction)(char *in,char *out);   //���庯��ָ�룬ע�⺯��ԭ�ͣ��βΡ�
//����dll�еĺ���ָ��
ADCDLLFunction ADC_OpenDevice,ADC_Init,ADC_Read,ADC_USB_EnumBoard,ADC_SelectDevice;
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
         ADC_OpenDevice = (ADCDLLFunction)GetProcAddress(hinstLib, "VTIF_OpenDevice");    //ȡ����ָ���ַ 
		 ADC_Read = (ADCDLLFunction)GetProcAddress(hinstLib, "VTIF_ADC_ReadData");    //ȡ����ָ���ַ   
		 ADC_Init = (ADCDLLFunction)GetProcAddress(hinstLib, "VTIF_ADC_Init");    //ȡ����ָ���ַ   
		 ADC_SelectDevice = (ADCDLLFunction)GetProcAddress(hinstLib, "VTIF_ADC_SelectDevice");    //ȡ����ָ���ַ   
		 ADC_USB_EnumBoard = (ADCDLLFunction)GetProcAddress(hinstLib, "VTIF_USB_EnumBoard");    //ȡ����ָ���ַ    
	}else{
		MessagePopup("��ʾ","���ض�̬��ʧ�ܣ�");
	}
	if ((panelHandle = LoadPanel (0, "EnvironmentalMonitoring.uir", PANEL)) < 0)
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
			ret = ADC_OpenDevice(strTemp,outData);
			if(ret){
				MessagePopup("����","���豸ʧ�ܣ�"); 
				return ret;
			}
			ret = ADC_SelectDevice(strTemp,outData);
			if(ret){
				MessagePopup("����","ѡ���豸ʧ�ܣ�"); 
				return ret;
			}
			sprintf(strTemp,"0x3F|1"); 
		    ret = ADC_Init(strTemp,outData);
		    if(ret!=0){
				MessagePopup("����","��ʼ���豸ʧ�ܣ�"); 
		        return ret;
		    }else{
				MessagePopup("��ʾ","��ʼ���豸�ɹ���"); 
			}
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
			vt_board_num=ADC_USB_EnumBoard((char*)"10",outData);
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
    double MeasureData[100]={0};
	char getDataTmp[100]={0};
	char *strTemp[10]={NULL};
	int adcData[1024]={0};
	int i=0,data=0,j=0;
	strcpy(getDataTmp,indata);
    i=0;
	//�����������ص�ADC�����ַ���ת��Ϊ���ݲ���������ʾ�ڽ�����
    strTemp[i]= (char *)strtok(getDataTmp,"|");
    while(strTemp[i++]!=NULL) {
        strTemp[i]=(char *)strtok(NULL,"|");
    }
	for(i=0;i<6;i++){
		adcData[i] = (int)strtol(strTemp[i],NULL,0);
		MeasureData[i] = adcData[i]*3.30/4096.0;
	}
	SetCtrlVal(panelHandle,PANEL_SMOKE,MeasureData[0]);
	SetCtrlVal(panelHandle,PANEL_ALCOHOL,MeasureData[1]); 
	SetCtrlVal(panelHandle,PANEL_NATURAL_GAS,MeasureData[2]);
	SetCtrlVal(panelHandle,PANEL_COAL_GAS,MeasureData[3]);
	SetCtrlVal(panelHandle,PANEL_CARBON_GAS,MeasureData[4]);
	SetCtrlVal(panelHandle,PANEL_LIGHT,MeasureData[5]);
}

int CVICALLBACK MeasureCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char strTemp[100]={0}; 
	int ret;
	switch (event)
	{
		case EVENT_TIMER_TICK:
			ret = ADC_Read("1",outData);
			if(ret){
				SetCtrlAttribute(panelHandle,PANEL_TIMER_START,ATTR_ENABLED,0); 
				SetCtrlAttribute(panelHandle,PANEL_COMMANDBUTTON_START,ATTR_LABEL_TEXT,"__StartMeasure"); 
				MessagePopup("����","������ʧ�ܣ�"); 
				return ret;
			}else{
				displayData(outData);
			}
			break;
	}
	return 0;
}
