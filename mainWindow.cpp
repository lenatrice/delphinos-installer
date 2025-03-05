#include "mainWindow.hpp"
#include "localizationPage.hpp"
#include "networkPage.hpp"
#include "partitionPage.hpp"
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <QLineEdit>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPainterPath>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QTimer::singleShot(0, this, [this]() {
        if (windowHandle()) {
            screen = windowHandle()->screen();
            screenGeometry = screen->geometry();
            if (screen) {
                connect(screen, &QScreen::geometryChanged,
                        this, &MainWindow::onScreenGeometryChanged);
            }
        }
        windowCenter = QPoint(screenGeometry.width() / 2, screenGeometry.height() / 2);
        centerWindow();
    });

    
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Window layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    pageStack = new QStackedWidget(centralWidget);

    mainLayout->addWidget(pageStack, 0, Qt::AlignTop);
    
    // Window pages

    LocalizationPage* localizationPage = new LocalizationPage(this);
    pageList.append(localizationPage->getPage());

    NetworkPage* networkPage = new NetworkPage(this);
    pageList.append(networkPage->getPage());

    PartitionPage* partitionPage = new PartitionPage(this);
    pageList.append(partitionPage->getPage());

    for (PageContent* page : pageList)
    {
        pageStack->addWidget(page);
    }

    pageStack->setCurrentIndex(0);
    setFixedSize(pageList[0]->getSize());

    // Navigation buttons
    QWidget * navigationWidget = new QWidget(centralWidget);
    QHBoxLayout * navigationButtonLayout = new QHBoxLayout(navigationWidget);

    buttonBack = new QPushButton("Voltar", navigationWidget);
    navigationButtonLayout->addWidget(buttonBack);

    buttonNext = new QPushButton("Avançar", navigationWidget);
    navigationButtonLayout->addWidget(buttonNext);

    navigationButtonLayout->setAlignment(Qt::AlignBottom | Qt::AlignRight);

    mainLayout->addStretch();
    mainLayout->addWidget(navigationWidget);

    connect(buttonBack, &QPushButton::clicked, this, &MainWindow::onBackClicked);
    connect(buttonNext, &QPushButton::clicked, this, &MainWindow::onNextClicked);

};

void MainWindow::onScreenGeometryChanged(const QRect& geometry)
{
    screenGeometry = geometry;
    centerWindow();
};

// Custom window background 
void MainWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen backgroundBorder(Qt::black);
    backgroundBorder.setWidth(1);
    painter.setPen(backgroundBorder);

    QBrush brush(QColor("#16171c"));
    painter.setBrush(brush);

    QRectF background(0, 0, this->width(), this->height());
    painter.drawRoundedRect(background, 20, 20);

    QMainWindow::paintEvent(event);
}

// Allows Mouse to drag window on the top
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    int dragAreaHeight = 64;

    if (event->button() == Qt::LeftButton && event->pos().y() <= dragAreaHeight)
    {
        dragging = true;
        dragPosition = event->globalPos() - this->frameGeometry().topLeft();
        event->accept();
    }
    else
    {
        QMainWindow::mousePressEvent(event);
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (dragging == true && event->buttons() & Qt::LeftButton)
    {
        this->move(event->globalPos() - dragPosition);
        event->accept();
    }
    else
    {
        QMainWindow::mouseMoveEvent(event);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && dragging == true)
    {
        dragging = false;
        event->accept();
    }
    else
    {
        QMainWindow::mouseReleaseEvent(event);
    }
}

void MainWindow::moveEvent(QMoveEvent* event)
{
    windowCenter = geometry().center();
    QMainWindow::moveEvent(event);
}

// Keep window centralized to its position when resizing
void MainWindow::resizeEvent(QResizeEvent *event)
{
    qDebug() << "Resize event:" << event->size();

    QMainWindow::resizeEvent(event);

    pageList[pageStack->currentIndex()]->updateGeometry();
    
    move(windowCenter.x() - width() / 2, windowCenter.y() - height() / 2);
}

void MainWindow::centerWindow()
{
    if (!screen) return;

    windowCenter = QPoint(screenGeometry.width()/2, screenGeometry.height()/2);

    move(windowCenter.x() - width() / 2, windowCenter.y() - height() / 2);
}

// Window buttons functions
void MainWindow::onBackClicked()
{
    if (pageStack->currentIndex() > 0)
    {
        int newIndex = pageStack->currentIndex() - 1;
        pageStack->setCurrentIndex(newIndex);
        setFixedSize(pageList[newIndex]->getSize());
    }
}

void MainWindow::onNextClicked()
{
    qDebug() << "Page:" << pageStack->currentIndex() + 1 << "/" << pageStack->count() ;
    if (pageStack->currentIndex() < pageStack->count() - 1)
    {
        int newIndex = pageStack->currentIndex() + 1;
        pageStack->setCurrentIndex(newIndex);
        setFixedSize(pageList[newIndex]->getSize());
    } else
    {
        close();
    }
}