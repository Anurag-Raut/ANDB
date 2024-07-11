
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "../storage/btree.cpp"
#include "../include/database.hpp"
#include "../include/table.hpp"
#include "../include/globals.hpp"


using namespace std;



Table::Table(string table_name, vector<string> types, vector<string> names,fstream* fp ) {
    if (types.size() != names.size()) {
        return;
    }

    for (size_t i = 0; i < types.size(); ++i) {
        columns.push_back({names[i], types[i]});
    }
    
    this->btree=new Btree(fp);

    
}

void Table::Insert( vector<string> args) {
    if (args.size() != columns.size()) {
        cout << "ERROR : args size not equal to column data";
        return;
    }
    btree->insert(args[0],args[1]);




}

void Table::Print(){
    btree->printTree(this->rootPageNumber);
}


string Table::Search(string key){
    return btree->search(key);

}



