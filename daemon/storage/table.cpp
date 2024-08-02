
#include "./include/table.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include "../storage/btree.cpp"
#include "./include/database.hpp"
#include "./index.cpp"
#include "../cli-table-cpp/src/Table.hpp"

using namespace std;
struct PageMap {
    uint64_t pageNumber;
    uint16_t availableSpace;
    uint16_t noOfBlocksAvailable;
    const int size() { return sizeof(pageNumber) + sizeof(availableSpace) + sizeof(noOfBlocksAvailable); }
    const void writeToFile(fstream* page_file) {
        char buffer[size()];
        // cout << "WRITE OFFSET:" << pageNumber * size() << endl;
        page_file->seekp(pageNumber * size());
        memcpy(buffer, &pageNumber, sizeof(pageNumber));
        size_t offset = sizeof(pageNumber);
        memcpy(buffer + offset, &availableSpace, sizeof(availableSpace));
        offset += sizeof(availableSpace);
        memcpy(buffer + offset, &noOfBlocksAvailable, sizeof(noOfBlocksAvailable));
        offset += sizeof(noOfBlocksAvailable);
        page_file->write(buffer, size());
        page_file->flush();
    }
    const void readFromFile(uint64_t pageNumber, fstream* page_file) {
        page_file->seekg(pageNumber * size(), ios::beg);
        char buffer[size()];
        page_file->read(buffer, size());
        memcpy(&this->pageNumber, buffer, sizeof(this->pageNumber));
        size_t offset = sizeof(this->pageNumber);
        memcpy(&this->availableSpace, buffer + offset, sizeof(this->availableSpace));
        offset += sizeof(this->availableSpace);
        memcpy(&this->noOfBlocksAvailable, buffer + offset, sizeof(this->noOfBlocksAvailable));
        offset += sizeof(this->noOfBlocksAvailable);
    }
};
Table::Table(string table_name, vector<string> types, vector<string> names, Database* database, fstream* data_file, fstream* page_file,
             int primary_key_index) {
    if (types.size() != names.size()) {
        return;
    }

    for (size_t i = 0; i < types.size(); ++i) {
        this->columns.push_back({names[i], types[i]});
        cout<<"column: "<<columns[i].name<<" "<<columns[i].type<<endl;
    }
    std::filesystem::path tableDir = BASE_DIRECTORY + "/" + database->name + "/" + table_name;
    if (!std::filesystem::exists(tableDir)) {
        std::filesystem::create_directories(tableDir);
    }

    Index* index = new Index(database->name, table_name, columns[primary_key_index].name);

    indexes.push_back(index);

    // Debugging output
    this->table_name = table_name;
    this->database = database;

    this->data_file = data_file;
    this->page_file = page_file;
    // Ensure directories exist
}
bool parseArgument(const string& arg, const string& type) {
    if (type == "string") {
        // No parsing needed for strings, always valid
        return true;
    } else if (type == "int") {
        try {
            stoi(arg);
            return true;
        } catch (invalid_argument& e) {
            return false;
        } catch (out_of_range& e) {
            return false;
        }
    } else if (type == "float") {
        try {
            stof(arg);
            return true;

        } catch (invalid_argument& e) {
            return false;
        } catch (out_of_range& e) {
            return false;
        }
    } else if (type == "bool") {
        if (arg == "true" || arg == "false" || arg == "1" || arg == "0") {
            return true;
        } else {
            return false;
        }
    }
    return false;  // Unknown type
}
vector<pair<vector<string>,pair<uint64_t,uint16_t>>> Table::RangeQuery(string* key1, string* key2, vector<Column> types, uint64_t transaction_id, bool includeKey1,
                                         bool includeKey2, string column_name = "") {
    vector<pair<vector<string>,pair<uint64_t,uint16_t>>> transactionVisibleRows;
    cout<<"CRAZYY"<<endl;

    vector<pair<vector<string>, pair<uint64_t, uint64_t>>> rows;
    vector<pair<uint64_t,uint8_t>> pageAndBlockNumbers;
    Index* index = this->getIndex(column_name);
    cout<<"THING"<<endl;
    if (!index) {
        return transactionVisibleRows;
    }
    // cout<<"KEY: "<<*key1<<endl;

    pair<BTreeNode*, optional<Block>> SearchResult1 = (!key1) ? index->btree->beg() : index->btree->search(*key1);
    BTreeNode* currentNode = SearchResult1.first;
    optional<Block> optData = SearchResult1.second;

    if (!optData.has_value()) {
        cout << "KEY NOT FOUND" << endl;
        return transactionVisibleRows;
    }
    if (currentNode == NULL) {
        cout << "EMPTY";
        return transactionVisibleRows;
    }
    Block data = optData.value();
    string key = data.key;
    uint64_t pgNumber = data.pageNumber.value();
    uint16_t blNumber = data.blockNumber.value();
    int i = 0;
    // cout<<"hello"<<endl;
    while (key1 && i < currentNode->blocks.size() && currentNode->blocks[i].key < *key1) {
        i++;
    }

    // cout<<(currentNode->nextSibling!=-1 || (currentNode->nextSibling==-1 && i<currentNode->blocks.size()))<<endl;
    while ((key2) ? (key <= *key2) : (currentNode->nextSibling != -1 || (currentNode->nextSibling == -1 && i < currentNode->blocks.size()))) {
        blNumber = currentNode->blocks[i].blockNumber.value();
        // cout<<"BL NUMBER HEHEHE : "<<blNumber<<endl;
        pgNumber = currentNode->blocks[i].pageNumber.value();
        pair<optional<string>, pair<uint64_t, uint64_t>> read_value_data = readValue(pgNumber, blNumber);
        optional<string> foundValue = read_value_data.first;
        uint64_t t_ins = read_value_data.second.first;
        uint64_t t_del = read_value_data.second.second;

        // cout << "FOUND VALUE :" << t_ins << " " << t_del << endl;
        vector<string> rowData = this->Deconstruct(foundValue.value(), types);
        // cout << "ROW DATA:" << rowData[0] << endl;
        // if(){
        // if(tx->IsVisible(t_ins,t_del)){
        // }
        // cout<<"HELLOOOO: "<< currentNode->blocks.size()<<endl;
        if (key1 && key == *key1 && includeKey1) {
            rows.push_back({rowData, {t_ins, t_del}});
            pageAndBlockNumbers.push_back({pgNumber,blNumber});
        } else if (key2 && key == *key2 && includeKey2) {
            rows.push_back({rowData, {t_ins, t_del}});
            pageAndBlockNumbers.push_back({pgNumber,blNumber});

        } else if (!key1 || !key2) {
            rows.push_back({rowData, {t_ins, t_del}});
            pageAndBlockNumbers.push_back({pgNumber,blNumber});

        }
        // cout << "HELLO " << rows.size() << endl;

        if ((i + 1) < currentNode->blocks.size()) {
            i = i + 1;
        } else {
            if (currentNode->nextSibling == -1) {
                break;
            }
            // cout<<"NEXT SIBLING: "<<currentNode->nextSibling<<endl;
            BTreeNode* nextNode = index->btree->readPage(currentNode->nextSibling);
            // cout<<"AVVVEEEE: "<<nextNode->blocks[0].key<<endl;
            i = 0;
            // cout<<"AVVEE BHAIII :"<<nextNode->pageNumber<<endl;
            currentNode = nextNode;
        }
        key = currentNode->blocks[i].key;
    }

    cout<<"ROWS: "<<rows.size()<<endl;
    cout<<"transaction_id "<<transaction_id<<endl;
    database->PrintAllTransactions();
    for (int i=0;i<rows.size();i++) {
        auto row=rows[i];
            for(auto item:row.first){
                cout<<item<<" ";
            }
            cout<<endl;
        // cout<<"row first : "<<row.first[1]<<endl;
        if (database->IsVisible(row.second.first, row.second.second, transaction_id)) {
            cout<<"ISVISIBLE"<<endl;
            transactionVisibleRows.push_back({row.first,pageAndBlockNumbers[i]});
        }
        else{
            cout<<"is not visible"<<endl;
        }
    }

    // CliTable::Options opt;
    // // Contructing the table structure
    // CliTable::TableBody content;

    // for (auto row : transactionVisibleRows) {
    //     content.push_back(row.first);
    // }
    // CliTable::Table printTable(opt, content);

    //  printTable.generate();
    

    return transactionVisibleRows;
}

