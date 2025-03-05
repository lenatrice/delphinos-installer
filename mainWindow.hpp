#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <QMainWindow>
#include <QWindow>
#include <QScreen>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QTimer>
#include <QMouseEvent>
#include <QComboBox>
#include <QListView>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTranslator>
#include "networkDBus.hpp"

class PageContent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QScreen* screen = nullptr;
    QRect screenGeometry;
    explicit MainWindow(QWidget* parent = nullptr);
    bool userMovedWindow = false;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;

private: 
    // Window
    QStackedWidget* pageStack;
    QList<PageContent*> pageList;
    QPoint windowCenter;
    QPushButton* buttonBack;
    QPushButton* buttonNext;

    void centerWindow();

    QPoint dragPosition;
    bool dragging = false;

private slots:
    // Screen geometry change slot
    void onScreenGeometryChanged(const QRect& geometry);

    // Navigation slots
    void onNextClicked();
    void onBackClicked();

signals:
    void connectionState(const QDBusObjectPath& connectionPath, const uint& state);
    void networkDeviceState(const QDBusObjectPath& connectionPath, const uint& state);
};

struct PageTitle : public QLabel
{
    explicit PageTitle(QWidget* parent = nullptr) : QLabel(parent) {
        setWordWrap(true);
        setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        setStyleSheet("font-weight: bold; color:rgb(58, 124, 230); font-size: 24px");
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setMaximumHeight(sizeHint().height());
    }

    explicit PageTitle(const QString text, QWidget* parent = nullptr) : QLabel(parent) {
        setWordWrap(true);
        setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        setStyleSheet("font-weight: bold; color:rgb(58, 124, 230); font-size: 24px");
        setText(text);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setMaximumHeight(sizeHint().height());
    }
};

struct PageDescription : public QLabel
{
    explicit PageDescription(QWidget* parent = nullptr) : QLabel(parent) {
        setWordWrap(true);
        setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        setStyleSheet("font-weight: regular; color:rgb(91, 126, 180); font-size: 12px");
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
    explicit PageDescription(const QString text, QWidget* parent = nullptr) : QLabel(parent) {
        setWordWrap(true);
        setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        setStyleSheet("font-weight: regular; color:rgb(91, 126, 180); font-size: 12px");
        setText(text);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
};

class PageContent : public QWidget
{
Q_OBJECT
private:
    QVBoxLayout* layout;
    PageTitle* title;
    PageDescription* description;
    QSize pageSize;

public:
    PageContent(const QString& _title, const QString& _description, int width, int height)
    {
        layout = new QVBoxLayout(this);
        title = new PageTitle(_title, this);
        layout->addWidget(title, 0, Qt::AlignTop);

        description = new PageDescription(_description, this);
        layout->addWidget(description, 0, Qt::AlignTop);

        layout->setAlignment(Qt::AlignTop);

        pageSize = QSize(width, height);
    }

    ~PageContent() override = default;

    void setTitle(const QString& _title)
    {
        title->setText(_title);
    }

    void setDescription(const QString& _description)
    {
        description->setText(_description);
    }

    void addStretch()
    {
        layout->addStretch();
    }

    void addWidget(QWidget* widget)
    {
        layout->addWidget(widget, 0, Qt::AlignVCenter | Qt::AlignLeft);
    }

    void addLayout(QLayout* widget)
    {
        layout->addLayout(widget);
    }

    QSize getSize()
    {
        return pageSize;
    }
};

#endif //MAINWINDOW_H