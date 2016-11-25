#include "chatroom.h"
#include "ui_chatroom.h"
#include "server.h"
#include "client.h"
#include <QtNetwork>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QProcess>
#include <QHeaderView>
#include <QColorDialog>
#include <QFileDialog>
#include <QDebug>
#include <QTextCharFormat>
chatroom::chatroom(QWidget *parent,QString usrname) :
    QDialog(parent),m_udpSocket(new QUdpSocket(this)),m_sName(usrname),
    ui(new Ui::chatroom)
{
    ui->setupUi(this);
    setupUi();
}

void chatroom::setupUi()
{
    ui->tblUsr->verticalHeader()->setHidden(true);
    ui->tblUsr->horizontalHeader()->setHidden(false);
    ui->tblUsr->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    m_iPort = 23232;
    m_udpSocket->bind(m_iPort,QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);//绑定接收

    srv = new server(this);
    /*文件发送服务器点击发送按钮*/
    connect(srv,SIGNAL(sndFileName(QString)),this,SLOT(getFileName(QString)));
    /*光标改变时，读取光标所在位置字体*/
    connect(ui->teMsg,SIGNAL(currentCharFormatChanged(QTextCharFormat)),this,SLOT(curFmtChanged(QTextCharFormat)));
}

chatroom::~chatroom()
{
    delete ui;
}

void chatroom::usrEnter(QString usrname, QString ipaddr)
{
    bool isEmpty = ui->tblUsr->findItems(usrname,Qt::MatchExactly).isEmpty();
    if(isEmpty)
    {
        QTableWidgetItem *usr = new QTableWidgetItem(usrname);
        QTableWidgetItem *ip = new QTableWidgetItem(ipaddr);
        ui->tblUsr->insertRow(0);
        ui->tblUsr->setItem(0,0,usr);
        ui->tblUsr->setItem(0,1,ip);
        ui->tbsrMsg->setTextColor(Qt::gray);
        ui->tbsrMsg->setCurrentFont(QFont("Times New Roman",10));
        ui->tbsrMsg->append(QStringLiteral("%1在线！").arg(usrname));
        ui->lbUsrNum->setText(QStringLiteral("在线人数：%1").arg(ui->tblUsr->rowCount()));
        sndMsg(UsrEnter);   //告知新加入用户
    }
}

void chatroom::usrLeft(QString usrname, QString time)
{
    int rowNum = ui->tblUsr->findItems(usrname,Qt::MatchExactly).first()->row();
    ui->tblUsr->removeRow(rowNum);
    ui->tbsrMsg->setTextColor(Qt::gray);
    ui->tbsrMsg->setCurrentFont(QFont("Times New Roman",10));
    ui->tbsrMsg->append(QStringLiteral("%1于%2离开！").arg(usrname).arg(time));
    ui->lbUsrNum->setText(QStringLiteral("在线人数：%1").arg(ui->tblUsr->rowCount()));
}
//广播UDP消息
void chatroom::sndMsg(MsgType type, QString srvaddr)
{
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    QString address = getIP();
    out << type << getUsr();
    switch(type)
    {
    case Msg:
        if(ui->teMsg->toPlainText() == "")
        {
            QMessageBox::warning(0,QStringLiteral("警告"),QStringLiteral("发送的内容不能为空"),QMessageBox::Ok);
            return;
        }
        out << address << getMsg();
        ui->tbsrMsg->verticalScrollBar()->setValue(ui->tbsrMsg->verticalScrollBar()->maximum());
        break;
    case UsrEnter:
        out << address;
        break;
    case FileName:
        {
        int row = ui->tblUsr->currentRow();
        QString clntaddr = ui->tblUsr->item(row,1)->text();
        out << address << clntaddr << m_sFileName;
        break;
        }
    case UsrLeft:
        break;
    case Refuse:
        out << srvaddr;
        break;
    }
    m_udpSocket->writeDatagram(data.data(),data.size(),QHostAddress::Broadcast,m_iPort);
}

QString chatroom::getIP()
{
    QString localHostName = QHostInfo::localHostName();
    QHostInfo info = QHostInfo::fromName(localHostName);
    QList<QHostAddress> list = info.addresses();
   // QList<QHostAddress> list = QNetworkInterface::allAddresses();
    qDebug() << list;
    foreach (QHostAddress addr, list) {
        if(addr.protocol() == QAbstractSocket::IPv4Protocol)
            return addr.toString();
    }
    return 0;
}

QString chatroom::getUsr()
{
    return m_sName;
}

