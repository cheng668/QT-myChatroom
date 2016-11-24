#ifndef DRAWER_H
#define DRAWER_H

#include <QToolBox>

class chatroom;

namespace Ui {
class drawer;
}

class drawer : public QToolBox
{
    Q_OBJECT

public:
    explicit drawer(QWidget *parent = 0,Qt::WindowFlags f = 0);
    ~drawer();

private slots:
    void on_btnFriend1_clicked();

    void on_btnFriend2_clicked();
private:
    Ui::drawer *ui;

    chatroom* chatroom1 = nullptr;
    chatroom* chatroom2 = nullptr;
};

#endif // DRAWER_H
