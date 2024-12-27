#include "reader.hpp"
#include "SCIPsovler.hpp"

int main(int argc, char* argv[]){
    string filepath;
    if (argc == 1)
    {
        filepath = "/mnt/d/DATN/solver/prj/data/general.txt";
    }
    
    else{
        filepath = "/mnt/d/DATN/pathPlanningSimulation/TSG.txt";
        
    }
    Reader reader(filepath);
    Coef coef;
    if (argc == 1){
        coef = reader.set_coef();
    }
    else{
        coef = reader.set_coef_directed();
    }
    Solver sol(coef);
    // sol.set_end_queue();
    SCIP_CALL(sol.Solve());
    SCIP_CALL(sol.Result());
    SCIP_CALL(sol.Free_resources());
    
    return 0;
}