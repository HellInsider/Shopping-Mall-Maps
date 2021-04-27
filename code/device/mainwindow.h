#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "map.h"
#include "viewer.h"
#include "pathwidget.h"


class manager;

QT_BEGIN_NAMESPACE
class QGraphicsView;
class QSignalMapper;
class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QLabel;
QT_END_NAMESPACE

enum ui_button{
    BUTTON_SHOW,
    BUTTON_HIDE,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LOAD,
    BUTTON_SET_PATH_START,
    BUTTON_SET_PATH_END,
    BUTTON_DRAW_PATH
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    MainWindow(manager &mgr, bool customGraphicsView = false);
    ~MainWindow();

    path_widget * GetPathWidget();
    QWidget * GetCentralWidget();

    void AddLabel(QString text, int x, int y);

    void ClearLabels();
    void SetView(QGraphicsView * view);

    void Show();
    void ShowMenu();
    void HideMenu();

public slots:
    bool event(QEvent *event) override;

private slots:
    void updateZoomLabel();

private:
    void SetupUi(bool customGraphicsView);

    QPushButton *buttonShow;
    QPushButton *buttonHide;
    QPushButton *buttonUp;
    QPushButton *buttonDown;
    QSignalMapper *buttonMapper;
    path_widget *pathWidget;

    QLabel *zoomLabel;
    std::vector<QLabel*> itemsLabels;

    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QGraphicsView *graphicsView;

    manager &manager;
};

#endif
