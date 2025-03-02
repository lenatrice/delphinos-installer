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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    const QScreen* screen = QGuiApplication::primaryScreen();
    const QRect screenGeometry = screen->geometry();
    int screenHeight = screenGeometry.height();
    int screenWidth = screenGeometry.width();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private: 
    QStackedWidget* page;
    QWidget* pageContent[3];

    QWidget* pageCreateLocalization();
    bool keymapLayoutChanged = false;

    QWidget* pageCreateNetwork();
    const int networkDevicePathRole = Qt::UserRole;
    const int networkDeviceTypeRole = Qt::UserRole + 1;
    const int wifiAccessPointNameRole = Qt::UserRole;
    const int wifiAccessPointPathRole = Qt::UserRole + 1;

    QWidget* pageCreatePartition();

    void populateKeymapLayouts(QComboBox* keymapLayoutCombobox);
    void populateTimezones(QComboBox* keymapLayoutCombobox);
    void populateNetworkDevices(QFormLayout* networkFormLayout, QListWidget* networkDeviceList, QListWidget* wifiAccessPointList);

    QPoint dragPosition;
    bool dragging = false;

    QPushButton* buttonBack;
    QPushButton* buttonNext;

private slots:
    void onNextClicked();
    void onBackClicked();

    // Localization slots
    void updateLanguage();
    void updateKeymapLayout(QComboBox* keymapLayoutCombobox, QComboBox* keymapVariantCombobox);
    void updateKeymapVariant(QComboBox* keymapLayoutCombobox, QComboBox* keymapVariantCombobox);
    void updateTimezone(QComboBox* timezoneCombobox, int timezoneComboboxIndex);

    // Network slots
    void updateNetworkDevice(QFormLayout* networkFormLayout, QListWidgetItem* networkDeviceItem, QListWidget* wifiAccessPointsList);
    void connectNetwork(QListWidgetItem* networkDeviceItem, QListWidgetItem* wifiAccessPointItem);
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


class WindowPage : public QWidget
{
    QVBoxLayout* layout;
    PageTitle* title;
    PageDescription* description; 

public:
    WindowPage(const QString& _title, const QString& _description)
    {
        layout = new QVBoxLayout(this);

        title = new PageTitle(_title);
        layout->addWidget(title);

        description = new PageDescription(_description, this);
        layout->addWidget(description);

        layout->setAlignment(Qt::AlignTop);

        this->show();
    }

    void setTitle(const QString& _title)
    {
        title->setText(_title);
    }

    void setDescription(const QString& _description)
    {
        description->setText(_description);
    }

    void addWidget(QWidget* widget)
    {
        layout->addWidget(widget, 0, Qt::AlignTop);
    }

};

#endif //MAINWINDOW_H