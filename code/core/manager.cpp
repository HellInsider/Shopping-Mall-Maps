#include <QFileInfo>
#include <QLabel>
#include <QPushButton>
#include "device/viewer.h"
#include "device/manager.h"
#include "device/fine_graph.h"
#include "device/pathwidget.h"
#include "shops_data.h"

void manager::SetPath(path *p)
{
    activePath = p;
    mapViewer->AddPolyline(activePath, currentFloor);
    for (auto& e : p->defaultEdges)
    {
        mapViewer->ChangeVisibility(e, true);
    }
}

manager::manager() : window(*this, true), currentMap(), currentFloor(0)
{
    mapViewer = new viewer(window.GetCentralWidget());
    window.SetView(mapViewer);
    connect(&currentMap, &map::PathGraphChanged, this, &manager::OnNewGraph);
    connect(&currentMap, &map::MapPictureDataChanged, this, &manager::OnNewMap);
}

void manager::OnButton(int butttonPressed)
{
    std::vector<QString> ids;
    QMap<QString, quint32> mapping;
    QString text("fuck");

    switch (butttonPressed){
    case BUTTON_SHOW:
        window.ShowMenu();
        break;
    case BUTTON_HIDE:
        window.HideMenu();
        break;
    case BUTTON_UP:
        SetFloor(quint32(currentFloor + 1));
        break;
    case BUTTON_DOWN:
        SetFloor(quint32(currentFloor - 1));
        break;
    case BUTTON_DRAW_PATH:
        path_data pd = window.GetPathWidget()->GetData();
        vec start(pd.x1, pd.y1);
        vec end(pd.x2, pd.y2);
        graph->SetStart(start, quint32(pd.floor1)-1);
        graph->SetEnd(end, quint32(pd.floor2)-1);
        graph->FindPath();
        break;
    }
}


void manager::SetFloor(quint32 index)
{
    if (index < floorLayers->size() && index >= 0 /*&& index != quint32(currentFloor)*/)
    {
        currentFloor = index;

        mapViewer->ClearUnstableVisible();
        mapViewer->ClearPolyline();

        QString bgr = (*floorLayers)[index].GetBckgrndLr().GetName();
        mapViewer->ChangeBgrLayer(bgr);

        std::vector<QString> ed = (*floorLayers)[index].GetPathsLr().GetObjects();
        for (auto& s : ed)
        {
            mapViewer->AddUnstableVisible(s);
            mapViewer->ChangeVisibility(s, false);
        }

        DrawShopsWithLabels();

        mapViewer->AddPolyline(activePath, index);
        if (activePath != nullptr)
        {
            for (auto& e : activePath->defaultEdges)
            {
                mapViewer->ChangeVisibility(e, true);
            }
        }
    }
}


void manager::DrawShopsWithLabels()
{
    QMap<QString, quint32> mapping;
    mapping = (*floorLayers)[currentFloor].GetShopsLr().GetShops();
    mapViewer->ClearSelectables();
    mapViewer->ClearLabels();
    if(currentMap.GetInfo() != nullptr)
    {
        const QStringList& names = currentMap.GetInfo()->getShortNames();

        for (auto shop = mapping.begin(); shop != mapping.end(); shop++)
        {
            mapViewer->AddSelectable(shop.key());
            mapViewer->AddLabel( names[shop.value()], 40, 100, shop.key());
        }
    }
}



void manager::OnNewGraph(fine_graph *newGraph)
{
    graph = newGraph;
    connect(graph, SIGNAL(PathFound(path*)), this, SLOT(SetPath(path*)));
}

void manager::OnNewMap(std::vector<floor_layer>* svgIds)
{
    if (svgIds)
    {
        floorLayers = svgIds;
        SetFloor(0);
    }
}

void manager::LoadData(const QString &svgFileName, const QString &xmlFileName)
{
    if (QFileInfo::exists(svgFileName))
    {
        mapViewer->InitMap(svgFileName);
    }
    if (QFileInfo::exists(svgFileName) && QFileInfo::exists(xmlFileName))
    {
        currentMap.SetAnotherMap(svgFileName, xmlFileName);
        DrawShopsWithLabels();
    }
}

int manager::SearchShop(const QString nameShop)
{
    QStringList namesShops = currentMap.GetInfo()->getShortNames();
    for(int i = 0; i < namesShops.size(); ++i)
    {
        if(namesShops.at(i) == nameShop)
        {
            return i;
        }
    }
    return -1;
}





