#include "widget.h"
#include "ui_widget.h"
#include <iconhelper.h>
#include "commonhelper.h"
#include <QSettings>
#include <QLibrary>
#include <QDir>

#include <windows.h>

int www;            //label自适应 宽
int hhh;            //label自适应 高
int mode;           //全屏切换

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);


    mode = 1;       //默认不全屏
    play_stop = true;
    Suodin = true;
    StepSize = 5;
    saveff = true;
    auto_send_count = 0;



    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    m_pTimer2 = new QTimer(this);
    connect(m_pTimer2, SIGNAL(timeout()), this, SLOT(handleTimeout2()));
    m_auto_show_tm = new QTimer(this);
    connect(m_auto_show_tm, SIGNAL(timeout()), this, SLOT(auto_show()));
    m_auto_send = new QTimer(this);
    connect(m_auto_send, SIGNAL(timeout()), this, SLOT(auto_send()));

    m_heartbeat = new QTimer(this);
    m_lastTime = QDateTime::currentDateTime();
    connect(m_heartbeat, &QTimer::timeout, this, &Widget::checkHeartbeat);

    ui->checkBox_2->setChecked(true);
    ui->checkBox_3->setChecked(false);
    ui->checkBox->setChecked(false);

    label_w = ui->label_play->width();
    label_h = ui->label_play->height();
    qDebug()<<label_w;
    qDebug()<<label_h;

    ui->label_play->changesize(label_w, label_h);

//    界面初始化
    ui_init();


//    视频保存路径 没有就新建
    QDir dir("./video/");
    if(!dir.exists())
    {
        dir.mkpath("./video/");
        qDebug()<<"新建目录成功";
    }



    //读取串口信息
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        {
            qDebug()<<"Name:"<<info.portName();
            qDebug()<<"Description:"<<info.description();
            qDebug()<<"Manufacturer:"<<info.manufacturer();

            //这里相当于自动识别串口号之后添加到了cmb，如果要手动选择可以用下面列表的方式添加进去
            QSerialPort serial;
            serial.setPort(info);
            if(serial.open(QIODevice::ReadWrite))
            {

                //将串口号添加到cmb
                ui->comboBox->addItem(info.portName());
                //关闭串口等待人为(打开串口按钮)打开
                serial.close();
            }
        }

        on_pushButton_8_clicked();

        //    tcp初始化
            TcpInit();
            connect(ui->label_play,SIGNAL(thread_quit()),this, SLOT(th_quit()));

            connect(ui->label_play, SIGNAL(Point(int,int,int,int)), this, SLOT(sendPoint(int,int,int,int)));

}

void Widget::TcpInit()
{
    server = new QTcpServer();
    socket = new QTcpSocket();

    server->listen(QHostAddress::Any, 6767);
    connect(server, SIGNAL(newConnection()), this, SLOT(NewConnect()));
    ui->state_lab->setText("控制服务端建立成功，等待连接···");

}

void Widget::auto_show()
{
    play_stop = 1;
    on_play_btn_clicked();

    m_auto_show_tm->stop();
}

void Widget::auto_send()
{
    QByteArray ba;
    ba[0] = 0x00;
    ba[1] = 0x00;

//    socket->write(ba);

}

void Widget::checkHeartbeat()
{
    if (m_lastTime.secsTo(QDateTime::currentDateTime()) >= 6)   //超过30s即为掉线,停止心跳
    {
        qDebug() << "heartbeat 超时, 即将断开连接";
        m_auto_send->stop();
//        auto_send_count = 0;
        Tcpclose();
        m_heartbeat->stop();
//        auto_send_count++;
    }
    else
    {
        qDebug() << "heartbeat";
        auto_send_count = 0;
    }

//    if(auto_send_count >= 2)
//    {
//        qDebug("连接断开或者网络故障");


//    }
}


void Widget::NewConnect()
{
    qDebug()<<"new connect";
    ui->state_lab->setText("新客户端连接成功");

    socket = server->nextPendingConnection();

    //切换为 可见光
    on_pushButton_6_clicked();
    ui->label_play->clear();
    ui->label_play->setText("正在自动连接···");
qDebug("1");
//    Sleep(3);
qDebug("2");
    //当有连接是就开始采集视频
    m_auto_show_tm->start(4000);
    m_auto_send->start(2000);
    m_heartbeat->setInterval(6000);

    connect(socket, SIGNAL(readyRead()), this, SLOT(socket_Read_Data()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(Tcpclose()));

}

void Widget::Tcpclose()
{
    ui->state_lab->setText("新客户端断开");

    if(socket->isOpen())
    {
        socket->deleteLater();
    }
    //连接断开时 停止视频采集
    play_stop = 0;
    on_play_btn_clicked();
    ui->label_play->clear();
    ui->label_play->setText("网络连接断开···");

}

void Widget::socket_Read_Data()
{
    QByteArray buffer;
    buffer = socket->readAll();
    qDebug()<<"len:"<<buffer.length();
    qDebug()<<"read:"<<buffer;
    if (!m_heartbeat->isActive())
    {
        m_heartbeat->start();
    }
    m_lastTime = QDateTime::currentDateTime();
}

//串口 暂时没用
//void Widget::UartInit()
//{
//    serial = new QSerialPort;
//    serial->setPortName("\\\\.\\com24");
//    bool open_flag=serial->open(QIODevice::ReadWrite);
//        if(!open_flag)
//        {
//             qDebug()<<"串口连接失败！";
//            return;
//        }
//    serial->setBaudRate(QSerialPort::Baud9600);//设置波特率为115200
//    serial->setDataBits(QSerialPort::Data8);//设置数据位8
//    serial->setParity(QSerialPort::NoParity); //校验位设置为0
//    serial->setStopBits(QSerialPort::OneStop);//停止位设置为1
//    serial->setFlowControl(QSerialPort::NoFlowControl);//设置为无流控制
//    connect(serial,SIGNAL(readyRead()),this,SLOT(UartReadData()));
//}

void Widget::UartInit()
{
    serial = new QSerialPort;
    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
                this, &Widget::handleSerialError);    //连接槽，串口出现问题连接到错误处理函数
//    QString uartPort = ui->uart_lineEdit->text();
    serial->setPortName(ui->comboBox->currentText() );
    bool open_flag;
    serial->setBaudRate(QSerialPort::Baud115200);//设置波特率为115200
    serial->setDataBits(QSerialPort::Data8);//设置数据位8
    serial->setParity(QSerialPort::NoParity); //校验位设置为0
    serial->setStopBits(QSerialPort::OneStop);//停止位设置为1
    serial->setFlowControl(QSerialPort::NoFlowControl);//设置为无流控制
    open_flag = serial->open(QIODevice::ReadWrite);
    if(!open_flag)
    {
         qDebug()<<"串口连接失败！";
        return;
    }
    else
    {
        serial->setBaudRate(QSerialPort::Baud115200);//设置波特率为115200
        serial->setDataBits(QSerialPort::Data8);//设置数据位8
        serial->setParity(QSerialPort::NoParity); //校验位设置为0
        serial->setStopBits(QSerialPort::OneStop);//停止位设置为1
        serial->setFlowControl(QSerialPort::NoFlowControl);//设置为无流控制
        qDebug()<<"串口连接！";
        connect(serial, SIGNAL(readyRead()), this, SLOT(UartReadData()));
    }
}

