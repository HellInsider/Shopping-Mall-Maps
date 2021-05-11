#include "fine_graph.h"
#include <map>
#include <cassert>

constexpr double MAX_DBL = std::numeric_limits<double>::max();
constexpr quint32 MAX_INT = std::numeric_limits<quint32>::max();

constexpr double NO_PATH_DBL = MAX_DBL;
constexpr quint32 NO_PATH_INT = MAX_INT;



void RoundCoord (point& p)
{
    p.x = round(p.x / DELTA) * DELTA;
    p.y = round(p.y / DELTA) * DELTA;
}

void path::clear()
{
    prePath.clear();
    defaultEdges.clear();
    postPath.clear();
}


std::set<point> mall_floor::GetFirstPoints (point const& pt, point& perpendicularPoint, bool setPathLen = true)
{
    std::set<point> res;
    point bestPoint;
    double bestT = -1;
    double bestDistanceSqr = MAX_DBL;
    quint32 closestEdgeIndex = 0;

    vec currentEdgeVector;
    double t; // param, defining the perpendicular point position between start and end
              // t in [0;1]. t=0 => perpendicular at the start of the edge, t=1 => at the end
    point currPoint;

    for (quint32 i =0; i< edges.size(); i++)
    {
        currentEdgeVector = edges[i].end - edges[i].start;

        t = 1-(edges[i].end-pt).dot(currentEdgeVector)/currentEdgeVector.sqr_magnitude();
        currPoint = edges[i].start.lerp(edges[i].end, t);
        if((currPoint-pt).sqr_magnitude() < bestDistanceSqr)
        {
            if (t < 0 )
            {
                t = 0;
            }
            if (t > 1)
            {
                t = 1;
            }
            closestEdgeIndex = i;
            bestPoint = currPoint;
            bestT = t;
            bestDistanceSqr = (currPoint-pt).sqr_magnitude();
        }
    }
    RoundCoord(perpendicularPoint);
    if(setPathLen)
    {
        vertexes[edges[closestEdgeIndex].start].pathLen = edges[closestEdgeIndex].len*bestT;
        vertexes[edges[closestEdgeIndex].start].pathThroughFloorsLen = 0;
    }
    res.insert(edges[closestEdgeIndex].start);
    if(setPathLen)
    {
        vertexes[edges[closestEdgeIndex].end].pathLen = edges[closestEdgeIndex].len*(1-bestT);
        vertexes[edges[closestEdgeIndex].end].pathThroughFloorsLen = 0;
    }
    res.insert(edges[closestEdgeIndex].end);

    perpendicularPoint = bestPoint;
    return res;
}

void mall_floor::ResetPathWeigts()
{
    pointsToMap.clear();
    for (auto& v: vertexes)
    {
        v.second.pathLen = MAX_DBL;
        v.second.pathThroughFloorsLen = MAX_DBL;
    }
}


bool mall_floor::ExtractClosestEdge (point& toSet)
{
    double minPathLen = MAX_DBL;
    point firstPoint;
    bool res = false;

    for (auto p : pointsToMap)
    {
        if (vertexes[p].pathLen < minPathLen)
        {
            res = true;
            firstPoint = p;
            minPathLen = vertexes[p].pathLen;
        }
    }

    if (res)
    {
        assert(minPathLen != MAX_DBL);
        pointsToMap.erase(firstPoint);
        toSet = firstPoint;
    }
    return res;
}


