#include "database.hpp"

#include <math.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <fstream> 

using namespace std;

const int MAX_PAGE_SIZE = 4096;

Database::Database(std::string database_name) {
    file.open(database_name + ".txt");

    if (file.is_open()) {

        std::cout << "DATABASE CREATED." << std::endl;
    } else {
        throw std::runtime_error("Failed to open the file: " + database_name + ".txt");
    }
}

void Database::insert(string key, string value) {
  if (root == NULL) {
    BTreeNode* newRoot = new BTreeNode(true);
    key_struct newData = key_struct({key, value});
    newRoot->nodes.push_back(newData);
    root = newRoot;
    root->size += newData.size();
    return;
  }

  int index = root->insertHelper({key, value});

  if (index != -1) {
    BTreeNode* newRoot = new BTreeNode(false);
    cout << "HELLO" << endl;
    newRoot->nodes.push_back(root->nodes[index]);
    newRoot->size=root->nodes[index].size();
    BTreeNode* c1 = new BTreeNode(root->leafNode);
    BTreeNode* c2 = new BTreeNode(root->leafNode);

    for (int z = 0; z < root->nodes.size(); z++) {
      if (z < index) {
        c1->nodes.push_back(root->nodes[z]);
        c1->size+=root->nodes[z].size();
        cout << "C1 : " << root->nodes[z].key << endl;
      } else if (z > index) {
        c2->nodes.push_back(root->nodes[z]);
                c2->size+=root->nodes[z].size();

        cout << "C2 : " << root->nodes[z].key << endl;
      }
    }

    for (int z = 0; z < root->children.size(); z++) {
      if (z <= index) {
        c1->children.push_back(root->children[z]);
      } else if (z > index) {
        c2->children.push_back(root->children[z]);
      }
    }

    newRoot->children.push_back(c1);
    newRoot->children.push_back(c2);
    
    root = newRoot;


  }
      cout << "INDEX AT TOP: " << index<<"ROOT SIZE: "<<root->size << endl;
}

string Database::search(string key) {
  if (root == NULL) return "Key not found";
  return root->search(key);
}

void Database::printTree() {
  std::queue<BTreeNode*> q;
  int size = 1;
  q.push(root);
  while (!q.empty()) {
    int newSize = 0;
    for (int i = 0; i < size; i++) {
      BTreeNode* newNode = q.front();
      q.pop();
      cout << "   [ ";
      for (auto i : newNode->nodes) {
        cout << i.key << ",";
      }
      cout << "]   ";

      for (auto c : newNode->children) {
        q.push(c);
        newSize += 1;
      }
    }
    size = newSize;
    cout << endl;
  }
}

int BTreeNode::insertHelper(key_struct data) {
  if (leafNode) {
    // insert
    int j = nodes.size() - 1;
    nodes.push_back(data);
    while (j >= 0 && nodes[j].key > data.key) {
      swap(nodes[j + 1], nodes[j]);
      j--;
    }
    size += data.size();
    cout << "SIZE: " << size << endl;
    if (size > MAX_PAGE_SIZE) {
      return nodes.size() / 2;
    }

    return -1;

  } else {
    int j = 0;
    while (j < nodes.size() && nodes[j].key < data.key) {
      j++;
    }
    cout << "HALLOOO: "<<j << endl;
    int index = children[j]->insertHelper(data);
    if (index != -1) {
      cout<<"VERYYY BAAD "<<index<<endl;
      nodes.push_back(key_struct());
      children.push_back(new BTreeNode(children[j]->leafNode));
      size+=children[j]->nodes[index].size();
      for (int k = nodes.size() - 2; k >= j; k--) {
        // cout<<"K "<<k<<" J "<<j<<endl;
        nodes[k + 1].key = nodes[k].key;
        nodes[k + 1].value = nodes[k].value;
      // cout<<"KHEKEK"<<endl;
        children[k + 2] = children[k + 1];
      }

      cout<<"SIZE OF :"<<size<<endl;


      nodes[j].key = children[j]->nodes[index].key;
      nodes[j].value = children[j]->nodes[index].value;
      BTreeNode* c1 = new BTreeNode(children[j]->leafNode);
      BTreeNode* c2 = new BTreeNode(children[j]->leafNode);

      for (int z = 0; z < children[j]->nodes.size(); z++) {
        if (z < index) {
          c1->nodes.push_back(children[j]->nodes[z]);
          c1->size+=children[j]->nodes[z].size();
        } else if (z > index) {
          c2->nodes.push_back(children[j]->nodes[z]);
            c2->size+=children[j]->nodes[z].size();

        }
      }

      for (int z = 0; z < children[j]->children.size(); z++) {
        if (z <= index) {
          c1->children.push_back(children[j]->children[z]);
        } else if (z > index) {
          c2->children.push_back(children[j]->children[z]);
        }
      }

    cout<<"HALLOOO"<<endl;
      children[j] = c1;
      children[j + 1] = c2;
      if (size >MAX_PAGE_SIZE){
        return nodes.size()/2;
      }
      else{
        return -1;
      }
    }
    else{
      return -1;
    }
  }
}

