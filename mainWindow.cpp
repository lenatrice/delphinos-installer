#include "mainWindow.hpp"

WindowContent::WindowContent(MainWindow* parent) : QWidget(parent)
{
    // Parent
    this->parent = parent;

    // Title
    title = new QLabel("Seja bem vindo ao instalador do DelphinOS", this);
    title->setGeometry(0, 10, parent->width(), 32);
    title->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    title->setStyleSheet(" font-weight: bold; color:rgb(58, 124, 230); font-size: 24px");

    // Description
    description = new QLabel("Sistema operacional baseado em ArchLinux, customizado para a sua conveniência e completamente personalizável. Esse instalador irá lhe guiar por todas as etapas da instalação. Clique em próximo para avancar para iniciar a primeira etapa.", this);
    description->setWordWrap(true);
    description->setGeometry(10, title->y()+title->height()+10, parent->width()-10, parent->height()-10);
    description->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    description->setStyleSheet(" font-weight: regular; color:rgb(91, 126, 180); font-size: 12px");

    // Next button
    buttonNext = new QPushButton("Avançar", this);
    buttonNext->setGeometry(parent->width() - 90, parent->height() - 50, 80, 40);

    // Back button
    buttonBack = new QPushButton("Voltar", this);
    buttonBack->setGeometry(buttonNext->x() - buttonNext->width() - 10, parent->height() - 50, 80, 40);

    connect(buttonBack, &QPushButton::clicked, this, &WindowContent::onBackClicked);
    connect(buttonNext, &QPushButton::clicked, this, &WindowContent::onNextClicked);


    QTimer* pageLoop = new QTimer(this);
    connect(pageLoop, &QTimer::timeout, this, &WindowContent::checkCurrentPage);
    pageLoop->start(50);
};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    this->resize(640, 480);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    content = new WindowContent(this);
    setCentralWidget(content);
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
    painter.drawRoundedRect(background, 20, 20);

    QMainWindow::paintEvent(event);
}

// Allows Mouse to drag window on the top

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    int dragAreaHeight = this->content->title->height() + this->content->title->y();

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

void WindowContent::checkCurrentPage()
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

        title->setText("Página " + QString::number(currentPage));
        lastPage = currentPage;
    }
}

// Window buttons functions

void WindowContent::onBackClicked()
{
    if (currentPage > 1)
    {
        currentPage--;
    }
}

void WindowContent::onNextClicked()
{
    if (currentPage < maxPages)
    {
        currentPage++;
    } else
    {
        parent->close();
    }
}