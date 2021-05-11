#include "graph_parser.h"
#include "fine_graph.h"
#include "map.h"
#include <QFile>
#include <fstream>

#define FIRST_LADDER_INDEX 128 // costyl TM, look at LoadLadder fun
#define ERROR_STR(str) (((QString)str)+" at "+__FILE__+" "+__LINE__)


fine_graph *resulGraph = nullptr;
std::vector<floor_layer> *floorSignatures = nullptr;

using floor_pos_pair_array = std::vector<std::pair<quint32,point>>;

std::map<QString, floor_pos_pair_array> laddersAndElevators = std::map<QString, floor_pos_pair_array>();
std::vector<std::pair<quint32,edge>> edgesToAdd = std::vector<std::pair<quint32,edge>>();





graph_parser::graph_parser() :  docName(), doc(), edges(), errorFlag(false), lastError()
{
}

graph_parser::graph_parser(QString fileName):  docName(fileName), doc(), edges(), errorFlag(false), lastError()
{
    QFile f(fileName);
    if(!f.exists())
    {
        errorFlag = true;
        lastError = ERROR_STR("Failed to find the file with such name as "+fileName);
        return;
    }
    if(!f.open(QIODevice::ReadOnly))
    {
        errorFlag = true;
        lastError = ERROR_STR("Failed to open file "+fileName);
        return;
    }
    errorFlag = !doc.setContent(&f, true);
    if (errorFlag)
    {
        lastError = "Failed to parse opened file";
        return;
    }
}

bool graph_parser::loadFile(QString fileName)
{
    QFile f(fileName);
    if (errorFlag == true)
    {
        return false;
    }

    if(!f.exists())
    {
        errorFlag = true;
        lastError = ERROR_STR("Failed to find the file with such name as "+fileName);
        return false;
    }

    if(!f.open(QIODevice::ReadOnly))
    {
        errorFlag = true;
        lastError = ERROR_STR("Failed to open file "+fileName);
        return false;
    }

    errorFlag = !doc.setContent(&f, true);
    if (errorFlag)
    {
        lastError = "Failed to parse opened file";
        return false;
    }
    return true;
}


bool HasAttribute (QDomNode const & n, QString const & name, QString const & value)
{
    Q_ASSERT(!n.toElement().isNull());

    return (n.toElement().attribute(name) == value);
}

bool HasAttribute (QDomNode const & n, QString const & name, QRegExp const & mask)
{
    Q_ASSERT(!n.toElement().isNull());

    return (mask.exactMatch(n.toElement().attribute(name)));
}

bool HasAttributes (QDomNode const & n, QString const & name1, QString const & value1,
                    QString const & name2, QString const & value2)
{
    QDomElement e = n.toElement();
    Q_ASSERT(!e.isNull());

    return (e.attribute(name1) == value1 && e.attribute(name2) == value2);
}

bool HasAttributes (QDomNode const & n, QString const & name1, QString const & value1,
                    QString const & name2, QRegExp const & mask2)
{
    QDomElement e = n.toElement();
    Q_ASSERT(!e.isNull());

    bool firstCheck = (e.attribute(name1) == value1);
    //QString tmp = e.attribute(name2);
    bool secondCheck = (mask2.exactMatch(e.attribute(name2)));
    return (firstCheck && secondCheck);
}


void LoadLadder (fine_graph* g, quint32 floor, QDomElement& l)
{
    Q_ASSERT(g->floors.size() >= floor);

    point pos;

    QString ladderName = l.attribute(__LADDER_NAME_ATTRIBUTE);
    QStringList ladderParams = ladderName.split("_");
    Q_ASSERT(ladderParams.size() == 3);

    pos.x = l.attribute(__LADDER_X_COORDINATES_ATTRIBUTE).toDouble();
    pos.y = l.attribute(__LADDER_Y_COORDINATES_ATTRIBUTE).toDouble();

    RoundCoord(pos);

    quint32 floorIndex =  ladderParams.at(0).toULong() - 1;
    Q_ASSERT(floorIndex == floor-1);

    QString nameWithoutFloorPrefix = ladderParams[1]+"_"+ladderParams[2];
    if (laddersAndElevators.find(nameWithoutFloorPrefix) == laddersAndElevators.end())
    {
        laddersAndElevators[nameWithoutFloorPrefix] = floor_pos_pair_array();
    }
    laddersAndElevators[nameWithoutFloorPrefix].push_back(std::make_pair(floorIndex, pos));
}