void Table::Insert(vector<string> args, uint64_t transaction_id, fstream* wal_file) {
    cout << "LES F GOO" << endl;
    cout<<columns.size()<<endl;
    if (args.size() != columns.size()) {
        cout << "COLUMNS SIZE: " << columns.size() << " ARgs Size: " << args.size() << endl;
        cout << "ERROR : args size not equal to column data";
        return;
    }
    cout<<"REALLY???"<<endl;

    // type checking
  
    string key = args[this->primary_key_index];

    vector<pair<vector<string>,pair<uint64_t,uint16_t>>> foundRows = this->RangeQuery(&key, &key, this->columns,transaction_id, true, true);
    cout << "ENDL: " << args[this->primary_key_index] << endl;
  
    if (foundRows.size() > 0) {
        cout << "ROW ALREADY EXISTS" << endl;
        return;
    }
    for (int i = 0; i < args.size(); i++) {
        string arg = args[i];
        string type = columns[i].type;
            cout<<"RIGHTT??"<<i<<endl;
        bool isValid = parseArgument(arg, type);
        if (!isValid) {
            cerr << "Error: Argument " << arg << " cannot be parsed as type " << type << endl;
            return;
        }
    }
    stringstream ss;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i != 0) {
            ss << ",";
        }
        ss << args[i];
    }
    string joinedArgs = ss.str();
    cout<<"WE DONEEE"<<endl;
    Block newData = writeData(args[primary_key_index], joinedArgs, transaction_id);
    // cout<<"key: "<<newData.key<<"BLOCK: " <<newData.blockNumber.value()<<endl;

    for (auto index : indexes) {
        cout << "INSERTING: " << args[1] << endl;
        index->btree->insert(newData);
        // pair<BTreeNode*, optional<Block>> SearchResult1 =index->btree->search(newData.key);
        // cout<<SearchResult1.second.value().key<<endl;
    }
    cout<<"table insert done"<<endl;
    // if (wal_file) {
    //     WAL wal(OPERATION::INSERT, transaction_id, &newData.key, &joinedArgs);
    //     wal.write(wal_file);
    // }
}

