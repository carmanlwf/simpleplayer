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

#include "playerwindow.h"
#include "EventFilter.h"
#include <QApplication>
#include <QDebug>
#include <QDrag>
#include <QEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QLayout>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPushButton>
#include <QSlider>
#include <QtAV/VideoOutput.h>
#include <QtAVWidgets>

using namespace QtAV;

PlayerWindow::PlayerWindow(QWidget *parent) : QWidget(parent) {
    setAcceptDrops(true);
    m_unit = 1000;

    // QtAV::setLogLevel(QtAV::LogOff);
    // QtAV::setFFmpegLogLevel("quiet");
    setWindowTitle(QString::fromLatin1("QtAV simple player example"));
    m_player = new AVPlayer(this);
    // QHash<QString, QVariant> dict;
    //    dict.insert("probesize", 5000000);
    //    dict.insert("analyzeduration", 5000000);
    //    dict.insert("avioflags", "direct");

    auto dict = m_player->optionsForFormat();
    dict.insert("rtsp_transport", "tcp");
    m_player->setOptionsForFormat(dict);

    // qDebug() << "PlayerWindow::PlayerWindow  setVideoDecoderPriority" << m_player->videoDecoderPriority();
    QStringList strList;
    strList << "DXVA"
            << "D3D11"
            << "VAAPI"
            << "CUDA"
            << "FFmpeg";
    // m_player->setVideoDecoderPriority(strList);
    //  m_player->setBufferMode(QtAV::BufferPackets);

    QVariantHash dec_opt(m_player->optionsForVideoCodec());
    dec_opt["copyMode"] = "OptimizedCopy"; // or "GenericCopy"
    // m_player->setOptionsForVideoCodec(dec_opt);

    auto *vl = new QVBoxLayout();
    setLayout(vl);
    vl->setMargin(0);

    setContentsMargins(1, 1, 1, 1);
    m_fileName = new QLabel(this);
    m_fileName->setVisible(false);
    m_fileName->setMaximumHeight(20);
    vl->addWidget(m_fileName);
    // VideoRenderer *vo = VideoRenderer::create(VideoRendererId_GLWidget2); // VideoRendererId_GLWidget2);
    VideoOutput *vo = new VideoOutput(this);
    if (!vo->widget()) {
        QMessageBox::warning(nullptr, QString::fromLatin1("QtAV error"), tr("Can not create video renderer"));
        return;
    }

    m_player->setRenderer(vo);
    vl->addWidget(vo->widget());

    m_slider = new QSlider();
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setVisible(false);
    connect(m_slider, SIGNAL(sliderMoved(int)), SLOT(seekBySlider(int)));
    connect(m_slider, SIGNAL(sliderPressed()), SLOT(seekBySlider()));
    connect(m_player, SIGNAL(positionChanged(qint64)), SLOT(updateSlider(qint64)));
    connect(m_player, SIGNAL(started()), SLOT(updateSlider()));
    connect(m_player, SIGNAL(notifyIntervalChanged()), SLOT(updateSliderUnit()));
    connect(m_player, SIGNAL(error(QtAV::AVError)), this, SLOT(handleError(QtAV::AVError)));

    // vl->addWidget(m_slider);
    auto *hb = new QHBoxLayout();
    vl->addLayout(hb);
    m_openBtn = new QPushButton(tr("Open"));
    m_playBtn = new QPushButton(tr("Play/Pause"));
    m_stopBtn = new QPushButton(tr("Stop"));
    hb->addWidget(m_openBtn);
    hb->addWidget(m_playBtn);
    hb->addWidget(m_stopBtn);
    connect(m_openBtn, SIGNAL(clicked()), SLOT(openMedia()));
    connect(m_playBtn, SIGNAL(clicked()), SLOT(playPause()));
    connect(m_stopBtn, SIGNAL(clicked()), m_player, SLOT(stop()));

    m_openBtn->setHidden(true);
    m_playBtn->setHidden(true);
    m_stopBtn->setHidden(true);

    if (m_player != nullptr) {
        m_player->renderer()->setOutAspectRatioMode(VideoRenderer::RendererAspectRatio);
        // m_player->renderer()->setQuality(VideoRenderer::QualityFastest);
        auto *ao = m_player->audio();
        if (ao != nullptr) {
            ao->close();
        }
    }
    // CreateMenu();
}

