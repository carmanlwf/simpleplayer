#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QWidget>

namespace Ui {
class TitleWidget;
}

class TitleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TitleWidget(QWidget *parent = 0);
    ~TitleWidget();

    void setTitleIcon(const QString &iconPath) const;
    void setTitleInfo(const QString &titleInfo) const;
    void setWeeked(const QString &Weeked) const;

signals:
    void signalMin();
    void signalClose();
private slots:
    void updateDateTime();

    void on_pushButtonmin_clicked();

    void on_pushButtonClose_clicked();

private:
    Ui::TitleWidget *ui;
};

#endif // TITLEWIDGET_H
