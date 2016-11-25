#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QTime>
class QFile;
class QTcpServer;
class QTcpSocket;

namespace Ui {
class server;
}

class server : public QDialog
{
    Q_OBJECT

public:
    explicit server(QWidget *parent = 0);
    ~server();
    void initSrv();         //初始化服务器
    void refused();         //关闭服务器
protected:
    void closeEvent(QCloseEvent *);
private slots:
    /*发送数据*/
    void sndMsg();
    /*更新进度条*/
    void updClntProgress(qint64 numBytes);
    void on_btnOpen_clicked();

    void on_btnSend_clicked();

signals:
    void sndFileName(QString fileName);

private:
    Ui::server *ui;
    qint16 m_iPort;
    QTcpServer* m_pSrv;
    QString m_sFileName;
    QString m_sTheFileName;
    QFile* m_pFile;             //待发送文件
    qint64 m_iTotalBytes;       //总需发送字节数
    qint64 m_iBytesWriteen;     //已发送字节数
    qint64 m_iBytesToWrite;     //待发送字节数
    qint64 m_iPayloadSize;      //最大传输量
    QByteArray m_bOutBlock;     //缓存一次发送的数据
    QTcpSocket* m_pConn;        //客户端连接套接字
    QTime time;
};

#endif // SERVER_H
