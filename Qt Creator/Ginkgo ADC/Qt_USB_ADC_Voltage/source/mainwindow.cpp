#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    autoScanDeviceTimerID = startTimer(1000);
    deviceNum = 0;
}

QString MainWindow::mytr(const char* chars)
{
    QTextCodec *codec = QTextCodec::codecForName("GB18030");
    return codec->toUnicode(chars);
}
void MainWindow::timerEvent(QTimerEvent *event)
{
    int vt_board_num,i;
    QString deviceName;
    QStringList addDeviceList;
    QString stateStr;
    if(event->timerId()==autoScanDeviceTimerID){
        vt_board_num=VAI_ScanDevice(1);
        if(vt_board_num!=deviceNum){//����һ���豸
            for(i=0;i<vt_board_num;i++){
                deviceName.sprintf("%d",i);
                addDeviceList.push_back(mytr("�豸")+deviceName);
            }
            ui->deviceSelectComboBox->clear();
            ui->deviceSelectComboBox->addItems(addDeviceList);
            if(vt_board_num>0){
                //ui->deviceComboBox->addItem("�����豸");
            }else{
                ui->deviceSelectComboBox->addItem(mytr("���豸����"));
            }
        }
        deviceNum=vt_board_num;
        stateStr.sprintf("%d",deviceNum);
        ui->statusBar->showMessage(mytr("�豸��������")+stateStr);
    }
    if(event->timerId()==autoGetDataTimerID){
        //��ȡ��Ҫ��ȡ���ݵ�ͨ����
        displayData(readData());
    }
}
void MainWindow::displayData(QStringList dataList)
{
    ui->ch0ValueLabel->setText(dataList.at(0));
    ui->ch1ValueLabel->setText(dataList.at(1));
    ui->ch2ValueLabel->setText(dataList.at(2));
    ui->ch3ValueLabel->setText(dataList.at(3));
    ui->ch4ValueLabel->setText(dataList.at(4));
    ui->ch5ValueLabel->setText(dataList.at(5));
    ui->ch6ValueLabel->setText(dataList.at(6));
    ui->ch7ValueLabel->setText(dataList.at(7));
}

unsigned char MainWindow::getChannelSelInfo(void)
{
    unsigned char Channel=0;
    if(ui->ch0EnableCheckBox->isChecked()){
        Channel |= (0x01<<0);
    }else{
        Channel &= ~(0x01<<0);
    }
    if(ui->ch1EnableCheckBox->isChecked()){
        Channel |= (0x01<<1);
    }else{
        Channel &= ~(0x01<<1);
    }
    if(ui->ch2EnableCheckBox->isChecked()){
        Channel |= (0x01<<2);
    }else{
        Channel &= ~(0x01<<2);
    }
    if(ui->ch3EnableCheckBox->isChecked()){
        Channel |= (0x01<<3);
    }else{
        Channel &= ~(0x01<<3);
    }
    if(ui->ch4EnableCheckBox->isChecked()){
        Channel |= (0x01<<4);
    }else{
        Channel &= ~(0x01<<4);
    }
    if(ui->ch5EnableCheckBox->isChecked()){
        Channel |= (0x01<<5);
    }else{
        Channel &= ~(0x01<<5);
    }
    if(ui->ch6EnableCheckBox->isChecked()){
        Channel |= (0x01<<6);
    }else{
        Channel &= ~(0x01<<6);
    }
    if(ui->ch7EnableCheckBox->isChecked()){
        Channel |= (0x01<<7);
    }else{
        Channel &= ~(0x01<<7);
    }
    return Channel;
}

int MainWindow::initDevice(void)
{
    //���豸��ѡ���豸
    int ret;
    ret=VAI_OpenDevice(VAI_USBADC,
                       ui->deviceSelectComboBox->currentIndex(),
                       0);
    if(ret!=ERR_SUCCESS){
        return ret;
    }
    //��ʼ���豸
    ret = VAI_InitADC(VAI_USBADC,
                      ui->deviceSelectComboBox->currentIndex(),
                      getChannelSelInfo(),
                      0x01);
    if(ret!=ERR_SUCCESS){
        return ret;
    }
    return 0;
}

