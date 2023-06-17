#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QToolBar>
#include <QLabel>

#include <QMap>
#include "editor_plugin_interface.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void loadPlugins();

    void createActions();
    void createShortcuts();
    void initUI();
    void showImage(QString path);

private:
    QMap<QString, EditorPluginInterface*> editPlugins_;

//    Ui::MainWindow *ui;
    QMenu *fileMenu_;
    QMenu *viewMenu_;
    QMenu *editMenu_;

    QToolBar *fileToolBar_;
    QToolBar *viewToolBar_;
    QToolBar *editToolBar_;

    QAction *openAction_;
    QAction *saveAsAction_;
    QAction *exitAction_;

    QAction *zoomInAction_;
    QAction *zoomOutAction_;
    QAction *previousImageAction_;
    QAction *nextImageAction_;

    QAction *blurAction_;

    QString currentImagePath_;
    QGraphicsPixmapItem *currentImage_;
    QGraphicsScene *imageScene_;
    QGraphicsView *imageView_;

    QStatusBar *mainStatusBar_;
    QLabel *mainStatusLabel_;

private slots:
    void pluginPerform();

    void openImage();
    void saveAs();

    void zoomIn();
    void zoomOut();
    void previousImage();
    void nextImage();

    void blurImage();
};
#endif // MAINWINDOW_H
