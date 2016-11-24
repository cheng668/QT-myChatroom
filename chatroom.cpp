#include "chatroom.h"
#include "ui_chatroom.h"
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
chatroom::chatroom(QWidget *parent,QString usrname) :
    QDialog(parent),m_udpSocket(new QUdpSocket(this)),m_sName(usrname),
    ui(new Ui::chatroom)
{
    ui->setupUi(this);
    setupUi();
    sndMsg(UsrEnter);
}

void chatroom::setupUi()
{
    ui->tblUsr->verticalHeader()->setHidden(true);
    ui->tblUsr->horizontalHeader()->setHidden(false);
    ui->tblUsr->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    m_iPort = 23232;
    m_udpSocket->bind(m_iPort,QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);//绑定接收
    connect(m_udpSocket,SIGNAL(readyRead()),this,SLOT(processPendingDatagrams()));
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
        break;
    case UsrLeft:
        break;
    case Refuse:
        break;
    }
    m_udpSocket->writeDatagram(data.data(),data.size(),QHostAddress::Broadcast,m_iPort);
}

QString chatroom::getIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
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
            break;
        case Refuse:
            break;
        }
    }
}

void chatroom::on_btnSend_clicked()
{
    sndMsg(Msg);
}

void chatroom::on_cbxSize_currentIndexChanged(const QString &pointsize)
{
    ui->teMsg->setFontPointSize(pointsize.toDouble());
    ui->teMsg->setFocus();
}

void chatroom::on_btnBold_clicked(bool checked)
{
    if(checked)
        ui->teMsg->setFontWeight(QFont::Bold);
    else
        ui->teMsg->setFontWeight(QFont::Normal);
    ui->teMsg->setFocus();
}

void chatroom::on_cbxFont_currentFontChanged(const QFont &f)
{
    ui->teMsg->setCurrentFont(f);
    ui->teMsg->setFocus();
}

void chatroom::on_btnItalic_clicked(bool checked)
{
    ui->teMsg->setFontItalic(checked);
    ui->teMsg->setFocus();
}

void chatroom::on_btnUnderline_clicked(bool checked)
{
    ui->teMsg->setFontUnderline(checked);
    ui->teMsg->setFocus();
}

void chatroom::on_btnColor_clicked()
{
    color = QColorDialog::getColor(color,this,QStringLiteral("选择字体颜色"));
    if(color.isValid())
        ui->teMsg->setTextColor(color);
    ui->teMsg->setFocus();
}

void chatroom::on_btnClear_clicked()
{
    ui->tbsrMsg->clear();
}

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
    sndMsg(UsrLeft);
    QWidget::closeEvent(e);
    delete this; //彻底删除本对话框，防止隐藏对话框接收发送消息
}
