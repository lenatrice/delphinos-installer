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
#include <QDialogButtonBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QTimer::singleShot(0, this, [this]() {
        if (windowHandle()) {
            screen = windowHandle()->screen();
            screenGeometry = screen->geometry();

            qDebug() << "Screen geometry: " << screenGeometry;

            if (screen) {
                connect(screen, &QScreen::geometryChanged,
                        this, &MainWindow::onScreenGeometryChanged);
            }
        } else {
            qWarning() << "windowHandle() not yet initialized.";
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

    PageContent* emptyPage = new PageContent("Página vazia", "Para fins de teste", 640, 480, this);// empty last page. For testing purposes.
    pageList.append(emptyPage);

    for (PageContent* page : pageList)
    {
        pageStack->addWidget(page);
    }

    connect(pageStack, &QStackedWidget::currentChanged, this, &MainWindow::onPageChanged);

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
}



void MainWindow::onPageChanged(int index)
{
    connect(pageList[index], &PageContent::canAdvanceChanged, this, &MainWindow::onCanAdvanceChanged);
    buttonNext->setEnabled(pageList[index]->getCanAdvance());
}

void MainWindow::onCanAdvanceChanged(bool canAdvance)
{
    if (canAdvance) buttonNext->setEnabled(true); else buttonNext->setEnabled(false);
}


void MainWindow::onScreenGeometryChanged(const QRect& geometry)
{
    screenGeometry = geometry;
    centerWindow();
}


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
        dragPosition = event->globalPosition().toPoint() - this->frameGeometry().topLeft();
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
        this->move(event->globalPosition().toPoint() - dragPosition);
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
    PageContent* currentPage = pageList[pageStack->currentIndex()];

    if (currentPage->getRequireWarning())
    {
        QMessageBox::StandardButton warningDialog;
        warningDialog = QMessageBox::warning(this, "Atenção", currentPage->getWarningMessage(), QMessageBox::Ok | QMessageBox::Cancel);

        if (warningDialog == QMessageBox::Cancel) return;
    }

    if (currentPage->getRequireConfirmation())
    {
        QMessageBox::StandardButton confirmationDialog;
        confirmationDialog = QMessageBox::information(this, "Confirmação", currentPage->getConfirmationMessage(), QMessageBox::Ok | QMessageBox::Cancel);

        if (confirmationDialog == QMessageBox::Cancel) return;
    }

    if (pageStack->currentIndex() < pageStack->count() - 1)
    {
        int newIndex = pageStack->currentIndex() + 1;
        pageStack->setCurrentIndex(newIndex);
        setFixedSize(pageList[newIndex]->getSize());
    } else {
        close();
    }
}