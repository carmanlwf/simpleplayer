#ifndef FORM_H
#define FORM_H

#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QList>
#include <QMetaType>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslError>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QWidget>
namespace Ui {
class Form;
}

class Form : public QWidget {
    Q_OBJECT

    public:
    explicit Form(QWidget *parent = 0);
    ~Form();

    struct devstatus {
        QString deviceMac;     //设备sn号
        bool connectionStatus; //在线状态；0：离线；1：在线
        bool streamStatus;     //在播状态；0：未播；1：在播
    };

    struct urlMessage {
        bool result;     //获取流地址结果，0：失败，1：成功
        QString m3u8Url; //直播m3u8流地址
        QString rtmpUrl; // rtmp流地址
    };

    private:
    public slots:
    void downloadFinished(QNetworkReply *reply);
    // void sslErrors(const QList<QSslError> &errors);
    void slotUpdate();
    void slotPlay();
    void getDevSNlist();
    void getDevInfo(int currentIndex = 0, int limitNumber = 9);
    void getRtmpUrl();

    void slotGetDevRet();
    void slotGetRtmpUrRetl();
    void slotChanged();
    void loadInI();
    void saveInI() const;

    void initPlayWidget();

    protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    private:
    Ui::Form *ui;
    QNetworkAccessManager manager;
    QNetworkReply *reply;
    QList<QString> devSNList;
    // QList<QWidget *> widegetList;
    // QList<QWidget *> widegetTwoList;
    QList<QWidget *> playList;

    QHash<QString, devstatus> devStatusHash;
    QHash<QString, urlMessage> devUrlMessageHash;
    QTimer *changedTimer;

    int currentIndex{ 0 };
    int limitNumber{ 0 };
    int m_interval{ 0 };
    bool mouse_press{ false };
    QPoint move_point;
};

#endif // FORM_H
