#include "graph_alternative.h"

constexpr double MAX_DBL = std::numeric_limits<double>::max();
constexpr quint32 MAX_INT = std::numeric_limits<quint32>::max();

constexpr double NO_PATH_DBL = MAX_DBL;
constexpr quint32 NO_PATH_INT = MAX_INT;



//graph_alternative::graph_alternative()
//{
//}

graph_alternative::graph_alternative(QObject *parent) : QObject(parent), startFloorIndex(0), endFloorIndex(0)
{
}

void graph_alternative::SetStart(point p, quint32 floorIndex)
{
    if ((startFloorIndex != floorIndex || start != p) && floorIndex < floors.size())
    {
        startFloorIndex = floorIndex;
        start = p;
        ClearPath();
    }
}

void graph_alternative::SetEnd(point p, quint32 floorIndex)
{
    if ((endFloorIndex != floorIndex || end != p) && floorIndex < floors.size())
    {
        endFloorIndex = floorIndex;
        end = p;
        ClearPath();
    }
}

bool edge::CrossesOnStart(const edge &neigbour) const
{
    if (neigbour.start == start || neigbour.end == start)
    {
        return true;
    }
    return false;
}



void graph_alternative::MapOnEnd(mall_floor& endFloor, quint32 lastEdgeIndex, double t)
{
    edge& lastEdge = endFloor.edges.at(lastEdgeIndex);
    for (quint32 neighborIndex : lastEdge.adjacentEdges)
    {
        if (lastEdge.CrossesOnStart(endFloor.edges.at(neighborIndex)))
        {
            endFloor.edges.at(neighborIndex).currPathLen = lastEdge.len * t;
        }
        else
        {
            endFloor.edges.at(neighborIndex).currPathLen = lastEdge.len * (1-t);
        }
    }
    endFloor.ProceedPath(lastEdgeIndex); // done Dijkstra’s algorithm mapping

    for (auto &c : endFloor.connections)
    {
        if(c.second.adjacentEdges.find(lastEdgeIndex) != c.second.adjacentEdges.end())
        {
            c.second.currPathLen = (c.second.pos == lastEdge.start ? t : (1-t)) * lastEdge.len;
        }
    }
}


void graph_alternative::TransmitPathLength(quint32 floorFromIndex, quint32 floorToIndex)
{
    for (auto &c : floors.at(floorFromIndex).connections)
    {
        if (c.second.floorsConnected.find(floorToIndex) != c.second.floorsConnected.end())
        {
            Q_ASSERT(floors.at(floorToIndex).connections.find(c.first) != floors.at(floorToIndex).connections.end());
            floors.at(floorToIndex).connections[c.first].currPathLen = c.second.currPathLen;
        }
    }
}



