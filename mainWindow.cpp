#include "mainWindow.hpp"
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <QLineEdit>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPainterPath>

#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <NetworkManager.h>

#define signals Q_SIGNALs

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setFixedSize(640, 480);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Window layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setAlignment(Qt::AlignTop);

    // Page content
    page = new QStackedWidget(this);
    pageContent[0] = pageCreateLocalization();
    pageContent[1] = pageCreateNetwork();
    pageContent[2] = pageCreatePartition();
    page->addWidget(pageContent[0]);
    page->addWidget(pageContent[1]);
    page->addWidget(pageContent[2]);
    page->setCurrentIndex(0);

    mainLayout->addWidget(page);

    // Navigation buttons
    QWidget * navigationWidget = new QWidget(this);
    QHBoxLayout * navigationButtonLayout = new QHBoxLayout(navigationWidget);

    buttonBack = new QPushButton("Voltar", this);
    navigationButtonLayout->addWidget(buttonBack);

    buttonNext = new QPushButton("Avançar", this);
    navigationButtonLayout->addWidget(buttonNext);

    navigationButtonLayout->setAlignment(Qt::AlignBottom | Qt::AlignRight);

    mainLayout->addWidget(navigationWidget);


    connect(buttonBack, &QPushButton::clicked, this, &MainWindow::onBackClicked);
    connect(buttonNext, &QPushButton::clicked, this, &MainWindow::onNextClicked);
};

QWidget* MainWindow::pageCreatePartition()
{
    WindowPage* page = new WindowPage(
        "Configurar partições para o sistema",
        "Nessa etapa iremos criar as partições que serão usadas pelo sistema ou selecionar partições já existentes."
    );

    return page;
};

// Custom window background 
void MainWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen backgroundBorder(Qt::black);
    backgroundBorder.setWidth(1);
    painter.setPen(backgroundBorder);

    QBrush brush(QColor("#191B21"));
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

// Window buttons functions

void MainWindow::onBackClicked()
{
    if (page->currentIndex() > 0)
    {
        page->setCurrentIndex(page->currentIndex()-1);
    }
}

void MainWindow::onNextClicked()
{
    if (page->currentIndex() != page->count() - 1)
    {
        page->setCurrentIndex(page->currentIndex()+1);
    } else
    {
        close();
    }
}