MetadataDataPage readMetadata(char buffer[PAGE_SIZE]) {
    MetadataDataPage metadata;
    memcpy(&metadata.noOfBlocks, buffer, sizeof(metadata.noOfBlocks));
    size_t offset = sizeof(metadata.noOfBlocks);

    memcpy(&metadata.begOffset, buffer + offset, sizeof(metadata.begOffset));
    offset += sizeof(metadata.begOffset);

    memcpy(&metadata.endOffset, buffer + offset, sizeof(metadata.endOffset));

    return metadata;
}

int insertValueToDataFile(string val, uint64_t pageNumber, fstream* data_file, fstream* page_file, bool newPage, uint64_t transaction_id) {
    int pageOffset = pageNumber * PAGE_SIZE;
    char buffer[PAGE_SIZE] = {0};
    uint32_t headerSize = 3 * sizeof(uint16_t);
    MetadataDataPage metadata = {0, headerSize, PAGE_SIZE};

    if (!newPage) {
        data_file->seekg(pageOffset, ios::beg);
        data_file->read(buffer, PAGE_SIZE);
        metadata = readMetadata(buffer);
    }
    data_file->seekp(0, ios::beg);
    // cout<<"BegOffset: "<<metadata.begOffset<<"END OFFSET "<<metadata.endOffset<<endl;
    uint16_t newBegOffset = metadata.begOffset + (Block_HEADER_SIZE);
    uint16_t newEndOffset = metadata.endOffset - val.size();
    // cout<<"newBeg offset: "<<newBegOffset<<"  NEW END OFFSET: "<<newEndOffset<<endl;

    if (newEndOffset <= newBegOffset) {
        return -1;  // Not enough space
    }

    uint16_t blockOffset = newEndOffset;
    // cout << "WRITE: BLOCK OFFSET block : " << metadata.noOfBlocks << " is : " << blockOffset << endl;

    metadata.noOfBlocks++;
    memcpy(buffer, &metadata.noOfBlocks, sizeof(metadata.noOfBlocks));
    memcpy(buffer + sizeof(uint16_t), &newBegOffset, sizeof(newBegOffset));
    memcpy(buffer + sizeof(uint16_t) * 2, &newEndOffset, sizeof(newEndOffset));
    uint64_t zero = 0;

    int valueCellSize = val.size();
    // cout << "value cell size: " << valueCellSize << endl;
    // Copy the block offset for the new entry
    size_t offset = 0;
    memcpy(buffer + headerSize + (metadata.noOfBlocks - 1) * (Block_HEADER_SIZE), &blockOffset, sizeof(blockOffset));
    offset += sizeof(blockOffset);
    memcpy(buffer + headerSize + ((metadata.noOfBlocks - 1) * (Block_HEADER_SIZE)) + offset, &valueCellSize, uint16_t(val.size()));
    offset += sizeof(uint16_t(val.size()));
    // cout << "CAS: " << (headerSize + ((metadata.noOfBlocks - 1) * (Block_HEADER_SIZE)) + offset) << endl;
    memcpy(buffer + headerSize + ((metadata.noOfBlocks - 1) * (Block_HEADER_SIZE)) + offset, &transaction_id, sizeof(transaction_id));
    offset += sizeof(transaction_id);
    memcpy(buffer + headerSize + ((metadata.noOfBlocks - 1) * (Block_HEADER_SIZE)) + offset, &zero, sizeof(zero));

    // Copy the actual value into the page
    memcpy(buffer + blockOffset, val.c_str(), val.size());

    // Write the updated page back to the file
    data_file->clear();
    data_file->seekp(pageOffset, ios::beg);
    data_file->write(buffer, PAGE_SIZE);
    data_file->flush();

    PageMap newPageMapping{.pageNumber = pageNumber, .availableSpace = (newEndOffset - newBegOffset), .noOfBlocksAvailable = 0

    };
    newPageMapping.writeToFile(page_file);
    // cout << "EXPECTED: pageNumber: " << pageNumber << " Available spve: " << newEndOffset - newBegOffset
    //      << "  no of blocks : " << 0 << endl;
    // PageMap checkMapping;
    // checkMapping.readFromFile(pageNumber, page_file);
    // cout << "ACTIUAL: pageNumber: " << checkMapping.pageNumber << " Available spve: " << checkMapping.availableSpace
    //      << "  no of blocks : " << checkMapping.noOfBlocksAvailable << endl;
    // ;

    return metadata.noOfBlocks - 1;
}

