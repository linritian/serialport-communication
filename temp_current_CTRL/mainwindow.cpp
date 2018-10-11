#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QThread>
#include <QButtonGroup>
#include <QObject>

QByteArray MainWindow::rxbuf;
QByteArray MainWindow::txbuf;
QByteArray MainWindow::rxdata;
QByteArray MainWindow::rxdata_curr;
QTimer *m_Timer = new QTimer();
static bool status_flag = false;
int8_t curr_flag;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{    ui->setupUi(this);
     setMinimumSize(600, 400);//设置主窗体大小
     setWindowTitle(tr("TempCurrentCtrl"));
     ui->lineEdit->setText(QString("25.50"));
     ui->lineEdit_2->setText(QString("2000"));
     ui->lineEdit_3->setText(QString("400.0"));
     ui->lcdNumber->display("0.00 ");
     ui->lcdNumber_2->display("0.00 ");
     ui->lcdNumber_3->display("0.00 ");
     ui->lineEdit_4->setText(QString("100.0"));
     ui->lineEdit_5->setText(QString("200.0"));
     ui->lineEdit_6->setText(QString("300.0"));
     ui->lineEdit_7->setText(QString("400.0"));
     ui->lineEdit_8->setText(QString("500.0"));
     ui->lineEdit_9->setText(QString("600.0"));
     ui->lineEdit_10->setText(QString("700.0"));
     ui->lineEdit_11->setText(QString("800.0"));


     foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
     {
         qDebug() << "Name        : " << info.portName();
         qDebug() << "Description : " << info.description();
         qDebug() << "Manufacturer: " << info.manufacturer();
         QSerialPort serial;
         serial.setPort(info);
         if (serial.open(QIODevice::ReadWrite))
         {
             ui->comboBox->addItem(info.portName());
             serial.close();
         }
     }
    lcdNumber_refresh();
}

//打开串口
void MainWindow::on_pushButton_7_clicked()
{
    static bool btnflag = false;
    if(btnflag == false)
    {
        m_serialPort.setPortName(ui->comboBox->currentText());
        m_serialPort.setBaudRate(115200);
        m_serialPort.setDataBits(QSerialPort::Data8);
        m_serialPort.setStopBits(QSerialPort::OneStop);
        m_serialPort.setParity(QSerialPort::NoParity);
        m_serialPort.setFlowControl(QSerialPort::NoFlowControl);

        if(m_serialPort.open(QIODevice::ReadWrite) != true)
        {
            QMessageBox::information(this,tr("error"),tr("未找到设备，请重试！！"));
            return;
        }
    }
    btnflag = true;
}

//刷新串口端口号
void MainWindow::on_pushButton_6_clicked()
{
    ui->comboBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        qDebug() << "Name        : " << info.portName();
        qDebug() << "Description : " << info.description();
        qDebug() << "Manufacturer: " << info.manufacturer();
        QSerialPort serial;
        serial.setPort(info);
        if (serial.open(QIODevice::ReadWrite))
        {
            ui->comboBox->addItem(info.portName());
            serial.close();
        }
    }
}

void MainWindow::on_lineEdit()
{
     //当linedit有输入时开启按钮才可使用
    if(m_serialPort.open(QIODevice::ReadWrite) == true)
    {
        if((ui->lineEdit->hasAcceptableInput()==true && ui->lineEdit_2->hasAcceptableInput()) == true)
        {
            pushButton->setEnabled(true);
        }
    }
 }



