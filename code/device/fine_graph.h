#ifndef FINE_GRAPH_H
#define FINE_GRAPH_H

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

    void clear();
    bool isEmpty;
};


struct edge
{
    point start;
    point end;
    double len;
    QString svgId;

};

struct mulifloor_connection
{
    quint32 floorConnected;
    point otherEnding;
    double weight;

    mulifloor_connection (quint32 fC,    point oE,    double w) : floorConnected(fC), otherEnding(oE),  weight(w)
    {}
};


struct vertex
{
    point pos;
    bool isShowable;
    QString svgId;
    std::vector<quint32> edgesConnected;
    std::vector<mulifloor_connection> adjacentVertexesOnOtherFloors;

    vertex();

    double pathLen;
    double pathThroughFloorsLen;
};





struct mall_floor
{
    std::vector<edge> edges; // edges on this floor
    std::map<point, vertex> vertexes;


   // PATHING

    std::set<point> pointsToMap;


    //methods

    void ResetPathWeigts(void);

    std::set<point> GetFirstPoints (point const& pt, point& perpendicularPoint, bool setPathLen); // changes weigts

    bool ExtractClosestEdge (point& toSet);
    bool ProceedPath(std::vector<mall_floor>& allFloors); // return value is true = at least one iteration done

    point NextVertex(path &toSet, point const &currentVertex, std::vector<mall_floor>& allFloors, quint32 &floor);
};




class fine_graph : public QObject
{
    Q_OBJECT
public:
    //graph_alternative();
    explicit fine_graph(QObject *parent = nullptr);
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



    void ClearPath();
    void ClearGraph();


    friend fine_graph CreateGraphFromSvg (QString picFileName);
    friend class graph_parser;

    // those are really bad, althoug it guaratees that you cant change parser without checking in here
    friend void LoadGraphObjectsOfTheFloor (fine_graph* res, QDomNode& pathGroupHolder);
    friend void LoadEdge (fine_graph* g, quint32 floor, QDomElement& e);
    friend void LoadElevator (fine_graph* g, quint32 floor, QDomElement& e);
    friend void LoadLadder (fine_graph* g, quint32 floor, QDomElement& l);


};

void RoundCoord (point& p);

#ifndef GRAPH_H
//typedef graph_alternative graph;
#endif

#endif // FINE_GRAPH_H
