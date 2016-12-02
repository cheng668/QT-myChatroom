#include "localDBConnect.h"
#include <QDebug>
#include <QMessageBox>

bool localDBConnect::isExitTable(QString &table)
{
    QString sSql = "SELECT * FROM sqlite_master WHERE type='table' and name = '" + table + "' ";
    if(!query.exec(sSql)) qDebug() << "查询表"+table+"出错！："<< query.lastError() ;
    return query.next();
}

void localDBConnect::createTable(QString &table)
{
    QString sSql = " ";
    if(table == "user_login")
    {
         sSql.append( " Create table user_login (userno TEXT primary key ,password TEXT ,"
                      " isSavePw char(1) default '0' , isAutoLog char(1) default '0')");
    }
    if(!query.exec(sSql))
        QMessageBox::warning(this,QStringLiteral("创建数据库" + table + "失败！"));
}

void localDBConnect::deleteTable(QString &table)
{
    if(!query.exec(QStringLiteral(" drop table "+ table)))
        QMessageBox::warning(this,QStringLiteral("删除数据库" + table + "失败\n" + query.lastError()));
}

localDBConnect::localDBConnect()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.transaction();
    db.setDatabaseName("localDB");

}
