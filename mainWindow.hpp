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
#include <QString>

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

    // Page management slots
    void onPageChanged(int index);
    void onCanAdvanceChanged(bool canAdvance);

    // Navigation slots
    void onNextClicked();
    void onBackClicked();

signals:
    void connectionState(const QDBusObjectPath& connectionPath, const uint& state);
    void networkDeviceState(const QDBusObjectPath& connectionPath, const uint& state);
};

struct PageTitle : public QLabel
{
    const QString color = "color: rgb(30, 100, 255);";
    explicit PageTitle(QWidget* parent = nullptr) : QLabel(parent) {
        setWordWrap(true);
        setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        setStyleSheet("font-weight: bold; " + color + " font-size: 24px");
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setMaximumHeight(sizeHint().height());
    }

    explicit PageTitle(const QString text, QWidget* parent = nullptr) : QLabel(parent) {
        setWordWrap(true);
        setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        setStyleSheet("font-weight: bold; " + color + " font-size: 24px");
        setText(text);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setMaximumHeight(sizeHint().height());
    }
};

struct PageDescription : public QLabel
{
    const QString color = "color:rgb(60, 138, 255);";
    explicit PageDescription(QWidget* parent = nullptr) : QLabel(parent) {
        setWordWrap(true);
        setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        setStyleSheet("font-weight: regular; " + color + " font-size: 12px");
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
    explicit PageDescription(const QString text, QWidget* parent = nullptr) : QLabel(parent) {
        setWordWrap(true);
        setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        setStyleSheet("font-weight: regular; " + color + " font-size: 12px");
        setText(text);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
};

class PageContent : public QWidget
{
Q_OBJECT
private:
    QVBoxLayout* layout;
    QVBoxLayout* topLayout;
    PageTitle* title;
    PageDescription* description;
    QSize pageSize;

    // Whether next button is clickable to advance the page. By default, canAdvance is true.
    bool canAdvance = true;

    // Whether the page requires confirmation to advance. By default, requireConfirmation is false.
    bool requireConfirmation = false;
    QString confirmationMessage = "Confirmar?";

    // Whether the page issues a warning before continuing. By default, no warning is issued.
    bool requireWarning = false;
    QString warningMessage = "";
    
public:

    PageContent(const QString& _title, const QString& _description, int width, int height, QWidget* _ownerObject) : QWidget(_ownerObject)
    {
        pageSize = QSize(width, height);

        layout = new QVBoxLayout(this);
        topLayout = new QVBoxLayout;
        
        title = new PageTitle(_title, this);
        topLayout->addWidget(title, 0, Qt::AlignTop);
        
        description = new PageDescription(_description, this);
        topLayout->addWidget(description, 0, Qt::AlignTop);
        
        layout->addLayout(topLayout);
        layout->setAlignment(Qt::AlignLeft);

        layout->addSpacing(20);
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
        layout->addWidget(widget, 0, Qt::AlignLeft);
    }

    void addWidget(QWidget* widget, int stretch, Qt::Alignment alignment)
    {
        layout->addWidget(widget, stretch, alignment);
    }

    void addLayout(QLayout* newLayout)
    {
        layout->addLayout(newLayout);
    }

    QSize getSize()
    {
        return pageSize;
    }

    const PageTitle& getTitle()
    {
        return *title;
    };

    const PageDescription& getDescription()
    {
        return *description;
    };

    // Functions for controlling and checking whether page can be advanced

    void setCanAdvance(bool _canAdvance)
    {
        canAdvance = _canAdvance;
        emit canAdvanceChanged(canAdvance);
    }

    bool getCanAdvance() 
    {
        return canAdvance;
    }

    // Functions for controlling and checking confirmation requirement and its message

    void setRequireConfirmation(bool _requireConfirmation)
    {
        requireConfirmation = _requireConfirmation;
    }

    bool getRequireConfirmation()
    {
        return requireConfirmation;
    }

    void setConfirmationMessage(QString _confirmationMessage)
    {
        confirmationMessage = _confirmationMessage;
    }
    
    QString getConfirmationMessage()
    {
        return confirmationMessage;
    }

    // Functions for controlling and checking warning requirement and its message

    void setRequireWarning(bool _requireWarning)
    {
        requireWarning = _requireWarning;
    }

    bool getRequireWarning()
    {
        return requireWarning;
    }

    void setWarningMessage(QString _warningMessage)
    {
        warningMessage = _warningMessage;
    }

    QString getWarningMessage()
    {
        return warningMessage;
    }


signals:
    void canAdvanceChanged(bool _canAdvance);
};

#endif //MAINWINDOW_H