void Widget::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {

        qDebug()<<"串口连接中断，请检查是否正确连接！";

    if (serial->isOpen())
        {
            serial->close();
        }
    }
}

void Widget::UartReadData()
{
    QByteArray buf;
    buf = serial->readAll();
    if(!buf.isEmpty())
    {
        QString str/* = ui->textEdit->toPlainText()*/;
        str+=tr(buf);
        qDebug()<<str;

    }
    buf.clear();
}

//界面初始化
void Widget::ui_init()
{
    ReadSetTing();

    this->setProperty("form", true);
    this->setProperty("canMove", true);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    this->setWindowTitle("模板界面");

    //换肤 图标
    IconHelper::Instance()->setIcon(ui->change_color_btn, QChar(0xf2d2));
//    最小 图标
    IconHelper::Instance()->setIcon(ui->min_btn, QChar(0xf068));
//    最大 图标
    IconHelper::Instance()->setIcon(ui->max_btn, QChar(0xf067));
//    关闭 图标
    IconHelper::Instance()->setIcon(ui->close_btn, QChar(0xf00d));
//    播放 图标
    IconHelper::Instance()->setIcon(ui->play_btn, QChar(0xf04b), 18);
//    左 回退图标
    IconHelper::Instance()->setIcon(ui->left_btn, QChar(0xf04a), 15);
//    右 快进图标
    IconHelper::Instance()->setIcon(ui->right_btn, QChar(0xf04e), 15);

//    上 图标
    IconHelper::Instance()->setIcon(ui->up_btn, QChar(0xf062), 18);
//    下 图标
    IconHelper::Instance()->setIcon(ui->down_btn, QChar(0xf063), 18);
//    左 图标
    IconHelper::Instance()->setIcon(ui->left_btn_2, QChar(0xf060), 18);
//    右 图标
    IconHelper::Instance()->setIcon(ui->right_btn_2, QChar(0xf061), 18);

    QSize icoSize(32, 32);
    int icoWidth = 85;
    //设置顶部导航按钮
    QList<QToolButton *> tbtns = ui->widget_tool->findChildren<QToolButton *>();
    foreach (QToolButton *btn, tbtns)
    {
        btn->setIconSize(icoSize);
        btn->setMinimumWidth(icoWidth);
        btn->setCheckable(true);
        connect(btn, SIGNAL(clicked()), this, SLOT(buttonClick()));
    }

    ui->main_btn->click();/*默认主窗口*/

    qDebug()<<skin;

    if(skin == "white")
    {
        CommonHelper::setStyle(":/qss/mywhite.css");
    }
    else
    {
        CommonHelper::setStyle(":/qss/mycss.css");
    }

}

//主界面切换
void Widget::buttonClick()
{
    QToolButton *b = (QToolButton *)sender();
    QString name = b->text();

    QList<QToolButton *> tbtns = ui->widget->findChildren<QToolButton *>();
    foreach (QToolButton *btn, tbtns) {
        if (btn == b) {
            btn->setChecked(true);
        } else {
            btn->setChecked(false);
        }
    }

    if (name == "主界面") {
        ui->stackedWidget->setCurrentIndex(0);
    } else if (name == "设置") {
        ui->stackedWidget->setCurrentIndex(1);
    } else if (name == "帮助") {
        ui->stackedWidget->setCurrentIndex(2);
    }
}

//关闭窗口按钮
void Widget::on_close_btn_clicked()
{
    ui->label_play->stop(1);
    close();
}

//最大窗口按钮
void Widget::on_max_btn_clicked()
{
    static bool max = false;
    static QRect location = this->geometry();
    ui->label_play->clear();

    if (max)
    {
        this->setGeometry(location);
        int w = ui->label_play->width();
        int h = ui->label_play->height();
        qDebug("mode 1 label_w:%d", w);
        qDebug("mode 1 label_d:%d", h);
        ui->label_play->changesize(w, h);
        mode = 1;
    }
    else
    {
        location = this->geometry();
        this->setGeometry(qApp->desktop()->availableGeometry());
        int w = ui->label_play->width();
        int h = ui->label_play->height();
        qDebug("mode 2 label_w:%d", w);
        qDebug("mode 2 label_d:%d", h);
//        label_w = ui->
        www = w;
        hhh = h;
        mode = 2;
        ui->label_play->changesize(www, hhh);
    }
    this->setProperty("canMove", max);
    max = !max;
}

//最小按钮
void Widget::on_min_btn_clicked()
{
    showMinimized();
}

//换肤
void Widget::on_change_color_btn_clicked()
{
    if (skin == "white")
    {
        CommonHelper::setStyle(":/qss/mycss.css");
        skin = "black";
    }
    else
    {
        CommonHelper::setStyle(":/qss/mywhite.css");
        skin = "white";
    }
}

Widget::~Widget()
{
    WriteSetTing();
    delete ui;
}

