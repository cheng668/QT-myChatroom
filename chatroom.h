#ifndef CHATROOM_H
#define CHATROOM_H

#include <QDialog>
#include <QEvent>
class QUdpSocket;
class QColor;
class server;
class QTextCharFormat;
namespace Ui {
class chatroom;
}

enum MsgType{Msg,UsrEnter,UsrLeft,FileName,Refuse}; //消息类型

class chatroom : public QDialog
{
    Q_OBJECT

public:
    explicit chatroom(QWidget *parent,QString usrname);
    ~chatroom();
protected:
    void setupUi();
    void usrEnter(QString usrname,QString ipaddr);       //新用户加入群聊
    void usrLeft(QString usrname,QString time);          //用户离开聊天室
    void sndMsg(MsgType type,QString srvaddr = "");      //广播UDP消息
    QString getIP();
    QString getUsr();
    QString getMsg();
    /*判断是否接收filename类型udp消息*/
    void hasPendingFile(QString usrname,QString srvaddr,QString clntaddr,QString filename);
private slots:
    /*接收UDP消息*/
    void processPendingDatagrams();                      //接收UDP消息
    /*发送聊天消息*/
    void on_btnSend_clicked();
    /*字体大小*/
    void on_cbxSize_currentIndexChanged(const QString &pointsize);
    /*粗体*/
    void on_btnBold_clicked(bool checked);
    /*字体*/
    void on_cbxFont_currentFontChanged(const QFont &f);
    /*斜体*/
    void on_btnItalic_clicked(bool checked);
    /*下划线*/
    void on_btnUnderline_clicked(bool checked);
    /*字体颜色*/
    void on_btnColor_clicked();
    /*清空聊天记录*/
    void on_btnClear_clicked();
    /*保存历史消息*/
    void on_btnSave_clicked();
    /*获取文件服务器发送的文件名，并发送filename消息*/
    void getFileName(QString);
    /*打开文件服务器*/
    void on_btnFileSend_clicked();
    /*光标改变时，读取光标所在位置字体*/
    void curFmtChanged(const QTextCharFormat &fmt);
private:
    bool saveFile(const QString &filename);

    void closeEvent(QCloseEvent *e);

    void showEvent(QShowEvent *);

private:
    Ui::chatroom *ui;

    QUdpSocket* m_udpSocket;
    qint16 m_iPort;
    QString m_sName;
    QColor color;

    server *srv;
    QString m_sFileName;
};

#endif // CHATROOM_H