int updateValueInDataFile(string val, uint64_t pageNumber, uint16_t blockNumber, fstream* data_file, fstream* page_file, uint64_t transaction_id) {
    int pageOffset = pageNumber * PAGE_SIZE;
    char buffer[PAGE_SIZE];

    // Read the page data
    data_file->seekg(pageOffset, ios::beg);
    data_file->read(buffer, PAGE_SIZE);

    MetadataDataPage metadata = readMetadata(buffer);
    if (blockNumber >= metadata.noOfBlocks) {
        return -1;  // Invalid block number
    }

    uint16_t blockOffset;
    memcpy(&blockOffset, buffer + metadata.begOffset + blockNumber * sizeof(uint16_t), sizeof(blockOffset));

    uint16_t nextBlockOffset;
    if (blockNumber == metadata.noOfBlocks - 1) {
        nextBlockOffset = PAGE_SIZE;
    } else {
        memcpy(&nextBlockOffset, buffer + metadata.begOffset + (blockNumber + 1) * sizeof(uint16_t), sizeof(nextBlockOffset));
    }

    uint16_t oldValueSize = nextBlockOffset - blockOffset;

    // Remove the old value by shifting the blocks above it
    memmove(buffer + blockOffset, buffer + nextBlockOffset, PAGE_SIZE - nextBlockOffset);

    // Update the metadata
    uint16_t newEndOffset = metadata.endOffset + oldValueSize;
    metadata.noOfBlocks--;
    memcpy(buffer, &metadata.noOfBlocks, sizeof(metadata.noOfBlocks));
    memcpy(buffer + sizeof(uint16_t), &metadata.begOffset, sizeof(metadata.begOffset));
    memcpy(buffer + sizeof(uint16_t) * 2, &newEndOffset, sizeof(newEndOffset));

    // Write the updated page back to the file
    data_file->seekp(pageOffset, ios::beg);
    data_file->write(buffer, PAGE_SIZE);

    // Insert the new value
    return insertValueToDataFile(val, pageNumber, data_file, page_file, false, transaction_id);
}

