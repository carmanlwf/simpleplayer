#include "customwall.h"
#include "ui_customwall.h"

CustomWall::CustomWall(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomWall)
{
    ui->setupUi(this);
}

CustomWall::~CustomWall()
{
    delete ui;
}
