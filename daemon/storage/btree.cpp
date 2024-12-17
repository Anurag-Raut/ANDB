#include "./include/btree.hpp"

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

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>


using namespace std;

Btree::Btree(fstream* fp) { index_file = fp; }

// Assuming BTreeNode and Btree are defined elsewhere in your code

BTreeNode* Btree::readPage(uint64_t pageNumber) {
    char buffer[PAGE_SIZE];
    this->index_file->clear();
    this->index_file->seekg(pageNumber * PAGE_SIZE, std::ios::beg);

    if (!this->index_file->read(buffer, PAGE_SIZE)) {
        // std::cerr << "ERROR: " << std::strerror(errno) << std::endl;
        this->index_file->clear();

        return nullptr;
    }

    size_t offset = 0;
    bool leafNode;
    uint16_t noOfBlocks;
    uint16_t size;
    int64_t prevSibling;
    int64_t nextSibling;


    std::memcpy(&leafNode, buffer + offset, sizeof(leafNode));
    offset += sizeof(leafNode);
    std::memcpy(&noOfBlocks, buffer + offset, sizeof(noOfBlocks));
    offset += sizeof(noOfBlocks);
    std::memcpy(&size, buffer + offset, sizeof(size));
    offset += sizeof(size);

    memcpy(&prevSibling, buffer + offset, sizeof(prevSibling));
    offset += sizeof(prevSibling);
    memcpy(&nextSibling, buffer + offset, sizeof(nextSibling));
    offset += sizeof(nextSibling);

    BTreeNode* newNode = new BTreeNode(leafNode);
    newNode->pageNumber = pageNumber;
    newNode->size = size;
    newNode->prevSibling=prevSibling;
    newNode->nextSibling=nextSibling;
    if (!leafNode) {
        newNode->children.resize(noOfBlocks + 1);
    }

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

    return newNode;
}

uint64_t getNewIndexPageNumber(Btree* btree) {
    btree->index_file->seekp(0, ios::end);
    std::streampos file_size = btree->index_file->tellp();
    uint64_t current_page = file_size / PAGE_SIZE;
    return current_page;
}

