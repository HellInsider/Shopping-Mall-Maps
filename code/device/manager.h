#ifndef MGR_H
#define MGR_H

#include "device/mainwindow.h"
#include "device/map.h"


class fine_graph;
class viewer;


class manager : public QObject
{
    Q_OBJECT

    MainWindow window;
    map currentMap;
    viewer *mapViewer; //NOTE: ownership of viewer has window but manager can manage its creation and call its methods
    fine_graph *graph; //NOTE: ownership of graph has map but manager can manage its creation and call its methods
    std::vector<floor_layer>* floorLayers; //NOTE: ownership of floorLayers has map but manager can manage its creation and call its methods
    path* activePath;
    int currentFloor;

    void DrawShopsWithLabels();
public:
    manager();

    void LoadData(const QString &svgFileName, const QString &xmlFileName);

    void Start() { window.Show(); }

public slots:
    void SetPath(path* p);
    void OnButton(int butttonPressed);
    void OnNewMap(std::vector<floor_layer>* svgIds);
    void OnNewGraph(fine_graph *graph);
};

#endif // MGR_H
