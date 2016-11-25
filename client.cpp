#include "client.h"
#include "ui_client.h"
#include <QMessageBox>
#include <QTcpSocket>
#include <QFile>
#include <QTime>

client::client(QWidget *parent) :
    QDialog(parent),m_pConn(new QTcpSocket(this)),
    ui(new Ui::client)
{
    ui->setupUi(this);
    initClient();
}

client::~client()
{
    delete ui;
}

void client::setHostAddr(QHostAddress addr)
{
    hostAddr = addr;
    newConn();
}

void client::setFileName(QString name)
{
    m_pFile = new QFile(name);
}

void client::initClient()
{
    setFixedSize(305,105);
    m_iTotalBytes = 0;
    m_iBytesRec = 0;
    m_iFileNameSize = 0;
    m_iPort = 5555;
    connect(m_pConn,SIGNAL(readyRead()),this,SLOT(readMsg()));
    connect(m_pConn,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(displayErr(QAbstractSocket::SocketError)));
}

void client::closeEvent(QCloseEvent *)
{
    m_pConn->abort();
    if(m_pFile->isOpen())
        m_pFile->close();
    close();
}

void client::newConn()
{
    m_uBlockSize = 0;
    m_pConn->abort();
    m_pConn->connectToHost(hostAddr,m_iPort);
    time.start();
}

void client::readMsg()
{
    QDataStream in(m_pConn);
    in.setVersion(QDataStream::Qt_5_3);
    float useTime = time.elapsed();
    /*首次接收*/
    if(m_iBytesRec <= sizeof(qint64)*2)
    {
        if(m_pConn->bytesAvailable() >= sizeof(sizeof(qint64)*2) && m_iFileNameSize == 0)
        {
            in >> m_iTotalBytes >> m_iFileNameSize;
            m_iBytesRec += sizeof(qint64)*2;
        }
        if(m_pConn->bytesAvailable() >= m_iFileNameSize && m_iFileNameSize != 0)
        {
            in >> m_sFileName;
            m_iBytesRec += m_iFileNameSize;
            if(!m_pFile->open(QFile::WriteOnly)){
                QMessageBox::warning(this,QStringLiteral("应用程序"),
                                     QStringLiteral("无法读取文件%1:\n%2.").arg(m_sFileName).arg(m_pFile->errorString()));
                return;
            }else
                return;
        }
    }

    if(m_iBytesRec < m_iTotalBytes)
    {
        m_iBytesRec += m_pConn->bytesAvailable();
        m_bInBlock = m_pConn->readAll();
        m_pFile->write(m_bInBlock);
        m_bInBlock.resize(0);
    }
    ui->progressBar->setMaximum(m_iTotalBytes);
    ui->progressBar->setValue(m_iBytesRec);
    double speed = m_iBytesRec/useTime;
    ui->lbStatus->setText(QStringLiteral("已接收+%1MB(%2MB/s)\n共%3MB 已用时:%4秒\n估计剩余时间:%5秒")
                          .arg(m_iBytesRec/(1024*1024))
                          .arg(speed*1000/(1024*1024),0,'f',2)
                          .arg(m_iTotalBytes/(1024*1024))
                          .arg(useTime/1000,0,'f',0)
                          .arg(m_iTotalBytes/speed/1000 - useTime/1000,0,'f',0));
   // ui->lbStatus->adjustSize();

    if(m_iBytesRec == m_iTotalBytes)
    {
        m_pFile->close();
        m_pConn->close();
        ui->lbStatus->setText(QStringLiteral("接收文件%1完毕").arg(m_sFileName));
        ui->btnCancel->setText(QStringLiteral("完成"));
    }
}

void client::displayErr(QAbstractSocket::SocketError error)
{
    switch(error)
    {
    case QAbstractSocket::RemoteHostClosedError:
        QMessageBox::warning(this,QStringLiteral("客户端连接出错"),QStringLiteral("服务器已关闭！"));
        break;
    default:
        QMessageBox::warning(this,QStringLiteral("客户端连接出错"),m_pConn->errorString());
    }
}

void client::on_btnCancel_clicked()
{
    m_pConn->abort();
    if(m_pFile->isOpen())
        m_pFile->close();
    close();
}
