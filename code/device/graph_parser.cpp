#include "graph_parser.h"
#include "graph_alternative.h"
#include "map.h"
#include <QFile>

#define FIRST_LADDER_INDEX 128 // costyl TM, look at LoadLadder fun
#define ERROR_STR(str) (((QString)str)+" at "+__FILE__+" "+__LINE__)

std::vector<std::map // each map reffers to single floor
<vec, std::vector<quint32>> // key = point, value = edge indexes
> floorsWithPoints;


std::map<quint32,               // key = elevator index
std::vector<std::pair<vec, quint32>>   // each value has the set of coordinates+floor_indexes
> connections;


graph_alternative *resulGraph = nullptr;
std::vector<floor_layer>* floorSignatures = nullptr;

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


void LoadLadder (graph_alternative* g, quint32 floor, QDomElement& l)
{
    Q_ASSERT(g->floors.size() >= floor);
    floor_connection toAdd;

    QString ladderName = l.attribute(__LADDER_NAME_ATTRIBUTE);
    QStringList ladderParams = ladderName.split("_");
    Q_ASSERT(ladderParams.size() == 3);

    toAdd.pos.x = l.attribute(__LADDER_X_COORDINATES_ATTRIBUTE).toDouble();
    toAdd.pos.y = l.attribute(__LADDER_Y_COORDINATES_ATTRIBUTE).toDouble();

    quint32 ladderIndex = ladderParams.at(2).toULong() + FIRST_LADDER_INDEX;
    quint32 floorIndex =  ladderParams.at(0).toULong() - 1;
    Q_ASSERT(floorIndex == floor-1);

    if (connections.find(ladderIndex) == connections.end())
    {
        connections[ladderIndex] = std::vector<std::pair<vec, quint32>>();
    }
    connections[ladderIndex].push_back(std::pair<vec, quint32>(toAdd.pos, floorIndex));

    toAdd.svgId = ladderName;

    g->floors.at(floorIndex).connections[ladderIndex] = toAdd;
}

void LoadElevator (graph_alternative* g, quint32 floor, QDomElement& e)
{
    Q_ASSERT(g->floors.size() >= floor);
    floor_connection toAdd;

    QString elevatorName = e.attribute(__ELEVATOR_NAME_ATTRIBUTE);
    QStringList elevatorParams = elevatorName.split("_");
    Q_ASSERT(elevatorParams.size() == 3);

    toAdd.pos.x = e.attribute(__ELEVATOR_X_COORDINATES_ATTRIBUTE).toDouble();
    toAdd.pos.y = e.attribute(__ELEVATOR_Y_COORDINATES_ATTRIBUTE).toDouble();

    quint32 elevatorIndex = elevatorParams.at(2).toULong();
    quint32 floorIndex =  elevatorParams.at(0).toULong() - 1;
    Q_ASSERT(floorIndex == floor-1);

    if (connections.find(elevatorIndex) == connections.end())
    {
        connections[elevatorIndex] = std::vector<std::pair<vec, quint32>>();
    }
    connections[elevatorIndex].push_back(std::pair<vec, quint32>(toAdd.pos, floorIndex));

    toAdd.svgId = elevatorName;

    g->floors.at(floorIndex).connections[elevatorIndex] = toAdd;
}

void LoadEdge (graph_alternative* g, quint32 floor, QDomElement& e)
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

    mall_floor& f = g->floors.at(floor-1);
    quint32 indexOfToAddEdge = f.edges.size();



    if (floorsWithPoints.at(floor-1).find(toAdd.start) != floorsWithPoints.at(floor-1).end())
    {
        for (auto& neigbour : floorsWithPoints.at(floor-1).at(toAdd.start))
        {
//            f.edges.at(neigbour).adjacentEdges.push_back(indexOfToAddEdge);
//            toAdd.adjacentEdges.push_back(neigbour);
        }
    }
//    else
//    {
//        floorsWithPoints.at(floor-1)[toAdd.start]=std::vector<quint32>();
//    }
    floorsWithPoints.at(floor-1)[toAdd.start].push_back(indexOfToAddEdge);



    if (floorsWithPoints.at(floor-1).find(toAdd.end) != floorsWithPoints.at(floor-1).end())
    {
        for (auto& neigbour : floorsWithPoints.at(floor-1).at(toAdd.end))
        {
//            f.edges.at(neigbour).adjacentEdges.push_back(indexOfToAddEdge);
//            toAdd.adjacentEdges.push_back(neigbour);
        }
    }
//    else
//    {
//        floorsWithPoints.at(floor-1).emplace(toAdd.end, std::vector<quint32>());
//    }
    floorsWithPoints.at(floor-1)[toAdd.end].push_back(indexOfToAddEdge);


//    f.edges.push_back(toAdd);
}

void LoadGraphObjectsOfTheFloor (graph_alternative* res, QDomNode& pathGroupHolder)
{
    QDomNodeList pathGroup = pathGroupHolder.childNodes();
    QDomElement graphElement;
    quint32 floorIndex =
            pathGroupHolder.toElement().attribute(__LAYER_LABEL_ATTRIBUTE).split("_").at(0).toULong();

    mall_floor toAdd;
    res->floors.push_back(toAdd);
    floorsWithPoints.push_back(std::map<vec, std::vector<quint32>>());

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
            //LoadEdge(res, floorIndex, graphElement);
        }
    }
}



bool graph_parser::proceedFile()
{
    Q_ASSERT(resulGraph == nullptr);
    resulGraph = new graph_alternative();
    QDomNode floorNode;

    std::vector<floor_layer>* signatures = new std::vector<floor_layer>();
    floor_layer currentFloor;


    floorsWithPoints.clear();
    connections.clear();

    if (errorFlag == true || docName.length() == 0) //failed to open or didnt set the file
    {
        delete resulGraph;
        delete signatures;
        delete floorSignatures;
        floorSignatures = nullptr;
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

    for (quint32 i=0; i<resulGraph->floors.size(); i++)
    {
        mall_floor& f = resulGraph->floors.at(i);
        for (auto& con : f.connections)
        {
            for (auto& neigbours : connections[con.first])
            {
                if (neigbours.second != i)
                {
                    con.second.floorsConnected.emplace(neigbours.second);
                }
            }
            for (quint32& edges : floorsWithPoints.at(i)[con.second.pos])
            {
                 con.second.adjacentEdges.emplace(edges);
            }
        }
    }

    delete floorSignatures;
    floorSignatures = signatures;
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

graph_alternative *graph_parser::produceOtherGraph()
{
    if (errorFlag == true)
    {
        return nullptr;
    }
    graph_alternative* res = resulGraph;
    resulGraph = nullptr;
    return res;
}

QString graph_parser::getLastError()
{
    return errorFlag? "" : lastError;
}