void PlayerWindow::playUrl(const QString &url) {
    if (m_player != nullptr) {
        m_player->play(url);
    }
}

void PlayerWindow::showDevSn(const QString &sn) const {

    if (m_fileName != nullptr) {
        m_fileName->setStyleSheet("background-color:transparent; color: white");
        m_fileName->setWindowOpacity(0.8);
        m_fileName->setText(sn);
        m_fileName->raise();
    }
}

void PlayerWindow::dragEnterEvent(QDragEnterEvent *event) {
    auto *source = qobject_cast<PlayerWindow *>(event->source());
    if (source && source != this) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void PlayerWindow::dragMoveEvent(QDragMoveEvent *event) {
    auto *source = qobject_cast<PlayerWindow *>(event->source());
    if (source && source != this) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void PlayerWindow::dropEvent(QDropEvent *event) {
    auto *source = qobject_cast<PlayerWindow *>(event->source());
    if (source && source != this) {
        // todo  处理拖拽事件
        QString localfile = m_fileName->text();
        QString file = event->mimeData()->text();
        this->playFile(file);
        source->playFile(localfile);

        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void PlayerWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        startPos = event->pos();
    }

    QWidget::mousePressEvent(event);
}

void PlayerWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - startPos).manhattanLength();
        if (distance >= QApplication::startDragDistance()) {
            performDrag();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void PlayerWindow::contextMenuEvent(QContextMenuEvent *event) {
    Q_UNUSED(event);
    menuMain->move(cursor().pos()); //让菜单显示的位置在鼠标的坐标上
    menuMain->show();
}

void PlayerWindow::resizeEvent(QResizeEvent *e) {
    Q_UNUSED(e);
    QWidget::resizeEvent(e);
}

void PlayerWindow::paintEvent(QPaintEvent *e) {
    QPainter painter2(this);
    painter2.setPen(Qt::NoPen);
    painter2.setBrush(QColor(47, 51, 56));
    painter2.drawRect(QRect(0, 0, this->width(), this->height()));

    QPainter painter3(this);
    painter3.setPen(QColor(63, 63, 73));
    painter3.drawLine(0, 0, this->width() - 1, 0);
    painter3.drawLine(0, 0, 0, this->height() - 1);
    painter3.drawLine(this->width() - 1, 0, this->width() - 1, this->height() - 1);
    painter3.drawLine(0, this->height() - 1, this->width() - 1, this->height() - 1);
}

void PlayerWindow::enterEvent(QEvent *event) {
    if (m_fileName != nullptr) {
        m_fileName->setVisible(true);
    }
}

void PlayerWindow::leaveEvent(QEvent *event) {
    if (m_fileName != nullptr) {
        m_fileName->setVisible(false);
    }
}

void PlayerWindow::openMedia() {
    QString file = QInputDialog::getText(nullptr, tr("Open an url"), tr("Url"));
    if (file.isEmpty())
        return;
    playFile(file);
    qDebug() << "PlayerWindow::openMedia " << file;
}

void PlayerWindow::seekBySlider(int value) {
    if (!m_player->isPlaying())
        return;
    m_player->seek(qint64(value * m_unit));
    // qDebug()<<"PlayerWindow::seekBySlider "<<value*m_unit;
}

void PlayerWindow::seekBySlider() {
    seekBySlider(m_slider->value());
}

void PlayerWindow::playPause() {
    if (!m_player->isPlaying()) {
        m_player->play();
        return;
    }
    m_player->pause(!m_player->isPaused());
    // qDebug()<<"PlayerWindow::playPause "<<m_player->isPlaying();
}

void PlayerWindow::playFile(const QString &file) {
    m_player->play(file);
    setWindowTitle(file);
    m_fileName->setText(file);
}

void PlayerWindow::updateSlider(qint64 value) {
    m_slider->setRange(0, int(m_player->duration() / m_unit));
    m_slider->setValue(int(value / m_unit));
    // qDebug()<<"PlayerWindow::updateSlider"<< value/m_unit;
}

void PlayerWindow::updateSlider() {
    updateSlider(m_player->position());
    // qDebug()<<"PlayerWindow::updateSlider"<< m_player->position();
}

void PlayerWindow::updateSliderUnit() {
    m_unit = m_player->notifyInterval();
    updateSlider();
    // qDebug()<<"PlayerWindow::updateSliderUnit"<< m_unit;
}

void PlayerWindow::handleError(const AVError &e) {
    qDebug() << "PlayerWindow::handleError  Player error " << e.string();
}

void PlayerWindow::handleFullscreenChange() {
    if (m_player->renderer() != nullptr) {
        QSize s = rect().size();
        // resize(QSize(s.width()-1, s.height()-1));
        // resize(s); //window resize to fullscreen size will create another fullScreenChange event
        m_player->renderer()->widget()->resize(QSize(s.width() + 1, s.height() + 1));
        m_player->renderer()->widget()->resize(s);
    }
}

void PlayerWindow::onMenuActStopPlay() {
    m_player->stop();
}

void PlayerWindow::onMenuActVideoFull() {

    if (Qt::WindowFullScreen == windowState()) {
        setWindowState(windowState() ^ Qt::WindowFullScreen);
    } else {
        showFullScreen();
    }

    this->updateGeometry();
    update();
    QSize s = rect().size();
    // m_player->renderer()->widget()->resize(QSize(s.width()+1, s.height()+1));
    // m_player->renderer()->widget()->resize(s);

    qDebug() << "PlayerWindow::onMenuActVideoFull  size and  state" << s << windowState();
}

void PlayerWindow::onMenuActSnap() {
    if (m_player != nullptr && m_player->isPlaying()) {
        // qDebug()<<"PlayerWindow::onMenuActSnap " << m_player->videoCapture()->captureDir();
        m_player->videoCapture()->capture();
    }
}

void PlayerWindow::onMenuActRemote() {
}

void PlayerWindow::onMenuActZoomIn() {
}

void PlayerWindow::videoMenuTriggered(QAction *action) {
    bool bopen = action->data().toInt() > OpenVideo;
}

void PlayerWindow::streamMenuTriggered(QAction *action) {
    bool bMain = action->data().toInt() > MainStream;
}

void PlayerWindow::intercomMenuTriggered(QAction *action) {
    bool bopen = action->data().toInt() > OpenIntercom;
}

void PlayerWindow::ratioMenuTriggered(QAction *action) {
    if (m_player != nullptr) {
        m_player->renderer()->setOutAspectRatioMode(action->data().toInt() > OriginalRatio ? VideoRenderer::RendererAspectRatio :
                                                                                             VideoRenderer::VideoAspectRatio);
    }
}

void PlayerWindow::audioMenuTriggered(QAction *action) {
    if (m_player != nullptr) {
        auto *ao = m_player->audio();
        if (ao != nullptr) {
            action->data().toInt() > OpenAudio ? ao->close() : ao->open();
        }
    }
}

void PlayerWindow::performDrag() {
    QVariant var = m_fileName->text();
    auto *mimeData = new QMimeData;
    mimeData->setText(var.toString());
    auto *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    if (drag->exec(Qt::MoveAction) == Qt::MoveAction) {
    }
}

void PlayerWindow::CreateMenu() {
    menuMain = new QMenu(this);
    menuVideo = new QMenu(tr("Video"), this);
    menuStream = new QMenu(tr("StreamType"), this);
    menuIntercom = new QMenu(tr("Intercom"), this);
    menuRatio = new QMenu(tr("Ratio"), this);
    menuAudio = new QMenu(tr("Audio"), this);

    auto *closeAct = new QAction(tr("Stop Play"), this);
    menuMain->addAction(closeAct);
    menuMain->addMenu(menuVideo);

    auto *agVideo = new QActionGroup(menuVideo);
    agVideo->setExclusive(true);
    connect(menuVideo, SIGNAL(triggered(QAction *)), SLOT(videoMenuTriggered(QAction *)));
    menuVideo->addAction(tr("Open Record"))->setData(OpenVideo);
    menuVideo->addAction(tr("Close Record"))->setData(CloseVideo);
    foreach (auto *action, menuVideo->actions()) {
        action->setActionGroup(agVideo);
        action->setCheckable(true);
    }

    auto *captureAct = new QAction(tr("Capture"), this);
    menuMain->addAction(captureAct);
    menuMain->addMenu(menuStream);

    auto *agStream = new QActionGroup(menuStream);
    agStream->setExclusive(true);
    connect(menuStream, SIGNAL(triggered(QAction *)), SLOT(streamMenuTriggered(QAction *)));
    menuStream->addAction(tr("Main Stream"))->setData(MainStream);
    menuStream->addAction(tr("subStream"))->setData(SubStream);
    foreach (auto *action, menuStream->actions()) {
        action->setActionGroup(agStream);
        action->setCheckable(true);
    }

    menuMain->addMenu(menuIntercom);
    auto *agIntercom = new QActionGroup(menuIntercom);
    agIntercom->setExclusive(true);
    connect(menuIntercom, SIGNAL(triggered(QAction *)), SLOT(intercomMenuTriggered(QAction *)));
    menuIntercom->addAction(tr("Open Intercom"))->setData(OpenIntercom);
    menuIntercom->addAction(tr("Close Intercom"))->setData(CloseIntercom);
    foreach (auto *action, menuIntercom->actions()) {
        action->setActionGroup(agIntercom);
        action->setCheckable(true);
    }

    menuMain->addMenu(menuAudio);
    auto *agAudio = new QActionGroup(menuAudio);
    agAudio->setExclusive(true);
    connect(menuAudio, SIGNAL(triggered(QAction *)), SLOT(audioMenuTriggered(QAction *)));
    menuAudio->addAction(tr("Open Audio"))->setData(OpenAudio);
    menuAudio->addAction(tr("Close Audio"))->setData(CloseAudio);
    foreach (auto *action, menuAudio->actions()) {
        action->setActionGroup(agAudio);
        action->setCheckable(true);
    }

    menuMain->addMenu(menuRatio);

    auto *agRatio = new QActionGroup(menuRatio);
    agRatio->setExclusive(true);
    connect(menuRatio, SIGNAL(triggered(QAction *)), SLOT(ratioMenuTriggered(QAction *)));
    menuRatio->addAction(tr("Original Ratio"))->setData(OriginalRatio);
    menuRatio->addAction(tr("Window Ratio"))->setData(WindowRatio);
    foreach (auto *action, menuRatio->actions()) {
        action->setActionGroup(agRatio);
        action->setCheckable(true);
    }

    auto *remoteSettingAct = new QAction(tr("Remote Setting"), this);
    menuMain->addAction(remoteSettingAct);

    auto *zoomInAct = new QAction(tr("Zoom In"), this);
    menuMain->addAction(zoomInAct);

    auto *fullScreenAct = new QAction(tr("Full Screen"), this);
    menuMain->addAction(fullScreenAct);

    connect(closeAct, &QAction::triggered, this, &PlayerWindow::onMenuActStopPlay);
    connect(captureAct, &QAction::triggered, this, &PlayerWindow::onMenuActSnap);
    connect(remoteSettingAct, &QAction::triggered, this, &PlayerWindow::onMenuActRemote);
    connect(fullScreenAct, &QAction::triggered, this, &PlayerWindow::onMenuActVideoFull);
    connect(zoomInAct, &QAction::triggered, this, &PlayerWindow::onMenuActZoomIn);
}
