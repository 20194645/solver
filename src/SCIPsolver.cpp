#include "SCIPsovler.hpp"

Solver::Solver(Coef problem) : Problem(problem) {};

SCIP_RETCODE Solver::mainproblem()
{

    for (auto agv : this->Problem.AGVs)
    {
        for (auto pair : this->Problem.outvertex)
        {
            int key = pair.first;
            vector<Edge> v = pair.second;
            for (auto e : v)
            {
                SCIP_VAR *var = nullptr;
                string varname = "x_" + to_string(agv.id) + "_" + to_string(e.start_node) + "_" + to_string(e.end_node);
                if (varmap.find(varname) == varmap.end())
                {
                    SCIP_CALL(SCIPcreateVarBasic(this->scip,
                                                 &var,
                                                 varname.c_str(),
                                                 0.0,
                                                 1.0,
                                                 this->Problem.alpha * e.weight,
                                                 SCIP_VARTYPE_BINARY));
                    SCIP_CALL(SCIPaddVar(this->scip, var));
                    this->varmap[varname] = var; // Lưu lại biến vào map
                }
            }
        }
    }

    // Time_Window
    map<int, set<int>> destination;
    for (auto pair : this->Problem.invertex)
    {
        for (auto end_node : this->Problem.end_nodes)
        {
            if (end_node.id % this->Problem.N == pair.first % this->Problem.N)
            {
                destination[end_node.id].insert(pair.first);
            }
        }
    }
    for (auto agv : this->Problem.AGVs)
    {
        for (auto end_node : this->Problem.end_nodes)
        {
            if (agv.start_node % this->Problem.N == end_node.id % this->Problem.N)
            {
                destination[end_node.id].insert(agv.start_node);
            }
        }
    }
    for (auto agv : this->Problem.AGVs)
    {
        for (int i = 0; i < this->Problem.end_nodes.size(); i++)
        {
            for (auto Possible_end_node : destination[this->Problem.end_nodes[i].id])
            {
                int totaltime;
                if (Possible_end_node % this->Problem.N == 0)
                {
                    totaltime = (Possible_end_node / this->Problem.N) - 1;
                }
                else
                {
                    totaltime = (Possible_end_node / this->Problem.N);
                }
                int weight = max(this->Problem.end_nodes[i].earliness - totaltime, 0);
                weight = max(totaltime - this->Problem.end_nodes[i].tardliness, weight);
                SCIP_VAR *var = nullptr;
                string varname = "y" + to_string(i) + "_" + to_string(agv.id) + "_" + to_string(Possible_end_node);
                SCIP_CALL(SCIPcreateVarBasic(this->scip,
                                             &var,
                                             varname.c_str(),
                                             0.0,
                                             1.0,
                                             this->Problem.beta * weight,
                                             SCIP_VARTYPE_BINARY));
                SCIP_CALL(SCIPaddVar(this->scip, var));
                this->varmap[varname] = var;
            }
        }
    }

    // Restriction
    for (int i = 0; i < this->Problem.restriction.size(); ++i)
    {
        auto pair = this->Problem.restriction[i];
        SCIP_VAR *var = nullptr;
        string varname = "z_" + to_string(i);
        if (this->varmap.find(varname) == this->varmap.end())
        {
            SCIP_CALL(SCIPcreateVarBasic(this->scip,
                                         &var,
                                         varname.c_str(),
                                         0.0,
                                         SCIPinfinity(scip),
                                         this->Problem.gamma,
                                         SCIP_VARTYPE_CONTINUOUS));
            SCIP_CALL(SCIPaddVar(this->scip, var));
            this->varmap[varname] = var;
        }
    }

    return SCIP_OKAY;
}

SCIP_RETCODE Solver::Constraint1()
{

    for (auto pair : this->Problem.outvertex)
    {
        int node = pair.first;
        // Lặp qua các cạnh xuất phát từ đỉnh này
        for (auto e : pair.second)
        {
            SCIP_CONS *cons = nullptr;

            // Tạo ràng buộc tuyến tính cơ bản cho đỉnh node
            SCIP_CALL(SCIPcreateConsBasicLinear(this->scip,
                                                &cons,
                                                ("Max_flow_at_" + to_string(node)).c_str(),
                                                0,
                                                nullptr,
                                                nullptr,
                                                0.0,
                                                1.0));
            // Thêm biến dòng chảy cho mỗi AGV
            for (auto agv : this->Problem.AGVs)
            {
                std::string varname = "x_" + std::to_string(agv.id) + "_" + to_string(e.start_node) + "_" + to_string(e.end_node);

                // Thêm hệ số biến vào ràng buộc tuyến tính
                SCIP_CALL(SCIPaddCoefLinear(this->scip, cons, this->varmap[varname], 1.0));
            }
            // Thêm ràng buộc vào bài toán SCIP
            SCIP_CALL(SCIPaddCons(this->scip, cons));

            // Lưu trữ ràng buộc để sử dụng sau này
            this->cons.push_back(cons);
        }
    }

    return SCIP_OKAY; // Trả về mã lỗi thành công
}

