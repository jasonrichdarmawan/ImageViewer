#include "mainwindow.h"
//#include "ui_mainwindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include "opencv2/opencv.hpp"
#include <QPluginLoader>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
//    , ui(new Ui::MainWindow)
    , currentImage_(nullptr)
{
//    ui->setupUi(this);
    initUI();
}

MainWindow::~MainWindow()
{
//    delete ui;
}

void MainWindow::pluginPerform()
{
    qDebug() << "pluginPerform triggered";

    if (currentImage_ == nullptr)
    {
        QMessageBox::information(this, "Information", "No image to edit.");
        return;
    }

    // Go to: void MainWindow::loadPlugins()
    // connect(action, ..., ..., ...); is equal to
    // connect(sender(), ..., ..., ....);
    // *active_action will be destroyed when it goes out of scope.
    QAction *active_action = qobject_cast<QAction*>(sender());

    EditorPluginInterface *plugin_ptr = editPlugins_[active_action->text()];
    if (!plugin_ptr)
    {
        QMessageBox::information(this, "Information", "No plugin is found.");
        return;
    }

    QPixmap pixmap = currentImage_->pixmap();
    QImage image = pixmap.toImage();
    image = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat = cv::Mat(image.height(), image.width(), CV_8UC3, image.bits(), image.bytesPerLine());

    plugin_ptr->edit(mat, mat);
    QImage image_edited(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);

    pixmap = QPixmap::fromImage(image_edited);
    imageScene_->clear();
    imageView_->resetTransform();
    currentImage_ = imageScene_->addPixmap(pixmap);
    imageScene_->update();
    imageView_->setSceneRect(pixmap.rect());

    QString status = QString("(edited image), %1x%2").arg(pixmap.width()).arg(pixmap.height());
    mainStatusLabel_->setText(status);
}

void MainWindow::loadPlugins()
{

    QDir pluginsDir(QApplication::instance()->applicationDirPath()+"/plugins");

    QStringList nameFilters;
    // *.so for GNU/Linux, *.dylib for MacOS, *.dll for Windows
    nameFilters << "*.so" << "*.dylib" << "*.dll";
    // QDir::NoDotAndDotDot excludes the entries for the current directory "." and the parent directory ".." from the list of entries.
    QFileInfoList plugins = pluginsDir.entryInfoList(nameFilters, QDir::NoDotAndDotDot | QDir::Files, QDir::Name);

    qDebug() << pluginsDir;

    for (const QFileInfo& plugin : plugins)
    {
        QPluginLoader pluginLoader(plugin.absoluteFilePath(), this);
        EditorPluginInterface *plugin_ptr = dynamic_cast<EditorPluginInterface*>(pluginLoader.instance());

        if (plugin_ptr)
        {
            QAction *action = new QAction(plugin_ptr->name());

            action->setEnabled(false);

            editMenu_->addAction(action);
            editToolBar_->addAction(action);

            editPlugins_[plugin_ptr->name()] = plugin_ptr;

            connect(action, SIGNAL(triggered(bool)), this, SLOT(pluginPerform()));
        }
        else
        {
            qDebug() << "bad plugin: " << plugin.absoluteFilePath();
        }
    }
}