//写程序状态信息
int Widget::WriteSetTing()
{
    QSettings setting("setting.ini",QSettings::IniFormat);

    //皮肤状态
    setting.beginGroup(tr("set"));//节点开始
    setting.setValue("skin",skin);//
    setting.endGroup();//节点结束

    //网络参数
    setting.beginGroup(tr("net"));//节点开始
    QString ip = ui->IP_lineEdit->text();
    QString port = ui->PORT_lineEdit->text();
    setting.setValue("ip",ip);//
    setting.setValue("port",port);//

    //自动存文件标志
    QString sav_file = QString::number(savefile);
    setting.setValue("sav_file",sav_file);//

    //跟踪参数
    QString str1 = ui->win_height->text();
    QString str2 = ui->win_width->text();
    QString str3 = ui->grid_shift_step->text();
    QString str4 = ui->grid_num_scale_scan->text();
    QString str5 = ui->var_thr_coef->text();
    QString str6 = ui->var_update_coef->text();
    QString str7 = ui->fern_num->text();
    QString str8 = ui->fern_bit_num->text();
    QString str9 = ui->nn_max_input->text();
    QString str10 = ui->nn_patch_width->text();
    QString str11 = ui->nn_patch_height->text();
    QString str12 = ui->nn_max_cur->text();
    QString str13 = ui->nn_max_seed->text();
    QString str14 = ui->Reserve_1->text();
    QString str15 = ui->Reserve_2->text();
    setting.setValue("win_height",str1);//
    setting.setValue("win_width",str2);//
    setting.setValue("grid_shift_step",str3);//
    setting.setValue("grid_num_scale_scan",str4);//
    setting.setValue("var_thr_coef",str5);//
    setting.setValue("var_update_coef",str6);//
    setting.setValue("fern_num",str7);//
    setting.setValue("fern_bit_num",str8);//
    setting.setValue("nn_max_input",str9);//
    setting.setValue("nn_patch_width",str10);//
    setting.setValue("nn_patch_height",str11);//
    setting.setValue("nn_max_cur",str12);//
    setting.setValue("nn_max_seed",str13);//
    setting.setValue("Reserve_1",str14);//
    setting.setValue("Reserve_2",str15);//

    setting.endGroup();//节点结束

    return 0;

}

//读取程序启动状态信息
int Widget::ReadSetTing()
{
    //设置配置文件的目录和位置，如果有，则不动，没有，会自动创建
    QSettings setting("setting.ini",QSettings::IniFormat);

    //读取皮肤状态
    if(setting.contains(tr("set/skin")))//如果已经存在这个文件，那就进行读取
    {
        skin = setting.value("set/skin").toString();//将读取出的数据进行使用
    }

    //读取网络参数并设置
    if(setting.contains(tr("net/ip")))//如果已经存在这个文件，那就进行读取
    {
        QString ip;
        ip = setting.value("net/ip").toString();//将读取出的数据进行使用
        ui->IP_lineEdit->setText(ip);
    }
    if(setting.contains(tr("net/port")))//如果已经存在这个文件，那就进行读取
    {
        QString port;
        port = setting.value("net/port").toString();//将读取出的数据进行使用
        ui->PORT_lineEdit->setText(port);
    }

    //读取自动保存文件标志
    if(setting.contains(tr("net/sav_file")))//如果已经存在这个文件，那就进行读取
    {
        int sav_file = setting.value("net/sav_file").toString().toInt();//将读取出的数据进行使用
        if(sav_file)
        {
            savefile = 1;
            ui->auto_box->setChecked(true);
            ui->save_btn->setEnabled(false);
            ui->label_play->auto2save(1);
        }
        else
        {
            savefile = 0;
            ui->auto_box->setChecked(false);
//            ui->label_play->auto2save(0);
        }
    }

    //读取跟踪参数
    if(setting.contains(tr("net/win_height")))//如果已经存在这个文件，那就进行读取
    {

        QString str1 = setting.value("net/win_height").toString();//将读取出的数据进行使用
        ui->win_height->setText(str1);
        QString str2 = setting.value("net/win_width").toString();//将读取出的数据进行使用
        ui->win_width->setText(str2);
        QString str3 = setting.value("net/grid_shift_step").toString();//将读取出的数据进行使用
        ui->grid_shift_step->setText(str3);
        QString str4 = setting.value("net/grid_num_scale_scan").toString();//将读取出的数据进行使用
        ui->grid_num_scale_scan->setText(str4);
        QString str5 = setting.value("net/var_thr_coef").toString();//将读取出的数据进行使用
        ui->var_thr_coef->setText(str5);
        QString str6 = setting.value("net/var_update_coef").toString();//将读取出的数据进行使用
        ui->var_update_coef->setText(str6);
        QString str7 = setting.value("net/fern_num").toString();//将读取出的数据进行使用
        ui->fern_num->setText(str7);
        QString str8 = setting.value("net/fern_bit_num").toString();//将读取出的数据进行使用
        ui->fern_bit_num->setText(str8);
        QString str9 = setting.value("net/nn_max_input").toString();//将读取出的数据进行使用
        ui->nn_max_input->setText(str9);
        QString str10 = setting.value("net/nn_patch_width").toString();//将读取出的数据进行使用
        ui->nn_patch_width->setText(str10);
        QString str11 = setting.value("net/nn_patch_height").toString();//将读取出的数据进行使用
        ui->nn_patch_height->setText(str11);
        QString str12 = setting.value("net/nn_max_cur").toString();//将读取出的数据进行使用
        ui->nn_max_cur->setText(str12);
        QString str13 = setting.value("net/nn_max_seed").toString();//将读取出的数据进行使用
        ui->nn_max_seed->setText(str13);
        QString str14 = setting.value("net/Reserve_1").toString();//将读取出的数据进行使用
        ui->Reserve_1->setText(str14);
        QString str15 = setting.value("net/Reserve_2").toString();//将读取出的数据进行使用
        ui->Reserve_2->setText(str15);
    }
    return 0;

}

/****************************热像仪设置模块******************************************/


void Widget::th_quit()
{
    play_stop = true;
    IconHelper::Instance()->setIcon(ui->play_btn, QChar(0xf04b), 18);
}