bool mall_floor::ProceedPath(std::vector<mall_floor>& allFloors) // return value is true = at least one iteration done
{
    std::set<point> pointsMapped;

    bool res = false;


    while (pointsToMap.empty() == false)
    {
      point tmp;
      if (!ExtractClosestEdge(tmp))
      {
          assert(false);
          break;
      }
      vertex& next = vertexes[tmp];

      //on this floor
      for (auto e : next.edgesConnected)
      {
          assert (vertexes.find(edges[e].end)!=vertexes.end());
          assert (vertexes.find(edges[e].start)!=vertexes.end());

          vertex& neigbour = (edges[e].start == next.pos ? vertexes[edges[e].end] : vertexes[edges[e].start]);
          if (pointsMapped.find(neigbour.pos) == pointsMapped.end() &&
                  neigbour.pathThroughFloorsLen >= next.pathThroughFloorsLen  &&
                  neigbour.pathLen > next.pathLen + edges[e].len)
          {
              pointsToMap.insert(neigbour.pos);
              neigbour.pathLen = next.pathLen + edges[e].len;
              neigbour.pathThroughFloorsLen = next.pathThroughFloorsLen;
              res = true;
          }
      }

      //on other floors
      for (auto e : next.adjacentVertexesOnOtherFloors)
      {
          assert (allFloors[e.floorConnected].vertexes.find(e.otherEnding)!=allFloors[e.floorConnected].vertexes.end());

          vertex& neigbour = allFloors[e.floorConnected].vertexes[e.otherEnding];
          if (neigbour.pathThroughFloorsLen == next.pathThroughFloorsLen + e.weight &&
                 neigbour.pathLen > next.pathLen)
          {
              neigbour.pathLen = next.pathLen;
              allFloors[e.floorConnected].pointsToMap.insert(neigbour.pos);
              res = true;
          }
          else
          {
              if (neigbour.pathThroughFloorsLen > next.pathThroughFloorsLen + e.weight)
              {
                  neigbour.pathLen = next.pathLen;
                  neigbour.pathThroughFloorsLen = next.pathThroughFloorsLen + e.weight;
                  allFloors[e.floorConnected].pointsToMap.insert(neigbour.pos);
                  res = true;
              }
          }
      }
      pointsMapped.insert(next.pos);
   }

    return res;
}

point mall_floor::NextVertex(path &toSet, point const &currentVertex, std::vector<mall_floor>& allFloors, quint32 &floor)
{
    vertex& curr = vertexes[currentVertex];
    double currPathLen = curr.pathLen;
    point res = currentVertex;
    quint32 edgeToUse;
    bool onOtherFloor = false;
    for (auto n : curr.edgesConnected)
    {
        point p = edges[n].start == currentVertex ? edges[n].end : edges[n].start;
        if (vertexes[p].pathLen + edges[n].len < currPathLen + DELTA)
        {
            edgeToUse = n;
            //currPathLen = vertexes[p].pathLen;
            res = p;
            break;
        }
    }
    if (res == currentVertex)
    {
        currPathLen = curr.pathThroughFloorsLen;
        for (auto& n : curr.adjacentVertexesOnOtherFloors)
        {
            if (allFloors[n.floorConnected].vertexes[n.otherEnding].pathThroughFloorsLen + n.weight < currPathLen  + DELTA)
            {
                onOtherFloor = true;
                floor = n.floorConnected;
                res = n.otherEnding;
            }
        }
        assert(onOtherFloor);
    }
    if (!onOtherFloor)
    {
        toSet.defaultEdges.push_back(edges[edgeToUse].svgId);
        if (vertexes[res].isShowable && vertexes[res].svgId != "")
        {
            toSet.defaultEdges.push_back(vertexes[res].svgId);
        }
    }
    else
    {
        if (allFloors[floor].vertexes[res].isShowable && allFloors[floor].vertexes[res].svgId != "")
        {
            toSet.defaultEdges.push_back(allFloors[floor].vertexes[res].svgId);
        }
    }
    return res;
}

fine_graph::fine_graph(QObject *parent): QObject(parent)
{
}

void fine_graph::SetStart(point p, quint32 floorIndex)
{
        RoundCoord(p);

        if (floorIndex >= 0 && floorIndex < floors.size())
        {
            startFloorIndex = floorIndex ;
            start = /*point(80,80);//*/p;
        }
        else
        {
            startFloorIndex = 0; //floorIndex ;
            start = point(80,80);//p;
        }


}

void fine_graph::SetEnd(point p, quint32 floorIndex)
{
        RoundCoord(p);

        if (floorIndex >= 0 && floorIndex < floors.size())
        {
            endFloorIndex = /* 2;//*/ floorIndex;
            end = /*point(45,67);//*/ p;
        }
        else
        {
            endFloorIndex =  2;// floorIndex;
            end = point(45,67);// p;
        }
}

