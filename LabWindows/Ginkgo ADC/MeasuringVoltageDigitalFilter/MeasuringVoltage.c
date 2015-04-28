#include <cvirte.h>		
#include <userint.h>
#include "MeasuringVoltage.h"
#include "windows.h"
#include <analysis.h>
#include <ansi_c.h>

static int panelHandle, ctrlarray;
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
		 
		 
	if ((panelHandle = LoadPanel (0, "MeasuringVoltage.uir", PANEL)) < 0)
		return -1;
	ctrlarray = GetCtrlArrayFromResourceID (panelHandle, CTRLARRAY);
	DisplayPanel (panelHandle);
	RunUserInterface ();
	DiscardPanel (panelHandle);
	return 0;
}
//��ʼ���豸
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
			sprintf(strTemp,"0x01|100"); 
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
	static int flag=0;
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
    double Wave[1024]={0};
	double WaveOut[1024]={0}; 
	char getDataTmp[8168]={0};
	char *strTemp[2048]={NULL};
	int adcData[1024]={0};
	int i=0,data=0,j=0;
	double  FIR_CoefArray[20]={0};
	strcpy(getDataTmp,indata);
    i=0;
	//�����������ص�ADC�����ַ���ת��Ϊ���ݲ���������ʾ�ڽ�����
    strTemp[i]= (char *)strtok(getDataTmp,"|");
    while(strTemp[i++]!=NULL) {
        strTemp[i]=(char *)strtok(NULL,"|");
    }
	for(i=0;i<1024;i++){
		adcData[i] = (int)strtol(strTemp[i],NULL,0);
		Wave[i] = adcData[i]*3.30/4096.0;
	}
	//�����ݽ��е�ͨ�˲�������Ƶ��Ϊ10K����ֹƵ��Ϊ1K��
	Bw_LPF (Wave, 1024, 10000, 1000, 10, WaveOut);
	//�����һ�ε�����
	DeleteGraphPlot (panelHandle, PANEL_GRAPH, -1, VAL_IMMEDIATE_DRAW);
	
	//��ʾ����
	PlotWaveform (panelHandle, PANEL_GRAPH, WaveOut, 1024, VAL_DOUBLE, 1.0, 0.0, 0.0, 1.0, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, 1, VAL_RED);

	
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
			ret = ADC_Read("1024",outData);
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