void MainWindow::createActions()
{
    // new action
    openAction_ = new QAction("Open", this);
    saveAsAction_ = new QAction("Save As", this);
    exitAction_ = new QAction("Exit", this);

    zoomInAction_ = new QAction("Zoom In", this);
    zoomOutAction_ = new QAction("Zoom Out", this);
    previousImageAction_ = new QAction("Previous Image", this);
    nextImageAction_ = new QAction("Next Image", this);

    blurAction_ = new QAction("Blur", this);

    // disable action when imageScene_ is clear.
    saveAsAction_->setEnabled(false);

    zoomInAction_->setEnabled(false);
    zoomOutAction_->setEnabled(false);
    previousImageAction_->setEnabled(false);
    nextImageAction_->setEnabled(false);

    blurAction_->setEnabled(false);

    // add action to the menu bar
    fileMenu_->addAction(openAction_);
    fileMenu_->addAction(saveAsAction_);
    /// known behavior: The Exit action in the menu bar is not displayed on Mac OS.
    fileMenu_->addAction(exitAction_);

    viewMenu_->addAction(zoomInAction_);
    viewMenu_->addAction(zoomOutAction_);
    viewMenu_->addAction(previousImageAction_);
    viewMenu_->addAction(nextImageAction_);

    editMenu_->addAction(blurAction_);

    // add action to the tool bar
    fileToolBar_->addAction(openAction_);

    viewToolBar_->addAction(zoomInAction_);
    viewToolBar_->addAction(zoomOutAction_);
    viewToolBar_->addAction(previousImageAction_);
    viewToolBar_->addAction(nextImageAction_);

    editToolBar_->addAction(blurAction_);

    // connect signal to slots
    connect(openAction_, SIGNAL(triggered(bool)), this, SLOT(openImage()));
    connect(saveAsAction_, SIGNAL(triggered(bool)), this, SLOT(saveAs()));
    connect(exitAction_, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));

    connect(zoomInAction_, SIGNAL(triggered(bool)), this, SLOT(zoomIn()));
    connect(zoomOutAction_, SIGNAL(triggered(bool)), this, SLOT(zoomOut()));
    connect(previousImageAction_, SIGNAL(triggered(bool)), this, SLOT(previousImage()));
    connect(nextImageAction_, SIGNAL(triggered(bool)), this, SLOT(nextImage()));

    connect(blurAction_, SIGNAL(triggered(bool)), this, SLOT(blurImage()));
}

void MainWindow::createShortcuts()
{
    QList<QKeySequence> shortcuts;

    // zoom in
    shortcuts << Qt::Key_Plus << Qt::Key_Equal;
    zoomInAction_->setShortcuts(shortcuts);
    shortcuts.clear();

    // zoom out
    shortcuts << Qt::Key_Minus << Qt::Key_Underscore;
    zoomOutAction_->setShortcuts(shortcuts);
    shortcuts.clear();

    // previous image
    shortcuts << Qt::Key_Up << Qt::Key_Left;
    previousImageAction_->setShortcuts(shortcuts);
    shortcuts.clear();

    // next image
    shortcuts << Qt::Key_Down << Qt::Key_Right;
    nextImageAction_->setShortcuts(shortcuts);
    shortcuts.clear();
}

void MainWindow::initUI()
{
    this->resize(800,600);

    // add to the menu bar
    fileMenu_ = menuBar()->addMenu("File");
    viewMenu_ = menuBar()->addMenu("View");
    editMenu_ = menuBar()->addMenu("Edit");

    // add to the tool bar
    fileToolBar_ = addToolBar("File");
    viewToolBar_ = addToolBar("View");
    editToolBar_ = addToolBar("Edit");

    createActions();
    createShortcuts();

    loadPlugins();

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
    /// necessary when user save the image.
    currentImage_ = imageScene_->addPixmap(image);
    imageScene_->update();
    /// necessary when user open 2nd image with smaller width and height.
    imageView_->setSceneRect(image.rect());

    QString status = QString("%1, %2x%3, %4 Bytes")
                         .arg(path)
                         .arg(image.width())
                         .arg(image.height())
                         .arg(QFile(path).size());
    mainStatusLabel_->setText(status);

    /// necessary when user navigate between previous and next image.
    currentImagePath_ = path;
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

    // enable action when imageScene_ is not clear.
    zoomInAction_->setEnabled(true);
    zoomOutAction_->setEnabled(true);
    previousImageAction_->setEnabled(true);
    nextImageAction_->setEnabled(true);

    blurAction_->setEnabled(true);

    for (auto it = editPlugins_.cbegin(); it != editPlugins_.cend(); it++)
    {
        const EditorPluginInterface *plugin = it.value();

        QList<QAction *> actions = editMenu_->actions();
        for (auto actionIt = actions.begin(); actionIt != actions.end(); actionIt++)
        {
            QAction* action = *actionIt;
            if (action->text() == plugin->name())
            {
                action->setEnabled(true);
            }
        }
    }
}