void fine_graph::FindPath()
{
    ClearPath();

    point endingPoint;
    bool continueFlag = true;
    std::set<point> endCandidates = floors[endFloorIndex].GetFirstPoints(end, endingPoint);
    floors[endFloorIndex].pointsToMap = endCandidates;


    while (continueFlag)
    {
        continueFlag = false;
        for (auto& f: floors)
        {
            continueFlag |= f.ProceedPath(floors);
        }
    }

    double minimumLen = MAX_DBL;
    point startingPoint;
    point firstVertexPos;
    std::set<point> startCandidates =  floors[startFloorIndex].GetFirstPoints(start, startingPoint, false);
    for (auto p : startCandidates)
    {
        if(floors[startFloorIndex].vertexes[p].pathLen < minimumLen)
        {
            minimumLen = floors[startFloorIndex].vertexes[p].pathLen;
            firstVertexPos = p;
        }
    }

    currentPath.startFloorIndex = startFloorIndex;
    currentPath.endFloorIndex = endFloorIndex;

    currentPath.prePath.push_back(start);
    if(start != startingPoint)
    {
        currentPath.prePath.push_back(startingPoint);
    }

    if(startingPoint != endingPoint || startFloorIndex != endFloorIndex)
    {
        currentPath.postPath.push_back(endingPoint);
    }
    if(end != endingPoint)
    {
        currentPath.postPath.push_back(end);
    }

    if (startCandidates == endCandidates && startFloorIndex == endFloorIndex)
    {
        emit PathFound(&currentPath);
        return;
    }

    if (floors[startFloorIndex].vertexes[firstVertexPos].isShowable && floors[startFloorIndex].vertexes[firstVertexPos].svgId != "")
    {
        currentPath.defaultEdges.push_back(floors[startFloorIndex].vertexes[firstVertexPos].svgId);
    }
    else
    {
        if (firstVertexPos != startingPoint)
        {
            currentPath.prePath.push_back(firstVertexPos);
        }
    }

    quint32 currentFloor = startFloorIndex;
    point currentPos = firstVertexPos;
    while (currentFloor != endFloorIndex || endCandidates.find(currentPos) == endCandidates.end())
    {
        currentPos = floors[currentFloor].NextVertex(currentPath, currentPos, floors, currentFloor);
    }

    if (!(floors[endFloorIndex].vertexes[currentPos].isShowable && floors[endFloorIndex].vertexes[currentPos].svgId != ""))
    {
        if (currentPos != endingPoint)
        {
            currentPath.postPath.push_front(currentPos);
        }
    }
    emit PathFound(&currentPath);
}

void fine_graph::ClearPath()
{
    for (auto& f : floors)
    {
        f.ResetPathWeigts();
    }
    currentPath.clear();
}

void fine_graph::ClearGraph()
{
    floors.clear();
    ClearPath();
}

vertex::vertex(): pos(), isShowable(false), svgId (""), edgesConnected(),adjacentVertexesOnOtherFloors (), pathLen(MAX_DBL), pathThroughFloorsLen (MAX_DBL)
{

}


//bool edge::CrossesOnStart(const edge &neigbour) const
//{
//    if (neigbour.start == start || neigbour.end == start)
//    {
//        return true;
//    }
//    return false;
//}



//void graph_alternative::MapOnEnd(mall_floor& endFloor, quint32 lastEdgeIndex, double t)
//{
//    edge& lastEdge = endFloor.edges.at(lastEdgeIndex);
//    for (quint32 neighborIndex : lastEdge.adjacentEdges)
//    {
//        if (lastEdge.CrossesOnStart(endFloor.edges.at(neighborIndex)))
//        {
//            endFloor.edges.at(neighborIndex).currPathLen = lastEdge.len * t;
//        }
//        else
//        {
//            endFloor.edges.at(neighborIndex).currPathLen = lastEdge.len * (1-t);
//        }
//    }
//    endFloor.ProceedPath(lastEdgeIndex); // done Dijkstra’s algorithm mapping

//    for (auto &c : endFloor.connections)
//    {
//        if(c.second.adjacentEdges.find(lastEdgeIndex) != c.second.adjacentEdges.end())
//        {
//            c.second.currPathLen = (c.second.pos == lastEdge.start ? t : (1-t)) * lastEdge.len;
//        }
//    }
//}


//void graph_alternative::TransmitPathLength(quint32 floorFromIndex, quint32 floorToIndex)
//{
//    for (auto &c : floors.at(floorFromIndex).connections)
//    {
//        if (c.second.floorsConnected.find(floorToIndex) != c.second.floorsConnected.end())
//        {
//            Q_ASSERT(floors.at(floorToIndex).connections.find(c.first) != floors.at(floorToIndex).connections.end());
//            floors.at(floorToIndex).connections[c.first].currPathLen = c.second.currPathLen;
//        }
//    }
//}



//void graph_alternative::FindPath(void)
//{
//    if (!currentPath.empty()) // path already exists
//    {
//        emit PathFound(&currentPath);
//        return;
//    }

//    currentPath.startFloorIndex = startFloorIndex;
//    currentPath.endFloorIndex = endFloorIndex;


