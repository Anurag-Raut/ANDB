#include "../include/btree.hpp"

#include <math.h>

#include <chrono>  // For std::chrono::seconds
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <thread>  // For std::this_thread::sleep_for
#include <vector>

#include "../include/globals.hpp"

using namespace std;

Btree::Btree(fstream* fp) { index_file = fp; }

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>

// Assuming BTreeNode and Btree are defined elsewhere in your code

BTreeNode* readPage(int pageNumber, Btree* btree) {
    // cout << "READING PAGE: " << pageNumber << endl;
    char buffer[PAGE_SIZE];
    btree->index_file->clear();
    btree->index_file->seekg(pageNumber * PAGE_SIZE, std::ios::beg);

    if (!btree->index_file->read(buffer, PAGE_SIZE)) {
        // std::cerr << "ERROR: " << std::strerror(errno) << std::endl;
        btree->index_file->clear();
        // cout << "ENDING READ \n\n\n";

        return nullptr;
    }

    size_t offset = 0;
    bool leafNode;
    uint16_t noOfBlocks;
    uint16_t size;

    std::memcpy(&leafNode, buffer + offset, sizeof(leafNode));
    offset += sizeof(leafNode);
    std::memcpy(&noOfBlocks, buffer + offset, sizeof(noOfBlocks));
    offset += sizeof(noOfBlocks);
    std::memcpy(&size, buffer + offset, sizeof(size));
    offset += sizeof(size);

    BTreeNode* newNode = new BTreeNode(leafNode);
    newNode->pageNumber = pageNumber;
    newNode->size = size;
    if (!leafNode) {
        newNode->children.resize(noOfBlocks + 1);
    }
    // std::cout << "READ LEAF NODE: " << leafNode << "  READ NO OF NODES: " << noOfBlocks << " READ SIZE: " << size << std::endl;

    for (int i = 0; i < noOfBlocks; i++) {
        Block blockData;
        uint16_t keySize;

        std::memcpy(&keySize, buffer + offset, sizeof(keySize));
        offset += sizeof(keySize);

        if (keySize > 0 && keySize <= PAGE_SIZE - offset) {
            blockData.key.resize(keySize);
            std::memcpy(&blockData.key[0], buffer + offset, keySize);
            offset += keySize;
        } else {
            // std::cerr << "Invalid key size: " << keySize << std::endl;
            delete newNode;
            return nullptr;
        }

        std::memcpy(&blockData.pageNumber, buffer + offset, sizeof(blockData.pageNumber));
        offset += sizeof(blockData.pageNumber);
        std::memcpy(&blockData.blockNumber, buffer + offset, sizeof(blockData.blockNumber));
        offset += sizeof(blockData.blockNumber);
        if (!leafNode) {
            memcpy(&newNode->children[i], buffer + offset, sizeof(newNode->children[i]));
            offset += sizeof(newNode->children[i]);
        }
        newNode->blocks.push_back(blockData);
    }
    if (!leafNode) {
        memcpy(&newNode->children.back(), buffer + offset, sizeof(newNode->children.back()));
        offset += sizeof(newNode->children.back());
    }

    // cout << "ENDING READ \n\n\n";
    return newNode;
}

void writePage(BTreeNode* node, Btree* btree) {
    // cout << "WRITING: " << endl;
    char buffer[PAGE_SIZE];
    // Write leaf node flag

    btree->index_file->seekp(0, ios::end);
    std::streampos file_size = btree->index_file->tellp();
    // cout << "FILE SSIZE: " << file_size << endl;
    uint64_t current_page = file_size / PAGE_SIZE;
    // cout << "CUURENT PAGE: " << current_page << endl;
    node->pageNumber = current_page;
    bool leafNode = node->leafNode;
    memcpy(buffer, &leafNode, sizeof(leafNode));
    size_t offset = sizeof(leafNode);

    // Write number of blocks
    uint16_t noOfBlocks = (node->blocks.size());

    memcpy(buffer + offset, &noOfBlocks, sizeof(noOfBlocks));
    offset += sizeof(noOfBlocks);
    uint16_t size = (node->size);
    memcpy(buffer + offset, &size, sizeof(size));

    offset += sizeof(size);

    for (int i = 0; i < node->blocks.size(); i++) {
        Block blockData = node->blocks[i];
        // cout << "BLOCKS" << endl;

        // Assuming blocks vector contains key-value pairs
        uint16_t keySize = blockData.key.size();
        memcpy(buffer + offset, &keySize, sizeof(keySize));
        // cout << "WRITINF KEYSIZE: " << keySize << " ACTUAL KEY " << blockData.key << endl;

        offset += sizeof(keySize);

        memcpy(buffer + offset, blockData.key.c_str(), keySize);
        offset += keySize;
        memcpy(buffer + offset, &blockData.pageNumber, sizeof(blockData.pageNumber));
        offset += sizeof(blockData.pageNumber);
        memcpy(buffer + offset, &blockData.blockNumber, sizeof(blockData.blockNumber));
        offset += sizeof(blockData.blockNumber);
        if (!leafNode) {
            memcpy(buffer + offset, &node->children[i], sizeof(node->children[i]));
            offset += sizeof(node->children[i]);
        }
    }
    if (!leafNode) {
        memcpy(buffer + offset, &node->children.back(), sizeof(node->children.back()));
        offset += sizeof(node->children.back());
    }
    // For debugging (optional)
    // cout << "LEAF NODE: " << leafNode << " Number of blocks: " << noOfBlocks << endl;
    // cout << "BUFFER: " << buffer << endl;
    // S    eek to the end of the file to append the new page
    // index_file->seekp(0, ios::end);

    // Write the buffer to the file
    // btree->index_file->seekp(0, ios::end);

    btree->index_file->write(buffer, sizeof(buffer));
    // Flush to ensure data is written
    btree->index_file->flush();
    // cout << "ENDING WRITE \n\n\n";
}

