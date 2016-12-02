#include "login.h"
#include "ui_login.h"

login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
}

login::~login()
{
    delete ui;
}

void login::on_ccbUserName_currentTextChanged(const QString &arg1)
{
    ui->ccbUserName->addItem(arg1);
}

void login::createLocalDB()
{

}