//温控模块设置
void MainWindow::on_pushButton_clicked()
{
    status_flag = true;
    m_Timer->stop();
    rxbuf.clear();
    QString str = ui->lineEdit->text();//从LineEdit得到字符串
    QString str1 = ui->lineEdit_2->text();//从LineEdit_2得到字符串
    if((!str.isEmpty()) && (!str1.isEmpty()) && status_flag == true)
    {
        float sendData = str.toFloat();
        //将输入的温度值转为十六进制高低八位
        float sendData_temp2 = floor(sendData);
        float sendData_temp3 = sendData - sendData_temp2;
        int sendData_temp = sendData_temp3 * 100;

        QString sendData_temp1 = QString("%1").arg(sendData_temp2,  0, QLatin1Char('0'));
        QString sendData_temp4 = QString("%1").arg(sendData_temp,  0, QLatin1Char('0'));

        QByteArray temp = sendData_temp1.toLatin1();
        char* h_temp_set=temp.data();

        QByteArray temp1 = sendData_temp4.toLatin1();
        char* l_temp_set=temp1.data();


        //将输入的电流值转为十六进制高低八位
        int sendData1 = str1.toInt();
        QString sendData1_curr = QString("%1").arg(sendData1,2,16 ,QLatin1Char('0'));
        QString sendData1_curr1 = sendData1_curr.mid(0, 1);
        QString sendData1_curr2 = sendData1_curr.mid(1);
        bool ok;
        int hex =sendData1_curr1.toInt(&ok,16);
        int hex1 =sendData1_curr2.toInt(&ok,16);
        char h_curr_send = *(char*)&hex;
        char l_curr_send = *(char*)&hex1;

         //将温度设定值发送至上位机
         rxbuf = m_serialPort.readAll();
         rxbuf.clear();
         txbuf.resize(5);
         txbuf[0] = 0xC0;
         txbuf[1] = 0x82;
         txbuf[2] = *h_temp_set;
         txbuf[3] = *l_temp_set;
         txbuf[4] = 0x00;
         m_serialPort.write(txbuf,5);
         m_serialPort.waitForBytesWritten(500);
         QThread::msleep(500);


         //将电流设定值发送至上位机
         rxbuf = m_serialPort.readAll();
         unsigned char rxbuf_set_flag = int(0xC0);
         if(rxbuf[0] = rxbuf_set_flag)
         {
             txbuf.resize(5);
             txbuf[0] = 0xC0;
             txbuf[1] = 0x88;
             txbuf[2] = h_curr_send;
             txbuf[3] = l_curr_send;
             txbuf[4] = 0x00;
             m_serialPort.write(txbuf,5);
             m_serialPort.waitForBytesWritten(500);
             QThread::msleep(500);
         }
         //开启TEC
         QByteArray txbuf_start;
         txbuf_start.resize(2);
         txbuf_start[0] = 0xCC;
         txbuf_start[1] = 0x00;
         rxbuf = m_serialPort.readAll();
         if(rxbuf[0] = 0xC0)
         {
             m_serialPort.write(txbuf_start,2);
         }
    }
    else
    {
        QMessageBox::information(this,tr("error"),tr("请设置正确的温度和电流！！"));
        return;
    }
    status_flag = false;
    m_Timer->start();
}

