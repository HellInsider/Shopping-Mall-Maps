#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>

struct vertex_graph
{
    std::pair<float, float> vertexCoordinates;     // x and y coordinates
    int vertexFloor;                               //
    std::vector<std::pair<int, float>> edgeWeight; // int - index of adjacent vertex, float - edge Weight
};

class graph
{
public:
    float CalculateWeight(std::pair<float, float> firstVertex, std::pair<float, float> secondVertex);
    void AddVertex(vertex_graph vertex);
    void DeleteVertex(int index);
    void AddTemporaryVertex(std::pair<float, float> coordinates);
    std::vector<int> SearchWay(int vertexStart, int vertexFinish);
private:
    std::vector<vertex_graph> adjacencyList;
    vertex_graph SearchVertex(std::pair<float, float> coordinates);
    int SearchVertexInAdjacenct(std::pair<float, float> coordinates);
    std::vector<std::vector<std::pair<int, float>>> CopyAdjacencyList();
};

#endif // GRAPH_H
