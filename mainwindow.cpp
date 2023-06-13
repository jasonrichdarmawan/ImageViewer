#include "mainwindow.h"
//#include "ui_mainwindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
//    , ui(new Ui::MainWindow)
{
//    ui->setupUi(this);
    initUI();
}

MainWindow::~MainWindow()
{
//    delete ui;
}

void MainWindow::createActions()
{
    // new action
    openAction_ = new QAction("Open", this);
    exitAction_ = new QAction("Exit", this);
    zoomInAction_ = new QAction("Zoom In", this);
    zoomOutAction_ = new QAction("Zoom Out", this);
    previousImageAction_ = new QAction("Previous Image", this);
    nextImageAction_ = new QAction("Next Image", this);

    // add action to the menu bar
    fileMenu_->addAction(openAction_);
    /// known behavior: The Exit action in the menu bar is not displayed on Mac OS.
    fileMenu_->addAction(exitAction_);

    viewMenu_->addAction(zoomInAction_);
    viewMenu_->addAction(zoomOutAction_);
    viewMenu_->addAction(previousImageAction_);
    viewMenu_->addAction(nextImageAction_);

    // add action to the tool bar
    fileToolBar_->addAction(openAction_);

    viewToolBar_->addAction(zoomInAction_);
    viewToolBar_->addAction(zoomOutAction_);
    viewToolBar_->addAction(previousImageAction_);
    viewToolBar_->addAction(nextImageAction_);

    // connect signal to slots
    connect(openAction_, SIGNAL(triggered(bool)), this, SLOT(openImage()));
    connect(exitAction_, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
}

void MainWindow::initUI()
{
    this->resize(800,600);

    // add to the menu bar
    fileMenu_ = menuBar()->addMenu("File");
    viewMenu_ = menuBar()->addMenu("View");

    // add to the tool bar
    fileToolBar_ = addToolBar("File");
    viewToolBar_ = addToolBar("View");

    createActions();

    // main area for image display
    imageScene_ = new QGraphicsScene(this);
    imageView_ = new QGraphicsView(imageScene_);
    setCentralWidget(imageView_);

    // add widget to the status bar
    mainStatusBar_ = statusBar();
    mainStatusLabel_ = new QLabel(mainStatusBar_);
    mainStatusBar_->addPermanentWidget(mainStatusLabel_);
    mainStatusLabel_->setText("Image Information will be here!");
}

void MainWindow::showImage(QString path)
{
    imageScene_->clear();
    imageView_->resetTransform();

    QPixmap image(path);
    imageScene_->addPixmap(image);
    imageScene_->update();
    /// necessary when user open 2nd image with smaller width and height.
    imageView_->setSceneRect(image.rect());

    QString status = QString("%1, %2x%3, %4 Bytes")
                         .arg(path)
                         .arg(image.width())
                         .arg(image.height())
                         .arg(QFile(path).size());
    mainStatusLabel_->setText(status);
}

void MainWindow::openImage()
{
    qDebug() << "slot openImage is called.";

    QFileDialog dialog(this);
    dialog.setWindowTitle("Open Image");

    dialog.setFileMode(QFileDialog::ExistingFile);
    /// reference: https://doc.qt.io/qt-6/qfiledialog.html
    /// reference: https://doc.qt.io/qt-6/i18n-source-translation.html#c-use-tr
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));

    QStringList filePaths;
    if (dialog.exec())
    {
        filePaths = dialog.selectedFiles();
        showImage(filePaths.at(0));
    }
}