//关闭TEC
void MainWindow::on_pushButton_2_clicked()
{
    QByteArray txbuf_close;
    txbuf_close.resize(2);
    txbuf_close[0] = 0xCC;
    txbuf_close[1] = 0xFF;
    m_serialPort.write(txbuf_close);
}
void MainWindow::on_pushButton_8_clicked()
{
    status_flag = true;
    m_Timer->stop();
    rxbuf = m_serialPort.readAll();
    rxbuf.clear();
    QByteArray txbuf_init;
    txbuf_init.resize(1);
    txbuf_init[0] = 0xA1;
    m_serialPort.write(txbuf_init,1);
    m_serialPort.waitForReadyRead(500);
    rxbuf = m_serialPort.readAll();
    unsigned char rxbuf_read_flag = int(0xA1);
    if(rxbuf[0] = rxbuf_read_flag)
    {
        //脉冲一读取
        int curr1_config_msb = (unsigned char) rxbuf.at(4);
        int curr1_config_lsb = (unsigned char) rxbuf.at(5);
        int data1_sum= curr1_config_msb<<8|curr1_config_lsb;
        float data1_sum1 = data1_sum;
        float curr_config_1 = data1_sum1 / 10;
        QString curr_pulse1 = QString::number(curr_config_1);
        ui->lineEdit_4->setText(curr_pulse1);
        //脉冲二读取
        int curr2_config_msb = (unsigned char) rxbuf.at(6);
        int curr2_config_lsb = (unsigned char) rxbuf.at(7);
        int data2_sum= curr2_config_msb<<8|curr2_config_lsb;
        float data2_sum1 = data2_sum;
        float curr_config_2 = data2_sum1 / 10;
        QString curr_pulse2 = QString::number(curr_config_2);
        ui->lineEdit_5->setText(curr_pulse2);
        //脉冲三读取
        int curr3_config_msb = (unsigned char) rxbuf.at(8);
        int curr3_config_lsb = (unsigned char) rxbuf.at(9);
        int data3_sum= curr3_config_msb<<8|curr3_config_lsb;
        float data3_sum1 = data3_sum;
        float curr_config_3 = data3_sum1 / 10;
        QString curr_pulse3 = QString::number(curr_config_3);
        ui->lineEdit_6->setText(curr_pulse3);
        //脉冲四读取
        int curr4_config_msb = (unsigned char) rxbuf.at(10);
        int curr4_config_lsb = (unsigned char) rxbuf.at(11);
        int data4_sum= curr4_config_msb<<8|curr4_config_lsb;
        float data4_sum1 = data4_sum;
        float curr_config_4 = data4_sum1 / 10;
        QString curr_pulse4 = QString::number(curr_config_4);
        ui->lineEdit_7->setText(curr_pulse4);
        //脉冲五读取
        int curr5_config_msb = (unsigned char) rxbuf.at(12);
        int curr5_config_lsb = (unsigned char) rxbuf.at(13);
        int data5_sum= curr5_config_msb<<8|curr5_config_lsb;
        float data5_sum1 = data5_sum;
        float curr_config_5 = data5_sum1 / 10;
        QString curr_pulse5 = QString::number(curr_config_5);
        ui->lineEdit_8->setText(curr_pulse5);
        //脉冲六读取
        int curr6_config_msb = (unsigned char) rxbuf.at(14);
        int curr6_config_lsb = (unsigned char) rxbuf.at(15);
        int data6_sum= curr6_config_msb<<8|curr6_config_lsb;
        float data6_sum1 = data6_sum;
        float curr_config_6 = data6_sum1 / 10;
        QString curr_pulse6 = QString::number(curr_config_6);
        ui->lineEdit_9->setText(curr_pulse6);
        //脉冲七读取
        int curr7_config_msb = (unsigned char) rxbuf.at(16);
        int curr7_config_lsb = (unsigned char) rxbuf.at(17);
        int data7_sum= curr7_config_msb<<8|curr7_config_lsb;
        float data7_sum1 = data7_sum;
        float curr_config_7 = data7_sum1 / 10;
        QString curr_pulse7 = QString::number(curr_config_7);
        ui->lineEdit_10->setText(curr_pulse7);
        //脉冲八读取
        int curr8_config_msb = (unsigned char) rxbuf.at(18);
        int curr8_config_lsb = (unsigned char) rxbuf.at(19);
        int data8_sum= curr8_config_msb<<8|curr8_config_lsb;
        float data8_sum1 = data8_sum;
        float curr_config_8 = data8_sum1 / 10;
        QString curr_pulse8 = QString::number(curr_config_8);
        ui->lineEdit_11->setText(curr_pulse8);
    }
    rxbuf.clear();



}

QByteArray MainWindow::input_convert(QString settext)
{
    //将输入的值转为十六进制高低八位
    QByteArray setinputsend;
    QString setinput_curr2;
    QString setinput_curr3;
    setinputsend.resize(2);
    float setinput = settext.toFloat();
    float setinput1 = setinput * 10;
    int setinput2 = *(float*)&setinput1;
    QString setinput_curr = QString("%1").arg(setinput2,2,16 ,QLatin1Char('0'));
    if(setinput2 > 4095)
    {
        setinput_curr2 = setinput_curr.mid(0, 2);
        setinput_curr3 = setinput_curr.mid(2);
    }
    else
    {
        setinput_curr2 = setinput_curr.mid(0, 1);
        setinput_curr3 = setinput_curr.mid(1);
    }
    bool ok;
    int hex =setinput_curr2.toInt(&ok,16);
    int hex1 =setinput_curr3.toInt(&ok,16);
    char h_set_send = *(char*)&hex;
    char l_set_send = *(char*)&hex1;
    setinputsend[0] = h_set_send;
    setinputsend[1] = l_set_send;
    return setinputsend;
}