void LoadElevator (fine_graph* g, quint32 floor, QDomElement& e)
{
    Q_ASSERT(g->floors.size() >= floor);

    point pos;

    QString elevatorName = e.attribute(__ELEVATOR_NAME_ATTRIBUTE);
    QStringList elevatorParams = elevatorName.split("_");
    Q_ASSERT(elevatorParams.size() == 3);

    pos.x = e.attribute(__ELEVATOR_X_COORDINATES_ATTRIBUTE).toDouble();
    pos.y = e.attribute(__ELEVATOR_Y_COORDINATES_ATTRIBUTE).toDouble();

    RoundCoord(pos);

    RoundCoord(pos);

    quint32 floorIndex =  elevatorParams.at(0).toULong() - 1;
    Q_ASSERT(floorIndex == floor-1);

    QString nameWithoutFloorPrefix = elevatorParams[1]+"_"+elevatorParams[2];
    if (laddersAndElevators.find(nameWithoutFloorPrefix) == laddersAndElevators.end())
    {
        laddersAndElevators[nameWithoutFloorPrefix] = floor_pos_pair_array();
    }
    laddersAndElevators[nameWithoutFloorPrefix].push_back(std::make_pair(floorIndex, pos));
}

void LoadEdge (fine_graph* g, quint32 floor, QDomElement& e)
{
    QString asdasda = e.toElement().attribute("id");
    Q_ASSERT(g->floors.size() >= floor);
    edge toAdd;

    QStringList pathCoords = e.attribute(__EDGE_COORDINATES_ATTRIBUTE).split(" ");
    Q_ASSERT(pathCoords.size() >= 3 && pathCoords.at(0) == __PATH_ATTRIBUTES[PA_START]);

    QStringList startCoords = pathCoords.at(1).split(__XY_COORDS_SEPARATOR);
    toAdd.start = vec (startCoords.at(0).toDouble(), startCoords.at(1).toDouble());

    if (pathCoords.at(2) == __PATH_ATTRIBUTES[PA_SHIFT_HORISONTAL])
    {
         toAdd.end = toAdd.start;
         toAdd.end.x = pathCoords.at(3).toDouble();
    }
    else
    {
        if (pathCoords.at(2) == __PATH_ATTRIBUTES[PA_SHIFT_VERTICAL])
        {
             toAdd.end = toAdd.start;
             toAdd.end.y = pathCoords.at(3).toDouble();
        }
        else
        {
            if (pathCoords.at(2) == __PATH_ATTRIBUTES[MYSTERIOUS_L_TO_REFACTOR])
            {
                QStringList endCoords = pathCoords.at(3).split(__XY_COORDS_SEPARATOR);
                Q_ASSERT(endCoords.size() == 2);
                toAdd.end = vec (endCoords.at(0).toDouble(), endCoords.at(1).toDouble());
            }
            else
            {
                 QStringList endCoords = pathCoords.at(2).split(__XY_COORDS_SEPARATOR);
                 Q_ASSERT(endCoords.size() == 2);
                 toAdd.end = vec (endCoords.at(0).toDouble(), endCoords.at(1).toDouble());
            }
        }
    }
    toAdd.len = toAdd.start.distance(toAdd.end);
    toAdd.svgId = e.attribute(__EDGE_NAME_ATTRIBUTE);

    RoundCoord(toAdd.start);
    RoundCoord(toAdd.end);
    edgesToAdd.push_back(std::make_pair(floor-1, toAdd));


}

void LoadGraphObjectsOfTheFloor (fine_graph* res, QDomNode& pathGroupHolder)
{
    QDomNodeList pathGroup = pathGroupHolder.childNodes();
    QDomElement graphElement;
    quint32 floorIndex =
            pathGroupHolder.toElement().attribute(__LAYER_LABEL_ATTRIBUTE).split("_").at(0).toULong();

    mall_floor toAdd;
    res->floors.push_back(toAdd);

    for (int i=0; i<pathGroup.length(); i++)
    {
        graphElement = pathGroup.at(i).toElement();
        if (graphElement.isNull())
        {
            continue;
        }

        if (graphElement.tagName() == __ELEVATOR_TAG &&
                HasAttribute(graphElement, __ELEVATOR_NAME_ATTRIBUTE, __ELEVATOR_NAME_MASK))
        {
            LoadElevator(res, floorIndex, graphElement);
        }
        if (graphElement.tagName() == __LADDER_TAG &&
                 HasAttribute(graphElement, __LADDER_NAME_ATTRIBUTE, __LADDER_NAME_MASK))
        {
            LoadLadder(res, floorIndex, graphElement);
        }
        if (graphElement.tagName() == __EDGE_TAG)
        {
            LoadEdge(res, floorIndex, graphElement);
        }
    }
}