void BTreeNode::splitChild(int i, BTreeNode* c) {
  BTreeNode* c1 = new BTreeNode(c->leafNode);
  BTreeNode* c2 = new BTreeNode(c->leafNode);
  // cout << "Node size:" << nodes.size() << endl;
  // cout << "CHildren size:" << children.size() << endl;
  key_struct obj;
  obj.key = "";
  obj.value = "";
  nodes.push_back(obj);

  children.push_back(new BTreeNode(false));
  // cout << "JASSAD : " <<i<< endl;

  if (nodes.size() > 1) {
    for (int j = nodes.size() - 1; j > i; j--) {
      nodes[j].key = nodes[j - 1].key;
      nodes[j].value = nodes[j - 1].value;
      children[j + 1] = children[j];
    }
  }

  //   cout << "are we good now uwu" << endl;

  int j_size = 0;
  int j = 0;
  int half = c->nodes.size() / 2;
  cout << "HAF: " << half << endl;
  // coppy halk
  while (j < half && j < c->nodes.size()) {
    c1->nodes.push_back(c->nodes[j]);
    c1->size += nodes[j].value.size();
    j_size += c->nodes[j].size();
    j++;
  }
  //   cout << "OYEEEE " <<j<< endl;
  key_struct bubbleUpValue = c->nodes[j];
  j++;
  while (j < c->nodes.size()) {
    c2->nodes.push_back(c->nodes[j]);
    c2->size += c->nodes[j].size();

    j++;
  }
  //   cout << "bacha le re bababa" << endl;

  if (!c->leafNode) {
    for (int k = 0; k <= c1->nodes.size(); k++) {
      c1->children.push_back(c->children[k]);
    }
    for (int k = c1->nodes.size() + 1; k <= c->nodes.size(); k++) {
      c2->children.push_back(c->children[k]);
    }
  }
  //   cout << "copium" << endl;

  //   if (c2->nodes.size() == 0) {
  //     cout << "KIthe: " << c1->children.size() << c2->children.size() <<
  //     endl; if (c1->leafNode == false) {
  //       c2->children.push_back(c1->children.back());
  //     }
  //     cout << "pathe" << endl;

  //     cout<<"BUBBLE VALUE: " << bubbleUpValue.key<<endl;
  //     c2->nodes.push_back(bubbleUpValue);
  //     cout<<c2->nodes.size()<<endl;
  //     bubbleUpValue = c1->nodes.back();
  //     c1->nodes.pop_back();
  //     if (c1->leafNode == false) {
  //       c1->children.pop_back();
  //     }
  //   }

  children[i] = c1;
  children[i + 1] = c2;

  cout << "HEALLLL " << c2->children.size() << endl;

  nodes[i] = bubbleUpValue;

  size = bubbleUpValue.size() + size;
}

string BTreeNode::search(string key) {
  int i = 0;
  // cout<<"SEARCH "<<" NODE SIZE "<<nodes.size()<<"  "<<endl;
  while (i < nodes.size() && key > nodes[i].key) i++;
  // cout<<"THIS IS I: "<<i<<endl;
  if (i < nodes.size() && nodes[i].key == key) return nodes[i].value;
  // cout << nodes[i].key << " ";
  if (leafNode) return "Key not found";
  return children[i]->search(key);
}
