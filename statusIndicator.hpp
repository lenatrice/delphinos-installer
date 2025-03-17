/*                      Delphinos Installer
              Copyright © Helena Beatrice Xavier Pedro

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QApplication>

#ifndef LOADINGANIMATION_H
#define LOADINGANIMATION_H

// A dynamic status indicator with icons

class StatusIndicator : public QWidget {
    Q_OBJECT

public:
    enum Status {
        Loading,
        Warning,
        Error,
        Ok,
        None
    };

    explicit StatusIndicator(QWidget *parent = nullptr)
        : QWidget(parent), frameIndex(0), currentStatus(None) {
        setFixedSize(16, 16);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setVisible(false);
        setStyleSheet("background: transparent;");

        loadingTimer = new QTimer(this);
        connect(loadingTimer, &QTimer::timeout, this, &StatusIndicator::nextFrame);
    }

    void setStatus(Status _status) {
        if (_status == currentStatus)
            return;  // Avoid redundant updates

        currentStatus = _status;
        updateVisibility();

        if (_status == Loading) {
            if (!loadingTimer->isActive()) {
                qDebug() << "Starting animation";
                loadingTimer->start(40);
            }
        } else {
            if (loadingTimer->isActive()) {
                loadingTimer->stop();
            }
        }

        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QString imagePath;
        if (currentStatus == Loading) {
            imagePath = QString(QApplication::applicationDirPath() + "/statusIndicator/loading/loading%1.png").arg(frameIndex);
        } else if (currentStatus == Warning) {
            imagePath = QApplication::applicationDirPath() + "/statusIndicator/warning.png";
        } else if (currentStatus == Error) {
            imagePath = QApplication::applicationDirPath() + "/statusIndicator/error.png";
        } else if (currentStatus == Ok) {
            imagePath = QApplication::applicationDirPath() + "/statusIndicator/ok.png";
        } else {
            return;  
        }

        QPixmap image(imagePath);
        if (image.isNull()) {
            qWarning() << "Falha ao carregar a imagem:" << imagePath;
            return;
        }

        // Resize to exactly 60x60
        QPixmap resized = image.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // Calculate centering offsets
        int xOffset = (width() - resized.width()) / 2;
        int yOffset = (height() - resized.height()) / 2;

        // Draw image centered
        painter.drawPixmap(xOffset, yOffset, resized);
    }

private slots:
    void nextFrame() {
        frameIndex = (frameIndex + 1) % 94;
        update();
    }

private:
    QHBoxLayout *layout = new QHBoxLayout(this);
    void updateVisibility() {
        setVisible(currentStatus != None);
    }

    int frameIndex;
    QTimer *loadingTimer;
    Status currentStatus;
};

#endif
