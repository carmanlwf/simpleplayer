#ifndef CUSTOMWALL_H
#define CUSTOMWALL_H

#include <QWidget>

namespace Ui {
class CustomWall;
}

class CustomWall : public QWidget
{
    Q_OBJECT

public:
    explicit CustomWall(QWidget *parent = nullptr);
    ~CustomWall();

private:
    Ui::CustomWall *ui;
};

#endif // CUSTOMWALL_H
