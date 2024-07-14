#pragma once
#include <string>
#include <vector>


using namespace std;
class Statement{

    public:
    string operation;



    virtual void parse() = 0;

    


};


class SelectStatement: public Statement{
    public:
    string table_name;
    vector<string> columns;
    string where_condition;

    void parse(){
        
    }

};