//    mall_floor& startFloor = floors.at(startFloorIndex);
//    mall_floor& endFloor = floors.at(endFloorIndex);

//    point startPerp,
//            endPerp;

//    double startEdgeSplit,
//            endEdgeSplit;

//    quint32 firstEdgeIndex = startFloor.NearestEdge(start, startPerp, startEdgeSplit);
//    quint32 lastEdgeIndex = endFloor.NearestEdge(end, endPerp, endEdgeSplit);




//   if (firstEdgeIndex == lastEdgeIndex) // path takes only temporary points
//   {
//       currentPath.prePath.push_back(start);
//       currentPath.prePath.push_back(startPerp);
//       currentPath.prePath.push_back(endPerp);
//       currentPath.prePath.push_back(end);
//       emit PathFound(&currentPath);
//       return;
//   }

//    edge& firstEdge = startFloor.edges.at(firstEdgeIndex),
//          lastEdge= endFloor.edges.at(lastEdgeIndex);


//    lastEdge.currPathLen = 0;
//    for (quint32& neighborIndex : lastEdge.adjacentEdges) // choosing the best one to start with
//    {
//        if (lastEdge.CrossesOnStart(endFloor.edges.at(neighborIndex)))
//        {
//            endFloor.edges.at(neighborIndex).currPathLen = lastEdge.len * startEdgeSplit;
//        }
//        else
//        {
//            endFloor.edges.at(neighborIndex).currPathLen = lastEdge.len * (1-startEdgeSplit);
//        }
//    }

//    endFloor.ProceedPath(NO_PATH_INT);
//    std::vector<quint32> floorsInPath = PathThroughFloors();

//    if (startFloorIndex != endFloorIndex) // path takes more than one floor
//    {
//        Q_ASSERT(!floorsInPath.empty());

//        for (quint32 f = floorsInPath.size() - 1; f>0; f--)
//        {
//            TransmitPathLength(floorsInPath.at(f), floorsInPath.at(f-1));
//            floors.at(floorsInPath.at(f-1)).ProceedPath(MAX_INT);
//        }
//    }

//    currentPath.prePath.push_back(start);
//    currentPath.prePath.push_back(startPerp);
//    endFloor.edges[lastEdgeIndex].currPathLen = 0; // kostyl TM
//    RecoverPath(firstEdgeIndex, startEdgeSplit, lastEdgeIndex, floorsInPath);
//    currentPath.postPath.push_back(endPerp);
//    currentPath.postPath.push_back(end);

//    emit PathFound(&currentPath);
//    return;
//}


//quint32 mall_floor::GetNextEdgeInPath (quint32 currEdgeInd, bool& isEdge) const
//{
//    edge const & e = edges.at(currEdgeInd);
//    double bestWay = e.currPathLen;
//    quint32 res = MAX_INT;
//    isEdge = true;

//    for (auto ae : e.adjacentEdges)
//    {
//        if (edges.at(ae).currPathLen < bestWay)
//        {
//            bestWay = edges.at(ae).currPathLen;
//            res = ae;
//        }
//    }

//    for (auto & c : connections)
//    {
//        if (c.second.adjacentEdges.find(currEdgeInd) != c.second.adjacentEdges.end() &&
//                c.second.currPathLen < bestWay)
//        {
//            isEdge = false;
//            bestWay = c.second.currPathLen;
//            res = c.first;
//        }
//    }
//    return res;
//}

//quint32 graph_alternative::RecoverOnStart(mall_floor const & startFloor, quint32 firstEdgeIndex, double t, bool& edgeFlag) const
//{
//    edge const &firstEdge = startFloor.edges.at(firstEdgeIndex);
//    quint32 currentEdge = MAX_INT;
//    quint32 currentConnection = MAX_INT;

//    double weight = MAX_DBL;//firstEdge.currPathLen + firstEdge.len;

//    for (quint32 ae : firstEdge.adjacentEdges)
//    {
//        //double ghfghgf = startFloor.edges.at(ae).currPathLen;
//        //double fgdfgdf = (firstEdge.CrossesOnStart(startFloor.edges.at(ae)) ? t : (1-t)) * firstEdge.len;
//        //double asdasdas = startFloor.edges.at(ae).currPathLen + (firstEdge.CrossesOnStart(startFloor.edges.at(ae)) ? t : (1-t)) * firstEdge.len;
//        if (startFloor.edges[ae].currPathLen + // path to the neighbour
//                (firstEdge.CrossesOnStart(startFloor.edges.at(ae)) ?
//                t : (1-t)) * firstEdge.len // path to the perpendicular point
//                < weight)
//        {
//            weight = startFloor.edges.at(ae).currPathLen +
//                    (firstEdge.CrossesOnStart(startFloor.edges.at(ae)) ? t : (1-t)) * firstEdge.len;
//            currentEdge = ae;
//        }
//    }

