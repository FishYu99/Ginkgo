#include <cvirte.h>		
#include <userint.h>
#include "MeasuringVoltage.h"
#include "windows.h"
#include <ansi_c.h>

static int panelHandle, ctrlarray;
typedef int (*ADCDLLFunction)(char *in,char *out);   //���庯��ָ�룬ע�⺯��ԭ�ͣ��βΡ�
//����dll�еĺ���ָ��
ADCDLLFunction ADC_OpenDevice,ADC_Init,ADC_Read,ADC_USB_EnumBoard,ADC_SelectDevice;
//���庯���������ݻ�����
char inData[10240],outData[10240];
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
		 
		 
	if ((panelHandle = LoadPanel (0, "MeasuringVoltage.uir", PANEL)) < 0)
		return -1;
	ctrlarray = GetCtrlArrayFromResourceID (panelHandle, CTRLARRAY);
	DisplayPanel (panelHandle);
	RunUserInterface ();
	DiscardPanel (panelHandle);
	return 0;
}
//��ȡͨ��ѡ��״̬
unsigned char GetCheckedState(void)
{
	int channel = 0,data=0;
	
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH7, &data);
	channel <<= 1;
	channel |= data;
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH6, &data);
	channel <<= 1;
	channel |= data;   
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH5, &data);
	channel <<= 1;
	channel |= data;   
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH4, &data);
	channel <<= 1;
	channel |= data;   
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH3, &data);
	channel <<= 1;
	channel |= data;   
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH2, &data);
	channel <<= 1;
	channel |= data;   
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH1, &data);
	channel <<= 1;
	channel |= data;   
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH0, &data);
	channel <<= 1;
	channel |= data;   
	return channel;
}
int CVICALLBACK InitDeviceButtonCallback (int panel, int control, int event,
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
			sprintf(strTemp,"0x%02X|0x01",GetCheckedState()); 
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

int CVICALLBACK StartButtonCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static flag=0;
	switch (event)
	{
		case EVENT_COMMIT:
			if(flag==0){
				SetCtrlAttribute(panelHandle,PANEL_TIMER_MEASURE,ATTR_ENABLED,1);
				SetCtrlAttribute(panelHandle,PANEL_StartButton,ATTR_LABEL_TEXT,"__StopMeasure");  
				flag = 1;
			}else{
				SetCtrlAttribute(panelHandle,PANEL_TIMER_MEASURE,ATTR_ENABLED,0);
				SetCtrlAttribute(panelHandle,PANEL_StartButton,ATTR_LABEL_TEXT,"__StartMeasure"); 
				flag = 0;
			}
			break;
	}
	return 0;
}
void displayData(char *indata)
{
    char getDataTmp[1024]={0};
	char *strTemp[64]={NULL};
	char dspTemp[50]={0};
	int adcData[8]={0};
	int i=0,data=0,j=0;
	strcpy(getDataTmp,indata);
    i=0;
	//�����������ص�ADC�����ַ���ת��Ϊ���ݲ���������ʾ�ڽ�����
    strTemp[i]= (char *)strtok(getDataTmp,"|");
    while(strTemp[i++]!=NULL) {
        strTemp[i]=(char *)strtok(NULL,"|");
    }
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH0, &data);
	if(data){
		adcData[0] = (int)strtol(strTemp[j++],NULL,0);
		
	}else{
		adcData[0] = 0;
	}
	sprintf(dspTemp,"%.3f",adcData[0]*3300.0/4096.0);
	SetCtrlVal(panelHandle,PANEL_STRING_CH0,dspTemp);
	
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH1, &data);
	if(data){
		adcData[1] = (int)strtol(strTemp[j++],NULL,0);
	}else{
		adcData[1] = 0;
	} 
	sprintf(dspTemp,"%.3f",adcData[1]*3300.0/4096.0);
	SetCtrlVal(panelHandle,PANEL_STRING_CH1,dspTemp);
	
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH2, &data);
	if(data){
		adcData[2] = (int)strtol(strTemp[j++],NULL,0);
	}else{
		adcData[2] = 0;
	}
	sprintf(dspTemp,"%.3f",adcData[2]*3300.0/4096.0);
	SetCtrlVal(panelHandle,PANEL_STRING_CH2,dspTemp);
	
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH3, &data);
	if(data){
		adcData[3] = (int)strtol(strTemp[j++],NULL,0);
	}else{
		adcData[3] = 0;
	}  
	sprintf(dspTemp,"%.3f",adcData[3]*3300.0/4096.0);
	SetCtrlVal(panelHandle,PANEL_STRING_CH3,dspTemp);
	
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH4, &data);
	if(data){
		adcData[4] = (int)strtol(strTemp[j++],NULL,0);
	}else{
		adcData[4] = 0;
	} 
	sprintf(dspTemp,"%.3f",adcData[4]*3300.0/4096.0);
	SetCtrlVal(panelHandle,PANEL_STRING_CH4,dspTemp);
	
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH5, &data);
	if(data){
		adcData[5] = (int)strtol(strTemp[j++],NULL,0);
	}else{
		adcData[5] = 0;
	} 
	sprintf(dspTemp,"%.3f",adcData[5]*3300.0/4096.0);
	SetCtrlVal(panelHandle,PANEL_STRING_CH5,dspTemp);
	
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH6, &data);
	if(data){
		adcData[6] = (int)strtol(strTemp[j++],NULL,0);
	}else{
		adcData[6] = 0;
	}  
	sprintf(dspTemp,"%.3f",adcData[6]*3300.0/4096.0);
	SetCtrlVal(panelHandle,PANEL_STRING_CH6,dspTemp);
	
	GetCtrlVal (panelHandle, PANEL_CHECKBOX_CH7, &data);
	if(data){
		adcData[7] = (int)strtol(strTemp[j++],NULL,0);
	}else{
		adcData[7] = 0;
	}
	sprintf(dspTemp,"%.3f",adcData[7]*3300.0/4096.0);
	SetCtrlVal(panelHandle,PANEL_STRING_CH7,dspTemp);
	
}
//��ʱ��ȡ���ݶ�ʱ���ص�����
int CVICALLBACK TimerMeasureCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char strTemp[100]={0}; 
	int ret;
	switch (event)
	{
		case EVENT_TIMER_TICK:
			ret = ADC_Read("1",outData);
			if(ret){
				SetCtrlAttribute(panelHandle,PANEL_TIMER_MEASURE,ATTR_ENABLED,0); 
				MessagePopup("����","������ʧ�ܣ�"); 
				return ret;
			}else{
				displayData(outData);
			}
			break;
	}
	return 0;
}
//��ʱɨ���豸��ʱ���ص�����
int CVICALLBACK TimerCallback (int panel, int control, int event,
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

int CVICALLBACK SelectDeviceCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			
			break;
	}
	return 0;
}

int CVICALLBACK QuitButtonCallback (int panel, int control, int event,
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
