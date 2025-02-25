#include "mainWindow.hpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    this->resize(640, 480);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    // Text
    pageTitle = new QLabel("Seja bem vindo ao instalador do DelphinOS", this);
    pageTitle->setGeometry(0, 2, this->width(), 32);
    pageTitle->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    pageTitle->setStyleSheet(" font-weight: bold; color: #1c6ced; font-size: 24px");

    // Next button
    buttonNext = new QPushButton("Avançar", this);
    buttonNext->setGeometry(this->width() - 90, this->height() - 50, 80, 40);

    // Back button
    buttonBack = new QPushButton("Voltar", this);
    buttonBack->setGeometry(buttonNext->x() - buttonNext->width() - 10, this->height() - 50, 80, 40);

    if (currentPage <= 1)
    {
        buttonBack->setEnabled(false);
    }

    connect(buttonBack, &QPushButton::clicked, this, &MainWindow::onBackClicked);
    connect(buttonNext, &QPushButton::clicked, this, &MainWindow::onNextClicked);

    QTimer* pageLoop = new QTimer(this);
    connect(pageLoop, &QTimer::timeout, this, &MainWindow::checkCurrentPage);
    pageLoop->start(50);
}

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
    painter.drawRoundedRect(background, 18, 18);

    QMainWindow::paintEvent(event);
}

// Allows Mouse to drag window on the top

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    int dragAreaHeight = this->pageTitle->height();

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

// Page checking procedure

void MainWindow::checkCurrentPage()
{
    if (lastPage != currentPage)
    {    
        if (currentPage > 1)
        {
            buttonBack->setEnabled(true);
        }
        else
        {
            buttonBack->setEnabled(false);
        }

        if (currentPage==maxPages)
        {
            buttonNext->setText("Finalizar");
        }
        else
        {
            buttonNext->setText("Avançar");
        }

        pageTitle->setText("Página " + QString::number(currentPage));
        lastPage = currentPage;
    }
}

// Window buttons functions

void MainWindow::onBackClicked()
{
    if (currentPage > 1)
    {
        currentPage--;
    }
}

void MainWindow::onNextClicked()
{
    if (currentPage < maxPages)
    {
        currentPage++;
    } else
    {
        this->close();
    }
}