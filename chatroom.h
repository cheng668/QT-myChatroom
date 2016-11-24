#ifndef CHATROOM_H
#define CHATROOM_H

#include <QDialog>
#include <QEvent>
class QUdpSocket;
class QColor;
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
private slots:
    void processPendingDatagrams();                      //接收UDP消息

    void on_btnSend_clicked();

    void on_cbxSize_currentIndexChanged(const QString &pointsize);

    void on_btnBold_clicked(bool checked);

    void on_cbxFont_currentFontChanged(const QFont &f);

    void on_btnItalic_clicked(bool checked);

    void on_btnUnderline_clicked(bool checked);

    void on_btnColor_clicked();

    void on_btnClear_clicked();

    void on_btnSave_clicked();
private:
    bool saveFile(const QString &filename);

    void closeEvent(QCloseEvent *e);

private:
    Ui::chatroom *ui;

    QUdpSocket* m_udpSocket;
    qint16 m_iPort;
    QString m_sName;
    QColor color;
};

#endif // CHATROOM_H