//播放 暂停
void Widget::on_play_btn_clicked()
{
    if(play_stop)
    {
        QString str_ip = ui->IP_lineEdit->text();
        QString str_port = ui->PORT_lineEdit->text();
        ui->label_play->start(str_ip, str_port, 1);
        ui->label_play->changesize(label_w, label_h);
        IconHelper::Instance()->setIcon(ui->play_btn, QChar(0xf04d), 18);
        play_stop = false;
        if(savefile)
        {
            ui->label_play->auto2save(1);
        }
    }
    else
    {
        ui->label_play->stop(1);
        IconHelper::Instance()->setIcon(ui->play_btn, QChar(0xf04b), 18);
        play_stop = true;
    }
}

void Widget::resizeEvent(QResizeEvent *event)
{
    label_w = ui->label_play->width();
    label_h = ui->label_play->height();
}

//上
void Widget::on_up_btn_clicked()
{
    QByteArray ba;
    ba[0] = 0xaa;
    ba[1] = 0x55;
    ba[2] = 0x07;
//    ba[3] = 0x05;
    ba[3] = StepSize;
    ba[4] = 0x00;
    ba[5] = 0x00;
    ba[6] = 0x00;
    ba[7] = 0x00;
    ba[8] = 0x00;
    ba[9] = 0x00;
    ba[10] = 0x00;
    ba[11] = 0x00;
    ba[12] = 0x00;
    ba[13] = 0x00;
    ba[14] = 0x00;
    ba[15] = (0xaa + 0x55 + 0x07 + ba[3]) & 0x00ff;
    socket->write(ba);

//    ba[1] = 0;
//    ba[2] = 0;
//    ba[3] = 0;
//    Sleep(15);
//    socket->write(ba);
}

//下
void Widget::on_down_btn_clicked()
{
    QByteArray ba;
    ba[0] = 0xaa;
    ba[1] = 0x55;
    ba[2] = 0x08;
//    ba[3] = 0x05;
    ba[3] = StepSize;
    ba[4] = 0x00;
    ba[5] = 0x00;
    ba[6] = 0x00;
    ba[7] = 0x00;
    ba[8] = 0x00;
    ba[9] = 0x00;
    ba[10] = 0x00;
    ba[11] = 0x00;
    ba[12] = 0x00;
    ba[13] = 0x00;
    ba[14] = 0x00;
    ba[15] = (0xaa + 0x55 + 0x08 + ba[3]) & 0x00ff;
    socket->write(ba);

//    ba[1] = 0;
//    ba[2] = 0;
//    ba[3] = 0;
//    Sleep(15);
//    socket->write(ba);
}

//左
void Widget::on_left_btn_2_clicked()
{
    QByteArray ba;
    ba[0] = 0xaa;
    ba[1] = 0x55;
    ba[2] = 0x09;
//    ba[3] = 0x05;
    ba[3] = StepSize;
    ba[4] = 0x00;
    ba[5] = 0x00;
    ba[6] = 0x00;
    ba[7] = 0x00;
    ba[8] = 0x00;
    ba[9] = 0x00;
    ba[10] = 0x00;
    ba[11] = 0x00;
    ba[12] = 0x00;
    ba[13] = 0x00;
    ba[14] = 0x00;
    ba[15] = (0xaa + 0x55 + 0x09 + ba[3]) & 0x00ff;
    socket->write(ba);

//    ba[1] = 0;
//    ba[2] = 0;
//    ba[3] = 0;
//    Sleep(15);
//    socket->write(ba);
}

//右
void Widget::on_right_btn_2_clicked()
{
    QByteArray ba;
    ba[0] = 0xaa;
    ba[1] = 0x55;
    ba[2] = 0x0a;
//    ba[3] = 0x05;
    ba[3] = StepSize;
    ba[4] = 0x00;
    ba[5] = 0x00;
    ba[6] = 0x00;
    ba[7] = 0x00;
    ba[8] = 0x00;
    ba[9] = 0x00;
    ba[10] = 0x00;
    ba[11] = 0x00;
    ba[12] = 0x00;
    ba[13] = 0x00;
    ba[14] = 0x00;
    ba[15] = (0xaa + 0x55 + 0x0a + ba[3]) & 0x00ff;
    socket->write(ba);

//    ba[1] = 0;
//    ba[2] = 0;
//    ba[3] = 0;
//    Sleep(5);
//    socket->write(ba);
}

//锁定
void Widget::on_suoding_btn_clicked()
{
    QByteArray ba;
    if(Suodin)
    {
        ba[0] = 0xaa;
        ba[1] = 0x55;
        ba[2] = 0x03;
        ba[3] = 0x00;
        ba[4] = 0x00;
        ba[5] = 0x00;
        ba[6] = 0x00;
        ba[7] = 0x00;
        ba[8] = 0x00;
        ba[9] = 0x00;
        ba[10] = 0x00;
        ba[11] = 0x00;
        ba[12] = 0x00;
        ba[13] = 0x00;
        ba[14] = 0x00;
        ba[15] = (0xaa + 0x55 + 0x03) & 0x00ff;
        socket->write(ba);
        ui->suoding_btn->setText("解锁");
        Suodin = false;
    }
    else
    {
        ba[0] = 0xaa;
        ba[1] = 0x55;
        ba[2] = 0x02;
        ba[3] = 0x00;
        ba[4] = 0x00;
        ba[5] = 0x00;
        ba[6] = 0x00;
        ba[7] = 0x00;
        ba[8] = 0x00;
        ba[9] = 0x00;
        ba[10] = 0x00;
        ba[11] = 0x00;
        ba[12] = 0x00;
        ba[13] = 0x00;
        ba[14] = 0x00;
        ba[15] = (0xaa + 0x55 + 0x02) & 0x00ff;
        socket->write(ba);
        ui->suoding_btn->setText("锁定");
        Suodin = true;
    }
}

