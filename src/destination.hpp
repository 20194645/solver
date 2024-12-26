#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#ifndef DESTINATIO_HPP
#define DESTINATIO_HPP

class Destionation
{
    public:
        int id;
        int earliness;
        int tardliness;
        int TWNode;
        
    Destionation() : id(0), earliness(0), tardliness(0) {}
};
#endif // AGV_HPP