SCIP_RETCODE Solver::Constraint2()
{
    set<int> eliminate;
    for (auto agv : this->Problem.AGVs)
    {
        eliminate.insert(agv.start_node);
    }
    for (auto pair : this->Problem.invertex)
    {
        for (auto end_node : this->Problem.end_nodes)
        {
            if (end_node.id % this->Problem.N == pair.first % this->Problem.N)
            {
                eliminate.insert(pair.first);
            }
        }
    }
    for (auto agv : this->Problem.AGVs)
    {
        for (auto pair : this->Problem.outvertex)
        {
            int node = pair.first;
            if (eliminate.find(node) == eliminate.end())
            {
                SCIP_CONS *cons = nullptr;
                SCIP_CALL(SCIPcreateConsBasicLinear(
                    this->scip,
                    &cons,
                    ("flow_at_" + to_string(node)).c_str(),
                    0,
                    nullptr,
                    nullptr,
                    0.0,
                    0.0));
                for (auto e : this->Problem.invertex[node])
                {
                    string varname = "x_" + to_string(agv.id) + "_" + to_string(e.start_node) + "_" + to_string(e.end_node);
                    SCIP_CALL(SCIPaddCoefLinear(this->scip, cons, this->varmap[varname], 1.0));
                }
                for (auto e : this->Problem.outvertex[node])
                {
                    string varname = "x_" + to_string(agv.id) + "_" + to_string(e.start_node) + "_" + to_string(e.end_node);
                    SCIP_CALL(SCIPaddCoefLinear(this->scip, cons, this->varmap[varname], -1.0));
                }
                SCIP_CALL(SCIPaddCons(this->scip, cons));
                this->cons.push_back(cons);
            }
        }
    }

    return SCIP_OKAY;
}

SCIP_RETCODE Solver::Constraint3()
{
    set<int> destination;
    for (auto pair : this->Problem.invertex)
    {
        for (auto end_node : this->Problem.end_nodes)
        {
            if (end_node.id % this->Problem.N == pair.first % this->Problem.N)
            {
                destination.insert(pair.first);
            }
        }
    }
    for (auto agv : this->Problem.AGVs)
    {
        for (auto Possible_end_node : destination)
        {
            SCIP_CONS *cons = nullptr;
            SCIP_CALL(SCIPcreateConsBasicLinear(this->scip,
                                                &cons,
                                                "TW_Equal_1",
                                                0,
                                                nullptr,
                                                nullptr,
                                                0.0,
                                                0.0));
            for (auto e : this->Problem.invertex[Possible_end_node])
            {
                string varname = "x_" + to_string(agv.id) + "_" + to_string(e.start_node) + "_" + to_string(e.end_node);
                SCIP_CALL(SCIPaddCoefLinear(this->scip, cons, this->varmap[varname], 1.0));
            }
            if (this->Problem.outvertex.find(Possible_end_node) != this->Problem.outvertex.end())
            {
                for (auto e : this->Problem.outvertex[Possible_end_node])
                {
                    string varname = "x_" + to_string(agv.id) + "_" + to_string(e.start_node) + "_" + to_string(e.end_node);
                    SCIP_CALL(SCIPaddCoefLinear(this->scip, cons, this->varmap[varname], -1.0));
                }
            }
            for (int i = 0; i < this->Problem.end_nodes.size(); i++)
            {
                // if (Possible_end_node % this->Problem.N == this->Problem.end_nodes[i].id % this->Problem.N)
                //{
                string varname = "y" + to_string(i) + "_" + to_string(agv.id) + "_" + to_string(Possible_end_node);
                if (this->varmap.find(varname) != this->varmap.end())
                {
                    SCIP_CALL(SCIPaddCoefLinear(this->scip, cons, this->varmap[varname], -1.0));
                }
                //}
            }

            SCIP_CALL(SCIPaddCons(this->scip, cons));
            this->cons.push_back(cons);
        }
    }
    return SCIP_OKAY;
}