void Widget::on_light_add_btn_clicked()
{
    QByteArray ba;
    ba[0] = 0xaa;
    ba[1] = 0x55;
    ba[2] = 0x05;
    ba[3] = 0x00;
    ba[4] = 0x00;
    ba[5] = 0x00;
    ba[6] = 0x00;
    ba[7] = 0x00;
    ba[8] = 0x00;
    ba[9] = 0x00;
    ba[10] = 0x00;
    ba[11] = 0x00;
    ba[12] = 0x00;
    ba[13] = 0x00;
    ba[14] = 0x00;
    ba[15] = (0xaa + 0x55 + 0x05) & 0x00ff;
    socket->write(ba);
}

void Widget::on_light_jian_btn_clicked()
{
    QByteArray ba;
    ba[0] = 0xaa;
    ba[1] = 0x55;
    ba[2] = 0x06;
    ba[3] = 0x00;
    ba[4] = 0x00;
    ba[5] = 0x00;
    ba[6] = 0x00;
    ba[7] = 0x00;
    ba[8] = 0x00;
    ba[9] = 0x00;
    ba[10] = 0x00;
    ba[11] = 0x00;
    ba[12] = 0x00;
    ba[13] = 0x00;
    ba[14] = 0x00;
    ba[15] = (0xaa + 0x55 + 0x06) & 0x00ff;
    socket->write(ba);
}

//Reserve_1,Reserve_2
void Widget::on_pushButton_clicked()
{
    QString str1 = ui->Reserve_1->text();
    QString str2 = ui->Reserve_2->text();

    Reserve_1_num = str1.toInt();
    Reserve_2_num = str2.toInt();

    QByteArray ba;
    ba[0] = 0xaa;
    ba[1] = 0x55;
    ba[2] = 0x0f;
//    ba[3] = 0x00;
    ba[3] = Reserve_1_num;
    ba[4] = Reserve_2_num;
    ba[5] = 0x00;
    ba[6] = 0x00;
    ba[7] = 0x00;
    ba[8] = 0x00;
    ba[9] = 0x00;
    ba[10] = 0x00;
    ba[11] = 0x00;
    ba[12] = 0x00;
    ba[13] = 0x00;
    ba[14] = 0x00;
    ba[15] = (ba[0] + ba[1] + ba[2] + ba[3] + ba[4] + ba[5] + ba[6] + ba[7]) & 0x00ff;
    socket->write(ba);
}

//win_height,win_width
void Widget::on_pushButton_2_clicked()
{
    QString str1 = ui->win_height->text();
    QString str2 = ui->win_width->text();
    win_height_num = str1.toInt();
    win_width_num = str2.toInt();
    QByteArray ba;
    ba[0] = 0xaa;
    ba[1] = 0x55;
    ba[2] = 0x0b;
//    ba[3] = 0x00;
    ba[3] = win_height_num;
    ba[4] = win_width_num;
    ba[5] = 0x00;
    ba[6] = 0x00;
    ba[7] = 0x00;
    ba[8] = 0x00;
    ba[9] = 0x00;
    ba[10] = 0x00;
    ba[11] = 0x00;
    ba[12] = 0x00;
    ba[13] = 0x00;
    ba[14] = 0x00;
    ba[15] = (ba[0] + ba[1] + ba[2] + ba[3] + ba[4]) & 0x00ff;
    socket->write(ba);
}
//grid_shift_step,grid_num_scale_scan,var_thr_coef,var_update_coef
void Widget::on_pushButton_3_clicked()
{
    QString str1 = ui->grid_shift_step->text();
    QString str2 = ui->grid_num_scale_scan->text();
    QString str3 = ui->var_thr_coef->text();
    QString str4 = ui->var_update_coef->text();
    grid_shift_step_num = /*(int)*/(str1.toDouble());
    double num1 = grid_shift_step_num*100;
    grid_num_scale_scan_num = /*(int)*/(str2.toDouble());
    double num2 = grid_num_scale_scan_num*100;
    var_thr_coef_num = /*(int)*/(str3.toDouble());
    double num3 = var_thr_coef_num*100;
    var_update_coef_num = /*(int)*/(str4.toDouble());
    double num4 = var_update_coef_num*100;

    QByteArray ba;
    ba[0] = 0xaa;
    ba[1] = 0x55;
    ba[2] = 0x0c;
    ba[3] = (int)num1;
    ba[4] = (int)num2;
    ba[5] = (int)num3;
    ba[6] = (int)num4;
    ba[7] = 0x00;
    ba[8] = 0x00;
    ba[9] = 0x00;
    ba[10] = 0x00;
    ba[11] = 0x00;
    ba[12] = 0x00;
    ba[13] = 0x00;
    ba[14] = 0x00;
    ba[15] = (ba[0] + ba[1] + ba[2] + ba[3] + ba[4] + ba[5] + ba[6]) & 0x00ff;
    socket->write(ba);
}

//fern_num,fern_bit_num
void Widget::on_pushButton_4_clicked()
{
    QString str1 = ui->fern_num->text();
    QString str2 = ui->fern_bit_num->text();
    fern_num_num = str1.toInt();
    fern_bit_num_num = str2.toInt();
    QByteArray ba;
    ba[0] = 0xaa;
    ba[1] = 0x55;
    ba[2] = 0x0d;
//    ba[3] = 0x00;
    ba[3] = fern_num_num;
    ba[4] = fern_bit_num_num;
    ba[5] = 0x00;
    ba[6] = 0x00;
    ba[7] = 0x00;
    ba[8] = 0x00;
    ba[9] = 0x00;
    ba[10] = 0x00;
    ba[11] = 0x00;
    ba[12] = 0x00;
    ba[13] = 0x00;
    ba[14] = 0x00;
    ba[15] = (ba[0] + ba[1] + ba[2] + ba[3] + ba[4]) & 0x00ff;
    socket->write(ba);
}

