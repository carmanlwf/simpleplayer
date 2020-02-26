/******************************************************************************
    Simple Player:  this file is part of QtAV examples
    Copyright (C) 2012-2016 Wang Bin <wbsecg1@gmail.com>

*   This file is part of QtAV

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QContextMenuEvent>
#include <QLabel>
#include <QMenu>
#include <QPoint>
#include <QWidget>
#include <QtAV>

QT_BEGIN_NAMESPACE
class QSlider;
class QPushButton;
QT_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QDragEnterEvent;
class QDropEvent;
QT_END_NAMESPACE

namespace QtAV {
class AudioOutput;
class AVError;
class AVPlayer;
} // namespace QtAV

class PlayerWindow : public QWidget {
    Q_OBJECT
    public:
    explicit PlayerWindow(QWidget *parent = nullptr);

    enum VideoEnum {
        OpenVideo = 1,
        CloseVideo,
    };

    enum StreamTypeEnum {
        MainStream = 1,
        SubStream,
    };

    enum IntercomEnum {
        OpenIntercom = 1,
        CloseIntercom,
    };

    enum AudioEnum {
        OpenAudio = 1,
        CloseAudio,
    };

    enum RatioEnum {
        OriginalRatio = 1,
        WindowRatio,
    };

    void playUrl(const QString &url);

    void showDevSn(const QString &sn) const;

    protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

    void enterEvent(QEvent *event) override;

    void leaveEvent(QEvent *event) override;

    public Q_SLOTS:
    void openMedia();
    void seekBySlider(int value);
    void seekBySlider();
    void playPause();
    void playFile(const QString &file);

    private Q_SLOTS:
    void updateSlider(qint64 value);
    void updateSlider();
    void updateSliderUnit();
    void handleError(const QtAV::AVError &e);
    void handleFullscreenChange();

    void onMenuActStopPlay();
    void onMenuActVideoFull();
    void onMenuActSnap();
    void onMenuActRemote();
    void onMenuActZoomIn();

    void videoMenuTriggered(QAction *action);
    void streamMenuTriggered(QAction *action);
    void intercomMenuTriggered(QAction *action);
    void ratioMenuTriggered(QAction *action);
    void audioMenuTriggered(QAction *action);

    private:
    void performDrag(); // 处理拖拽
    QPoint startPos;

    void CreateMenu();

    private:
    QtAV::AVPlayer *m_player;
    QSlider *m_slider;
    QPushButton *m_openBtn;
    QPushButton *m_playBtn;
    QPushButton *m_stopBtn;
    QLabel *m_fileName{};
    int m_unit;

    QMenu *menuMain{};
    QMenu *menuVideo{};
    QMenu *menuStream{};
    QMenu *menuIntercom{};
    QMenu *menuRatio{};
    QMenu *menuAudio{};
};

#endif // PLAYERWINDOW_H