SCIP_RETCODE Solver::Constraint4()
{
    for (auto agv : this->Problem.AGVs)
    {
        SCIP_CONS *cons = nullptr;
        SCIP_CALL(SCIPcreateConsBasicLinear(this->scip,
                                            &cons,
                                            ("Start_at_" + to_string(agv.start_node)).c_str(),
                                            0,
                                            nullptr,
                                            nullptr,
                                            1.0,
                                            1.0));
        for (auto e : this->Problem.outvertex[agv.start_node])
        {
            string varname = "x_" + to_string(agv.id) + "_" + to_string(e.start_node) + "_" + to_string(e.end_node);
            if (this->varmap.find(varname) != this->varmap.end())
            {
                SCIP_CALL(SCIPaddCoefLinear(this->scip, cons, this->varmap[varname], 1.0));
            }
        }
        if (this->Problem.invertex.find(agv.start_node) != this->Problem.invertex.end())
        {
            for (auto e : this->Problem.invertex[agv.start_node])
            {
                string varname = "x_" + to_string(agv.id) + "_" + to_string(e.start_node) + "_" + to_string(e.end_node);
                if (this->varmap.find(varname) != this->varmap.end())
                {
                    SCIP_CALL(SCIPaddCoefLinear(this->scip, cons, this->varmap[varname], -1.0));
                }
            }
        }
        for (int i = 0; i < this->Problem.end_nodes.size(); i++)
        {

            string varname = "y" + to_string(i) + "_" + to_string(agv.id) + "_" + to_string(agv.start_node);
            if (this->varmap.find(varname) != this->varmap.end())
            {
                SCIP_CALL(SCIPaddCoefLinear(this->scip, cons, this->varmap[varname], 1.0));
            }
        }

        SCIP_CALL(SCIPaddCons(this->scip, cons));

        this->cons.push_back(cons);
    }

    return SCIP_OKAY;
}
SCIP_RETCODE Solver::Constraint5()
{
    map<int, set<int>> destination;
    for (auto pair : this->Problem.invertex)
    {
        for (auto end_node : this->Problem.end_nodes)
        {
            if (end_node.id % this->Problem.N == pair.first % this->Problem.N)
            {
                destination[end_node.id].insert(pair.first);
            }
        }
    }

    for (int i = 0; i < this->Problem.end_nodes.size(); i++)
    {
        SCIP_CONS *cons = nullptr;
        SCIP_CALL(SCIPcreateConsBasicLinear(this->scip,
                                            &cons,
                                            "End_Task_Equal_Given",
                                            0,
                                            nullptr,
                                            nullptr,
                                            1.0,
                                            1.0));
        for (auto Possible_end_node : destination[this->Problem.end_nodes[i].id])
        {
            for (auto agv : this->Problem.AGVs)
            {
                string varname = "y" + to_string(i) + "_" + to_string(agv.id) + "_" + to_string(Possible_end_node);
                if (this->varmap.find(varname) != this->varmap.end())
                {
                    SCIP_CALL(SCIPaddCoefLinear(this->scip, cons, this->varmap[varname], 1.0));
                }
            }
        }
        for (auto agv : this->Problem.AGVs)
        {
            string varname = "y" + to_string(i) + "_" + to_string(agv.id) + "_" + to_string(agv.start_node);
            if (this->varmap.find(varname) != this->varmap.end())
            {
                SCIP_CALL(SCIPaddCoefLinear(this->scip, cons, this->varmap[varname], 1.0));
            }
        }
        SCIP_CALL(SCIPaddCons(this->scip, cons));
        this->cons.push_back(cons);
    }
    return SCIP_OKAY;
}
SCIP_RETCODE Solver::Constraint6()
{

    return SCIP_OKAY;
}

SCIP_RETCODE Solver::SubTWcons()
{
    return SCIP_OKAY;
}

SCIP_RETCODE Solver::SubRestrictioncons()
{
    // z_i_j >= number of agv - Ur
    for (int i = 0; i < this->Problem.restriction.size(); ++i)
    {

        SCIP_CONS *cons = nullptr;
        SCIP_CALL(SCIPcreateConsBasicLinear(this->scip,
                                            &cons,
                                            "cons_z_ge_x",
                                            0,
                                            nullptr,
                                            nullptr,
                                            0.0,
                                            SCIPinfinity(scip)));
        string z_var = "z_" + to_string(i);
        SCIP_CALL(SCIPaddCoefLinear(this->scip, cons, this->varmap[z_var], 1.0));
        auto pair = this->Problem.restriction[i];
        for (auto agv : this->Problem.AGVs)
        {
            for (auto e : pair.first)
            {
                if (this->Problem.outvertex.find(e.start_node) != this->Problem.outvertex.end())
                {
                    int count = 0;
                    for (auto edge : this->Problem.outvertex[e.start_node])
                    {
                        if (e.end_node == edge.end_node)
                            count++;
                    }
                    if (count > 0)
                    {
                        string varname = "x_" + to_string(agv.id) + "_" + to_string(e.start_node) + "_" + to_string(e.end_node);
                        SCIP_CALL(SCIPaddCoefLinear(this->scip, cons, this->varmap[varname], -1.0));
                    }
                }
            }
        }
        SCIP_CALL(SCIPchgLhsLinear(this->scip, cons, -pair.second));
        SCIP_CALL(SCIPaddCons(this->scip, cons));
        this->cons.push_back(cons);
    }
    return SCIP_OKAY;
}

