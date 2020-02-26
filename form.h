#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include<QHash>
#include <QMetaType>
#include<QTimer>
namespace Ui {
class Form;
}


class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

  struct  devstatus{
     QString    deviceMac;  //设备sn号
     bool        connectionStatus; //在线状态；0：离线；1：在线
     bool       streamStatus;  //在播状态；0：未播；1：在播
     };

    struct  urlMessage{
       bool  result;   //获取流地址结果，0：失败，1：成功
        QString    m3u8Url;  //直播m3u8流地址
        QString    rtmpUrl;  //rtmp流地址
    } ;

private:


public slots:
    void downloadFinished(QNetworkReply *reply);
    //void sslErrors(const QList<QSslError> &errors);
    void slotUpdate();
    void slotPlay();
    void  getDevSNlist();
     void getDevInfo(int currentIndex = 0, int  limitNumber=9);
     void getRtmpUrl();

     void slotGetDevRet();
     void slotGetRtmpUrRetl();
     void slotChanged();

protected:
   void paintEvent(QPaintEvent *e) override;
  void resizeEvent(QResizeEvent *e) override;
private:
    Ui::Form *ui;
    QNetworkAccessManager manager;
    QNetworkReply *reply;
    QList<QString>  devSNList;
    QList<QWidget*> widegetList;
    QList<QWidget*> widegetTwoList;

    QHash<QString, devstatus>  devStatusHash;
    QHash<QString,  urlMessage>  devUrlMessageHash;
    QTimer *changedTimer;

    int currentIndex {0};
    int limitNumber {0};
};

#endif // FORM_H
