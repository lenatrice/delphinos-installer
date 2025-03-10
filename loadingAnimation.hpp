#include <QWidget>
#include <QPainter>
#include <QTimer>

#ifndef LOADINGANIMATION_H
#define LOADINGANIMATION_H

class LoadingAnimation : public QWidget {
    Q_OBJECT

public:
    LoadingAnimation(QWidget *parent = nullptr) : QWidget(parent), frameIndex(0) {
        // Set the window size (adjust as needed)
        setFixedSize(60, 60);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        setVisible(false);
    }

    void start() {
        if (!timer) {
            qDebug() << "Starting animation";
            timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, &LoadingAnimation::nextFrame);
            timer->start(40);
            setVisible(true);
        }
    }

    void stop() {
        if (timer) {
            timer->stop();
            timer = nullptr;
            setVisible(false); 
        }
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);

        // Load the current frame image
        QString framePath = QString("/home/lena/Projects/delphinos-installer/build/loading-animation/loading%1.png").arg(frameIndex);
        QPixmap frame(framePath);

        if (frame.isNull()) {
            qWarning() << "Failed to load frame:" << framePath;
            return;
        }

        QPixmap scaledFrame = frame.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        int xOffset = (this->width() - scaledFrame.width()) / 2;
        int yOffset = (this->height() - scaledFrame.height()) / 2;
        painter.drawPixmap(xOffset, yOffset, scaledFrame);
    }

private slots:
    void nextFrame() {
        frameIndex = (frameIndex + 1) % 78;

        update();
    }

private:
    int frameIndex;
    QTimer *timer = nullptr;
};

#endif