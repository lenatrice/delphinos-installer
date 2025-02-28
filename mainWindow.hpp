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
#include <QComboBox>
#include <QListView>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>

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
    QStackedWidget *page;
    QWidget *pageContent1;
    QWidget *pageContent2;

    QWidget *pageCreateLocalization();
    QWidget *pageCreatePartition();

    void populateKeymapLayouts(QComboBox* keymapLayoutCombobox);
    void populateTimezones(QComboBox* keymapLayoutCombobox);

    
    QPoint dragPosition;
    bool dragging = false;

public:
    int currentPage = 1;
    int maxPages=2;

    QPushButton *buttonBack;
    QPushButton *buttonNext;


private slots:
    //void pageShowNext();
    //void pageShowPrevious();
    void onNextClicked();
    void onBackClicked();
    void updateLayoutAndVariants(QComboBox* keymapLayoutCombobox, QComboBox* keymapVariantCombobox);
    void updateVariant(QComboBox* keymapLayoutCombobox, QComboBox* keymapVariantCombobox);
    void updateTimezone(QComboBox* timezoneCombobox, int timezoneComboboxIndex);

private: bool keymapLayoutChanged = false;
};

struct PageTitle : public QLabel
{
    explicit PageTitle(QWidget* parent = nullptr) : QLabel(parent) {
        setWordWrap(true);
        setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        setStyleSheet(" font-weight: bold; color:rgb(58, 124, 230); font-size: 24px");    
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }

    explicit PageTitle(const QString text, QWidget* parent = nullptr) : QLabel(parent) {
        setWordWrap(true);
        setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        setStyleSheet(" font-weight: bold; color:rgb(58, 124, 230); font-size: 24px");
        setText(text);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
};

struct PageDescription : public QLabel
{
    explicit PageDescription(QWidget* parent = nullptr) : QLabel(parent) {
        setWordWrap(true);
        setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        setStyleSheet(" font-weight: regular; color:rgb(91, 126, 180); font-size: 12px");
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
    explicit PageDescription(const QString text, QWidget* parent = nullptr) : QLabel(parent) {
        setWordWrap(true);
        setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        setStyleSheet(" font-weight: regular; color:rgb(91, 126, 180); font-size: 12px");
        setText(text);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
};

struct PageContent : public QWidget
{
    PageTitle title;
    PageDescription description;
    QVBoxLayout *pageLayout;
};

#endif //MAINWINDOW_H