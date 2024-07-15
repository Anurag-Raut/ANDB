#pragma once

#include <fstream>
#include <optional>
#include <string>
#include <vector>
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

// Forward declaration
class BTreeNode;

class Btree {
public:
    fstream* index_file;
    Btree(fstream* fp);

    void insert(Block data, string value);
    pair<BTreeNode*,optional<Block>>  search(string key);
    void printTree(uint64_t rootPageNumber);
    vector<Block> deleteNode(string key);
    BTreeNode* readPage(uint64_t pageNumber);
    pair<BTreeNode*,optional<Block>> beg();

    // Other methods as needed
};




class BTreeNode {
public:
    vector<Block> blocks;
    vector<uint64_t> children;
    uint64_t pageNumber;
    size_t size;
    bool leafNode;
    int64_t prevSibling;
    int64_t nextSibling;

    BTreeNode(bool isLeafNode)
        : prevSibling(-1), nextSibling(-1), leafNode(isLeafNode), size(sizeof(prevSibling) + sizeof(nextSibling) + sizeof(leafNode) + sizeof(size)) {}

    void insertHelper(Block newData, Btree* btree, vector<pair<BTreeNode*, int>>& parents);
    pair<BTreeNode*,optional<Block>>  search(string key, Btree* btree);
    void balance(Btree* btree, vector<pair<BTreeNode*, int>>& parents);
    void insertAtIndex(int index, Block block);

    pair<optional<Block>, vector<Block>> deleteHelper(string key, Btree* btree, vector<pair<BTreeNode*, int>> parents);
};