void writePage(BTreeNode* node, Btree* btree) {
    char buffer[PAGE_SIZE];
    // Write leaf node flag

    uint64_t current_page = getNewIndexPageNumber(btree);
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
    memcpy(buffer + offset, &node->prevSibling, sizeof(node->prevSibling));
    offset += sizeof(node->prevSibling);
    memcpy(buffer + offset, &node->nextSibling, sizeof(node->nextSibling));
    offset += sizeof(node->nextSibling);

    for (int i = 0; i < node->blocks.size(); i++) {
        Block blockData = node->blocks[i];

        // Assuming blocks vector contains key-value pairs
        uint16_t keySize = blockData.key.size();
        memcpy(buffer + offset, &keySize, sizeof(keySize));

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
    // Seek to the end of the file to append the new page
    // index_file->seekp(0, ios::end);

    // Write the buffer to the file
    // btree->index_file->seekp(0, ios::end);

    btree->index_file->write(buffer, sizeof(buffer));
    // Flush to ensure data is written
    btree->index_file->flush();
}
void updateSiblingPointers(BTreeNode* node, Btree* btree) {
    char buffer[PAGE_SIZE];
    btree->index_file->clear();
    btree->index_file->seekg(node->pageNumber * PAGE_SIZE, std::ios::beg);

    if (!btree->index_file->read(buffer, PAGE_SIZE)) {
        // std::cerr << "ERROR: " << std::strerror(errno) << std::endl;
        btree->index_file->clear();

        return;
    }
    size_t offset = sizeof(node->leafNode);
    offset += sizeof(uint16_t) + sizeof(uint16_t);
    memcpy(buffer + offset, &node->prevSibling, sizeof(node->prevSibling));
    offset += sizeof(node->prevSibling);
    memcpy(buffer + offset, &node->nextSibling, sizeof(node->nextSibling));

    btree->index_file->seekp(node->pageNumber * PAGE_SIZE, ios::beg);

    // Write the buffer to the file
    btree->index_file->write(buffer, sizeof(buffer));
    // Flush to ensure data is written
    btree->index_file->flush();
}
void updateFlush(BTreeNode* node, Btree* btree) {
    char buffer[PAGE_SIZE];
    // Write leaf node flag
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

    memcpy(buffer + offset, &node->prevSibling, sizeof(node->prevSibling));
    offset += sizeof(node->prevSibling);
    memcpy(buffer + offset, &node->nextSibling, sizeof(node->nextSibling));
    offset += sizeof(node->nextSibling);

    for (int i = 0; i < node->blocks.size(); i++) {
        Block blockData = node->blocks[i];

        // Assuming blocks vector contains key-value pairs
        uint16_t keySize = blockData.key.size();
        memcpy(buffer + offset, &keySize, sizeof(keySize));

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
    // S    eek to the end of the file to append the new page
    // index_file->seekp(0, ios::end);
    btree->index_file->seekp(node->pageNumber * PAGE_SIZE, ios::beg);

    // Write the buffer to the file
    btree->index_file->write(buffer, sizeof(buffer));
    // Flush to ensure data is written
    btree->index_file->flush();
}

void Btree::insert(Block newData) {
    vector<pair<BTreeNode*, int>> parents;
    BTreeNode* root = this->readPage(0);
    if (root == NULL) {
        int pageNumber = 0;
        BTreeNode* newRoot = new BTreeNode(true);
        newRoot->blocks.push_back(newData);
        newRoot->size += newData.size();
        root = newRoot;
        writePage(root, this);
        return;
    }

    root->insertHelper(newData, this, parents);

}

vector<Block> Btree::deleteNode(string key) {
    vector<pair<BTreeNode*, int>> parents;

    BTreeNode* root = this->readPage(0);
    if (root == NULL) {
        return {};
    } else {
        return root->deleteHelper(key, this, parents).second;
    }
}

void removeBlocksAtIndex(BTreeNode* node, int index) {
    int j = index + 1;
    while (j < node->blocks.size()) {
        swap(node->blocks[j - 1], node->blocks[j]);
        // swap(node->blocks[j - 1].blockNumber, node->blocks[j].blockNumber);
        j++;
    }
    node->blocks.pop_back();
}
// first block is to pass on minimum to top node 
//second is the actuall deleted block
pair<optional<Block>, vector<Block>> BTreeNode::deleteHelper(string key, Btree* btree, vector<pair<BTreeNode*, int>> parents) {
    if (this->leafNode) {
        int j = 0;
        while (j < blocks.size() && key > blocks[j].key) {
            j++;
        }
        pair<optional<Block>, vector<Block>> result = make_pair(nullopt, vector<Block>());
        while (j < blocks.size() && blocks[j].key == key) {
            Block deletedBlock = blocks[j];
            // removeBlocksAtIndex(this, j);
            this->balance(btree, parents);
            // updateFlush(this, btree);
            result.first=blocks[0];
            result.second.push_back(deletedBlock);
            j++;
        } 

        return result;
        


    } else {
        int j = 0;
        while (j < blocks.size() && key >= blocks[j].key) {
            j++;
        }

        BTreeNode* childrenNode = btree->readPage(this->children[j]);
        parents.push_back({this, j});
        pair<optional<Block>, vector<Block>> data = childrenNode->deleteHelper(key, btree, parents);
        optional<Block> minFrombottom = data.first;
        if (j != 0 && minFrombottom.has_value()) {

            blocks[j - 1].key = min(minFrombottom.value().key, blocks[j - 1].key);

            updateFlush(this, btree);
            // data.first.value().key=min(data.first.value().key,blocks[0].key);
        }

        return data;
    }
}

pair<BTreeNode*,optional<Block>> Btree::search(string key) {
    BTreeNode* root = this->readPage(0);
    if (root == NULL) return {NULL,nullopt};
    return root->search(key, this);
}

pair<BTreeNode*,optional<Block>> Btree::beg() {

    BTreeNode* node = this->readPage(0);

    if(node==NULL){
        return {NULL,nullopt};

    }
    while(!node->leafNode){
        node=this->readPage(node->children[0]);
        
    }

    Block begBlock =node->blocks[0];

    return {node,begBlock};

    
    
}

void Btree::printTree(uint64_t rootPageNumber) {
    BTreeNode* root = this->readPage(rootPageNumber);
    std::queue<BTreeNode*> q;
    int size = 1;
    q.push(root);
    while (!q.empty()) {
        int newSize = 0;
        for (int i = 0; i < size; i++) {
            BTreeNode* newNode = q.front();
            q.pop();
            for (auto i : newNode->blocks) {
            }

            for (auto children_pagenumber : newNode->children) {
                BTreeNode* children = this->readPage(children_pagenumber);
                q.push(children);
                newSize += 1;
            }
        }
        size = newSize;
    }
}

void BTreeNode::insertHelper(Block newData, Btree* btree, vector<pair<BTreeNode*, int>>& parents) {
    if (leafNode) {
        // insert
        int j = blocks.size() - 1;

        blocks.push_back(newData);

        while (j >= 0 && blocks[j].key > newData.key) {

            swap(blocks[j + 1], blocks[j]);
            j--;
        }
        size += blocks[j + 1].size();
        if (size > PAGE_SIZE) {
            // balance
            this->balance(btree, parents);
        }

        updateFlush(this, btree);

    } else {
        int j = 0;
        while (j < blocks.size() && blocks[j].key < newData.key) {
            j++;
        }

        BTreeNode* childrenNode = btree->readPage(children[j]);
        parents.push_back({this, j});
        childrenNode->insertHelper(newData, btree, parents);
    }
}

void BTreeNode::balance(Btree* btree, vector<pair<BTreeNode*, int>>& parents) {
    if (this->size <= PAGE_SIZE && blocks.size() > 0) {
        return;
    }
    if (blocks.size() == 0) {
        BTreeNode* parent = parents.back().first;
        int index = parents.back().second;
        BTreeNode* nextChildren = NULL;
        BTreeNode* prevChildren = NULL;

        if (index + 1 < parent->children.size()) {
            nextChildren = btree->readPage(parent->children[index + 1]);
            if (nextChildren) {
                if (nextChildren->blocks.size() > 1) {
                    parent->blocks[index].key = nextChildren->blocks[1].key;
                    insertAtIndex(0, Block{.key = nextChildren->blocks[0].key});
                    nextChildren->size -= nextChildren->blocks[0].size();

                    removeBlocksAtIndex(nextChildren, 0);
                    updateFlush(parent, btree);
                    updateFlush(nextChildren, btree);
                    return;
                }
            }
        }
        if (index - 1 >= 0) {
            prevChildren = btree->readPage(parent->children[index - 1]);
            if (prevChildren->blocks.size() > 1) {
                parent->blocks[index - 1].key = prevChildren->blocks.back().key;
                insertAtIndex(0, Block{.key = prevChildren->blocks.back().key});
                prevChildren->size -= prevChildren->blocks.back().size();
                removeBlocksAtIndex(prevChildren, prevChildren->blocks.size() - 1);
                updateFlush(parent, btree);
                updateFlush(prevChildren, btree);
                return;
            }
        }
        if (index + 1 < parent->children.size() && nextChildren) {

            parent->size -= blocks[index - 1].size();
            parent->blocks.erase(parent->blocks.begin() + index - 1);
            parent->children.erase(parent->children.begin() + index);
            if (parent->blocks.size() < 1) {
                vector<pair<BTreeNode*, int>> newParents(parents.begin(), parents.end() - 1);
                parent->balance(btree, newParents);
            }
            updateFlush(parent, btree);

        } else if (index - 1 >= 0 && prevChildren) {
            parent->size -= blocks[index - 1].size();
            parent->blocks.erase(parent->blocks.begin() + index - 1);
            parent->children.erase(parent->children.begin() + index);
            if (parent->blocks.size() < 1) {
                vector<pair<BTreeNode*, int>> newParents(parents.begin(), parents.end() - 1);
                parent->balance(btree, newParents);
            }
            updateFlush(parent, btree);
        }
    } else if (parents.size() == 0) {

        BTreeNode* newRoot = new BTreeNode(false);
        int mid = this->blocks.size() / 2;
        Block newData = Block{
            .key = this->blocks[mid].key,

        };
        int parentSize = blocks.size();

        newRoot->blocks.push_back(newData);
        BTreeNode* c1 = new BTreeNode(this->leafNode);
        BTreeNode* c2 = new BTreeNode(this->leafNode);

        for (int i = 0; i < mid; i++) {
            c1->blocks.push_back(this->blocks[0]);
            c1->size += this->blocks[0].size();
            this->blocks.erase(this->blocks.begin());
            if (!leafNode) {
                c1->children.push_back(this->children[0]);
                this->children.erase(this->children.begin());
            }
        }
        if (!leafNode) {

            c1->children.push_back(this->children[0]);
            this->children.erase(this->children.begin());
        }
        if (!leafNode) {
            c2->children.push_back(this->children[0]);
            this->children.erase(this->children.begin());
        }
        Block midBlock = this->blocks[0];
        int secondBlockStart;
        int offset;
        if (leafNode) {
            secondBlockStart = mid;
            offset = 0;

        } else {
            secondBlockStart = mid + 1;
            offset = 1;
        }
        for (int i = secondBlockStart; i < parentSize; i++) {
            c2->blocks.push_back(this->blocks[offset]);
            c2->size += this->blocks[offset].size();

            this->blocks.erase(this->blocks.begin() + offset);
            if (!leafNode) {
                c2->children.push_back(this->children[0]);
                this->children.erase(this->children.begin());
            }
        }

        writePage(c1, btree);
        writePage(c2, btree);

        c1->prevSibling = this->prevSibling;
        c1->nextSibling = c2->pageNumber;
        c2->prevSibling = c1->pageNumber;
        c2->nextSibling = this->nextSibling;


        updateSiblingPointers(c1, btree);
        updateSiblingPointers(c2, btree);

        if (leafNode) {
            this->blocks.push_back(midBlock);
        }
        this->children.push_back(c1->pageNumber);
        this->children.push_back(c2->pageNumber);
        this->leafNode = false;
        this->size = this->blocks[0].size();
        // updateFlush(this, btree);

    } else if (this->leafNode) {
        BTreeNode* newChildren = new BTreeNode(this->leafNode);
        int mid = this->blocks.size() / 2;
        int originalSize = blocks.size();
        BTreeNode* parent = parents.back().first;
        int indexToInsert = parents.back().second;
        Block newBlock = Block{.key = this->blocks[mid].key};

        parent->insertAtIndex(indexToInsert, newBlock);
        // size -= blocks[mid].size();

        // blocks.erase(blocks.begin() + mid);
        for (int i = mid; i < originalSize; i++) {
            newChildren->blocks.push_back(blocks[mid]);
            newChildren->size += blocks[mid].size();
            size -= blocks[mid].size();

            blocks.erase(blocks.begin() + mid);
        }
        newChildren->prevSibling = this->pageNumber;
      
        newChildren->nextSibling = this->nextSibling;
        writePage(newChildren, btree);
        parent->children[indexToInsert + 1] = newChildren->pageNumber;
        this->nextSibling = newChildren->pageNumber;

        updateFlush(this, btree);

        if (parent->size > PAGE_SIZE) {
            vector<pair<BTreeNode*, int>> newParents(parents.begin(), parents.end() - 1);
            parent->balance(btree, newParents);
        }
        updateFlush(parent, btree);
    } else {
        BTreeNode* newChildren = new BTreeNode(this->leafNode);
        int mid = this->blocks.size() / 2;
        int originalSize = blocks.size();
        BTreeNode* parent = parents.back().first;
        int indexToInsert = parents.back().second;
        Block newBlock = Block{.key = this->blocks[mid].key};
        parent->insertAtIndex(indexToInsert, newBlock);
        size -= blocks[mid].size();
        blocks.erase(blocks.begin() + mid);
        for (int i = mid + 1; i < originalSize; i++) {
            newChildren->blocks.push_back(blocks[mid]);
            newChildren->size += blocks[mid].size();
            newChildren->children.push_back(children[mid + 1]);
            BTreeNode* node = btree->readPage(children[mid + 1]);
            size -= blocks[mid].size();

            blocks.erase(blocks.begin() + mid);
            children.erase(children.begin() + mid + 1);
        }
        newChildren->children.push_back(children[mid + 1]);
        children.erase(children.begin() + mid + 1);

        newChildren->prevSibling = this->pageNumber;
        newChildren->nextSibling = indexToInsert + 2 < parent->children.size() ? parent->children[indexToInsert + 2] : -1;
        writePage(newChildren, btree);
        parent->children[indexToInsert + 1] = newChildren->pageNumber;
        this->nextSibling = newChildren->pageNumber;
        updateFlush(this, btree);
        if (parent->size > PAGE_SIZE) {
            vector<pair<BTreeNode*, int>> newParents(parents.begin(), parents.end() - 1);
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
    while (j >= index) {
        blocks[j + 1] = blocks[j];
        children[j + 2] = children[j + 1];
        j--;
    }
    children[j + 2] = children[j + 1];
    blocks[index] = data;
}

pair<BTreeNode*,optional<Block>> BTreeNode::search(string key, Btree* btree) {
    int i = 0;
    if (leafNode) {
        while (i < blocks.size() && key > blocks[i].key) i++;

    } else {
        while (i < blocks.size() && key > blocks[i].key) i++;
    }
    if (leafNode && i < blocks.size() && blocks[i].key == key) {
        return {this,blocks[i]};
    } else if (leafNode)
        return {NULL,nullopt};
    BTreeNode* childrenNode = btree->readPage(children[i]);
    return childrenNode->search(key, btree);
}
