#ifndef GRAPH_H
#define GRAPH_H


#include <vector>



struct vec2
{
    float x; // x coordinate
    float y; // y coordinates
};

struct edge
{
    int vertex_ID; // index of adjacent vertex
    float weight;  // edge Weight
};

struct vertex_graph
{
    vec2 vertexCoordinates;
    int vertexFloor;
    std::vector<edge> weightedEdges;
};

class graph
{
public:
    void DeleteVertex(int index);
    void AddTemporaryVertex(vec2 coordinates);
    std::vector<vertex_graph> SearchWay(int vertexStart, int vertexFinish);
    std::vector<vertex_graph> adjacencyList;
private:
    std::vector<edge> CopyDataweightedEdges(vec2 newVertex, edge adjVert, int h);
    void AddVertex(vertex_graph vertex);
    std::vector<float> CreateLineEquation(vertex_graph vert, int index);
    std::vector<float> CreateNormalEquation(vertex_graph vert, int index, vec2 coordinates);
    vec2 MethodKramer(const std::vector<float> & firstEquation, const std::vector<float> & secondEquation);
    float CalculateWeight(vec2 firstVertex, vec2 secondVertex); // calculates the weight of the edge
};

#endif // GRAPH_H
