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

Btree::Btree(fstream* fp) { data_file = fp; }

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>

// Assuming BTreeNode and Btree are defined elsewhere in your code

BTreeNode* readPage(int pageNumber, Btree* btree) {
    // cout << "READING PAGE: " << pageNumber << endl;
    char buffer[PAGE_SIZE];
    btree->data_file->clear();
    btree->data_file->seekg(pageNumber * PAGE_SIZE, std::ios::beg);

    if (!btree->data_file->read(buffer, PAGE_SIZE)) {
        // std::cerr << "ERROR: " << std::strerror(errno) << std::endl;
        btree->data_file->clear();
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

    btree->data_file->seekp(0,ios::end);
    std::streampos file_size = btree->data_file->tellp();
    // cout << "FILE SSIZE: " << file_size << endl;
    uint64_t current_page = file_size / PAGE_SIZE;
    cout << "CUURENT PAGE: " << current_page << endl;
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
    // data_file->seekp(0, ios::end);

    // Write the buffer to the file
    btree->data_file->seekp(0, ios::end);

    btree->data_file->write(buffer, sizeof(buffer));
    // Flush to ensure data is written
    btree->data_file->flush();
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
    // data_file->seekp(0, ios::end);
    btree->data_file->seekp(node->pageNumber * PAGE_SIZE, ios::beg);

    // Write the buffer to the file
    btree->data_file->write(buffer, sizeof(buffer));
    // Flush to ensure data is written
    btree->data_file->flush();
}

uint64_t Btree::insert(string key, string value) {
    BTreeNode* root = readPage(0, this);
    if (root == NULL) {
        int pageNumber = 0;
        // cout << "YAYYY" << endl;
        BTreeNode* newRoot = new BTreeNode(true);
        Block newData = Block({key, pageNumber, 0});
        newRoot->blocks.push_back(newData);
        newRoot->size += newData.size();
        root = newRoot;
        // cout << "FIRST NODE SIZE : " << root->size << endl;
        writePage(root, this);
        return root->pageNumber;
    }

    int index = root->insertHelper(key, this);

    if (index != -1) {
        cout << "INDEX DSFDSFDFD: " << index << endl;
        Block newData = Block{key : key, pageNumber : root->pageNumber, blockNumber : root->blocks.size()};
        int l = root->blocks.size()-1;
        root->blocks.push_back(newData);
        while (l >= 0 && root->blocks[l].key > key) {
            // cout << "BLOCK KEY " << blocks[j].key << " " << key << endl;
            swap(root->blocks[l + 1].blockNumber, root->blocks[l].blockNumber);
            swap(root->blocks[l + 1], root->blocks[l]);
            l--;
        }

        index = root->blocks.size()/2;
        

        BTreeNode* newRoot = new BTreeNode(false);
        // cout << "HELLO LEAF " << root->leafNode << endl;
        newRoot->blocks.push_back(root->blocks[index]);
        newRoot->size = root->blocks[index].size();
        BTreeNode* c1 = new BTreeNode(root->leafNode);
        BTreeNode* c2 = new BTreeNode(root->leafNode);

        for (int z = 0; z < root->blocks.size(); z++) {
            if (z < index) {
                c1->blocks.push_back(root->blocks[z]);
                c1->size += root->blocks[z].size();
                cout << "C1 : " << root->blocks[z].key << endl;
            } else if (z > index) {
                c2->blocks.push_back(root->blocks[z]);
                c2->size += root->blocks[z].size();

                cout << "C2 : " << root->blocks[z].key << endl;
            }
        }

        for (int z = 0; z < root->children.size(); z++) {
            if (z <= index) {
                c1->children.push_back(root->children[z]);
            } else if (z > index) {
                c2->children.push_back(root->children[z]);
            }
        }
        writePage(c1, this);
        writePage(c2, this);
        newRoot->children.push_back(c1->pageNumber);
        newRoot->children.push_back(c2->pageNumber);

        root = newRoot;
        updateFlush(root, this);
        // cout << " NEW ROOT PAGE NUMBER " << root->pageNumber << " New ROOT Chilren [0]: " << root->children[0]
        //  << " New ROOT Chilren [1]: " << root->children[1] << endl;
    }
    // cout << "INDEX AT TOP: " << index << "ROOT SIZE: " << root->size << endl;
    return root->pageNumber;
}

string Btree::search(string key) {
    BTreeNode* root = readPage(0, this);

    if (root == NULL) return "Key not found";
    return root->search(key, this);
}

void Btree::printTree(uint64_t rootPageNumber) {
    BTreeNode* root = readPage(rootPageNumber, this);
    // cout << "LYOO BHAI:" << root->children.size() << endl;
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

int BTreeNode::insertHelper(string key, Btree* btree) {
    if (leafNode) {
        // insert
        cout << "INSERTING: " << key << endl;
        int j = blocks.size() - 1;
        Block newData = Block{key, pageNumber : this->pageNumber, blockNumber : blocks.size()};
        if (size + newData.size() > PAGE_SIZE) {
            return blocks.size() / 2;
        }
        blocks.push_back(newData);

        while (j >= 0 && blocks[j].key > key) {
            // cout << "BLOCK KEY " << blocks[j].key << " " << key << endl;
            swap(blocks[j + 1].blockNumber, blocks[j].blockNumber);
            swap(blocks[j + 1], blocks[j]);
            j--;
        }
        cout << "j " << j + 2 << endl;
        size += blocks[j + 1].size();
        cout << "BLOCK KEY " << blocks[j + 1].key << " Block number " << blocks[j + 1].blockNumber << " SIZE: " << size << endl;
        updateFlush(this, btree);
        if (size > PAGE_SIZE) {
            return blocks.size() / 2;
        }

        return -1;

    } else {
        int j = 0;
        while (j < blocks.size() && blocks[j].key < key) {
            j++;
        }
        cout << "HALLOOO: " << j << endl;

        BTreeNode* childrenNode = readPage(children[j], btree);

        int index = childrenNode->insertHelper(key, btree);
        if (index != -1) {
            cout << "Children node : " << childrenNode->blocks[0].key << " VERYYY BAAD " << index << endl;
            Block newChildrenData=Block{key,pageNumber:childrenNode->pageNumber,blockNumber:childrenNode->blocks.size()};
            int l = childrenNode->blocks.size()-1;
            childrenNode->blocks.push_back(newChildrenData);
            while (l >= 0 && childrenNode->blocks[l].key > key) {
                // cout << "BLOCK KEY " << blocks[j].key << " " << key << endl;
                swap(childrenNode->blocks[l + 1].blockNumber, childrenNode->blocks[l].blockNumber);
                swap(childrenNode->blocks[l + 1], childrenNode->blocks[l]);
                l--;
            }
            cout << "PPRINTING BLOCK KEYS"<<endl;
            for(int b=0;b<childrenNode->blocks.size();b++){
                cout<<childrenNode->blocks[b].key<<" ";
            }
            cout <<endl;

            Block newData = Block{key : key, pageNumber : this->pageNumber, blockNumber : blocks.size()};
            index = childrenNode->blocks.size()/2;
            cout<<" INDIA: "<<index<<" AAES : "<<childrenNode->blocks[index].key<<endl;
            // l - > ndex to bubble up

            blocks.push_back(newData);
            children.push_back(childrenNode->pageNumber);
            size += childrenNode->blocks[index].size();
            for (int k = blocks.size() - 2; k >= j; k--) {
                // cout<<"K "<<k<<" J "<<j<<endl;
                blocks[k + 1].key = blocks[k].key;
                blocks[k + 1].pageNumber = blocks[k].pageNumber;
                blocks[k + 1].blockNumber = k + 1;

                // cout<<"KHEKEK"<<endl;
                children[k + 2] = children[k + 1];
            }

            // cout << "SIZE OF :" << size << endl;

            blocks[j].key = childrenNode->blocks[index].key;
            blocks[j].pageNumber = pageNumber;
            blocks[j].blockNumber = j;

            cout << "BROTHER KEY " << blocks[j].key << " j" << j << endl;
            BTreeNode* c1 = new BTreeNode(childrenNode->leafNode);
            BTreeNode* c2 = new BTreeNode(childrenNode->leafNode);

            for (int z = 0; z < childrenNode->blocks.size(); z++) {
                if (z < index) {
                    c1->blocks.push_back(childrenNode->blocks[z]);
                    cout << "C1: " << childrenNode->blocks[z].key << endl;
                    c1->size += childrenNode->blocks[z].size();
                } else if (z > index) {
                    c2->blocks.push_back(childrenNode->blocks[z]);
                    cout << "C2: " << childrenNode->blocks[z].key << endl;

                    c2->size += childrenNode->blocks[z].size();
                }
            }

            for (int z = 0; z < childrenNode->children.size(); z++) {
                if (z <= index) {

                    c1->children.push_back(childrenNode->children[z]);
                    cout << "C1 child: " << childrenNode->children[z] << endl;

                } else if (z > index) {
                    c2->children.push_back(childrenNode->children[z]);
                    cout << "C2 child: " << childrenNode->children[z] << endl;
                }
            }


            // cout << "HALLOOO" << endl;
            writePage(c1, btree);
            writePage(c2, btree);
                        cout<<"C1 < " <<c1->pageNumber<<endl;
            for (int y=0;y<c1->blocks.size();y++){
                cout<<c1->blocks[y].key<<" ";
            }
            cout<<endl;

                    cout<<"C2 < "<<c2->pageNumber<<endl;
            for (int y=0;y<c2->blocks.size();y++){
                cout<<c2->blocks[y].key<<" ";
            }
            cout<<endl;

            children[j] = c1->pageNumber;
            children[j + 1] = c2->pageNumber;

            if (size > PAGE_SIZE) {
                return blocks.size() / 2;
            } else {
                return -1;
            }
            updateFlush(this, btree);
            cout << size << endl;
        } else {
            return -1;
        }
    }
}

string BTreeNode::search(string key, Btree* btree) {
    int i = 0;
    // cout<<"SEARCH "<<" NODE SIZE "<<blocks.size()<<"  "<<endl;
    while (i < blocks.size() && key > blocks[i].key) i++;
    // cout<<"THIS IS I: "<<i<<endl;
    if (i < blocks.size() && blocks[i].key == key) {
        return to_string(blocks[i].pageNumber);
    }
    // cout << blocks[i].key << " ";
    if (leafNode) return "Key not found";
    BTreeNode* childrenNode = readPage(children[i], btree);
    return childrenNode->search(key, btree);
}
