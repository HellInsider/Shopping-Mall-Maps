#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "map.h"
#include "viewer.h"



QT_BEGIN_NAMESPACE
class QAction;
class QGraphicsView;
class QGraphicsScene;
class QGraphicsRectItem;
class QLabel;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    bool LoadFile(const QString &svgFileName, const QString &xmlFileName);
    virtual void Render(QPainter *painter);

public slots:
    bool event(QEvent *event) override;
    void timerEvent(QTimerEvent *) override;
    void Show();
    void setNewView(QGraphicsSvgItem * toSet);

private slots:
    void updateZoomLabel();

private:
    QAction *m_nativeAction;
    QAction *m_glAction;
    QAction *m_imageAction;
    QAction *m_antialiasingAction;
    QAction *m_backgroundAction;
    QAction *m_outlineAction;

    QLabel *m_zoomLabel;

private:
    int timerId;
    viewer *mapViewer;
    map *mapInfo;

    QString m_currentPath;
};

#endif