//    for (auto& c : startFloor.connections)
//    {
//        if(c.second.adjacentEdges.find(firstEdgeIndex) != c.second.adjacentEdges.end() &&
//                    c.second.currPathLen + (c.second.pos == firstEdge.start ? t : (1-t)) * firstEdge.len < weight)
//        {
//            weight = c.second.currPathLen + (c.second.pos == firstEdge.start ? t : (1-t)) * firstEdge.len;
//            currentConnection = c.first;
//            currentEdge = MAX_INT;
//        }
//    }

////    if ( t == 1 || t == 0)
////    {
////        currentPath.defaultEdges << firstEdge.svgId;
////    }

//    if (currentEdge == MAX_INT)
//    {
//        edgeFlag = false;
//    }
//    edgeFlag = true;

//    return currentEdge;

//}




//void graph_alternative::RecoverPath(quint32 firstEdgeIndex, double t, quint32 lastEdgeIndex, std::vector<quint32>& floorsSequence)
//{
//    bool keepTheFloor;
//    quint32 currentFloorIndex = 0;
//    mall_floor const * currentFloor = &(floors.at(floorsSequence.at(currentFloorIndex)));
//    quint32 currentEdge = MAX_INT;


//    currentEdge = RecoverOnStart(*currentFloor, firstEdgeIndex, t, keepTheFloor);

//    if(keepTheFloor)
//    {
//        currentPath.prePath.push_back(currentFloor->edges.at(firstEdgeIndex).CrossesOnStart(currentFloor->edges.at(currentEdge)) ?
//                                       currentFloor->edges.at(firstEdgeIndex).start : currentFloor->edges.at(firstEdgeIndex).end);
//    }
//    else
//    {
//        currentPath.prePath.push_back(currentFloor->connections.at(currentEdge).pos);
//    }

//    quint32 nextEdge = currentEdge;

//    while (currentEdge != MAX_INT)
//    {
//         currentEdge = nextEdge;
//         if (keepTheFloor)
//         {
//            currentPath.defaultEdges << currentFloor->edges.at(currentEdge).svgId;
//            nextEdge = currentFloor->GetNextEdgeInPath(currentEdge, keepTheFloor);
//            if (nextEdge == lastEdgeIndex && floorsSequence[currentFloorIndex] == floorsSequence.back())
//            {
//                currentPath.postPath.push_back(
//                            currentFloor->edges.at(lastEdgeIndex).CrossesOnStart(currentFloor->edges.at(currentEdge)) ?
//                                 currentFloor->edges.at(lastEdgeIndex).start :  currentFloor->edges.at(lastEdgeIndex).end);
//                break;
//            }
//         }
//         else
//         {
//             currentPath.defaultEdges << currentFloor->connections.at(currentEdge).svgId;
//             currentFloorIndex ++;
//             currentFloor = &(floors.at(floorsSequence.at(currentFloorIndex)));
//             currentPath.defaultEdges << currentFloor->connections.at(currentEdge).svgId;
//             double weight = currentFloor->connections.at(currentEdge).currPathLen;
//             for (quint32 ae : currentFloor->connections.at(currentEdge).adjacentEdges)
//             {
//                 if (currentFloor->edges.at(ae).currPathLen < weight)
//                 {
//                     weight = currentFloor->edges.at(ae).currPathLen;
//                     nextEdge = ae;
//                     keepTheFloor = true;
//                 }
//             }

//         }
//    }

//}



//std::vector<quint32> graph_alternative::PathThroughFloors () // TODO: add empty vector return on no path between floors (idk ?!?!)
//{
//    std::vector<quint32> res = {};
//    bool pathFound = false;

//    Q_ASSERT(startFloorIndex >= 0 && startFloorIndex < floors.size());
//    Q_ASSERT(endFloorIndex >= 0 && endFloorIndex < floors.size());

//    if (startFloorIndex == endFloorIndex)
//    {
//        res.push_back(startFloorIndex);
//        return res;
//    }

//    floors.at(endFloorIndex).currPathFloorJumps = 0;