void Solver::set_end_queue()
{
}

SCIP_RETCODE Solver::Solve()
{

    SCIP_CALL(SCIPcreate(&this->scip));
    SCIP_CALL(SCIPincludeDefaultPlugins(this->scip));

    SCIP_CALL(SCIPcreateProbBasic(this->scip, "Shortest_Path_Problem"));
    SCIP_CALL(mainproblem());
    SCIP_CALL(SCIPsetObjsense(this->scip, SCIP_OBJSENSE_MINIMIZE));

    SCIP_CALL(Constraint1());

    SCIP_CALL(Constraint2());

    SCIP_CALL(Constraint3());

    SCIP_CALL(Constraint4());

    SCIP_CALL(Constraint5());

    SCIP_CALL(Constraint6());

    SCIP_CALL(SubTWcons());

    SCIP_CALL(SubRestrictioncons());

    SCIP_CALL(SCIPsolve(this->scip));

    return SCIP_OKAY;
}

SCIP_RETCODE Solver::Result()
{
    SCIP_SOL *sol = SCIPgetBestSol(this->scip);

    if (sol != nullptr)
    {
        map<int, vector<pair<int, int>>> agvs_path;
        cout << "Optimal solution found:" << endl;
        for (auto &[varname, var] : this->varmap)
        {
            SCIP_Real val = SCIPgetSolVal(scip, sol, var);
            int int_val = static_cast<int>(val);

            if (int_val == 1)
            {
                //cout << varname << " = " << int_val << endl;
                if (varname[0] == 'x')
                {
                    stringstream ss(varname);

                    // Use getline to extract the partss
                    string tmp, id, pre_node, next_node;
                    getline(ss, tmp, '_');
                    getline(ss, id, '_');
                    getline(ss, pre_node, '_');
                    getline(ss, next_node, '_');
                    // agvs_path[stoi(id)].push_back(make_pair(stoi(pre_node), stoi(next_node)));
                    int pre_time = (stoi(pre_node) % this->Problem.N == 0) ? (stoi(pre_node) / this->Problem.N - 1) : (stoi(pre_node) / this->Problem.N);
                    int next_time = (stoi(next_node) % this->Problem.N == 0) ? (stoi(next_node) / this->Problem.N - 1) : (stoi(next_node) / this->Problem.N);
                    int time = next_time - pre_time;
                    cout << "a " << pre_node << " " << next_node << " " << pre_time << " + " << time << " = " << next_time << endl;
                }
                if (varname[0] == 'y')
                {
                    string tmp, id, TWnodesrc;
                    stringstream ss(varname);
                    getline(ss, tmp, '_');
                    getline(ss, id, '_');
                    getline(ss, TWnodesrc, '_');
                    string index_string = tmp.substr(1);
                    int index = stoi(index_string);
                    cout<<"a "<<TWnodesrc<<" "<<this->Problem.end_nodes[index].TWNode<<endl;
                }
            }
        }
        /*for (auto agv : this->Problem.AGVs)
        {
            int tmp = 0;
            for (auto pair : agvs_path[agv.id])
            {
                int pre_time = (pair.first % this->Problem.N == 0) ? (pair.first / this->Problem.N - 1) : (pair.first / this->Problem.N);
                int next_time = (pair.second % this->Problem.N == 0) ? (pair.second / this->Problem.N - 1) : (pair.second / this->Problem.N);
                int time = next_time - pre_time;
                cout << "a " << pair.first << " " << pair.second << " " << pre_time << " + " << time << " = " << next_time << endl;
                tmp = max(tmp, pair.second);
            }
            cout << "a " << tmp << " " << agv.destination_node << endl;
        }*/
    }

    return SCIP_OKAY;
}

SCIP_RETCODE Solver::Free_resources()
{

    for (auto &[varname, var] : this->varmap)
    {
        SCIP_CALL(SCIPreleaseVar(this->scip, &var)); // Đảm bảo tất cả các biến đều được giải phóng
    }

    // Giải phóng các constraints
    for (SCIP_CONS *con : this->cons)
    {
        SCIP_CALL(SCIPreleaseCons(this->scip, &con));
    }

    return SCIP_OKAY;
}