Block Table::writeData(string key, string value, uint64_t transaction_id) {
    string page_data;
    uint64_t pageNumber = 0;
    bool newPage = true;
    page_file->seekg(0, ios::beg);

    while (true) {
        PageMap pagemap;
        int offset = pageNumber * pagemap.size();
        page_file->seekg(0, ios::end);
        std::streampos endPos = page_file->tellg();
        // cout<<"ENDPOS:  "<<endPos<<endl;
        if (offset >= endPos) {
            break;
        }
        pagemap.readFromFile(pageNumber, page_file);
        if (pagemap.availableSpace > value.size() + (2 * sizeof(uint16_t))) {
            newPage = false;
            break;
        }
        pageNumber++;
    }
    //     cout << "INSERTING: " << key << endl;
    //     cout << "PWGEFE:  " << pageNumber << endl;
    // cout<<"newPage "<<newPage<<endl;
    int blockNumber = insertValueToDataFile(value, pageNumber, data_file, page_file, newPage, transaction_id);
    if (blockNumber == -1) {
        // cout << "FFF" << endl;
        return Block{.key = "asdsdsd"};
    }

    return Block{
        .key = key,
        .pageNumber = pageNumber,
        .blockNumber = blockNumber,
    };
}

void deleteData(uint64_t pageNumber, uint16_t blockNumber, fstream* data_file, fstream* page_file, uint64_t transaction_id) {
    char buffer[PAGE_SIZE];
    uint32_t headerSize = 3 * sizeof(uint16_t);

    int pageOffset = pageNumber * PAGE_SIZE;
    data_file->seekg(pageOffset, ios::beg);
    data_file->read(buffer, PAGE_SIZE);
    MetadataDataPage metadata = readMetadata(buffer);
    int ogNoOfBlocks = metadata.noOfBlocks;
    int ogEndOffset = metadata.endOffset;
    // char newBuffer[PAGE_SIZE];
    // cout << "DELETE : " << headerSize + (blockNumber * Block_HEADER_SIZE) + ((2 * sizeof(uint16_t)) + sizeof(uint64_t)) << endl;

    memcpy(buffer + headerSize + (blockNumber * Block_HEADER_SIZE) + ((2 * sizeof(uint16_t)) + sizeof(uint64_t)), &transaction_id,
           sizeof(transaction_id));

    // cout << "BLOCK NUMBER: " << blockNumber << endl;
    // for (int i = 0; i < ogNoOfBlocks; i++) {
    //     // get blockOffset
    //     // cout << "pepo pog i: " << i << endl;
    //     uint16_t blockOffset;
    //     memcpy(&blockOffset, buffer + headerSize + ((2 * sizeof(uint16_t)) * i), sizeof(blockOffset));
    //     uint16_t sizeOfValueCell;
    //     memcpy(&sizeOfValueCell, buffer + headerSize + ((2 * sizeof(uint16_t)) * i) + (sizeof(uint16_t)), sizeof(sizeOfValueCell));
    //     // cout << "size of value: " << sizeOfValueCell << endl;

    //     // cout << "DELETE for "<<i<<"  BLOCK OFFSET: " << metadata.endOffset - sizeOfValueCell << "  J :" << endl;
    //     if (i != blockNumber) {
    //         uint16_t newBlockOffset = metadata.endOffset - sizeOfValueCell;
    //         memcpy(newBuffer + headerSize + (2 * sizeof(uint16_t) * i), &newBlockOffset, sizeof(uint16_t));
    //         memcpy(newBuffer + headerSize + (2 * sizeof(uint16_t) * i) + sizeof(uint16_t), &sizeOfValueCell, sizeof(uint16_t));

    //     } else {
    //         uint16_t zero = 0;
    //         memcpy(newBuffer + headerSize + (2 * sizeof(uint16_t) * i), &zero, sizeof(uint16_t));
    //         memcpy(newBuffer + headerSize + (2 * sizeof(uint16_t) * i) + sizeof(uint16_t), &zero, sizeof(uint16_t));
    //     }

    //     if (i == blockNumber) {
    //         continue;
    //     }
    //     // cout << "newBuff: " << (metadata.endOffset - sizeOfValueCell) << endl;
    //     memcpy(newBuffer + (metadata.endOffset - sizeOfValueCell), buffer + blockOffset, sizeOfValueCell);
    //     metadata.begOffset += sizeof(uint16_t);
    //     metadata.endOffset -= sizeOfValueCell;

    //     // memcpy(&nextBlockOffset, buffer + headerSize + (sizeof(uint16_t) * (blockNumber + 1)), sizeof(nextBlockOffset));
    //     // uint16_t sizeOfNextValue=blockOffset-nextBlockOffset;
    //     // memcpy(&buffer+prevBlockOffset-sizeOfNextValue,buffer+nextBlockOffset,sizeOfNextValue);

    //     // //moving value
    //     // memcpy(&buffer+blockNumber,buffer+nextBlockOffset,blockOffset-nextBlockOffset);

    //     // memcpy(&buffer+headerSize+(sizeof(uint16_t)*blockNumber), buffer + headerSize + (sizeof(uint16_t) * blockNumber+1),
    //     sizeof(blockOffset));
    // }
    // cout << "no of blocks:" << metadata.noOfBlocks << " og blocks: " << ogNoOfBlocks << endl;
    // memcpy(buffer, &metadata.noOfBlocks, sizeof(metadata.noOfBlocks));
    // memcpy(buffer + sizeof(uint16_t), &metadata.begOffset, sizeof(metadata.begOffset));
    // memcpy(newBuffer + sizeof(uint16_t) * 2, &metadata.endOffset, sizeof(metadata.endOffset));
    data_file->clear();
    data_file->seekp(pageOffset, ios::beg);
    data_file->write(buffer, PAGE_SIZE);
    data_file->flush();
    page_file->clear();

    // PageMap pageMap;
    // pageMap.readFromFile(pageNumber, page_file);
    // pageMap.availableSpace = metadata.endOffset - metadata.begOffset;
    // pageMap.noOfBlocksAvailable += 2 * sizeof(uint16_t);
    // pageMap.writeToFile(page_file);
}

