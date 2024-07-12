
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "../storage/btree.cpp"
#include "../include/database.hpp"
#include "../include/table.hpp"
#include "../include/globals.hpp"


using namespace std;



Table::Table(string table_name, vector<string> types, vector<string> names,string database_name,fstream* data_file,fstream* page_file) {
    if (types.size() != names.size()) {
        return;
    }
       
    for (size_t i = 0; i < types.size(); ++i) {
        columns.push_back({names[i], types[i]});
    }
     string primaryIndexFile = getIndexFilePath(database_name,table_name,"primary");
    fstream *index_file=new fstream();
    indexes.push_back(index_file);
    // Debugging output
    index_file->open(primaryIndexFile, ios::out | ios::trunc| ios::in);
    
    this->data_file=data_file;
    this->page_file=page_file;
    // Ensure directories exist
    
    this->btree=new Btree(index_file);

    
}

void Table::Insert( vector<string> args) {
    if (args.size() != columns.size()) {
        cout << "ERROR : args size not equal to column data";
        return;
    }
    
    btree->insert(args[0],args[1]);




}

Block Table::writeData(string key,string value){

   string page_data;
    int lastpage=0;
    while (getline(*page_file,page_data)) {
        // Process each line here (e.g., print it)
        

    }
    writeValueToDataFile(value,lastpage,data_file,true);
}

int writeValueToDataFile(string val,uint64_t pageNumber,fstream* data_file,bool newPage){
    int pageOffset=pageNumber*PAGE_SIZE;
    data_file->seekp(pageOffset,ios::beg);
    char* buffer[PAGE_SIZE];
    uint16_t prevNumberOfNodes=0;
    if(!newPage){
        memcpy(&prevNumberOfNodes,buffer+sizeof(uint16_t),sizeof(uint16_t));

    }
    uint32_t headerSize=(3)*sizeof(uint16_t);
    uint16_t begOffset=headerSize+(prevNumberOfNodes*sizeof(uint16_t));
    uint16_t endOffset=PAGE_SIZE;

    uint16_t blockOffset=endOffset-PAGE_SIZE;

    memcpy(buffer,&prevNumberOfNodes+1,sizeof(prevNumberOfNodes));
    size_t offset=sizeof(prevNumberOfNodes);

    memcpy(buffer+offset,&begOffset,sizeof(begOffset));
    size_t offset=sizeof(begOffset);

    memcpy(buffer+offset,&begOffset,sizeof(endOffset));
    size_t offset=sizeof(endOffset);

    memcpy(buffer+begOffset,&blockOffset,sizeof(blockOffset));


    memcpy(buffer+endOffset-val.size(),&val,val.size());



    



}
// void readMetadata

void Table::Print(){
    btree->printTree(this->rootPageNumber);
}


string Table::Search(string key){
    return btree->search(key);

}