//    while (!pathFound) // Dijkstra’s algorithm
//    {
//        for (auto &f : floors)
//        {
//            for (quint32 adjInd : f.adjacentFloors)
//            {
//                if (floors.at(adjInd).currPathFloorJumps > f.currPathFloorJumps + 1)
//                {
//                    floors.at(adjInd).currPathFloorJumps = f.currPathFloorJumps + 1;
//                }
//            }
//        }
//        if (floors.at(startFloorIndex).currPathFloorJumps != NO_PATH_INT)
//        {
//            pathFound = true;
//        }
//    }

//    res.push_back(startFloorIndex);  // restoring path
//    quint32 currFloor = startFloorIndex;
//    while (currFloor != endFloorIndex)
//    {

//        for (quint32 adjInd : floors.at(currFloor).adjacentFloors)
//        {
//            if (floors.at(adjInd).currPathFloorJumps < floors.at(currFloor).currPathFloorJumps)
//            {
//                currFloor = adjInd;
//                break;
//            }
//        }
//        res.push_back(currFloor);
//    }

//    return res;
//}



//void mall_floor::ProceedPath(quint32 edgeToExclude) // TODO: N^2~N^3 to N*logN complexity
//{
//    for (auto &c : connections) // weights from floor connections
//    {
//        for (quint32 eInd : c.second.adjacentEdges)
//        {
//            if (edges.at(eInd).currPathLen > c.second.currPathLen)
//            {
//                edges.at(eInd).currPathLen = c.second.currPathLen;
//            }
//        }
//    }

//    for (quint32 i = 0; i < edges.size(); i++) // weights through edges
//    {
//        for (quint32 j = 0; j < edges.size(); j++)
//        {
//            if (edges.at(j).currPathLen == NO_PATH_DBL || j == edgeToExclude)
//            {
//                continue;
//            }
//            for (quint32 &neighbour : edges.at(j).adjacentEdges)
//            {
//                if (edges.at(j).currPathLen + edges.at(j).len < edges.at(neighbour).currPathLen)
//                {
//                    edges.at(neighbour).currPathLen = edges.at(j).currPathLen + edges.at(j).len;
//                }
//            }
//        }
//    }

//    for (auto &c : connections) // weights to floor connections
//    {
//        for (quint32 eInd : c.second.adjacentEdges)
//        {
//            if (edges.at(eInd).currPathLen + edges.at(eInd).len < c.second.currPathLen)
//            {
//                c.second.currPathLen = edges.at(eInd).currPathLen + edges.at(eInd).len;
//            }
//        }
//    }
//}




//void graph_alternative::ClearPath()
//{
//    ClearPathThroughFloors();
//    ClearPathOnFloors();
//    currentPath.clear();
//}

//void graph_alternative::ClearPathThroughFloors()
//{
//    for (auto &f : floors)
//    {
//        f.currPathFloorJumps = NO_PATH_INT;
//    }
//}

//void graph_alternative::ClearPathOnFloors()
//{
//    for (auto &f : floors)
//    {
//        for (auto &e : f.edges)
//        {
//            e.currPathLen = NO_PATH_DBL;
//        }
//        for (auto &c : f.connections)
//        {
//           c.second.currPathLen = NO_PATH_DBL;
//        }
//    }
//}

//void path::clear()
//{
//    prePath.clear();
//    defaultEdges.clear();
//    postPath.clear();
//    isEmpty = true;
//}


//vertex* vertex::GetBestNeighbour(quint32& edgeIndex, quint32 &floorIndex, bool& wasFloorChanged)
//{
//    double bestPath = this->pathLen;
//    vertex* res = this;
//    for (auto& connectionOnFloor : edgesConnected)
//    {
//        if (connectionOnFloor.otherEnding->pathLen < bestPath)
//        {
//            edgeIndex = connectionOnFloor.index;
//            res = connectionOnFloor.otherEnding;
//            bestPath = res->pathLen;
//        }
//    }
//    if (res == this)
//    {
//        bestPath = this->pathThroughFloorsLen;
//        for (auto& connectionOnOtherFloor : adjacentVertexesOnOtherFloors)
//        {
//            if (connectionOnOtherFloor.otherEnding->pathThroughFloorsLen < bestPath)
//            {
//                wasFloorChanged = true;
//                res = connectionOnOtherFloor.otherEnding;
//                floorIndex = connectionOnOtherFloor.floorConnected;
//                bestPath = res->pathThroughFloorsLen;
//            }
//        }
//    }
//    Q_ASSERT(res != this);
//    return res;
//}



