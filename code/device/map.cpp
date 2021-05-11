#include <QSvgRenderer>
#include "map.h"
#include "fine_graph.h"
#include "graph_parser.h"
#include "shops_data.h"


#define __MAP_SVG_CHANGED ((quint8)2)
#define __MAP_INFO_CHANGED ((quint8)4)
#define __MAP_ALL_CHANGED ((__MAP_SVG_CHANGED)|(__MAP_INFO_CHANGED))




shops_data* map::GetInfo (void)
{
    return info;
}


map::map(QObject *parent) :
    QObject(parent), svgMapFileName(""),paths(nullptr), mapInfoFileName(""), object_indexes()
{
}

map::~map()
{
    ClearAll();
    OnChange(__MAP_ALL_CHANGED);
}


quint32 map::GetIndex(QLatin1String name)
{
    return object_indexes[name];
}


void map::SetAnotherMap(QString const & mapToSet, QString const & extrasToSet)
{
    quint8 flags = 0;
    if (mapToSet != __LEAVE_MAP_AS_IT_IS && mapToSet != svgMapFileName)
    {
        flags |= __MAP_SVG_CHANGED;
    }

    if (extrasToSet != __LEAVE_MAP_AS_IT_IS && extrasToSet != mapInfoFileName)
    {
        flags |= __MAP_INFO_CHANGED;
    }

    if ((flags & __MAP_SVG_CHANGED) != 0)
    {
        ClearGraph();
        graph_parser gp(mapToSet);
        if (gp.proceedFile())
        {
            paths = gp.produceOtherGraph();
            floorsIdData = static_cast<std::vector<floor_layer>*>(gp.getSignatures());
        }
    }
    if ((flags & __MAP_INFO_CHANGED) != 0)
    {
        ClearObjects();
        info = InitInfoFromFile (extrasToSet, object_indexes);

        for (auto& f : *floorsIdData)
        {
            for (auto s = f.shopsLr.namesAndIndexes.keyValueBegin(); s != f.shopsLr.namesAndIndexes.keyValueEnd(); s++)
            {
                 s->second = object_indexes[s->first];
            }
        }
    }

    OnChange(flags);
}


void map::ClearObjects() // doesnt inform anyone. emit signals yourself!
{
    object_indexes.clear();

}
void map::ClearFloorData()  // doesnt inform anyone. emit signals yourself!
{
    delete floorsIdData;
    floorsIdData = nullptr;
}
void map::ClearGraph()  // doesnt inform anyone. emit signals yourself!
{
    delete paths;
    paths = nullptr;
}

void map::ClearAll()  // doesnt inform anyone. emit signals yourself!
{
    ClearFloorData();
    ClearGraph();
    ClearObjects();
}


void map::OnChange(quint8 singal_flags)
{

    if ((singal_flags & __MAP_SVG_CHANGED) != 0)
    {
        emit MapPictureDataChanged(floorsIdData);
        emit PathGraphChanged(paths);
    }
    if ((singal_flags & __MAP_INFO_CHANGED) != 0)
    {
        emit MapInfoChanged(info, object_indexes);
    }
}

background_on_floor floor_layer::GetBckgrndLr() const
{
    return bckgrndLr;
}

shops_on_floor floor_layer::GetShopsLr() const
{
    return shopsLr;
}

paths_on_floor floor_layer::GetPathsLr() const
{
    return pathsLr;
}