//nn_max_input,nn_patch_width,nn_patch_height,nn_max_cur,nn_max_seed
void Widget::on_pushButton_5_clicked()
{
    QString str1 = ui->nn_max_input->text();
    QString str2 = ui->nn_patch_width->text();
    QString str3 = ui->nn_patch_height->text();
    QString str4 = ui->nn_max_cur->text();
    QString str5 = ui->nn_max_seed->text();
    nn_max_input_num = str1.toInt();
    nn_patch_width_num = str2.toInt();
    nn_patch_height_num = str3.toInt();
    nn_max_cur_num = str4.toInt();
    nn_max_seed_num = str5.toInt();

    QByteArray ba;
    ba[0] = 0xaa;
    ba[1] = 0x55;
    ba[2] = 0x0e;
//    ba[3] = 0x00;
    ba[3] = nn_max_input_num;
    ba[4] = nn_patch_width_num;
    ba[5] = nn_patch_height_num;
    ba[6] = nn_max_cur_num;
    ba[7] = nn_max_seed_num;
    ba[8] = 0x00;
    ba[9] = 0x00;
    ba[10] = 0x00;
    ba[11] = 0x00;
    ba[12] = 0x00;
    ba[13] = 0x00;
    ba[14] = 0x00;
    ba[15] = (ba[0] + ba[1] + ba[2] + ba[3] + ba[4] + ba[5] + ba[6] + ba[7]) & 0x00ff;
    socket->write(ba);
}


void Widget::on_auto_box_clicked()
{
//    if(ui->auto_box->che)
//    {
//        qDebug()<<"auto_box1";
//    }
//    else
//    {
//        qDebug()<<"auto_box2";
//    }
}


//10
void Widget::on_checkBox_3_clicked()
{
    StepSize = 10;
    ui->checkBox->setChecked(false);
    ui->checkBox_2->setChecked(false);
    ui->checkBox_3->setChecked(true);
    qDebug()<<StepSize;
}
//1
void Widget::on_checkBox_clicked()
{
    StepSize = 1;
    ui->checkBox->setChecked(true);
    ui->checkBox_2->setChecked(false);
    ui->checkBox_3->setChecked(false);
    qDebug()<<StepSize;
}
//5
void Widget::on_checkBox_2_clicked()
{
    StepSize = 5;
    ui->checkBox->setChecked(false);
    ui->checkBox_2->setChecked(true);
    ui->checkBox_3->setChecked(false);
    qDebug()<<StepSize;
}


//自动 / 手动
void Widget::on_auto_box_clicked(bool checked)
{
    if(!saveff)
    {
        ui->auto_box->setChecked(false);
        QMessageBox message(QMessageBox::NoIcon, "警告", "请先手动停止保存文件，再执行此操作.");
        message.exec();

        return;
    }

    if(checked) //auto
    {
        savefile = 1;
        ui->label_play->auto2save(1);
        ui->save_btn->setText("开始保存");
        ui->save_btn->setEnabled(false);
        saveff = true;
    }
    else
    {
        savefile = 0;
//        ui->label_play->auto2save(0);
        ui->save_btn->setEnabled(true);
    }
}

void Widget::on_save_btn_clicked()
{
    if(saveff)
    {
        ui->save_btn->setText("停止保存");
        ui->label_play->auto2save(1);
        saveff = false;
    }
    else
    {
        ui->save_btn->setText("开始保存");
        ui->label_play->auto2save(0);
        saveff = true;
    }

}

void Widget::sendPoint(int x1, int y1, int x2, int y2)
{
    unsigned char data[100] = { 0 };

    POT.head = 0x68;
    POT.target = 0x01;
    POT.head2 = 0x68;
    POT.ctr = 0x01;
    POT.x1 = x1;
    POT.y1 = y1;
    POT.x2 = x2;
    POT.y2 = y2;
    POT.chk = (x1 + y1 + x2 + y2) % 255;
    POT.end = 0x16;
    int len = sizeof(POINTDATA);
    memcpy(/*(void*)*/data, &POT, len);

    int ret = socket->write((const char*)data,14);

    if(ret < 0)
    {
        qDebug()<<"send fail!!!";
    }
}

//qsimage 0x03
void Widget::on_track_btn1_clicked()
{
    unsigned char data[100] = { 0 };

    POT.head = 0x68;
    POT.target = 0x01;
    POT.head2 = 0x68;
    POT.ctr = 0x03;
    POT.x1 = 0;
    POT.y1 = 0;
    POT.x2 = 0;
    POT.y2 = 0;
    POT.chk = 0;
    POT.end = 0x16;
    int len = sizeof(POINTDATA);
    memcpy(/*(void*)*/data, &POT, len);

    int ret = socket->write((const char*)data,14);

    if(ret < 0)
    {
        qDebug()<<"send fail!!!";
    }
    else
    {
        ui->label_statues->setText("已切换为CamShift跟踪");
    }
}

//tld 0x04
void Widget::on_track_btn2_clicked()
{
    unsigned char data[100] = { 0 };

    POT.head = 0x68;
    POT.target = 0x01;
    POT.head2 = 0x68;
    POT.ctr = 0x04;
    POT.x1 = 0;
    POT.y1 = 0;
    POT.x2 = 0;
    POT.y2 = 0;
    POT.chk = 0;
    POT.end = 0x16;
    int len = sizeof(POINTDATA);
    memcpy(/*(void*)*/data, &POT, len);

    int ret = socket->write((const char*)data,14);

    if(ret < 0)
    {
        qDebug()<<"send fail!!!";
    }
    else
    {
        ui->label_statues->setText("已切换为TLD跟踪");
    }
}



//ncc 0x02
void Widget::on_track_btn3_clicked()
{
    unsigned char data[100] = { 0 };

    POT.head = 0x68;
    POT.target = 0x01;
    POT.head2 = 0x68;
    POT.ctr = 0x02;
    POT.x1 = 0;
    POT.y1 = 0;
    POT.x2 = 0;
    POT.y2 = 0;
    POT.chk = 0;
    POT.end = 0x16;
    int len = sizeof(POINTDATA);
    memcpy(/*(void*)*/data, &POT, len);

    int ret = socket->write((const char*)data,14);

    if(ret < 0)
    {
        qDebug()<<"send fail!!!";
    }
    else
    {
        ui->label_statues->setText("已切换为NCC跟踪");
    }
}

