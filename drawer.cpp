#include "drawer.h"
#include "ui_drawer.h"
#include "chatroom.h"
drawer::drawer(QWidget *parent,Qt::WindowFlags f) :
    QToolBox(parent,f),
    ui(new Ui::drawer)
{
    ui->setupUi(this);
}

drawer::~drawer()
{
    delete ui;
}

void drawer::on_btnFriend1_clicked()
{
    if(chatroom1 == nullptr)
        chatroom1 = new chatroom(this,ui->btnFriend1->text());
    chatroom1->setWindowTitle(ui->btnFriend1->text());
    chatroom1->setWindowIcon(ui->btnFriend1->icon());
    chatroom1->show();
}

void drawer::on_btnFriend2_clicked()
{
    if(chatroom2 == nullptr)
        chatroom2 = new chatroom(this,ui->btnFriend2->text());
    chatroom2->setWindowTitle(ui->btnFriend2->text());
    chatroom2->setWindowIcon(ui->btnFriend2->icon());
    chatroom2->show();
}