pair<optional<string>, pair<uint64_t, uint64_t>> Table::readValue(uint64_t pageNumber, uint16_t blockNumber) {
    char buffer[PAGE_SIZE];
    int pageOffset = pageNumber * PAGE_SIZE;
    data_file->seekg(pageOffset, ios::beg);
    data_file->read(buffer, PAGE_SIZE);
    MetadataDataPage metadata = readMetadata(buffer);
    int headerSize = metadata.size();
    // cout << "BLOCK NUMBER : " << blockNumber << " " << metadata.begOffset << " " << metadata.noOfBlocks << " " << metadata.endOffset << endl;
    if (blockNumber >= metadata.noOfBlocks) {
        return {nullopt, {0, 0}};  // Block number out of range
    }
    size_t offset = 0;
    uint16_t blockOffset;
    memcpy(&blockOffset, buffer + headerSize + (Block_HEADER_SIZE * blockNumber), sizeof(blockOffset));
    // cout << "READ: BLOCK OFFSET block : " << blockNumber << " is : " << blockOffset << endl;
    if (blockOffset == 0) {
        return {nullopt, {0, 0}};
    }
    offset += sizeof(blockOffset);
    uint16_t sizeOfValueCell;

    memcpy(&sizeOfValueCell, buffer + headerSize + (Block_HEADER_SIZE * blockNumber) + offset, sizeof(uint16_t));
    // cout << "VAL SIZE " << sizeOfValueCell << endl;
    offset += sizeof(sizeOfValueCell);
    uint64_t t_ins, t_del;
    memcpy(&t_ins, buffer + headerSize + (Block_HEADER_SIZE * blockNumber) + offset, sizeof(t_ins));
    // cout << "T INS : " << headerSize + (Block_HEADER_SIZE * blockNumber) + offset << endl;
    offset += sizeof(t_ins);
    memcpy(&t_del, buffer + headerSize + (Block_HEADER_SIZE * blockNumber) + offset, sizeof(t_del));

    string value(sizeOfValueCell, '\0');
    memcpy(&value[0], buffer + blockOffset, sizeOfValueCell);

    return {value, {t_ins, t_del}};
}

