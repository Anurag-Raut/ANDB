#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <optional>
using namespace std;


struct Block {
    string key;
    optional<uint64_t> pageNumber;
    optional<uint16_t> blockNumber;
    
    int size() const {
        int size = sizeof(key);
        if (pageNumber.has_value()) {
            size += sizeof(pageNumber.value());
        }
        if (blockNumber.has_value()) {
            size += sizeof(blockNumber.value());
        }
        return size;
    }
};





class Btree{


    public:
    fstream *index_file; 
        void insert(string key, string value) ;
        string search(string key) ;
        void printTree(uint64_t rootPageNumber) ;
        Btree(fstream* fp);
        // BTreeNode* readPage(int pageNumber);
        // void writePage(BTreeNode* node);
        // void updateFlush(BTreeNode* node);
        
        
};

class BTreeNode {
public:
    vector<Block> blocks;
    vector<uint64_t> children;
    uint64_t pageNumber;
    int size;
    bool leafNode;

    BTreeNode(bool isLeafNode) : size(0), leafNode(isLeafNode) {}

    void insertHelper(string key,Btree* btree,vector<pair<BTreeNode*,int>> &parents);
    string search(string key,Btree* Btree);
    void  balance(Btree* bt,vector<pair<BTreeNode*,int>> &parents);
    void insertAtIndex(int index,Block block);
    
    

};