void graph_alternative::FindPath(void)
{
    if (!currentPath.empty()) // path already exists
    {
        emit PathFound(&currentPath);
        return;
    }

    currentPath.startFloorIndex = startFloorIndex;
    currentPath.endFloorIndex = endFloorIndex;


    mall_floor startFloor = floors.at(startFloorIndex);
    mall_floor endFloor = floors.at(endFloorIndex);

    point startPerp,
            endPerp;

    double startEdgeSplit,
            endEdgeSplit;

    quint32 firstEdgeIndex = startFloor.NearestEdge(start, startPerp, startEdgeSplit);
    quint32 lastEdgeIndex = endFloor.NearestEdge(end, endPerp, endEdgeSplit);




   if (firstEdgeIndex == lastEdgeIndex) // path takes only temporary points
   {
       currentPath.prePath.push_back(start);
       currentPath.prePath.push_back(startPerp);
       currentPath.prePath.push_back(endPerp);
       currentPath.prePath.push_back(end);
       emit PathFound(&currentPath);
       return;
   }

    edge& firstEdge = startFloor.edges.at(firstEdgeIndex),
          lastEdge= endFloor.edges.at(lastEdgeIndex);


    lastEdge.currPathLen = 0;
    for (quint32& neighborIndex : lastEdge.adjacentEdges) // choosing the best one to start with
    {
        if (lastEdge.CrossesOnStart(endFloor.edges.at(neighborIndex)))
        {
            endFloor.edges.at(neighborIndex).currPathLen = lastEdge.len * startEdgeSplit;
        }
        else
        {
            endFloor.edges.at(neighborIndex).currPathLen = lastEdge.len * (1-startEdgeSplit);
        }
    }

    endFloor.ProceedPath(lastEdgeIndex);
    std::vector<quint32> floorsInPath = PathThroughFloors();

    if (startFloorIndex != endFloorIndex) // path takes more than one floor
    {
        Q_ASSERT(!floorsInPath.empty());

        for (quint32 f = floorsInPath.size() - 1; f>0; f--)
        {
            TransmitPathLength(floorsInPath.at(f), floorsInPath.at(f-1));
            floors.at(floorsInPath.at(f-1)).ProceedPath(MAX_INT);
        }
    }

    currentPath.prePath.push_back(start);
    currentPath.prePath.push_back(startPerp);
    RecoverPath(firstEdgeIndex, startEdgeSplit, lastEdgeIndex, floorsInPath);
    currentPath.postPath.push_back(endPerp);
    currentPath.postPath.push_back(end);

    emit PathFound(&currentPath);
    return;
}


quint32 mall_floor::GetNextEdgeInPath (quint32 currEdgeInd, bool& isEdge) const
{
    edge const & e = edges.at(currEdgeInd);
    double bestWay = e.currPathLen;
    quint32 res = MAX_INT;
    isEdge = true;

    for (auto ae : e.adjacentEdges)
    {
        if (edges.at(ae).currPathLen < bestWay)
        {
            bestWay = edges.at(ae).currPathLen;
            res = ae;
        }
    }

    for (auto & c : connections)
    {
        if (c.second.adjacentEdges.find(currEdgeInd) != c.second.adjacentEdges.end() &&
                c.second.currPathLen < bestWay)
        {
            isEdge = false;
            bestWay = c.second.currPathLen;
            res = c.first;
        }
    }
    return res;
}

quint32 graph_alternative::RecoverOnStart(mall_floor const & startFloor, quint32 firstEdgeIndex, double t, bool& edgeFlag) const
{
    edge const &firstEdge = startFloor.edges.at(firstEdgeIndex);
    quint32 currentEdge = MAX_INT;
    quint32 currentConnection = MAX_INT;

    double weight = firstEdge.currPathLen + firstEdge.len;

    for (quint32 ae : firstEdge.adjacentEdges)
    {
        if (startFloor.edges.at(ae).currPathLen + // path to the neighbour
                (firstEdge.CrossesOnStart(startFloor.edges.at(ae)) ?
                t : (1-t)) * firstEdge.len // path to the perpendicular point
                < weight)
        {
            weight = startFloor.edges.at(ae).currPathLen +
                    (firstEdge.CrossesOnStart(startFloor.edges.at(ae)) ? t : (1-t)) * firstEdge.len;
            currentEdge = ae;
        }
    }

    for (auto& c : startFloor.connections)
    {
        if(c.second.adjacentEdges.find(firstEdgeIndex) != c.second.adjacentEdges.end() &&
                    c.second.currPathLen + (c.second.pos == firstEdge.start ? t : (1-t)) * firstEdge.len < weight)
        {
            weight = c.second.currPathLen + (c.second.pos == firstEdge.start ? t : (1-t)) * firstEdge.len;
            currentConnection = c.first;
            currentEdge = MAX_INT;
        }
    }

    if (currentEdge == MAX_INT)
    {
        edgeFlag = false;
        return currentConnection;
    }
    edgeFlag = true;
    return currentEdge;

}




