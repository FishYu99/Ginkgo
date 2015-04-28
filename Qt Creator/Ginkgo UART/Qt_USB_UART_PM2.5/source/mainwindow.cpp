#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ControlUART.h"
#include "ControlADC.h"
/**
  * @brief  Ӳ������
  * @param  VCC->5V(PIN1),GND->GND(PIN19),VO->ADC_CH0(PIN3),TXD->UART1_RX(PIN8)
  * @retval None
  */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QApplication::addLibraryPath("./plugins");
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB2312"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("GB2312"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312"));
    deviceNum = 0;
    memset(PM25ADData,0,sizeof(PM25ADData));
    memset(PM25Data,0,sizeof(PM25Data));
    PM25PlotInit(ui->qwtPlot);

    QObject::connect(ui->aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    QObject::connect(ui->contactUsAction, SIGNAL(triggered()), this, SLOT(contactus()));
    QObject::connect(ui->quitAction, SIGNAL(triggered()), this, SLOT(quit()));

    autoScanDeviceTimerID=startTimer(1000);
}
/**
  * @brief  ��ʱ��ʱ�亯��
  * @param  event ��ʱ��ָ��
  * @retval None
  */
void MainWindow::timerEvent(QTimerEvent *event)
{
    int vt_board_num,i;
    char deviceName[100]={0};
    QStringList addDeviceList;
    QString stateStr;
    //Ѱ���豸
    if(event->timerId()==autoScanDeviceTimerID){
        vt_board_num=UART_ScanDevice(1);
        if(vt_board_num!=deviceNum){//����һ���豸
            for(i=0;i<vt_board_num;i++){
                sprintf(deviceName,"DEVICE %d",i);
                addDeviceList.push_back(deviceName);
            }
            ui->deviceComboBox->clear();
            ui->deviceComboBox->addItems(addDeviceList);
            if(vt_board_num>0){
            }else{
                ui->deviceComboBox->addItem("NO DEVICE");
            }
        }
        deviceNum=vt_board_num;
        stateStr.sprintf("�豸��������%d",deviceNum);
        ui->statusBar->showMessage(stateStr);
    }
    //��ʱ�������ݲ���ʾ
    if(event->timerId()==mesureTimerID){
        int ret;
        uint16_t Data;
        uint8_t dataTmp[1024];
        uint16_t Len = 0;
        ret = VAI_ReadDatas(1,
                            ui->deviceComboBox->currentIndex(),//�豸������
                            1,
                            &Data
                            );
        if(ret==ERR_SUCCESS){
            double PM25DataTmp = (Data*3.3/4096)*0.2*1000;
            QString dataStrTemp;
            for(int i=0;i<SIMPLE_NUMS-1;i++){
                PM25ADData[i]=PM25ADData[i+1];
            }
            PM25ADData[SIMPLE_NUMS-1]=PM25DataTmp;
            displayCurve(ui->qwtPlot,PM25ADCurve,PM25ADData,SIMPLE_NUMS);//��ʾ����
            dataStrTemp.sprintf("%.2f",PM25DataTmp);
            ui->PM25ADLabel->setText(dataStrTemp);
        }
        ret = UART_ReadBytes(ui->deviceComboBox->currentIndex(),
                             0,
                             dataTmp,
                             &Len);
        if(Len > 0){
            qDebug()<<"Len = "<<Len;
            PM25ADDataBuffer.append((const char*)dataTmp,Len);
            for(int i=0;(i<PM25ADDataBuffer.length())&&(PM25ADDataBuffer.length()>=7);){
                if((PM25ADDataBuffer.at(i)==(char)0xAA)&&(PM25ADDataBuffer.at(i+6)==(char)0xFF)){
                    double PM25DataTmp = (((uint8_t)PM25ADDataBuffer.at(i+1)*256+(uint8_t)PM25ADDataBuffer.at(i+2))*5/1024.0)*0.2*1000;
                    qDebug()<<"PM25DataTmp = "<<PM25DataTmp;
                    QString dataStrTemp;
                    for(int i=0;i<SIMPLE_NUMS-1;i++){
                        PM25Data[i]=PM25Data[i+1];
                    }
                    PM25Data[SIMPLE_NUMS-1]=PM25DataTmp;
                    displayCurve(ui->qwtPlot,PM25Curve,PM25Data,SIMPLE_NUMS);//��ʾ����
                    dataStrTemp.sprintf("%.2f",PM25DataTmp);
                    ui->PM25Label->setText(dataStrTemp);
                    PM25ADDataBuffer.clear();
                    break;
                }else{
                    i++;
                }
            }
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
/**
  * @brief  ��ʼ���豸
  * @param  None
  * @retval None
  */
void MainWindow::initDevice()
{
    int ret;
    UART_INIT_CONFIG InitConfig;
    //ѡ����Ҫ�������ݵ��豸
    if(ui->deviceComboBox->currentText()!="NO DEVICE"){
        ret = UART_OpenDevice(ui->deviceComboBox->currentIndex(),0);
        if(ret!=ERR_SUCCESS){
            QMessageBox::warning(this,"����","���豸ʧ�ܣ�");
            return;
        }else{
            InitConfig.BaudRate = 2400;
            InitConfig.Parity = 0;
            InitConfig.RS485Mode = 232;
            InitConfig.StopBits = 0;
            InitConfig.WordLength = 8;
            ret = UART_InitDevice(ui->deviceComboBox->currentIndex(),
                                  0,
                                  &InitConfig);
            if(ret != ERR_SUCCESS){
                QMessageBox::warning(this,"����","��ʼ���豸ʧ�ܣ�");
                return;
            }
        }
        ret = VAI_InitADC(1,
                          ui->deviceComboBox->currentIndex(),
                          VAI_ADC_CH0,
                          0);
        if(ret!=ERR_SUCCESS){
            QMessageBox::warning(this,"����","��ʼ���豸ʧ�ܣ�");
            return;
        }
    }
}

/**
  * @brief  �����������ݻ�ֹͣ��������
  * @param  None
  * @retval None
  */
void MainWindow::on_startPushButton_clicked()
{
    if(ui->startPushButton->text()=="��ʼ����"){
        initDevice();
        mesureTimerID = startTimer(50);
        ui->startPushButton->setText("ֹͣ����");
    }else{
        killTimer(mesureTimerID);
        ui->startPushButton->setText("��ʼ����");
    }
}
/**
  * @brief  ��ʼ��������ʾ
  * @param  None
  * @retval None
  */
void MainWindow::PM25Populate(void)
{
    // Insert new curves
    PM25Curve = new QwtPlotCurve( "����ֵ" );
    PM25Curve->setRenderHint( QwtPlotItem::RenderAntialiased );
    PM25Curve->setLegendAttribute( QwtPlotCurve::LegendShowLine, true );
    PM25Curve->setPen( QPen( QColor(255,0,0) ) );
    PM25Curve->attach(ui->qwtPlot);

    PM25ADCurve = new QwtPlotCurve( "ģ��ֵ" );
    PM25ADCurve->setRenderHint( QwtPlotItem::RenderAntialiased );
    PM25ADCurve->setLegendAttribute( QwtPlotCurve::LegendShowLine, true );
    PM25ADCurve->setPen( QPen( QColor(0,255,0) ) );
    PM25ADCurve->attach( ui->qwtPlot );
}

/**
  * @brief  ��ʾ��������
  * @param  pCurve ����ָ��
  * @param  pCurveData Y������
  * @param  dataNum ���ݸ���
  * @retval None
  */
void MainWindow::displayCurve(QwtPlot *plot,QwtPlotCurve *pCurve,double *pCurveData,int dataNum)
{
    double *pCurveXData = (double*)malloc(dataNum*sizeof(double));
    for(int i=0;i<dataNum;i++){
        pCurveXData[i] = i;
    }
    pCurve->setSamples(pCurveXData,pCurveData,dataNum);
    plot->setAutoReplot(true);
    plot->replot();
    free(pCurveXData);
}
/**
  * @brief  ��ʼ��������ʾ�ؼ�
  * @param  pPlot ��ʾ���εĿؼ�
  * @retval None
  */
void MainWindow::PM25PlotInit(QwtPlot *pPlot)
{
    // panning with the left mouse button
    ( void ) new QwtPlotPanner( pPlot->canvas() );

    // zoom in/out with the wheel
    ( void ) new QwtPlotMagnifier( pPlot->canvas() );

    pPlot->setAutoFillBackground( true );
    pPlot->setPalette( QPalette( QColor( 255, 255, 255 ) ) );
    //pPlot->updateGradient();

    //pPlot->setTitle( "A Simple QwtPlot Demonstration" );
    pPlot->insertLegend( new QwtLegend(), QwtPlot::RightLegend );

    // axes
    pPlot->setAxisScale( pPlot->xBottom, 0.0, SIMPLE_NUMS );

    pPlot->setAxisScale( pPlot->yLeft, 0, 700);

    // canvas
    //pPlot->canvas()->setLineWidth( 1 );
    //pPlot->canvas()->setFrameStyle( QFrame::Box | QFrame::Plain );
    //pPlot->canvas()->setBorderRadius( 15 );

    QPalette canvasPalette( Qt::black );
    canvasPalette.setColor( QPalette::Foreground, QColor( 133, 190, 232 ) );
    pPlot->canvas()->setPalette( canvasPalette );

    PM25Populate();
}

void MainWindow::about()
{
    QString AboutStr;
    AboutStr = "Ginkgo Adapter GP2Y1050AU0F 1.0.0\n";
    AboutStr.append("�ڽ�γͼ�����������޹�˾ 2009-2013 ��Ȩ����\n");
    AboutStr.append("֧��Ӳ����Ginkgo 2 Adapter");
    QMessageBox::about(this,"����Ginkgo Adapter GP2Y1050AU0F",AboutStr);
}
void MainWindow::contactus()
{
    QString AboutStr;
    AboutStr.append("�ٷ���վ<span style=\"font-size:12px;\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>��<a href=\"http://www.viewtool.com\">www.viewtool.com</a><br>");
    AboutStr.append("�ٷ���̳<span style=\"font-size:12px;\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>��<a href=\"http://www.viewtool.com/bbs\">www.viewtool.com/bbs</a><br>");
    AboutStr.append("�ٷ��Ա���<span style=\"font-size:9px;\">&nbsp;&nbsp;&nbsp;</span>��<a href=\"http://viewtool.taobao.com/\">viewtool.taobao.com</a><br>");
    AboutStr.append("����֧��QQ��1713047813<br>");
    AboutStr.append("��Ʒ��ѯQQ��191431236");
    QMessageBox::about(this,"��ϵ����",AboutStr);
}
void MainWindow::quit()
{
    this->close();
}
