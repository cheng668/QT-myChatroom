#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>

namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = 0);
    ~login();

private slots:
    void on_ccbUserName_editTextChanged(const QString &arg1);

    void on_ccbUserName_currentTextChanged(const QString &arg1);

private:
    void createLocalDB();
private:
    Ui::login *ui;
};

#endif // LOGIN_H
