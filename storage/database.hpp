#pragma once
#include <vector>
#include <string>
#include <fstream>

using namespace std;

struct key_struct {
    string key;
    string value;
    int size() const { return value.size(); }
};

class BTreeNode {
public:
    vector<key_struct> nodes;
    vector<BTreeNode*> children;
    int size;
    bool leafNode;

    BTreeNode(bool isLeafNode) : size(0), leafNode(isLeafNode) {}

    int insertHelper(key_struct key);
    void splitChild(int index, BTreeNode* node);
    string search(string key);
  
    void printNode();

};


class Database {
    private:
        std::ofstream file; 

    public:
        BTreeNode* root = NULL;
        void insert(string key, string value) ;
        string search(string key) ;
        void printTree() ;
        Database(string database_name);
};
