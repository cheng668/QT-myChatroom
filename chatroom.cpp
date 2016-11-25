#include "chatroom.h"
#include "ui_chatroom.h"
#include "server.h"
#include "client.h"
#include <QtNetwork>
#include <QHostInfo>
//#include <QNetworkInterface>
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
    QDialog(parent),m_udpSocket(new QUdpSocket(this)),srv(new server(this)),m_sName(usrname),
    ui(new Ui::chatroom)
{
    ui->setupUi(this);
    setupUi();
}

void chatroom::setupUi()
{
    /*只显示列头，表格宽度按内容调整*/
    ui->tblUsr->verticalHeader()->setHidden(true);
    ui->tblUsr->horizontalHeader()->setHidden(false);
    ui->tblUsr->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    /*固定端口23232*/
    m_iPort = 23232;
    /*绑定udp连接，ip为任意，端口23232，模式*/
    m_udpSocket->bind(m_iPort,QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    /*文件发送服务器点击发送按钮*/
    connect(srv,SIGNAL(sndFileName(QString)),this,SLOT(getFileName(QString)));
    /*光标改变时，读取光标所在位置字体*/
    connect(ui->teMsg,SIGNAL(currentCharFormatChanged(QTextCharFormat)),this,SLOT(curFmtChanged(QTextCharFormat)));
}

chatroom::~chatroom()
{
    delete ui;
}
/*用户进入聊天室*/
void chatroom::usrEnter(QString usrname, QString ipaddr)
{
    /*查看用户在线列表中是否有该用户*/
    bool isEmpty = ui->tblUsr->findItems(usrname,Qt::MatchExactly).isEmpty();
    if(isEmpty)
    {
        /*在用户在线列表中添加新进入用户用户名和ip地址*/
        QTableWidgetItem *usr = new QTableWidgetItem(usrname);
        QTableWidgetItem *ip = new QTableWidgetItem(ipaddr);
        ui->tblUsr->insertRow(0);
        ui->tblUsr->setItem(0,0,usr);
        ui->tblUsr->setItem(0,1,ip);
        ui->tbsrMsg->setTextColor(Qt::gray);
        ui->tbsrMsg->setCurrentFont(QFont("Times New Roman",10));
        ui->tbsrMsg->append(QStringLiteral("%1在线！").arg(usrname));
        ui->lbUsrNum->setText(QStringLiteral("在线人数：%1").arg(ui->tblUsr->rowCount()));
        /*发送消息，告知新进入用户，便于在新进入用户在线列表中显示已在线用户*/
        sndMsg(UsrEnter);
    }
}
/*用户离开聊天室*/
void chatroom::usrLeft(QString usrname, QString time)
{
    /*删除在线列表中离开的用户，此处默认在在线列表中能找到离开的用户*/
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
    /*发送聊天消息*/
    case Msg:
        if(ui->teMsg->toPlainText() == "")
        {
            QMessageBox::warning(0,QStringLiteral("警告"),QStringLiteral("发送的内容不能为空"),QMessageBox::Ok);
            return;
        }
        out << address << getMsg();
        ui->tbsrMsg->verticalScrollBar()->setValue(ui->tbsrMsg->verticalScrollBar()->maximum());
        break;
    /*发送用户进入聊天室的消息*/
    case UsrEnter:
        out << address;
        break;
    /*文件发送服务器待发送的文件名字，此消息是对文件客户端的接受请求*/
    case FileName:
        {
        /*根据所选的用户在线名单，写入接收用户ip*/
        int row = ui->tblUsr->currentRow();
        QString clntaddr = ui->tblUsr->item(row,1)->text();
        out << address << clntaddr << m_sFileName;
        break;
        }
    /*用户离开*/
    case UsrLeft:
        break;
    /*客户端拒绝接受文件*/
    case Refuse:
        out << srvaddr;
        break;
    }
    /*往udp连接中写入消息类型，用户名字，等信息*/
    m_udpSocket->writeDatagram(data.data(),data.size(),QHostAddress::Broadcast,m_iPort);
}
/*获取本机IPv4,用于udp信息发送*/
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
/*获取drawer穿进来的头像名字，用于udp发送*/
QString chatroom::getUsr()
{
    return m_sName;
}
/*获取输入框中文本，转化为html格式，用于udp msg发送*/
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
    /*如果接受人ip非本机，则不做操作*/
    if(ipAddr == clntaddr)
    {
        /*询问用户是否接受文件，不接受就发送udp refuse消息*/
        int btn = QMessageBox::information(this,QStringLiteral("接收文件"),
                                               QStringLiteral("来自%1（%2）的文件:%3,是否接收？")
                                           .arg(usrname).arg(srvaddr).arg(filename),
                                           QMessageBox::Yes,QMessageBox::No);
        if(btn == QMessageBox::Yes)
        {
            QString name = QFileDialog::getSaveFileName(0,QStringLiteral("保存文件"),filename);
            if(!name.isEmpty())
            {
                /*创建接收文件客户端，客户端连接服务器，服务器再发送文件内容*/
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
    /*判断是否有udp信息等待读取*/
    while(m_udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        /*Returns the size of the first pending UDP datagram*/
        datagram.resize(m_udpSocket->pendingDatagramSize());
        /*读取udp信息*/
        m_udpSocket->readDatagram(datagram.data(),datagram.size());
        /*用流读数据*/
        QDataStream in(&datagram,QIODevice::ReadOnly);
        int msgtype;
        in >> msgtype;
        QString usrName,ipAddr,msg;
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        switch(msgtype)
        {
        /*获取聊天消息，并显示*/
        case Msg:
            in >> usrName >> ipAddr >> msg;
            ui->tbsrMsg->setTextColor(Qt::blue);
            ui->tbsrMsg->setCurrentFont(QFont("Times New Roman",12));
            ui->tbsrMsg->append(QStringLiteral("[%1]%2").arg(usrName).arg(time));
            ui->tbsrMsg->append(msg);
            break;
        /*获取用户进入消息，进行显示和再发送*/
        case UsrEnter:
            in >> usrName >> ipAddr;
            usrEnter(usrName,ipAddr);
            break;
        /*获取用户离开消息，删除在线列表纪录*/
        case UsrLeft:
            in >> usrName;
            usrLeft(usrName,time);
            break;
        /*获取文件服务器发送的文件发送请求，并在本机进行提示是否愿意接受*/
        case FileName:
            {
            in >> usrName >> ipAddr;
            QString clntAddr,fileName;
            in >> clntAddr >> fileName;
            hasPendingFile(usrName,ipAddr,clntAddr,fileName);
            break;
            }
        /*收到文件服务器发送文件请求后，本机拒绝接受文件*/
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
/*保存历史聊天纪录*/
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
/*关闭窗口，关闭udp接受消息绑定，发送用户离开udp信息，和showEvent一起保证用户在线列表的正确性*/
void chatroom::closeEvent(QCloseEvent *e)
{
    disconnect(m_udpSocket,SIGNAL(readyRead()),this,SLOT(processPendingDatagrams()));
    sndMsg(UsrLeft);
    QWidget::closeEvent(e);
}
/*每次展示聊天窗口，都进行udp消息接受绑定，发送用户在线udp消息*/
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
    /*字体大小超出可选最小值，则默认选12*/
    if(fmt.fontPointSize() < 8)
    {
        ui->cbxSize->setCurrentIndex(4);
    }else{
        ui->cbxSize->setCurrentIndex(ui->cbxSize->findText(QString::number(fmt.fontPointSize())));
    }
    /*设置各个字体按钮的状态*/
    ui->btnBold->setChecked(fmt.font().bold());
    ui->btnItalic->setChecked(fmt.font().italic());
    ui->btnUnderline->setChecked(fmt.font().underline());
    color = fmt.foreground().color();
}
