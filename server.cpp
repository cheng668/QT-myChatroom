#include "server.h"
#include "ui_server.h"
#include <QFile>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

server::server(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::server)
{
    ui->setupUi(this);
    initSrv();
}

server::~server()
{
    delete ui;
}

void server::initSrv()
{
    setFixedSize(240,140);
    m_iPort = 5555;
    m_iPayloadSize = 64*1024;
    m_iTotalBytes = 0;
    m_iBytesToWrite = 0;
    m_iBytesWriteen = 0;
    ui->lbStatus->setText(QStringLiteral("请选择要传送的文件"));
    ui->progressBar->reset();
    ui->btnOpen->setEnabled(true);
    ui->btnSend->setEnabled(false);

    m_pSrv = new QTcpServer(this);
    connect(m_pSrv,SIGNAL(newConnection()),this,SLOT(sndMsg()));
    m_pSrv->close();
}

void server::refused()
{
    m_pSrv->close();
    ui->lbStatus->setText(QStringLiteral("对方拒绝接受！"));
}
/*发送文件大小消息*/
void server::sndMsg()
{
    ui->btnSend->setEnabled(false);
    ui->btnOpen->setEnabled(false);
    m_pConn = m_pSrv->nextPendingConnection();
    connect(m_pConn,SIGNAL(bytesWritten(qint64)),this,SLOT(updClntProgress(qint64)));
    ui->lbStatus->setText(QStringLiteral("开始发送文件%1!").arg(m_sTheFileName));
    m_pFile = new QFile(m_sFileName);
    if(!m_pFile->open(QFile::ReadOnly))
    {
        QMessageBox::warning(this,QStringLiteral("应用程序"),
                             QStringLiteral("无法读取文件%1:\n%2").arg(m_sFileName).arg(m_pFile->errorString()));
        return;
    }
    m_iTotalBytes = m_pFile->size();
    QDataStream out(&m_bOutBlock,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);
    time.start();
    out << qint64(0) << qint64(0) << m_sTheFileName;
    m_iTotalBytes += m_bOutBlock.size();
    out.device()->seek(0);
    out << m_iTotalBytes << qint64((m_bOutBlock.size() - sizeof(qint64)*2));
    m_iBytesToWrite = m_iTotalBytes - m_pConn->write(m_bOutBlock);
    m_bOutBlock.resize(0);
}

void server::updClntProgress(qint64 numBytes)
{
    qApp->processEvents(); //界面不冻结
    m_iBytesWriteen += static_cast<int>(numBytes);
    /*发送剩余数据*/
    if(m_iBytesToWrite > 0){
        m_bOutBlock = m_pFile->read(qMin(m_iBytesToWrite,m_iPayloadSize));
        m_iBytesToWrite -= static_cast<int>(m_pConn->write(m_bOutBlock));
        m_bOutBlock.resize(0);
    }
    else
        m_pFile->close();

    ui->progressBar->setMaximum(m_iTotalBytes);
    ui->progressBar->setValue(m_iBytesWriteen);
    float useTime = time.elapsed();
    double speed = m_iBytesWriteen/useTime;
    ui->lbStatus->setText(QStringLiteral("已发送%1MB(%2MB/s)\n共%3MB 已用时:%4秒\n估计剩余时间:%5秒")
                          .arg(m_iBytesWriteen/(1024*1024))
                          .arg(speed*1000/(1024*1024),0,'f',2)
                          .arg(m_iTotalBytes/(1024*1024))
                          .arg(useTime/1000,0,'f',0)
                          .arg(m_iTotalBytes/speed/1000 - useTime/1000,0,'f',0));
    //ui->lbStatus->adjustSize();
    if(m_iBytesWriteen == m_iTotalBytes)
    {
        m_pFile->close();
        m_pSrv->close();
        ui->lbStatus->setText(QStringLiteral("传送文件%1成功").arg(m_sTheFileName));
    }
}

void server::on_btnOpen_clicked()
{
    m_sFileName = QFileDialog::getOpenFileName(this,QStringLiteral("选择要传送的文件/文件夹"),"./");
    if(!m_sFileName.isEmpty())
    {
        m_sTheFileName = m_sFileName.right(m_sFileName.size()-m_sFileName.lastIndexOf('/')-1);
        ui->lbStatus->setText(QStringLiteral("要传送的文件:%1").arg(m_sTheFileName));
        ui->btnSend->setEnabled(true);
    }
}

void server::on_btnSend_clicked()
{
    if(!m_pSrv->listen(QHostAddress::Any,m_iPort))
    {
        qDebug() << m_pSrv->errorString();
        QMessageBox::warning(this,QStringLiteral("传送失败"),m_pSrv->errorString(),QMessageBox::Ok);
        close();
        return;
    }
    ui->lbStatus->setText(QStringLiteral("等待对方接受... ..."));
    emit sndFileName(m_sTheFileName);
}

void server::closeEvent(QCloseEvent *e)
{
    if(m_pSrv->isListening())
    {
        m_pSrv->close();
        if(m_pFile->isOpen())
            m_pFile->close();
        m_pConn->abort();
    }
    close();
}