void graph_alternative::RecoverPath(quint32 firstEdgeIndex, double t, quint32 lastEdgeIndex, std::vector<quint32>& floorsSequence)
{
    bool keepTheFloor;
    quint32 currentFloorIndex = 0;
    mall_floor const * currentFloor = &(floors.at(floorsSequence.at(currentFloorIndex)));
    quint32 currentEdge = MAX_INT;


    currentEdge = RecoverOnStart(*currentFloor, firstEdgeIndex, t, keepTheFloor);

    if(keepTheFloor)
    {
        currentPath.prePath.push_back(currentFloor->edges.at(firstEdgeIndex).CrossesOnStart(currentFloor->edges.at(currentEdge)) ?
                                       currentFloor->edges.at(firstEdgeIndex).start : currentFloor->edges.at(firstEdgeIndex).end);
    }
    else
    {
        currentPath.prePath.push_back(currentFloor->connections.at(currentEdge).pos);
    }

    quint32 nextEdge = currentEdge;

    while (currentEdge != MAX_INT)
    {
         currentEdge = nextEdge;
         if (keepTheFloor)
         {
            currentPath.defaultEdges << currentFloor->edges.at(currentEdge).svgId;
            nextEdge = currentFloor->GetNextEdgeInPath(currentEdge, keepTheFloor);
            if (nextEdge == lastEdgeIndex && currentFloorIndex == floorsSequence.back())
            {
                currentPath.postPath.push_back(
                            currentFloor->edges.at(lastEdgeIndex).CrossesOnStart(currentFloor->edges.at(currentEdge)) ?
                                 currentFloor->edges.at(lastEdgeIndex).start :  currentFloor->edges.at(lastEdgeIndex).end);
                break;
            }
         }
         else
         {
             currentPath.defaultEdges << currentFloor->connections.at(currentEdge).svgId;
             currentFloorIndex ++;
             currentFloor = &(floors.at(floorsSequence.at(currentFloorIndex)));
             currentPath.defaultEdges << currentFloor->connections.at(currentEdge).svgId;
             double weight = currentFloor->connections.at(currentEdge).currPathLen;
             for (quint32 ae : currentFloor->connections.at(currentEdge).adjacentEdges)
             {
                 if (currentFloor->edges.at(ae).currPathLen < weight)
                 {
                     weight = currentFloor->edges.at(ae).currPathLen;
                     nextEdge = ae;
                     keepTheFloor = true;
                 }
             }

         }
    }

}


quint32 mall_floor::NearestEdge (point pt, point& perpendicularPoint, double& t_res) const
{
    point bestPoint;
    double bestT = -1;
    double bestDistanceSqr = MAX_DBL;

    point currPoint;
    quint32 bestEdgeIndex = -1;

    vec currentEdgeVector;
    double t; // param, defining the perpendicular point position between start and end
    for (size_t i =0; i<edges.size(); i++)
    {
        currentEdgeVector = edges.at(i).end - edges.at(i).start;

//        (X-pt)*AB = 0;
//        X-pt = At + B(1-t) - pt
//        (X-pt)*AB =  A*AB*t + B*AB*(1-t) - pt*AB
//        A*AB*t + B*AB*(1-t) - pt*AB = 0
//        (A-B)*AB*t + B*AB = pt*AB
//        AB^2*t  = (B-pt)*AB

        t = (edges.at(i).end-pt).dot(currentEdgeVector)/currentEdgeVector.sqr_magnitude();
        currPoint = edges.at(i).start.lerp(edges.at(i).end, t);
        if((currPoint-pt).sqr_magnitude() > bestDistanceSqr)
        {
            if (t < 0 )
            {
                t = 0;
            }
            if (t > 1)
            {
                t = 1;
            }

            bestEdgeIndex = i;
            bestPoint = currPoint;
            bestT = t;
            bestDistanceSqr = (currPoint-pt).sqr_magnitude();
        }
    }
    perpendicularPoint = bestPoint;
    t_res = bestT;
    return bestEdgeIndex;
}