void Widget::on_pushButton_6_clicked()
{
    unsigned char data[100] = { 0 };

    POT.head = 0x68;
    POT.target = 0x01;
    POT.head2 = 0x68;
    POT.ctr = 0x05;
    POT.x1 = 0;
    POT.y1 = 0;
    POT.x2 = 0;
    POT.y2 = 0;
    POT.chk = 0;
    POT.end = 0x16;
    int len = sizeof(POINTDATA);
    memcpy(/*(void*)*/data, &POT, len);

    int ret2 = socket->write((const char*)data,14);

    if(ret2 < 0)
    {
        qDebug()<<"send fail!!!";
    }
    QByteArray ACK0;
    ACK0[0] = 0xEB;
    ACK0[1] = 0x90;
    ACK0[2] = 0x01;
    ACK0[3] = 0x02;
    ACK0[4] = 0x00;
    ACK0[5] = 0x00;
    ACK0[6] = 0xFF;
    ACK0[7] = 0xBF;
    ACK0[8] = 0x00;
    ACK0[9] = 0x00;
    ACK0[10] = 0x00;
    ACK0[11] = 0x00;
    ACK0[12] = 0x00;
    ACK0[13] = 0x00;
    ACK0[14] = 0x00;
    ACK0[15] = 0x00;
    ACK0[16] = 0x00;
    ACK0[17] = 0x00;
    ACK0[18] = 0x00;
    ACK0[19] = 0x00;
    ACK0[20] = 0x00;
    ACK0[21] = 0x00;
    ACK0[22] = 0x00;
    ACK0[23] = 0x00;
    ACK0[24] = 0x00;
    ACK0[25] = 0x00;
    ACK0[26] = 0x00;
    ACK0[27] = 0x00;
    ACK0[28] = 0x00;
    ACK0[29] = 0x00;
    ACK0[30] = 0x00;
    ACK0[31] = 0x00;
    ACK0[32] = 0x00;
    ACK0[33] = 0x00;
    ACK0[34] = 0x00;
    ACK0[35] = 0x00;
    ACK0[36] = 0x00;
    ACK0[37] = 0x00;
    ACK0[38] = 0x00;
    ACK0[39] = 0x00;
    ACK0[40] = 0x00;
    ACK0[41] = 0x00;
    ACK0[42] = 0x00;
    ACK0[43] = 0x00;
    ACK0[44] = 0x00;
    ACK0[45] = 0x00;
    ACK0[46] = 0x00;
    ACK0[47] = 0x00;
    ACK0[48] = 0x00;
    ACK0[49] = 0x00;
    ACK0[50] = 0x00;
    ACK0[51] = 0x00;
    ACK0[52] = 0x00;
    ACK0[53] = 0x00;
    ACK0[54] = 0x00;
    ACK0[15] = 0x00;
    ACK0[56] = 0x00;
    ACK0[57] = 0x00;
    ACK0[58] = 0x00;
    ACK0[59] = 0x00;
    ACK0[60] = 0x00;
    ACK0[61] = 0x00;
    ACK0[62] = 0x00;
    ACK0[63]= 0x3C;
    int ret =serial->write(ACK0);
    if(ret < 0)
        {
            qDebug()<<"send fail!!!";
        }
    m_pTimer->start(2000);
//    ui->track_btn2->setEnabled(false);

}


//红外视频 跟踪自动切换为ncc
void Widget::on_pushButton_7_clicked()
{
    unsigned char data[100] = { 0 };

    POT.head = 0x68;
    POT.target = 0x01;
    POT.head2 = 0x68;
    POT.ctr = 0x06;
    POT.x1 = 0;
    POT.y1 = 0;
    POT.x2 = 0;
    POT.y2 = 0;
    POT.chk = 0;
    POT.end = 0x16;
    int len = sizeof(POINTDATA);
    memcpy(/*(void*)*/data, &POT, len);

    int ret = socket->write((const char*)data,14);

    if(ret < 0)
    {
        qDebug()<<"send fail!!!";
    }

    QByteArray ACK0;
    ACK0[0] = 0xEB;
    ACK0[1] = 0x90;
    ACK0[2] = 0x01;
    ACK0[3] = 0x02;
    ACK0[4] = 0x00;
    ACK0[5] = 0x00;
    ACK0[6] = 0xFF;
    ACK0[7] = 0xBF;
    ACK0[8] = 0x00;
    ACK0[9] = 0x00;
    ACK0[10] = 0x00;
    ACK0[11] = 0x00;
    ACK0[12] = 0x00;
    ACK0[13] = 0x00;
    ACK0[14] = 0x00;
    ACK0[15] = 0x00;
    ACK0[16] = 0x00;
    ACK0[17] = 0x00;
    ACK0[18] = 0x00;
    ACK0[19] = 0x00;
    ACK0[20] = 0x00;
    ACK0[21] = 0x00;
    ACK0[22] = 0x00;
    ACK0[23] = 0x00;
    ACK0[24] = 0x00;
    ACK0[25] = 0x00;
    ACK0[26] = 0x00;
    ACK0[27] = 0x00;
    ACK0[28] = 0x00;
    ACK0[29] = 0x00;
    ACK0[30] = 0x00;
    ACK0[31] = 0x00;
    ACK0[32] = 0x00;
    ACK0[33] = 0x00;
    ACK0[34] = 0x00;
    ACK0[35] = 0x00;
    ACK0[36] = 0x00;
    ACK0[37] = 0x00;
    ACK0[38] = 0x00;
    ACK0[39] = 0x00;
    ACK0[40] = 0x00;
    ACK0[41] = 0x00;
    ACK0[42] = 0x00;
    ACK0[43] = 0x00;
    ACK0[44] = 0x00;
    ACK0[45] = 0x00;
    ACK0[46] = 0x00;
    ACK0[47] = 0x00;
    ACK0[48] = 0x00;
    ACK0[49] = 0x00;
    ACK0[50] = 0x00;
    ACK0[51] = 0x00;
    ACK0[52] = 0x00;
    ACK0[53] = 0x00;
    ACK0[54] = 0x00;
    ACK0[15] = 0x00;
    ACK0[56] = 0x00;
    ACK0[57] = 0x00;
    ACK0[58] = 0x00;
    ACK0[59] = 0x00;
    ACK0[60] = 0x00;
    ACK0[61] = 0x00;
    ACK0[62] = 0x00;
    ACK0[63]= 0x3C;
    serial->write(ACK0);
    m_pTimer2->start(2000);
}