//设置电流模块
void MainWindow::on_pushButton_3_clicked()
{
    QByteArray set_temp;

    QString str3 = ui->lineEdit_3->text();//从LineEdit_3得到字符串
    QString str4 = ui->lineEdit_4->text();//从脉冲一得到字符串
    QString str5 = ui->lineEdit_5->text();//从脉冲二得到字符串
    QString str6 = ui->lineEdit_6->text();//从脉冲三得到字符串
    QString str7 = ui->lineEdit_7->text();//从脉冲四得到字符串
    QString str8 = ui->lineEdit_8->text();//从脉冲五得到字符串
    QString str9 = ui->lineEdit_9->text();//从脉冲六得到字符串
    QString str10 = ui->lineEdit_10->text();//从脉冲七得到字符串
    QString str11 = ui->lineEdit_11->text();//从脉冲八得到字符串
    QString text4,text5,text6,text7,text8,text9,text10,text11;
    QButtonGroup *bg=new QButtonGroup(this);
    bg->addButton(ui->radioButton,0);//值为0
    bg->addButton(ui->radioButton_2,1);//值为1
    bg->addButton(ui->radioButton_5,2);//值为2
    bg->addButton(ui->radioButton_6,3);//值为3
    bg->addButton(ui->radioButton_7,4);//值为4
    bg->addButton(ui->radioButton_8,5);//值为5
    bg->addButton(ui->radioButton_9,6);//值为6
    bg->addButton(ui->radioButton_10,7);//值为7
    int btnid=bg->checkedId();
    switch(btnid)
    {
        case 0:
            text4 = ui->lineEdit_4->text();
            ui->lineEdit_3->setText(text4);
            str4 = ui->lineEdit_3->text();
            break;
        case 1:
            text5 = ui->lineEdit_5->text();
            ui->lineEdit_3->setText(text5);
            str5 = ui->lineEdit_3->text();
            break;
        case 2:
            text6 = ui->lineEdit_6->text();
            ui->lineEdit_3->setText(text6);
            str6 = ui->lineEdit_3->text();
            break;
        case 3:
            text7 = ui->lineEdit_7->text();
            ui->lineEdit_3->setText(text7);
            str7 = ui->lineEdit_3->text();
            break;
        case 4:
            text8 = ui->lineEdit_8->text();
            ui->lineEdit_3->setText(text8);
            str8 = ui->lineEdit_3->text();
            break;
        case 5:
            text9 = ui->lineEdit_9->text();
            ui->lineEdit_3->setText(text9);
            str9 = ui->lineEdit_3->text();
            break;
        case 6:
            text10 = ui->lineEdit_10->text();
            ui->lineEdit_3->setText(text10);
            str10 = ui->lineEdit_3->text();
            break;
        case 7:
            text11 = ui->lineEdit_11->text();
            ui->lineEdit_3->setText(text11);
            str11 = ui->lineEdit_3->text();
            break;
    }

    if((!str3.isEmpty()))
    {
        status_flag = true;
        m_Timer->stop();
        rxbuf = m_serialPort.readAll();
        rxbuf.clear();
        //恒流电流模式电流设置
        QByteArray constant_curr;
        constant_curr = input_convert(str3);
        char h_curr3_send = constant_curr[0];
        char l_curr3_send = constant_curr[1];


        //脉冲一设置
        QByteArray curr_1;
        curr_1 = input_convert(str4);
        char h_curr4_send = curr_1[0];
        char l_curr4_send = curr_1[1];


        //脉冲二设置
        QByteArray curr_2;
        curr_2 = input_convert(str5);
        char h_curr5_send = curr_2[0];
        char l_curr5_send = curr_2[1];


        //脉冲三设置
        QByteArray curr_3;
        curr_3 = input_convert(str6);
        char h_curr6_send = curr_3[0];
        char l_curr6_send = curr_3[1];


        //脉冲四设置
        QByteArray curr_4;
        curr_4 = input_convert(str7);
        char h_curr7_send = curr_4[0];
        char l_curr7_send = curr_4[1];

        //脉冲五设置
        QByteArray curr_5;
        curr_5 = input_convert(str8);
        char h_curr8_send = curr_5[0];
        char l_curr8_send = curr_5[1];


        //脉冲六设置
        QByteArray curr_6;
        curr_6 = input_convert(str9);
        char h_curr9_send = curr_6[0];
        char l_curr9_send = curr_6[1];

        //脉冲七设置
        QByteArray curr_7;
        curr_7 = input_convert(str10);
        char h_curr10_send = curr_7[0];
        char l_curr10_send = curr_7[1];

        //脉冲八设置
        QByteArray curr_8;
        curr_8 = input_convert(str11);
        char h_curr11_send = curr_8[0];
        char l_curr11_send = curr_8[1];


        set_temp.resize(21);
        set_temp[0] = 0xA0;
        set_temp[1] = 0x00;
        set_temp[2] = h_curr3_send;
        set_temp[3] = l_curr3_send;
        set_temp[4] = h_curr4_send;
        set_temp[5] = l_curr4_send;
        set_temp[6] = h_curr5_send;
        set_temp[7] = l_curr5_send;
        set_temp[8] = h_curr6_send;
        set_temp[9] = l_curr6_send;
        set_temp[10] = h_curr7_send;
        set_temp[11] = l_curr7_send;
        set_temp[12] = h_curr8_send;
        set_temp[13] = l_curr8_send;
        set_temp[14] = h_curr9_send;
        set_temp[15] = l_curr9_send;
        set_temp[16] = h_curr10_send;
        set_temp[17] = l_curr10_send;
        set_temp[18] = h_curr11_send;
        set_temp[19] = l_curr11_send;
        set_temp[20] = 0x00;
        m_serialPort.write(set_temp);
        set_temp.clear();
        m_serialPort.waitForBytesWritten(500);
        QThread::msleep(100);


        QByteArray txbuf_start_1;
        txbuf_start_1.resize(3);
        txbuf_start_1[0] = 0xAA;
        txbuf_start_1[1] = 0xFF;
        txbuf_start_1[2] = 0x01;

        rxbuf = m_serialPort.readAll();
        unsigned char rxbuf_trigread_flag = int(0xAA);
        if(rxbuf[0] = rxbuf_trigread_flag)
        {
            m_serialPort.write(txbuf_start_1,3);
            txbuf_start_1.clear();
        }
    }
    else
    {
        QMessageBox::information(this,tr("error"),tr("请输入数值！！"));
        return;
    }
    status_flag = false;
    m_Timer->start();
    curr_flag = 1;
}

