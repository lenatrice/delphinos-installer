#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <QMainWindow>
#include <QScreen>
#include <QDesktopWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QGuiApplication>
#include <QLabel>
#include <QTimer>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPainterPath>
#include <QMouseEvent>

class MainWindow;
class WindowContent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    const QScreen* screen = QGuiApplication::primaryScreen();
    const QRect screenGeometry = screen->geometry();
    int screenHeight = screenGeometry.height();
    int screenWidth = screenGeometry.width();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private: 
    WindowContent* content;
    QPoint dragPosition;
    bool dragging = false;
};

class WindowContent : public QWidget
{
Q_OBJECT

public:
    WindowContent(MainWindow* parent);

    MainWindow* parent; 

    int currentPage = 1;
    int lastPage = currentPage;
    const int maxPages = 4;

    QLabel *title;
    QLabel *description;

    QPushButton *buttonBack;
    QPushButton *buttonNext;

private slots:
    void checkCurrentPage();
    void onBackClicked();
    void onNextClicked();

};


#endif //MAINWINDOW_H