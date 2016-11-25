#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QTime>
class QFile;
class QTcpSocket;

namespace Ui {
class client;
}

class client : public QDialog
{
    Q_OBJECT

public:
    explicit client(QWidget *parent = 0);
    ~client();
    void setHostAddr(QHostAddress addr);        //获取发送端IP地址
    void setFileName(QString name);             //获取文件保存路径
    void initClient();
protected:
    void closeEvent(QCloseEvent*);
private slots:
    void newConn();                                     //连接到服务器
    void readMsg();                                     //读取问价数据
    void displayErr(QAbstractSocket::SocketError);      //显示错误信息
    void on_btnCancel_clicked();

private:
    Ui::client *ui;

    QTcpSocket* m_pConn;
    quint16 m_uBlockSize;
    QHostAddress hostAddr;
    qint16 m_iPort;
    qint64 m_iTotalBytes;       //总需发送字节数
    qint64 m_iBytesRec;         //已接收字节数
    qint64 m_iFileNameSize;     //文件名字节数
    QString m_sFileName;        //文件名
    QFile* m_pFile;             //待接收的文件
    QByteArray m_bInBlock;      //缓存一次接收的数据
    QTime time;
};

#endif // CLIENT_H