//关闭电流输出
void MainWindow::on_pushButton_4_clicked()
{
    QByteArray txbuf_temp_close;
    txbuf_temp_close.resize(2);
    txbuf_temp_close[0] = 0xAA;
    txbuf_temp_close[1] = 0x00;
    m_serialPort.write(txbuf_temp_close);
    txbuf_temp_close.clear();
    curr_flag = 0;
}

//脉冲模式设置
void MainWindow::on_pushButton_5_clicked()
{
    QByteArray set_temp;

    QString str3 = ui->lineEdit_3->text();//从LineEdit_3得到字符串
    QString str4 = ui->lineEdit_4->text();//从脉冲一得到字符串
    QString str5 = ui->lineEdit_5->text();//从脉冲二得到字符串
    QString str6 = ui->lineEdit_6->text();//从脉冲三得到字符串
    QString str7 = ui->lineEdit_7->text();//从脉冲四得到字符串
    QString str8 = ui->lineEdit_8->text();//从脉冲五得到字符串
    QString str9 = ui->lineEdit_9->text();//从脉冲六得到字符串
    QString str10 = ui->lineEdit_10->text();//从脉冲七得到字符串
    QString str11 = ui->lineEdit_11->text();//从脉冲八得到字符串
    if((!str3.isEmpty()))
    {
        status_flag = true;
        m_Timer->stop();
        rxbuf = m_serialPort.readAll();
        rxbuf.clear();
        //恒流电流模式电流设置
        QByteArray constant_curr;
        constant_curr = input_convert(str3);
        char h_curr3_send = constant_curr[0];
        char l_curr3_send = constant_curr[1];


        //脉冲一设置
        QByteArray curr_1;
        curr_1 = input_convert(str4);
        char h_curr4_send = curr_1[0];
        char l_curr4_send = curr_1[1];


        //脉冲二设置
        QByteArray curr_2;
        curr_2 = input_convert(str5);
        char h_curr5_send = curr_2[0];
        char l_curr5_send = curr_2[1];


        //脉冲三设置
        QByteArray curr_3;
        curr_3 = input_convert(str6);
        char h_curr6_send = curr_3[0];
        char l_curr6_send = curr_3[1];


        //脉冲四设置
        QByteArray curr_4;
        curr_4 = input_convert(str7);
        char h_curr7_send = curr_4[0];
        char l_curr7_send = curr_4[1];

        //脉冲五设置
        QByteArray curr_5;
        curr_5 = input_convert(str8);
        char h_curr8_send = curr_5[0];
        char l_curr8_send = curr_5[1];


        //脉冲六设置
        QByteArray curr_6;
        curr_6 = input_convert(str9);
        char h_curr9_send = curr_6[0];
        char l_curr9_send = curr_6[1];

        //脉冲七设置
        QByteArray curr_7;
        curr_7 = input_convert(str10);
        char h_curr10_send = curr_7[0];
        char l_curr10_send = curr_7[1];

        //脉冲八设置
        QByteArray curr_8;
        curr_8 = input_convert(str11);
        char h_curr11_send = curr_8[0];
        char l_curr11_send = curr_8[1];


        set_temp.resize(21);
        set_temp[0] = 0xA0;
        set_temp[1] = 0x00;
        set_temp[2] = h_curr3_send;
        set_temp[3] = l_curr3_send;
        set_temp[4] = h_curr4_send;
        set_temp[5] = l_curr4_send;
        set_temp[6] = h_curr5_send;
        set_temp[7] = l_curr5_send;
        set_temp[8] = h_curr6_send;
        set_temp[9] = l_curr6_send;
        set_temp[10] = h_curr7_send;
        set_temp[11] = l_curr7_send;
        set_temp[12] = h_curr8_send;
        set_temp[13] = l_curr8_send;
        set_temp[14] = h_curr9_send;
        set_temp[15] = l_curr9_send;
        set_temp[16] = h_curr10_send;
        set_temp[17] = l_curr10_send;
        set_temp[18] = h_curr11_send;
        set_temp[19] = l_curr11_send;
        set_temp[20] = 0x00;
        m_serialPort.write(set_temp);
        set_temp.clear();
        m_serialPort.waitForBytesWritten(500);
        QThread::msleep(100);
    }

    //内时钟触发和外时钟触发
    rxbuf = m_serialPort.readAll();
    rxbuf.clear();
    QByteArray txbuf_trig;
    QButtonGroup *bg1=new QButtonGroup(this);
    bg1->addButton(ui->radioButton_3,0);//一个值为0
    bg1->addButton(ui->radioButton_4,1);//一个值为1
    int sel=bg1->checkedId();
    char rxbuf_trig_flag = int(0xAA);
    switch(sel)
    {
        case 0://内触发脉冲模式
            txbuf_trig.resize(3);
            txbuf_trig[0] = 0xAA;
            txbuf_trig[1] = 0xFF;
            txbuf_trig[2] = 0x00;
            m_serialPort.write(txbuf_trig);
            txbuf_trig.clear();
            m_serialPort.readAll();
            if(rxbuf[0] == rxbuf_trig_flag)
            {
               curr_flag = 0;
               rxbuf.clear();
            }
            status_flag = false;
            m_Timer->start();
            break;
        case 1://外触发脉冲模式
            txbuf_trig.resize(3);
            txbuf_trig[0] = 0xAA;
            txbuf_trig[1] = 0xFF;
            txbuf_trig[2] = 0x02;
            m_serialPort.write(txbuf_trig);
            txbuf_trig.clear();
            m_serialPort.readAll();

            if(rxbuf[0] == rxbuf_trig_flag)
            {
               curr_flag = 0;
               rxbuf.clear();
            }
            status_flag = false;
            m_Timer->start();
            break;
        default:
            QMessageBox::information(this,tr("error"),tr("请选择触发方式！！"));
            return;
            break;
    }
    curr_flag = 0;
}