std::vector<quint32> graph_alternative::PathThroughFloors () // TODO: add empty vector return on no path between floors (idk ?!?!)
{
    std::vector<quint32> res = {};
    bool pathFound = false;

    Q_ASSERT(startFloorIndex >= 0 && startFloorIndex < floors.size());
    Q_ASSERT(endFloorIndex >= 0 && endFloorIndex < floors.size());

    if (startFloorIndex == endFloorIndex)
    {
        res.push_back(startFloorIndex);
        return res;
    }

    floors.at(endFloorIndex).currPathFloorJumps = 0;

    while (!pathFound) // Dijkstra’s algorithm
    {
        for (auto &f : floors)
        {
            for (quint32 adjInd : f.adjacentFloors)
            {
                if (floors.at(adjInd).currPathFloorJumps > f.currPathFloorJumps + 1)
                {
                    floors.at(adjInd).currPathFloorJumps = f.currPathFloorJumps + 1;
                }
            }
        }
        if (floors.at(startFloorIndex).currPathFloorJumps != NO_PATH_INT)
        {
            pathFound = true;
        }
    }

    res.push_back(startFloorIndex);  // restoring path
    quint32 currFloor = startFloorIndex;
    while (currFloor != endFloorIndex)
    {

        for (quint32 adjInd : floors.at(currFloor).adjacentFloors)
        {
            if (floors.at(adjInd).currPathFloorJumps < floors.at(currFloor).currPathFloorJumps)
            {
                currFloor = adjInd;
                break;
            }
        }
        res.push_back(currFloor);
    }

    return res;
}



void mall_floor::ProceedPath(quint32 edgeToExclude) // TODO: N^2~N^3 to N*logN complexity
{
    for (auto &c : connections) // weights from floor connections
    {
        for (quint32 eInd : c.second.adjacentEdges)
        {
            if (edges.at(eInd).currPathLen > c.second.currPathLen)
            {
                edges.at(eInd).currPathLen = c.second.currPathLen;
            }
        }
    }

    for (quint32 i = 0; i < edges.size(); i++) // weights through edges
    {
        for (quint32 j = 0; j < edges.size(); j++)
        {
            if (edges.at(j).currPathLen == NO_PATH_DBL || j == edgeToExclude)
            {
                continue;
            }
            for (quint32 &neighbour : edges.at(j).adjacentEdges)
            {
                if (edges.at(j).currPathLen + edges.at(j).len < edges.at(neighbour).currPathLen)
                {
                    edges.at(neighbour).currPathLen = edges.at(j).currPathLen + edges.at(j).len;
                }
            }
        }
    }

    for (auto &c : connections) // weights to floor connections
    {
        for (quint32 eInd : c.second.adjacentEdges)
        {
            if (edges.at(eInd).currPathLen + edges.at(eInd).len < c.second.currPathLen)
            {
                c.second.currPathLen = edges.at(eInd).currPathLen + edges.at(eInd).len;
            }
        }
    }
}




void graph_alternative::ClearPath()
{
    ClearPathThroughFloors();
    ClearPathOnFloors();
    currentPath.clear();
}

void graph_alternative::ClearPathThroughFloors()
{
    for (auto &f : floors)
    {
        f.currPathFloorJumps = NO_PATH_INT;
    }
}

void graph_alternative::ClearPathOnFloors()
{
    for (auto &f : floors)
    {
        for (auto &e : f.edges)
        {
            e.currPathLen = NO_PATH_DBL;
        }
        for (auto &c : f.connections)
        {
           c.second.currPathLen = NO_PATH_DBL;
        }
    }
}

void path::clear()
{
    prePath.clear();
    defaultEdges.clear();
    postPath.clear();
    isEmpty = true;
}

bool path::empty() const
{
    return isEmpty;
}