void updateFlush(BTreeNode* node, Btree* btree) {
    char buffer[PAGE_SIZE];
    // Write leaf node flag
    bool leafNode = node->leafNode;
    memcpy(buffer, &leafNode, sizeof(leafNode));
    size_t offset = sizeof(leafNode);
    // cout << "UPDATING LEAF NODE" << leafNode << endl;

    // Write number of blocks
    uint16_t noOfBlocks = (node->blocks.size());

    memcpy(buffer + offset, &noOfBlocks, sizeof(noOfBlocks));
    offset += sizeof(noOfBlocks);
    uint16_t size = (node->size);
    memcpy(buffer + offset, &size, sizeof(size));

    offset += sizeof(size);

    for (int i = 0; i < node->blocks.size(); i++) {
        Block blockData = node->blocks[i];
        // cout << "BLOCKS" << endl;

        // Assuming blocks vector contains key-value pairs
        uint16_t keySize = blockData.key.size();
        memcpy(buffer + offset, &keySize, sizeof(keySize));
        // cout << "WRITINF KEYSIZE: " << keySize << " ACTUAL KEY " << blockData.key << endl;

        offset += sizeof(keySize);

        memcpy(buffer + offset, blockData.key.c_str(), keySize);
        offset += keySize;
        memcpy(buffer + offset, &blockData.pageNumber, sizeof(blockData.pageNumber));
        offset += sizeof(blockData.pageNumber);
        memcpy(buffer + offset, &blockData.blockNumber, sizeof(blockData.blockNumber));
        offset += sizeof(blockData.blockNumber);
        if (!leafNode) {
            // cout << "Childrens: " << node->children[i] << endl;
            memcpy(buffer + offset, &node->children[i], sizeof(node->children[i]));
            offset += sizeof(node->children[i]);
        }
    }
    if (!leafNode) {
        // cout << "Childrens: " << node->children.back() << endl;

        memcpy(buffer + offset, &node->children.back(), sizeof(node->children.back()));
        offset += sizeof(node->children.back());
    }

    // For debugging (optional)
    // cout << "LEAF NODE: " << leafNode << " Number of blocks: " << noOfBlocks << endl;
    // cout << "BUFFER: " << buffer << endl;
    // S    eek to the end of the file to append the new page
    // index_file->seekp(0, ios::end);
    btree->index_file->seekp(node->pageNumber * PAGE_SIZE, ios::beg);

    // Write the buffer to the file
    btree->index_file->write(buffer, sizeof(buffer));
    // Flush to ensure data is written
    btree->index_file->flush();
}

void Btree::insert(Block newData, string value) {
    vector<pair<BTreeNode*, int>> parents;
    BTreeNode* root = readPage(0, this);
    if (root == NULL) {
        int pageNumber = 0;
        // cout << "YAYYY" << endl;
        BTreeNode* newRoot = new BTreeNode(true);
        newRoot->blocks.push_back(newData);
        newRoot->size += newData.size();
        root = newRoot;
        // cout << "FIRST NODE SIZE : " << root->size << endl;
        writePage(root, this);
        return;
    }

    root->insertHelper(newData, this, parents);

    // cout << "INDEX AT TOP: " << index << "ROOT SIZE: " << root->size << endl;
}

optional<Block> Btree::search(string key) {
    BTreeNode* root = readPage(0, this);

    if (root == NULL) return nullopt;
    return root->search(key, this);
}