QString chatroom::getMsg()
{
    QString msg = ui->teMsg->toHtml();
    ui->teMsg->clear();
    ui->teMsg->setFocus();
    return msg;
}
/*判断是否接收filename类型udp消息*/
void chatroom::hasPendingFile(QString usrname, QString srvaddr, QString clntaddr, QString filename)
{
    QString ipAddr = getIP();
    if(ipAddr == clntaddr)
    {
        int btn = QMessageBox::information(this,QStringLiteral("接收文件"),
                                               QStringLiteral("来自%1（%2）的文件:%3,是否接收？")
                                           .arg(usrname).arg(srvaddr).arg(filename),
                                           QMessageBox::Yes,QMessageBox::No);
        if(btn == QMessageBox::Yes)
        {
            QString name = QFileDialog::getSaveFileName(0,QStringLiteral("保存文件"),filename);
            if(!name.isEmpty())
            {
                client* clnt = new client(this);
                clnt->setFileName(name);
                clnt->setHostAddr(QHostAddress(srvaddr));
                clnt->show();
            }
        }
        else
            sndMsg(Refuse,srvaddr);
    }
}
/*接收UDP消息*/
void chatroom::processPendingDatagrams()
{
    while(m_udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());

        m_udpSocket->readDatagram(datagram.data(),datagram.size());

        QDataStream in(&datagram,QIODevice::ReadOnly);
        int msgtype;
        in >> msgtype;
        QString usrName,ipAddr,msg;
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        switch(msgtype)
        {
        case Msg:
            in >> usrName >> ipAddr >> msg;
            ui->tbsrMsg->setTextColor(Qt::blue);
            ui->tbsrMsg->setCurrentFont(QFont("Times New Roman",12));
            ui->tbsrMsg->append(QStringLiteral("[%1]%2").arg(usrName).arg(time));
            ui->tbsrMsg->append(msg);
            break;
        case UsrEnter:
            in >> usrName >> ipAddr;
            usrEnter(usrName,ipAddr);
            break;
        case UsrLeft:
            in >> usrName;
            usrLeft(usrName,time);
            break;
        case FileName:
            {
            in >> usrName >> ipAddr;
            QString clntAddr,fileName;
            in >> clntAddr >> fileName;
            hasPendingFile(usrName,ipAddr,clntAddr,fileName);
            break;
            }
        case Refuse:
            {
            in >> usrName;
            QString srvAddr;
            in >> srvAddr;
            QString ipAddr = getIP();
            if(ipAddr == srvAddr)
            {
                srv->refused();
            }
            break;
            }
        }
    }
}
/*发送聊天消息*/
void chatroom::on_btnSend_clicked()
{
    sndMsg(Msg);
}
/*字体大小*/
void chatroom::on_cbxSize_currentIndexChanged(const QString &pointsize)
{
    ui->teMsg->setFontPointSize(pointsize.toDouble());
    ui->teMsg->setFocus();
}
/*粗体*/
void chatroom::on_btnBold_clicked(bool checked)
{
    if(checked)
        ui->teMsg->setFontWeight(QFont::Bold);
    else
        ui->teMsg->setFontWeight(QFont::Normal);
    ui->teMsg->setFocus();
}
/*字体*/
void chatroom::on_cbxFont_currentFontChanged(const QFont &f)
{
    ui->teMsg->setCurrentFont(f);
    ui->teMsg->setFocus();
}
/*斜体*/
void chatroom::on_btnItalic_clicked(bool checked)
{
    ui->teMsg->setFontItalic(checked);
    ui->teMsg->setFocus();
}
/*下划线*/
void chatroom::on_btnUnderline_clicked(bool checked)
{
    ui->teMsg->setFontUnderline(checked);
    ui->teMsg->setFocus();
}
/*字体颜色*/
void chatroom::on_btnColor_clicked()
{
    color = QColorDialog::getColor(color,this,QStringLiteral("选择字体颜色"));
    if(color.isValid())
        ui->teMsg->setTextColor(color);
    ui->teMsg->setFocus();
}
/*清空聊天记录*/
void chatroom::on_btnClear_clicked()
{
    ui->tbsrMsg->clear();
}
/*获取文件服务器发送的文件名，并发送filename消息*/
void chatroom::getFileName(QString filename)
{
    m_sFileName = filename;
    sndMsg(FileName);
}
/*保存历史消息*/
void chatroom::on_btnSave_clicked()
{
    if(ui->tbsrMsg->document()->isEmpty())
    {
        QMessageBox::warning(0,QStringLiteral("警告"),QStringLiteral("聊天记录为空，无法保存!"),QMessageBox::Ok);
    }else{
        QString fname = QFileDialog::getSaveFileName(this,QStringLiteral("保存聊天记录"),
                                                     QStringLiteral("聊天记录"),
                                                     QStringLiteral("文本(*.txt);;所有文件(*.*)"));
        if(!fname.isEmpty())
            saveFile(fname);
    }
}

bool chatroom::saveFile(const QString &filename)
{
    QFile file(filename);
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this,QStringLiteral("保存文件"),
                             QStringLiteral("无法保存文件%1:\n%2").arg(filename).arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out << ui->tbsrMsg->toPlainText();
    return true;
}

void chatroom::closeEvent(QCloseEvent *e)
{
    disconnect(m_udpSocket,SIGNAL(readyRead()),this,SLOT(processPendingDatagrams()));
    sndMsg(UsrLeft);
    QWidget::closeEvent(e);
}

void chatroom::showEvent(QShowEvent *e)
{
    connect(m_udpSocket,SIGNAL(readyRead()),this,SLOT(processPendingDatagrams()));
    sndMsg(UsrEnter);
    QWidget::showEvent(e);
}
/*打开文件服务器*/
void chatroom::on_btnFileSend_clicked()
{
    if(ui->tblUsr->selectedItems().isEmpty()){
        QMessageBox::warning(this,QStringLiteral("选择用户"),QStringLiteral("请先选择目标用户"),QMessageBox::Ok);
        return;
    }
    srv->show();
    srv->initSrv();
}
/*光标改变时，读取光标所在位置字体*/
void chatroom::curFmtChanged(const QTextCharFormat &fmt)
{
    ui->cbxFont->setCurrentFont(fmt.font());
    if(fmt.fontPointSize() < 8)
    {
        ui->cbxSize->setCurrentIndex(4);
    }else{
        ui->cbxSize->setCurrentIndex(ui->cbxSize->findText(QString::number(fmt.fontPointSize())));
    }

    ui->btnBold->setChecked(fmt.font().bold());
    ui->btnItalic->setChecked(fmt.font().italic());
    ui->btnUnderline->setChecked(fmt.font().underline());
    color = fmt.foreground().color();
}
