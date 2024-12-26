#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#ifndef AGV_HPP
#define AGV_HPP
class AGV
{
    public:
        int id;
        int start_node;
        
    AGV() : id(0), start_node(0) {}
};
#endif // AGV_HPP