void Btree::printTree(uint64_t rootPageNumber) {
    BTreeNode* root = readPage(rootPageNumber, this);
    cout << "LYOO BHAI:" << root->blocks.size() << endl;
    std::queue<BTreeNode*> q;
    int size = 1;
    q.push(root);
    while (!q.empty()) {
        int newSize = 0;
        for (int i = 0; i < size; i++) {
            BTreeNode* newNode = q.front();
            q.pop();
            cout << "   [ ";
            for (auto i : newNode->blocks) {
                cout << i.key << ",";
            }
            cout << "]   ";

            for (auto children_pagenumber : newNode->children) {
                // cout << "ABEEE CHILDEREN : " << children_pagenumber << endl;
                BTreeNode* children = readPage(children_pagenumber, this);
                q.push(children);
                newSize += 1;
            }
        }
        size = newSize;
        cout << endl;
    }
}

void BTreeNode::insertHelper(Block newData, Btree* btree, vector<pair<BTreeNode*, int>>& parents) {
    if (leafNode) {
        // insert
        // cout << "INSERTING: " << key << endl;
        int j = blocks.size() - 1;

        blocks.push_back(newData);

        while (j >= 0 && blocks[j].key > newData.key) {
            // cout << "BLOCK KEY " << blocks[j].key << " " << key << endl;
            
            swap(blocks[j + 1], blocks[j]);
            j--;
        }
        // cout << "j " << j + 2 << endl;
        size += blocks[j + 1].size();
        // cout << "BLOCK KEY " << blocks[j + 1].key << " Block number " << blocks[j + 1].blockNumber << " SIZE: " << size << endl;
        if (size > PAGE_SIZE) {
            // balance
            // cout << "BALANCE SIZE: " << size << " " << blocks.size() << endl;
            this->balance(btree, parents);
        }
        // cout << "children size : " << this->children.size() << endl;

        updateFlush(this, btree);

    } else {
        int j = 0;
        while (j < blocks.size() && blocks[j].key < newData.key) {
            j++;
        }
        // cout << "HALLOOO: " << j << endl;

        BTreeNode* childrenNode = readPage(children[j], btree);
        parents.push_back({this, j});
        childrenNode->insertHelper(newData, btree, parents);
    }
}