QStringList MainWindow::readData(void)
{
    int ret;
    uint16_t adc_data[1024];
    QStringList adcDataStrList;
    ret = VAI_ReadDatas(VAI_USBADC,
                        ui->deviceSelectComboBox->currentIndex(),
                        1,
                        adc_data);
    if(ret!=ERR_SUCCESS){
        return adcDataStrList;
    }else{
        QString str;
        int i=0;
        if(ui->ch0EnableCheckBox->isChecked()){
            float readData = adc_data[i++]*3300.0/4096.0;
            str.sprintf("%.3f",readData);
            adcDataStrList.append(str);
        }else{
            adcDataStrList.append("None");
        }
        if(ui->ch1EnableCheckBox->isChecked()){
            float readData = adc_data[i++]*3300.0/4096.0;
            str.sprintf("%.3f",readData);
            adcDataStrList.append(str);
        }else{
            adcDataStrList.append("None");
        }
        if(ui->ch2EnableCheckBox->isChecked()){
            float readData = adc_data[i++]*3300.0/4096.0;
            str.sprintf("%.3f",readData);
            adcDataStrList.append(str);
        }else{
            adcDataStrList.append("None");
        }
        if(ui->ch3EnableCheckBox->isChecked()){
            float readData = adc_data[i++]*3300.0/4096.0;
            str.sprintf("%.3f",readData);
            adcDataStrList.append(str);
        }else{
            adcDataStrList.append("None");
        }
        if(ui->ch4EnableCheckBox->isChecked()){
            float readData = adc_data[i++]*3300.0/4096.0;
            str.sprintf("%.3f",readData);
            adcDataStrList.append(str);
        }else{
            adcDataStrList.append("None");
        }
        if(ui->ch5EnableCheckBox->isChecked()){
            float readData = adc_data[i++]*3300.0/4096.0;
            str.sprintf("%.3f",readData);
            adcDataStrList.append(str);
        }else{
            adcDataStrList.append("None");
        }
        if(ui->ch6EnableCheckBox->isChecked()){
            float readData = adc_data[i++]*3300.0/4096.0;
            str.sprintf("%.3f",readData);
            adcDataStrList.append(str);
        }else{
            adcDataStrList.append("None");
        }
        if(ui->ch7EnableCheckBox->isChecked()){
            float readData = adc_data[i++]*3300.0/4096.0;
            str.sprintf("%.3f",readData);
            adcDataStrList.append(str);
        }else{
            adcDataStrList.append("None");
        }
        return adcDataStrList;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_initDevPushButton_clicked()
{
    if(0==initDevice()){
        QMessageBox::information(this, mytr("��Ϣ"), mytr("��ʼ���豸�ɹ�!"), QMessageBox::Ok);
        ui->ch0EnableCheckBox->setDisabled(true);
        ui->ch1EnableCheckBox->setDisabled(true);
        ui->ch2EnableCheckBox->setDisabled(true);
        ui->ch3EnableCheckBox->setDisabled(true);
        ui->ch4EnableCheckBox->setDisabled(true);
        ui->ch5EnableCheckBox->setDisabled(true);
        ui->ch6EnableCheckBox->setDisabled(true);
        ui->ch7EnableCheckBox->setDisabled(true);
    }else{
        QMessageBox::warning(this, mytr("����"), mytr("��ʼ���豸ʧ��!"), QMessageBox::Ok);
    }
}

void MainWindow::on_startReadDataPushButton_clicked()
{
    if(ui->startReadDataPushButton->text()==mytr("��ʼ������")){
        bool ok;
        autoGetDataTimerID = startTimer(ui->dataUpdataLineEdit->text().toInt(&ok,0));
        ui->startReadDataPushButton->setText(mytr("ֹͣ������"));
    }else{
        killTimer(autoGetDataTimerID);
        ui->startReadDataPushButton->setText(mytr("��ʼ������"));
    }
}
