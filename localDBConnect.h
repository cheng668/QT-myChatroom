#ifndef LOCALDBCONNECT_H
#define LOCALDBCONNECT_H
#include <QObject>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
class localDBConnect : public QObject{
    Q_OBJECT
public:
    ~localDBConnect();

private:
    //单例
    explicit localDBConnect();
    //判断表是否存在
    bool isExitTable(QString& table);
    void createTable(QString& table);
    void deleteTable(QString& table);

private:
    QSqlDatabase db;
    QSqlQuery query;

};

#endif // LOCALDBCONNECT_H
