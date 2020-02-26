#include "titlewidget.h"
#include "ui_titlewidget.h"
#include <QDateTime>
#include <QPixmap>
#include <QTimer>

TitleWidget::TitleWidget(QWidget *parent) : QWidget(parent), ui(new Ui::TitleWidget) {
    ui->setupUi(this);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateDateTime()));
    timer->start(1000);

    setWeeked(QDate::currentDate().toString("dddd"));

    const QString &style = QString("background-color:transparent; color: white");
    ui->labelTitle->setStyleSheet(style);
    ui->labelWeeked->setStyleSheet(style);
    ui->labelDataTime->setStyleSheet(style);
}

TitleWidget::~TitleWidget() {
    delete ui;
}

void TitleWidget::setTitleIcon(const QString &iconPath) const {
    ui->labelIcon->setPixmap(QPixmap(iconPath));
    if (!ui->labelIcon->isVisible()) {
        ui->labelIcon->setVisible(true);
    }
}

void TitleWidget::setTitleInfo(const QString &titleInfo) const {
    ui->labelTitle->setText(titleInfo.trimmed());
}

void TitleWidget::setWeeked(const QString &Weeked) const {
    ui->labelWeeked->setText(Weeked.trimmed());
}

void TitleWidget::updateDateTime() {
    ui->labelDataTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

void TitleWidget::on_pushButtonmin_clicked() {
    emit signalMin();
}

void TitleWidget::on_pushButtonClose_clicked() {
    emit signalClose();
}