void MainWindow::lcdNumber_refresh()
{
    if(status_flag == false)
    {
        // 设置定时间隔
        m_Timer->setInterval(1000);
        connect(m_Timer, SIGNAL(timeout()), this, SLOT(onTimeOut()));
        // 启动定时器
        m_Timer->start();
    }
    else
    {
       m_Timer->stop();
    }
}

void MainWindow::onTimeOut()
{
    //查询温度并显示
    int timeout;
    txbuf.resize(2);
    txbuf[0] = 0xC2;
    txbuf[1] = 0x90;
    m_serialPort.write(txbuf);
    txbuf.clear();
    timeout = 0;
    while(m_serialPort.bytesAvailable()<5 && timeout < 100)
    {
        timeout++;
    }
    if(timeout < 100)
    {
        m_serialPort.waitForReadyRead(500);
        rxdata = m_serialPort.readAll();
        char radata_temp_flag = int(0xC2);
        if(rxdata[0] == radata_temp_flag)
        {
            int data_msb = rxdata[2];
            int data_lsb = rxdata[3];
            if(data_lsb < 10)
            {
                QString str_temp = QString::number(data_msb) + "." + "0" + QString::number(data_lsb);
                ui->lcdNumber->display(str_temp);
            }
            else
            {
                QString str_temp = QString::number(data_msb) + "." + QString::number(data_lsb);
                ui->lcdNumber->display(str_temp);
            }
            rxdata.clear();
        }
    }
    //查询电流和电压并显示
    if(curr_flag == 1)
    {
        txbuf.resize(1);
        txbuf[0] = 0xA2;
        m_serialPort.waitForBytesWritten(500);
        m_serialPort.write(txbuf);
        m_serialPort.waitForReadyRead(500);
        rxdata_curr = m_serialPort.readAll();
        char rxdata_curr_flag = int(0xA2);
        rxdata_curr.resize(7);
        if(rxdata_curr[0] == rxdata_curr_flag)
        {
            //显示电流
            int curr1_msb = (unsigned char) rxdata_curr.at(3);
            int curr1_lsb = (unsigned char) rxdata_curr.at(4);
            int curr1= curr1_msb<<8|curr1_lsb;
            float curr1_float = curr1;
            float curr_lcd = curr1_float / 10;
            if(curr_lcd >= 10  && curr_lcd <= 1500)
            {
                QString curr = QString::number(curr_lcd);
                ui->lcdNumber_2->display(curr);
            }
            //显示电压
            int volt_msb = (unsigned char) rxdata_curr.at(5);
            int volt_lsb = (unsigned char) rxdata_curr.at(6);
            int volt1= volt_msb<<8|volt_lsb;
            float volt = volt1;
            float volt_lcd = volt / 100;
            if(volt_lcd >= 2 && volt_lcd <=20)
            {
                QString str_volt = QString::number(volt_lcd);
                ui->lcdNumber_3->display(str_volt);
            }
            rxdata_curr.clear();
        }
    }
    if(curr_flag == 0)
    {
       ui->lcdNumber_2->display("0.00 ");
       ui->lcdNumber_3->display("0.00 ");
    }
}


MainWindow::~MainWindow()
{

}