void MainWindow::saveAs()
{
    qDebug() << "slot saveAs is called.";

    if (currentImage_ == nullptr)
    {
        QMessageBox::information(this, "Information", "Nothing to save.");
        return;
    }

    QFileDialog dialog(this);
    dialog.setWindowTitle("Save Image As ...");

    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));

    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
        if (QRegExp(".+\\.(png|bmp|jpg)").exactMatch(fileNames.at(0)))
        {
            // TODO: Tech Debt 1 the size of the saved image is not the same as the original image.
            currentImage_->pixmap().save(fileNames.at(0));
        }
        else
        {
            QMessageBox::information(this, "Information", "Save error: bad format or filename.");
        }
    }
}

void MainWindow::zoomIn()
{
    imageView_->scale(1.1, 1.1);
}

void MainWindow::zoomOut()
{
    imageView_->scale(0.9, 0.9);
}

void MainWindow::previousImage()
{
    QFileInfo currentFile(currentImagePath_);
    QDir dir = currentFile.absoluteDir();

    QStringList nameFilters;
    nameFilters << "*.png" << "*.bmp" << "*.jpg";
    QStringList fileNames = dir.entryList(nameFilters, QDir::Files, QDir::Name);
    qDebug() << fileNames;

    int currentIndex = fileNames.indexOf(QRegExp(QRegExp::escape(currentFile.fileName())));
    if (currentIndex > 0)
    {
        nextImageAction_->setEnabled(true);
        showImage(dir.absoluteFilePath(fileNames.at(currentIndex - 1)));
    }
    else {
        previousImageAction_->setEnabled(false);
        QMessageBox::information(this, "Information", "Current image is the first one.");
    }
}

void MainWindow::nextImage()
{
    QFileInfo currentFile(currentImagePath_);
    QDir dir = currentFile.absoluteDir();

    QStringList nameFilters;
    nameFilters << "*.png" << "*.bmp" << "*.jpg";
    QStringList fileNames = dir.entryList(nameFilters, QDir::Files, QDir::Name);
    qDebug() << fileNames;

    /// necessary when current.fileName() output is `a.jpg`
    /// `.` matches any character (except for line terminators)
    int currentIndex = fileNames.indexOf(QRegExp(QRegExp::escape(currentFile.fileName())));
    if (currentIndex < fileNames.size() - 1)
    {
        previousImageAction_->setEnabled(true);
        showImage(dir.absoluteFilePath(fileNames.at(currentIndex + 1)));
    }
    else
    {
        nextImageAction_->setEnabled(false);
        QMessageBox::information(this, "Information", "Current image is the last one.");
    }
}

void MainWindow::blurImage()
{
    qDebug() << "blurImage()";
    if (currentImage_ == nullptr)
    {
        QMessageBox::information(this, "Information", "No image to edit.");
        return;
    }

    QPixmap pixmap = currentImage_->pixmap();
    QImage image = pixmap.toImage();

    /// necessary for QImage -> cv::Mat conversion
    /// reason: default order of colors in OpenCV is BGR, while in Qt is RGB.
    image = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat = cv::Mat(image.height(), image.width(), CV_8UC3, image.bits(), image.bytesPerLine());

    cv::Mat tmp;
    cv::blur(mat, tmp, cv::Size(8,8));
    mat = tmp;

    /// necessary for cv::Mat -> QImage conversion
    QImage image_blurred(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
    pixmap = QPixmap::fromImage(image_blurred);

    imageScene_->clear();
    imageView_->resetTransform();

    currentImage_ = imageScene_->addPixmap(pixmap);
    imageScene_->update();
    imageView_->setSceneRect(pixmap.rect());

    QString status = QString("(edited image),%1x%2").arg(pixmap.width()).arg(pixmap.height());

    mainStatusLabel_->setText(status);
}
