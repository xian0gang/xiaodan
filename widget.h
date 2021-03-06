#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLibrary>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMessageBox>
#include <QDateTime>

typedef struct {
    unsigned char head;
    unsigned char target;
    unsigned char head2;
    unsigned char ctr;
    short x1;
    short y1;
    short x2;
    short y2;
    unsigned char chk;
    unsigned char end;
}POINTDATA;


namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

protected:
        void resizeEvent(QResizeEvent *event);

protected slots:
    void buttonClick();
    int WriteSetTing();         //启动参数写配置文件
    int ReadSetTing();          //读启动参数


private slots:
    void UartReadData();
    void socket_Read_Data();
    void th_quit();
    void NewConnect();
    void Tcpclose();
    void sendPoint(int, int, int, int);
//    菜单栏按钮
    void on_close_btn_clicked();
    void on_max_btn_clicked();
    void on_min_btn_clicked();
    void on_change_color_btn_clicked();

    void on_play_btn_clicked();
    void on_up_btn_clicked();
    void on_down_btn_clicked();
    void on_left_btn_2_clicked();
    void on_right_btn_2_clicked();
    void on_suoding_btn_clicked();

    void on_light_add_btn_clicked();

    void on_light_jian_btn_clicked();

    void on_auto_box_clicked();

//    void on_checkBox_clicked(bool checked);

//    void on_checkBox_2_clicked(bool checked);

    void on_pushButton_clicked();

    void on_checkBox_3_clicked();

    void on_checkBox_clicked();

    void on_checkBox_2_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_auto_box_clicked(bool checked);

    void on_save_btn_clicked();

    void on_track_btn1_clicked();

    void on_track_btn2_clicked();

    void on_track_btn3_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void handleSerialError(QSerialPort::SerialPortError error);

    void handleTimeout();  //超时处理函数

    void handleTimeout2();  //超时处理函数

    void on_pushButton_8_clicked();

    void auto_show();
    void auto_send();
    void checkHeartbeat();

private:
    Ui::Widget *ui;
    void ui_init();      //ui初始化
    void UartInit();     //串口初始化
    void TcpInit();      //tcp初始化

private:
    QString skin;         //换肤变量
    QLibrary *mylib;      //加载动态库
    QSerialPort *serial;  //fpga串口通信
    QTcpSocket *socket;   //主机通信
    QTcpServer *server;

    int label_w;          //播放label大小
    int label_h;
    bool play_stop;       //播放暂停 控制量
    bool Suodin;          //锁定  控制量

    int StepSize;
    int win_height_num;
    int win_width_num;
    double grid_shift_step_num;
    double grid_num_scale_scan_num;
    double var_thr_coef_num;
    double var_update_coef_num;
    int fern_num_num;
    int fern_bit_num_num;
    int nn_max_input_num;
    int nn_patch_width_num;
    int nn_patch_height_num;
    int nn_max_cur_num;
    int nn_max_seed_num;
    int Reserve_1_num;
    int Reserve_2_num;

    int savefile;
    bool saveff;

    POINTDATA POT;

    QTimer *m_pTimer;
    QTimer *m_pTimer2;
    QTimer *m_auto_show_tm;
    QTimer *m_auto_send;
    QTimer *m_heartbeat;

    int auto_send_count;
    QDateTime m_lastTime;

};

#endif // WIDGET_H
