#include "device/graph.h"
#include <math.h>
#include <algorithm>


float graph::CalculateWeight(vec2 firstVertex, vec2 secondVertex)
{
    return sqrt((firstVertex.x - secondVertex.x) * (firstVertex.x - secondVertex.x) +
             (firstVertex.y - secondVertex.y) * (firstVertex.y - secondVertex.y));
}

void graph::AddVertex(vertex_graph vertex)
{
    adjacencyList.push_back(vertex);
    for(auto vert : vertex.weightedEdges)
    {
        adjacencyList[vert.vertex_ID].weightedEdges.push_back({(long int)adjacencyList.size() - 1, vert.weight});
    }
}

void graph::DeleteVertex(int index)
{
    for(auto vert : adjacencyList)
    {
        auto edgeForDeleting = std::find_if(vert.weightedEdges.begin(), vert.weightedEdges.end(), [&](edge adj)
        {
            return adj.vertex_ID == index;
        });

        if (edgeForDeleting != vert.weightedEdges.end())
        {
            vert.weightedEdges.erase(edgeForDeleting);
        }
    }

    adjacencyList.erase(adjacencyList.begin() + index);
}

vec2 graph::MethodKramer(const std::vector<float> & firstEquation, const std::vector<float> & secondEquation)
{
    vec2 newCoordinate;
    newCoordinate.x = (firstEquation[2] * secondEquation[1] - firstEquation[1] * secondEquation[2]) /
                      (firstEquation[0] * secondEquation[1] - firstEquation[1] * secondEquation[0]);
    newCoordinate.y = (firstEquation[0] * secondEquation[2] - firstEquation[2] * secondEquation[0]) /
                      (firstEquation[0] * secondEquation[1] - firstEquation[1] * secondEquation[0]);

    return newCoordinate;
}

std::vector<float> graph::CreateLineEquation(vertex_graph vert, int index)
{
    std::vector<float> lineEquation;
    lineEquation.push_back(-vert.vertexCoordinates.y + adjacencyList[index].vertexCoordinates.y);
    lineEquation.push_back(-adjacencyList[index].vertexCoordinates.x + vert.vertexCoordinates.x);
    lineEquation.push_back(vert.vertexCoordinates.x * adjacencyList[index].vertexCoordinates.y -
                           adjacencyList[index].vertexCoordinates.x * vert.vertexCoordinates.y);

    return lineEquation;
}

std::vector<float> graph::CreateNormalEquation(vertex_graph vert, int index, vec2 coordinates)
{
    std::vector<float> normalEquation;
    normalEquation.push_back(adjacencyList[index].vertexCoordinates.x - vert.vertexCoordinates.x);
    normalEquation.push_back(vert.vertexCoordinates.y - adjacencyList[index].vertexCoordinates.y);
    normalEquation.push_back(-vert.vertexCoordinates.x * coordinates.x + adjacencyList[index].vertexCoordinates.x * coordinates.x +
                             coordinates.y * vert.vertexCoordinates.y - adjacencyList[index].vertexCoordinates.y * coordinates.y);

    return normalEquation;
}

std::vector<edge> graph::CopyDataweightedEdges(vec2 newVertex, edge adjVert, int h)
{
    std::vector<edge> weightedEdges;
    weightedEdges.push_back({h,CalculateWeight(adjacencyList[h].vertexCoordinates, newVertex)});
    weightedEdges.push_back({adjVert.vertex_ID,CalculateWeight(adjacencyList[adjVert.vertex_ID].vertexCoordinates,newVertex)});

    return weightedEdges;
}

void graph::AddTemporaryVertex(vec2 coordinates)
{
    vec2 newCoordinate;
    vertex_graph newVertex;
    float weight = 0;
    int index = 0;
    float minimalWeight = CalculateWeight(MethodKramer(CreateLineEquation(adjacencyList[0], adjacencyList[0].weightedEdges[0].vertex_ID),
                          CreateNormalEquation(adjacencyList[0], adjacencyList[0].weightedEdges[0].vertex_ID, coordinates)), coordinates);
    for(auto & vert : adjacencyList)
    {
        for(auto & adjVert : vert.weightedEdges)
        {
            if(index < adjVert.vertex_ID &&
               std::max(vert.vertexCoordinates.x, adjacencyList[adjVert.vertex_ID].vertexCoordinates.x) >= coordinates.x &&
               std::min(vert.vertexCoordinates.x, adjacencyList[adjVert.vertex_ID].vertexCoordinates.x) <= coordinates.x)
            {
                newCoordinate = MethodKramer(CreateLineEquation(vert, adjVert.vertex_ID),
                                             CreateNormalEquation(vert, adjVert.vertex_ID, coordinates));
                weight = CalculateWeight(newCoordinate, coordinates);

                if(minimalWeight > weight)
                {
                    minimalWeight = weight;
                    newVertex.vertexCoordinates = newCoordinate;
                    newVertex.weightedEdges = CopyDataweightedEdges(newCoordinate, adjVert, index);
                    newVertex.vertexFloor = vert.vertexFloor;
                }
            }
        }

        index++;
    }
    AddVertex(newVertex);
    AddVertex({{coordinates}, newVertex.vertexFloor, {{static_cast<long int>(adjacencyList.size()) - 1,CalculateWeight(coordinates, newVertex.vertexCoordinates)}}});
}

std::vector<vertex_graph> graph::SearchWay(int vertexStart, int vertexFinish)
{
    const int INF = INT_MAX;
    std::vector<vertex_graph> path;
    size_t graphSize = adjacencyList.size();
    std::vector<int> minimalWay(graphSize, INF);
    std::vector<int> temporatyValue(graphSize);
    minimalWay[vertexStart] = 0;
    std::vector<bool> isVisited(graphSize);
    for (size_t i = 0; i < graphSize; ++i)
    {
        int unattainableValue = -1;
        for (size_t j = 0; j < graphSize; ++j)
        if (!isVisited[j] && (unattainableValue == -1 || minimalWay[j] < minimalWay[unattainableValue]))
        {
            unattainableValue = j;
        }

        if (minimalWay[unattainableValue] == INF)
        {
            break;
        }

        isVisited[unattainableValue] = true;

        for (size_t j = 0; j < adjacencyList[unattainableValue].weightedEdges.size(); ++j)
        {
            int to = adjacencyList[unattainableValue].weightedEdges[j].vertex_ID,
            weight = adjacencyList[unattainableValue].weightedEdges[j].weight;
            if (minimalWay[unattainableValue] + weight < minimalWay[to])
            {
                minimalWay[to] = minimalWay[unattainableValue] + weight;
                temporatyValue[to] = unattainableValue;
            }
        }
    }

    for (int i = vertexFinish; i != vertexStart; i = temporatyValue[i])
    {
        path.push_back(adjacencyList[i]);
    }

    path.push_back(adjacencyList[vertexStart]);
    reverse(path.begin(), path.end());

    return path;
}