bool graph_parser::proceedFile()
{
    Q_ASSERT(resulGraph == nullptr);
    resulGraph = new fine_graph(); //RAII
    QDomNode floorNode;

    std::vector<floor_layer>* signatures = new std::vector<floor_layer>();
    floor_layer currentFloor;


    laddersAndElevators.clear();
    edgesToAdd.clear();

    if (errorFlag == true || docName.length() == 0) //failed to open or didnt set the file
    {
        delete resulGraph;
        resulGraph = nullptr;
        return false;
    }

    QDomNodeList groups = doc.elementsByTagName(__LAYER_TAG);

    for (int i = 0; i<groups.length(); i++)
    {
        floorNode = groups.item(i);
        if (floorNode.isNull())
        {
            continue;
        }


        if (HasAttributes (floorNode, __LAYER_ATTRIBUTE_NAME, __LAYER_ATTRIBUTE_VALUE,
                           __LAYER_LABEL_ATTRIBUTE, __LAYER_LABEL_MASK))
        {
//            QString asdasd = floorNode.toElement().attribute(__LAYER_LABEL_ATTRIBUTE);
//            int asd = floorNode.toElement().attribute(__LAYER_LABEL_ATTRIBUTE).toUInt();
//            quint32 gsdf = signatures->size()+1;
            Q_ASSERT(floorNode.toElement().attribute(__LAYER_LABEL_ATTRIBUTE).toUInt() == signatures->size()+1);
            currentFloor.layerName = floorNode.toElement().attribute("id");

            QDomElement currentNode = floorNode.firstChildElement();
            while (!currentNode.isNull())
            {
                if (HasAttributes (currentNode, __LAYER_ATTRIBUTE_NAME, __LAYER_ATTRIBUTE_VALUE,
                                   __LAYER_LABEL_ATTRIBUTE, __AREA_SUBLAYER_LABEL_MASK))
                {
                    Q_ASSERT(currentNode.attribute(__LAYER_LABEL_ATTRIBUTE).split("_").at(0).toUInt() == signatures->size()+1);
                    currentFloor.bckgrndLr.layerName = currentNode.attribute(__ELEMENT_NAME_ATTRIBUTE);
                }

                if (HasAttributes (currentNode, __LAYER_ATTRIBUTE_NAME, __LAYER_ATTRIBUTE_VALUE,
                                   __LAYER_LABEL_ATTRIBUTE, __SHOPS_SUBLAYER_LABEL_MASK))
                {
                    Q_ASSERT(currentNode.attribute(__LAYER_LABEL_ATTRIBUTE).split("_").at(0).toUInt() == signatures->size()+1);
                    currentFloor.shopsLr.layerName = currentNode.attribute(__ELEMENT_NAME_ATTRIBUTE);
                    QDomElement currentShop = currentNode.firstChildElement();
                    while (!currentShop.isNull())
                    {
                        Q_ASSERT(currentShop.attribute(__ELEMENT_NAME_ATTRIBUTE).split("_").at(0).toUInt() == signatures->size()+1);
                        currentFloor.shopsLr.namesAndIndexes.insert(currentShop.attribute(__ELEMENT_NAME_ATTRIBUTE), 0);
                        currentShop = currentShop.nextSiblingElement();
                    }
                }

                if (HasAttributes (currentNode, __LAYER_ATTRIBUTE_NAME, __LAYER_ATTRIBUTE_VALUE,
                                   __LAYER_LABEL_ATTRIBUTE, __PATHS_SUBLAYER_LABEL_MASK))
                {
                    Q_ASSERT(currentNode.attribute(__LAYER_LABEL_ATTRIBUTE).split("_").at(0).toUInt() == signatures->size()+1);
                    currentFloor.pathsLr.layerName = currentNode.attribute(__ELEMENT_NAME_ATTRIBUTE);
                    QDomElement currentGraphElement = currentNode.firstChildElement();
                    while (!currentGraphElement.isNull())
                    {
                        currentFloor.pathsLr.graphObjects.push_back(currentGraphElement.attribute(__ELEMENT_NAME_ATTRIBUTE));
                        currentGraphElement = currentGraphElement.nextSiblingElement();
                    }
                }
                currentNode = currentNode.nextSiblingElement();
            }
            signatures->push_back(currentFloor);
            currentFloor = floor_layer();
        }



        if(HasAttributes (floorNode,__LAYER_ATTRIBUTE_NAME, __LAYER_ATTRIBUTE_VALUE,
                          __LAYER_LABEL_ATTRIBUTE, __PATHS_SUBLAYER_LABEL_MASK ))
        {
            LoadGraphObjectsOfTheFloor(resulGraph, floorNode);
        }
    }

    delete floorSignatures;
    floorSignatures = signatures;


    for (auto &le : laddersAndElevators)
    {

        for (auto &f : le.second)
        {
            if(resulGraph->floors[f.first].vertexes.find(f.second) == resulGraph->floors[f.first].vertexes.end())
            {
               resulGraph->floors[f.first].vertexes[f.second] = vertex();
               resulGraph->floors[f.first].vertexes[f.second].pos = f.second;
            }
            resulGraph->floors[f.first].vertexes[f.second].isShowable = true;
            resulGraph->floors[f.first].vertexes[f.second].svgId = QString::number(f.first+1)+QString("_")+le.first;
            for (auto &fc : le.second)
            {
                if (fc.first == f.first)
                {
                    continue;
                }
                resulGraph->floors[f.first].vertexes[f.second].adjacentVertexesOnOtherFloors.push_back(mulifloor_connection(fc.first, fc.second, 1.0));
            }
        }
    }

    for (auto &e : edgesToAdd)
    {
        if (resulGraph->floors[e.first].vertexes.find(e.second.start) == resulGraph->floors[e.first].vertexes.end())
        {
            resulGraph->floors[e.first].vertexes[e.second.start] = vertex();
            resulGraph->floors[e.first].vertexes[e.second.start].pos = e.second.start;
        }
        if (resulGraph->floors[e.first].vertexes.find(e.second.end) == resulGraph->floors[e.first].vertexes.end())
        {
            resulGraph->floors[e.first].vertexes[e.second.end] = vertex();
            resulGraph->floors[e.first].vertexes[e.second.end].pos = e.second.end;
        }

        resulGraph->floors[e.first].vertexes[e.second.start].edgesConnected.push_back(resulGraph->floors[e.first].edges.size());
        resulGraph->floors[e.first].vertexes[e.second.end].edgesConnected.push_back(resulGraph->floors[e.first].edges.size());

        resulGraph->floors[e.first].edges.push_back(e.second);
    }

    std::ofstream f;
    f.open("D:/study/6sem/proj/blank/Shopping-Mall-Maps/resources/file4Misha.txt");

    quint32 vertexesCount = 0;
    for (auto& fl : resulGraph->floors)
    {
        vertexesCount += fl.vertexes.size();
    }

    f << vertexesCount << std::endl;

    for (quint32 i = 0; i<resulGraph->floors.size(); i++)
    {
        auto& fl = resulGraph->floors[i];
        for (auto& v : fl.vertexes)
        {

            f << (v.first.x) << " " << v.first.y << " " << i << std::endl;
        }
    }

    std::vector<quint32> vertexesBelow;
    for (quint32 i = 0; i<resulGraph->floors.size(); i++)
    {
        if (i==0)
        {
            vertexesBelow.push_back(0);
        }
        else
        {
            vertexesBelow.push_back(vertexesBelow[i-1]+resulGraph->floors[i-1].vertexes.size());
        }
    }

    for (quint32 i = 0; i<resulGraph->floors.size(); i++)
    {
        auto& fl = resulGraph->floors[i];
        for (auto& v : fl.vertexes)
        {
            for (auto n: v.second.edgesConnected)
            {
                point& neigbour = (fl.edges[n].end == v.first ? fl.edges[n].start  : fl.edges[n].end);
                f << vertexesBelow[i] + distance(fl.vertexes.begin(),fl.vertexes.find(neigbour)) << " ";
            }
            for (auto n: v.second.adjacentVertexesOnOtherFloors)
            {
                f << vertexesBelow[n.floorConnected] + distance(resulGraph->floors[n.floorConnected].vertexes.begin(),
                        resulGraph->floors[n.floorConnected].vertexes.find(n.otherEnding)) << " ";
            }
            f << std::endl;
        }

    }
    f.close();

    return true;
}

void* graph_parser::getSignatures() // use static_cast<vector<floor_layer>*>(...)
{
    if (errorFlag == true)
    {
        return nullptr;
    }
    void* res = static_cast<void*>(floorSignatures);
    floorSignatures = nullptr;
    return res;
}

graph *graph_parser::produceGraph()
{
    return nullptr;
}

fine_graph *graph_parser::produceOtherGraph()
{
    if (errorFlag == true)
    {
        return nullptr;
    }
    fine_graph* res = resulGraph;
    resulGraph = nullptr;
    return res;
}

QString graph_parser::getLastError()
{
    return errorFlag? "" : lastError;
}