void Table::Print(string column_name) {
    Index* index = this->getIndex(column_name);
    if (!index) {
        return;
    }
    index->btree->printTree(this->rootPageNumber);
}

vector<string> Table::Deconstruct(string row, vector<Column> types) {
    unordered_map<string, string> rowData;
    string column_data;

    vector<string> rowValues = splitString(row, ',');
    for (int j = 0; j < columns.size(); j++) {
        // cout<<"name: "<<columns[j].name<<" VALue: "<<rowValues[j]<<endl;
        rowData[columns[j].name] = rowValues[j];
    }
    vector<string> data;
    // cout<<"DELUSTIONAL: "<<types.size()<<endl;
    for (int i = 0; i < types.size(); i++) {
        Column type = types[i];
        data.push_back(rowData[type.name]);
    }

    return data;
}

vector<string> Table::Deconstruct(vector<string> rowValues, vector<Column> types) {
    unordered_map<string, string> rowData;
    string column_data;

    for (int j = 0; j < columns.size(); j++) {
        // cout<<"name: "<<columns[j].name<<" VALue: "<<rowValues[j]<<endl;
        rowData[columns[j].name] = rowValues[j];
    }
    vector<string> data;
    // cout<<"DELUSTIONAL: "<<types.size()<<endl;
    for (int i = 0; i < types.size(); i++) {
        Column type = types[i];
        data.push_back(rowData[type.name]);
    }

    return data;
}

Index* Table::getIndex(string column_name) {
    Index* index;

    if (column_name.empty()) {
        column_name = columns[primary_key_index].name;
    }
    for (auto ind : indexes) {
        if (ind->columnName == column_name) {
            index = ind;
            break;
        }
    }

    if (!index) {
        for (auto ind : indexes) {
            if (ind->columnName == columns[primary_key_index].name) {
                index = ind;
                break;
            }
        }
    }

    if (!index) {
        cout << "ERROR: index not found";
        return index;
    }

    return index;
}

string Table::Search(string key, string column_name, uint64_t transaction_id) {
    Index* index = this->getIndex(column_name);
    if (!index) {
        return "";
    }

    pair<BTreeNode*, optional<Block>> SearchData = index->btree->search(key);
    optional<Block> optData = SearchData.second;
    // cout<<"KEY: "<<key<<" PAGE NUMBER: " << optData.value().pageNumber.value()<<" BLOCK NUMBER: "<<optData.value().blockNumber.value()<<endl;
    if (optData.has_value()) {
        Block data = optData.value();
        // cout << "STRANGER: " << data.pageNumber.value() << endl;

        // BTreeNode* startNode=btree->readPage(data.pageNumber.value());
        // cout << "KEY:: " << key << "  MEDDIATORS:  " << SearchData.first->nextSibling << endl;

        // cout<<"key : "<<key<<data.pageNumber.value()<<data.blockNumber.value()<<endl;
        pair<optional<string>, pair<uint64_t, uint64_t>> read_value_data = readValue(data.pageNumber.value(), data.blockNumber.value());
        optional<string> foundValue = read_value_data.first;
        uint64_t t_ins = read_value_data.second.first;
        uint64_t t_del = read_value_data.second.second;
        if (foundValue.has_value()) {
            return foundValue.value();
        }

        return "KEY NOT FOUND";
    } else {
        return "KEY NOT FOUND";
    }
}

