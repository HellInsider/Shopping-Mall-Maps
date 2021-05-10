#ifndef GRAPH_ALTERNATIVE_H
#define GRAPH_ALTERNATIVE_H

#include <QObject>
#include "Vector2.hpp"
#include <set>

#define DELTA 1e-4 // maximum difference between edge endings to count them as one point

class QDomNode;
class QDomElement;

using point = Vector2<double>;
using vec = Vector2<double>;

struct path
{
    quint32 startFloorIndex;
    std::list<point> prePath;

    QStringList defaultEdges;

    quint32 endFloorIndex;
    std::list<point> postPath;


    bool empty() const;

private:
    friend class graph_alternative;
    void clear();
    bool isEmpty;
};


struct edge
{
    // coordinates within single floor
    point start;  // start.x <= end.x
    point end;  // if start.x == end.x then start.y <= end.y
    double len;

    std::vector<quint32> adjacentEdges; // other edges on this floor

    QString svgId; // edge name as svg object


    double currPathLen; // used for path finding


    //methods
    bool CrossesOnStart(edge const & neigbour) const;
};


struct floor_connection
{
    std::set<quint32> adjacentEdges; // endexes of the edges, connected to this elevator on current floor
    std::set<quint32> floorsConnected; // indexes of other floors connected via this elevator
    point pos;

    QString svgId; // elevator as svg object

    double currPathLen; // used for path finding
};


struct mall_floor
{
    std::vector<edge> edges; // edges on this floor

    std::vector<quint32> adjacentFloors; // floors which are connected to this one
    std::map<quint32, floor_connection> connections; // quint32 = elevator index


    quint32 currPathFloorJumps; // used for path finding


    //methods

    quint32 NearestEdge (point pt, point& perpendicularPoint, double& t) const;
    // t in [0;1]. t=0 => perpendicular at the start of the edge, t=1 => at the end
    void ProceedPath(quint32 edgeToExclude);
    quint32 GetNextEdgeInPath (quint32 currEdgeInd, bool& isEdge) const;
};




class graph_alternative : public QObject
{
    Q_OBJECT
public:
    //graph_alternative();
    explicit graph_alternative(QObject *parent = nullptr);
    //virtual ~graph_alternative() = default; // in order to fix graph_parser.cpp:300:9: warning: delete called on non-final 'graph_alternative' that has virtual functions but non-virtual destructor

signals:
    void PathFound (path* res);

public slots:
    void SetStart(point p, quint32 floorIndex);
    void SetEnd(point p, quint32 floorIndex);
    void FindPath (void);


private:
    // mall path graph data
    std::vector<mall_floor> floors;


    // pathing params
    point start;
    quint32 startFloorIndex;

    point end;
    quint32 endFloorIndex;

    path currentPath;

    // inner functions
    std::vector<quint32> PathThroughFloors ();
    void ClearPathThroughFloors(); // clears path through floors: elevators and ladder usage

    void MapOnEnd(mall_floor& endFloor, quint32 lastEdgeIndex, double t);
    void ClearPathOnFloors();   // clears path, calculated on every floor
    void TransmitPathLength(quint32 floorFromIndex, quint32 floorToIndex);

    quint32 RecoverOnStart(mall_floor const & startFloor, quint32 firstEdgeIndex, double t, bool& edgeFlag) const;
    void RecoverPath(quint32 firstEdgeIndex, double t, quint32 lastEdgeIndex, std::vector<quint32>& floorsSequence);
    void ClearPath();
    void ClearGraph();


    friend graph_alternative CreateGraphFromSvg (QString picFileName);
    friend class graph_parser;

    // those are really bad, althoug it guaratees that you cant change parser without cheking in here
    friend void LoadGraphObjectsOfTheFloor (graph_alternative* res, QDomNode& pathGroupHolder);
    friend void LoadEdge (graph_alternative* g, quint32 floor, QDomElement& e);
    friend void LoadElevator (graph_alternative* g, quint32 floor, QDomElement& e);
    friend void LoadLadder (graph_alternative* g, quint32 floor, QDomElement& l);


};

#ifndef GRAPH_H
//typedef graph_alternative graph;
#endif

#endif // GRAPH_ALTERNATIVE_H