void BTreeNode::balance(Btree* btree, vector<pair<BTreeNode*, int>>& parents) {
    if (this->size <= PAGE_SIZE) {
        return;
    }
    if (parents.size() == 0) {
        // cout << "ASDS" << endl;
        BTreeNode* newRoot = new BTreeNode(false);
        int mid = this->blocks.size() / 2;
        Block newData = Block{
            .key = this->blocks[mid].key,
    
            
        };
        int parentSize = blocks.size();
        // cout << "MID: " << mid << " " << parentSize << endl;

        newRoot->blocks.push_back(newData);
        BTreeNode* c1 = new BTreeNode(this->leafNode);
        BTreeNode* c2 = new BTreeNode(this->leafNode);

        // cout << "C11 : " << endl;
        for (int i = 0; i < mid; i++) {
            // cout << this->blocks[0].key << " ";
            c1->blocks.push_back(this->blocks[0]);
            c1->size += this->blocks[0].size();
            this->blocks.erase(this->blocks.begin());
            if (!leafNode) {
                // cout<<"CHILDDD DD: "<<this->children[0]<<endl;
                c1->children.push_back(this->children[0]);
                this->children.erase(this->children.begin());
            }
        }
        // cout << endl;
        // cout << "C22 : " << endl;
        if (!leafNode) {
            // cout<<"CHILDDD DD: "<<this->children[0]<<endl;
            c1->children.push_back(this->children[0]);
            this->children.erase(this->children.begin());
        }
         if (!leafNode) {
            // cout<<"CHILDDD DD: "<<this->children[0]<<endl;
            c2->children.push_back(this->children[0]);
            this->children.erase(this->children.begin());
        }
        Block midBlock=this->blocks[0];
        int secondBlockStart;
        int offset;
        if(leafNode){
            secondBlockStart=mid;
            offset=0;

        }
        else{
            secondBlockStart=mid+1;
            offset=1;
        }
        for (int i = secondBlockStart; i < parentSize; i++) {
            // cout << this->blocks[1].key << " ";
            c2->blocks.push_back(this->blocks[offset]);
            c2->size += this->blocks[offset].size();

            this->blocks.erase(this->blocks.begin()+offset );
            if (!leafNode) {
                // cout<<"CHILDDD DD: "<<this->children[0]<<endl;
                c2->children.push_back(this->children[0]);
                this->children.erase(this->children.begin());
            }
        }
        // cout << endl;
        // cout<<"ACIID: "<< this->children.size()<<endl;

        writePage(c1, btree);
        writePage(c2, btree);

        // cout << "C1 : " << c1->blocks.size() << "C2 : " << c2->blocks.size() << endl;
        // cout << "C :" << this->children.size() << endl;
        if(leafNode){
        this->blocks.push_back(midBlock);
        }
        this->children.push_back(c1->pageNumber);
        this->children.push_back(c2->pageNumber);
        this->leafNode = false;
        this->size = this->blocks[0].size();
        // cout << "children size : " << this->children.size() << endl;
        // updateFlush(this, btree);

    } else if (this->leafNode) {
        // cout << "YATTTAA" << endl;
        BTreeNode* newChildren = new BTreeNode(this->leafNode);
        int mid = this->blocks.size() / 2;
        int originalSize = blocks.size();
        BTreeNode* parent = parents.back().first;
        int indexToInsert = parents.back().second;
        // cout << "INDEX TO INSERT:  " << indexToInsert << endl;
        Block newBlock = Block{.key = this->blocks[mid].key};
        // cout << "Parent size " << parent->size << endl;

        parent->insertAtIndex(indexToInsert, newBlock);
        // size -= blocks[mid].size();
        
        // blocks.erase(blocks.begin() + mid);
        for (int i = mid ; i < originalSize; i++) {
            // cout << "NEW CHILD: " << blocks[mid].key << endl;
            newChildren->blocks.push_back(blocks[mid]);
            newChildren->size += blocks[mid].size();
            size -= blocks[mid].size();

            blocks.erase(blocks.begin() + mid);
        }

        writePage(newChildren, btree);
        // cout << "PAGEEGEGEGG: " << indexToInsert << endl;
        parent->children[indexToInsert + 1] = newChildren->pageNumber;
        // cout << "Parent size " << parent->size << endl;
        updateFlush(this,btree);
        if (parent->size > PAGE_SIZE) {
            // cout << "DOUBLE BALACEEEE" << endl;
            vector<pair<BTreeNode*, int>> newParents(parents.begin(), parents.end() - 1);
            // cout << " NEW  PARENS " << parent->size << endl;
            parent->balance(btree, newParents);
        }
        updateFlush(parent, btree);
    }
    else{
               

        // cout<<"ITS A RAINYV DAY: "<<endl;
            BTreeNode* newChildren = new BTreeNode(this->leafNode);
        int mid = this->blocks.size() / 2;
        int originalSize = blocks.size();
        BTreeNode* parent = parents.back().first;
        int indexToInsert = parents.back().second;
        Block newBlock = Block{.key = this->blocks[mid].key};
        parent->insertAtIndex(indexToInsert, newBlock);
        size -= blocks[mid].size();
        // cout<<"MID: "<<mid<<"endl";
        blocks.erase(blocks.begin() + mid);
        for (int i = mid+1 ; i < originalSize; i++) {
            // cout << "NEW CHILD: " << blocks[mid].key << endl;
            newChildren->blocks.push_back(blocks[mid]);
            newChildren->size += blocks[mid].size();
            newChildren->children.push_back(children[mid+1]);
            BTreeNode *node=readPage(children[mid+1],btree);
            // cout<<"NODE: " <<node->blocks[0].key<<endl;
            size -= blocks[mid].size();

            blocks.erase(blocks.begin() + mid);
            children.erase(children.begin() + mid+1);

        }
            newChildren->children.push_back(children[mid+1]);
            children.erase(children.begin() + mid+1);

        writePage(newChildren, btree);
        // cout << "PAGEEGEGEGG: " << newChildren->children.size() << endl;
        parent->children[indexToInsert + 1] = newChildren->pageNumber;
        // cout << "Parent size " << parent->size << endl;
        updateFlush(this,btree);
        if (parent->size > PAGE_SIZE) {
            // cout << "DOUBLE BALACEEEE" << endl;
            vector<pair<BTreeNode*, int>> newParents(parents.begin(), parents.end() - 1);
            // cout << " NEW  PARENS " << parent->leafNode << endl;
            parent->balance(btree, newParents);
        }
        updateFlush(parent, btree);
        


    }
}

void BTreeNode::insertAtIndex(int index, Block data) {
    int j = blocks.size() - 1;
    this->blocks.push_back(data);
    this->size += data.size();
    this->children.push_back(0);
    // cout << "ASSD INDEX: " << j << endl;
    while (j >= index) {
        blocks[j + 1] = blocks[j];
        children[j + 2] = children[j + 1];
        j--;
    }
    children[j + 2] = children[j + 1];
    blocks[index] = data;
}

optional<Block> BTreeNode::search(string key, Btree* btree) {
    int i = 0;
    // cout<<"SEARCH "<<" NODE SIZE "<<blocks.size()<<"  "<<endl;
    if(leafNode){
    while (i < blocks.size() && key > blocks[i].key) i++;

    }
    else{
        while (i < blocks.size() && key >= blocks[i].key) i++;

    }
    // cout<<"THIS IS I: "<<i<<endl;
    if (leafNode && i<blocks.size() && blocks[i].key == key) {
        return (blocks[i]);
    }
    else if (leafNode) return nullopt;
    BTreeNode* childrenNode = readPage(children[i], btree);
    return childrenNode->search(key, btree);
}

