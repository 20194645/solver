#ifndef COEFFICIENT_HPP
#define COEFFICIENT_HPP
#include "destination.hpp"
#include "AGV.hpp"
#include "Edge.hpp"

using namespace std;

typedef struct coefficient {
    int alpha;
    int beta;
    int gamma;
    int N;
    int max;
    vector<AGV> AGVs;
    vector<Destionation> end_nodes;
    vector<pair<vector<Edge>, int>> restriction;
    map<int, vector<Edge>> invertex;
    map<int, vector<Edge>> outvertex;
} Coef;

#endif // COEFFICIENT_HPP