void Table::Update(vector<string> args,uint64_t page_number,uint16_t block_number, uint64_t transaction_id, fstream* wal_file) {
     

    Delete(args[primary_key_index],page_number,block_number, transaction_id, wal_file);
    cout<<"WE DONE DELETING"<<endl;
    Insert(args, transaction_id, wal_file);
    
    
    cout<<"TABLE :UPDATED : "<<transaction_id<<" KEY: "<<args[primary_key_index]<<endl;
    

}

void Table::Delete(string key,uint64_t page_number,uint16_t block_number, uint64_t transaction_id, fstream* wal_file) {
    // cout << "OH YEAHHH BABBYY" << endl;
    vector<Block> deletedBlocks;


    // if (deletedBlocks.size() > 0) {
    //     for (auto delBlock : deletedBlocks) {
            // cout << "deleted page Number: " << delBlock.pageNumber.value() << " deleted page Block Number: " << delBlock.blockNumber.value() <<
            // endl;
            deleteData(page_number, block_number, data_file, page_file, transaction_id);
        // }
    // } else {
    //     cout << "Record Not found" << endl;
    // }
    if (wal_file) {
        WAL wal(OPERATION::DELETE, transaction_id, &key, NULL);
        wal.write(wal_file);
    }
}

void Table::CreateIndex(string column_name, uint64_t transaction_id) {
    for (auto index : indexes) {
        if (index->columnName == column_name) {
            cout << "INDEX ALREADY EXISTS" << endl;

            return;
        }
    }
    // cout << "EASSYYY ohh" << endl;
    Index* index = new Index(database->name, table_name, column_name);
    indexes.push_back(index);
    Index* primaryIndex = getIndex("");
    cout << primaryIndex->columnName << endl;
    // vector<vector<string>> rows=this->RangeQuery(NULL,NULL,columns);
    pair<BTreeNode*, optional<Block>> SearchResult1 = primaryIndex->btree->beg();
    BTreeNode* currentNode = SearchResult1.first;
    optional<Block> optData = SearchResult1.second;

    if (!optData.has_value()) {
        // cout << "DRAMME" << endl;

        // cout << "KEY NOT FOUND" << endl;
        return;
    }
    if (currentNode == NULL) {
        // cout << "EMPTY";
        return;
    }
    Block data = optData.value();
    string key = data.key;
    uint64_t pgNumber = data.pageNumber.value();
    uint16_t blNumber = data.blockNumber.value();
    int i = 0;
    while ((currentNode->nextSibling != -1 || (currentNode->nextSibling == -1 && i < currentNode->blocks.size()))) {
        blNumber = currentNode->blocks[i].blockNumber.value();
        // cout << "pageNumver: " << pgNumber << " BL NUMBER HEHEHE : " << blNumber << endl;
        pgNumber = currentNode->blocks[i].pageNumber.value();
        pair<optional<string>, pair<uint64_t, uint64_t>> read_value_data = readValue(data.pageNumber.value(), data.blockNumber.value());
        optional<string> foundValue = read_value_data.first;
        uint64_t t_ins = read_value_data.second.first;
        uint64_t t_del = read_value_data.second.second;
        if (!foundValue.has_value()) {
            cout << "ERROR VALUE NOT FOUND, corrupted";
            return;
        }
        index->btree->insert(currentNode->blocks[i]);
        // cout << "ROW DATA:" << rowData[0] << endl;

        // cout<<"HELLO"<<endl;

        if ((i + 1) < currentNode->blocks.size()) {
            i = i + 1;
        } else {
            if (currentNode->nextSibling == -1) {
                break;
            }
            // cout<<"NEXT SIBLING: "<<currentNode->nextSibling<<endl;
            BTreeNode* nextNode = primaryIndex->btree->readPage(currentNode->nextSibling);
            // cout<<"AVVVEEEE: "<<nextNode->blocks[0].key<<endl;
            i = 0;
            // cout<<"AVVEE BHAIII :"<<nextNode->pageNumber<<endl;
            currentNode = nextNode;
        }
    }
}