void Widget::handleTimeout()
{
    QByteArray ACK0;
    ACK0[0] = 0xEB;
    ACK0[1] = 0x90;
    ACK0[2] = 0x06;
    ACK0[3] = 0x01;
    ACK0[4] = 0x00;
    ACK0[5] = 0x00;
    ACK0[6] = 0xFF;
    ACK0[7] = 0xBF;
    ACK0[8] = 0x00;
    ACK0[9] = 0x00;
    ACK0[10] = 0x00;
    ACK0[11] = 0x00;
    ACK0[12] = 0x00;
    ACK0[13] = 0x00;
    ACK0[14] = 0x00;
    ACK0[15] = 0x00;
    ACK0[16] = 0x00;
    ACK0[17] = 0x00;
    ACK0[18] = 0x00;
    ACK0[19] = 0x00;
    ACK0[20] = 0x00;
    ACK0[21] = 0x00;
    ACK0[22] = 0x00;
    ACK0[23] = 0x00;
    ACK0[24] = 0x00;
    ACK0[25] = 0x00;
    ACK0[26] = 0x00;
    ACK0[27] = 0x00;
    ACK0[28] = 0x00;
    ACK0[29] = 0x00;
    ACK0[30] = 0x00;
    ACK0[31] = 0x00;
    ACK0[32] = 0x00;
    ACK0[33] = 0x00;
    ACK0[34] = 0x00;
    ACK0[35] = 0x00;
    ACK0[36] = 0x00;
    ACK0[37] = 0x00;
    ACK0[38] = 0x00;
    ACK0[39] = 0x00;
    ACK0[40] = 0x00;
    ACK0[41] = 0x00;
    ACK0[42] = 0x00;
    ACK0[43] = 0x00;
    ACK0[44] = 0x00;
    ACK0[45] = 0x00;
    ACK0[46] = 0x00;
    ACK0[47] = 0x00;
    ACK0[48] = 0x00;
    ACK0[49] = 0x00;
    ACK0[50] = 0x00;
    ACK0[51] = 0x00;
    ACK0[52] = 0x00;
    ACK0[53] = 0x00;
    ACK0[54] = 0x00;
    ACK0[15] = 0x00;
    ACK0[56] = 0x00;
    ACK0[57] = 0x00;
    ACK0[58] = 0x00;
    ACK0[59] = 0x00;
    ACK0[60] = 0x00;
    ACK0[61] = 0x00;
    ACK0[62] = 0x00;
    ACK0[63]= 0x40;

    int ret1=serial->write(ACK0);
    if(ret1 < 0)
        {
            qDebug()<<"send fail!!!";
        }
    else
    qDebug()<<"串口发送成功";
    m_pTimer->stop();

    int w = ui->label_play->width();
    int h = ui->label_play->height();
//    qDebug("mode 1 label_w:%d", w);
//    qDebug("mode 1 label_d:%d", h);
    ui->label_play->changesize(w, h);

    on_track_btn1_clicked();
}

void Widget::handleTimeout2()
{
    QByteArray ACK;
    ACK[0] = 0xEB;
    ACK[1] = 0x90;
    ACK[2] = 0x06;
    ACK[3] = 0x02;
    ACK[4] = 0x00;
    ACK[5] = 0x00;
    ACK[6] = 0xFF;
    ACK[7] = 0xBF;
    ACK[8] = 0x00;
    ACK[9] = 0x00;
    ACK[10] = 0x00;
    ACK[11] = 0x00;
    ACK[12] = 0x00;
    ACK[13] = 0x00;
    ACK[14] = 0x00;
    ACK[15] = 0x00;
    ACK[16] = 0x00;
    ACK[17] = 0x00;
    ACK[18] = 0x00;
    ACK[19] = 0x00;
    ACK[20] = 0x00;
    ACK[21] = 0x00;
    ACK[22] = 0x00;
    ACK[23] = 0x00;
    ACK[24] = 0x00;
    ACK[25] = 0x00;
    ACK[26] = 0x00;
    ACK[27] = 0x00;
    ACK[28] = 0x00;
    ACK[29] = 0x00;
    ACK[30] = 0x00;
    ACK[31] = 0x00;
    ACK[32] = 0x00;
    ACK[33] = 0x00;
    ACK[34] = 0x00;
    ACK[35] = 0x00;
    ACK[36] = 0x00;
    ACK[37] = 0x00;
    ACK[38] = 0x00;
    ACK[39] = 0x00;
    ACK[40] = 0x00;
    ACK[41] = 0x00;
    ACK[42] = 0x00;
    ACK[43] = 0x00;
    ACK[44] = 0x00;
    ACK[45] = 0x00;
    ACK[46] = 0x00;
    ACK[47] = 0x00;
    ACK[48] = 0x00;
    ACK[49] = 0x00;
    ACK[50] = 0x00;
    ACK[51] = 0x00;
    ACK[52] = 0x00;
    ACK[53] = 0x00;
    ACK[54] = 0x00;
    ACK[15] = 0x00;
    ACK[56] = 0x00;
    ACK[57] = 0x00;
    ACK[58] = 0x00;
    ACK[59] = 0x00;
    ACK[60] = 0x00;
    ACK[61] = 0x00;
    ACK[62] = 0x00;
    ACK[63]= 0x41;
    int ret1=serial->write(ACK);

    if(ret1 < 0)
        {
            qDebug()<<"send fail!!!";
        }
    else
    qDebug()<<"串口发送成功";
    m_pTimer2->stop();
    int w = ui->label_play->width();
    int h = ui->label_play->height();
//    qDebug("mode 1 label_w:%d", w);
//    qDebug("mode 1 label_d:%d", h);
    ui->label_play->changesize(w, h);

    //跟踪切换为ncc
    on_track_btn3_clicked();

}

void Widget::on_pushButton_8_clicked()
{
    if(ui->pushButton_8->text() == "打开串口")
    {
         UartInit();
         ui->pushButton_8->setText("关闭串口");
    }
    else if(ui->pushButton_8->text() =="关闭串口")
    {
        serial->close();
        ui->pushButton_8->setText("打开串口");
    }
}
