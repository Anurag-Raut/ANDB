
#include "../include/table.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../include/database.hpp"
#include "../include/globals.hpp"
#include "../storage/btree.cpp"

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
Table::Table(string table_name, vector<string> types, vector<string> names, string database_name, fstream* data_file, fstream* page_file,int primary_key_index) {
    if (types.size() != names.size()) {
        return;
    }

    for (size_t i = 0; i < types.size(); ++i) {
        columns.push_back({names[i], types[i]});
    }
    std::filesystem::path tableDir = BASE_DIRECTORY + "/" + database_name + "/" + table_name;
    if (!std::filesystem::exists(tableDir)) {
        std::filesystem::create_directories(tableDir);
    }

    string primaryIndexFile = getIndexFilePath(database_name, table_name, "primary");
    fstream* index_file = new fstream();
    indexes.push_back(index_file);
    // Debugging output
    index_file->open(primaryIndexFile, ios::out | ios::trunc | ios::in);

    this->data_file = data_file;
    this->page_file = page_file;
    // Ensure directories exist

    this->btree = new Btree(index_file);
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

void Table::Insert(vector<string> args) {
    if (args.size() != columns.size()) {
        cout << "ERROR : args size not equal to column data";
        return;
    }

    // type checking

    for (int i = 0; i < args.size(); i++) {
        string arg = args[i];
        string type = columns[i].type;

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

    Block newData = writeData(args[primary_key_index], joinedArgs);
    // cout<<"key: "<<newData.key<<"BLOCK: " <<newData.blockNumber.value()<<endl;
    btree->insert(newData, args[1]);
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

int insertValueToDataFile(string val, uint64_t pageNumber, fstream* data_file, fstream* page_file, bool newPage) {
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
    uint16_t newBegOffset = metadata.begOffset + (2 * sizeof(uint16_t));
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

    int valueCellSize = val.size();
    // cout << "value cell size: " << valueCellSize << endl;
    // Copy the block offset for the new entry

    memcpy(buffer + headerSize + (metadata.noOfBlocks - 1) * (2 * sizeof(uint16_t)), &blockOffset, sizeof(blockOffset));
    memcpy(buffer + headerSize + ((metadata.noOfBlocks - 1) * (2 * sizeof(uint16_t))) + sizeof(uint16_t), &valueCellSize, val.size());
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

int updateValueInDataFile(string val, uint64_t pageNumber, uint16_t blockNumber, fstream* data_file, fstream* page_file) {
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
    return insertValueToDataFile(val, pageNumber, data_file, page_file, false);
}

Block Table::writeData(string key, string value) {
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
    int blockNumber = insertValueToDataFile(value, pageNumber, data_file, page_file, newPage);
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

void deleteData(uint64_t pageNumber, uint16_t blockNumber, fstream* data_file, fstream* page_file) {
    char buffer[PAGE_SIZE];
    uint32_t headerSize = 3 * sizeof(uint16_t);

    int pageOffset = pageNumber * PAGE_SIZE;
    data_file->seekg(pageOffset, ios::beg);
    data_file->read(buffer, PAGE_SIZE);
    MetadataDataPage metadata = readMetadata(buffer);
    int ogNoOfBlocks = metadata.noOfBlocks;
    int ogEndOffset = metadata.endOffset;
    char newBuffer[PAGE_SIZE];
    metadata.noOfBlocks = ogNoOfBlocks;
    metadata.begOffset = headerSize;
    metadata.endOffset = PAGE_SIZE;
    // cout << "BLOCK NUMBER: " << blockNumber << endl;
    for (int i = 0; i < ogNoOfBlocks; i++) {
        // get blockOffset
        // cout << "pepo pog i: " << i << endl;
        uint16_t blockOffset;
        memcpy(&blockOffset, buffer + headerSize + ((2 * sizeof(uint16_t)) * i), sizeof(blockOffset));
        uint16_t sizeOfValueCell;
        memcpy(&sizeOfValueCell, buffer + headerSize + ((2 * sizeof(uint16_t)) * i) + (sizeof(uint16_t)), sizeof(sizeOfValueCell));
        // cout << "size of value: " << sizeOfValueCell << endl;

        // cout << "DELETE for "<<i<<"  BLOCK OFFSET: " << metadata.endOffset - sizeOfValueCell << "  J :" << endl;
        if (i != blockNumber) {
            uint16_t newBlockOffset = metadata.endOffset - sizeOfValueCell;
            memcpy(newBuffer + headerSize + (2 * sizeof(uint16_t) * i), &newBlockOffset, sizeof(uint16_t));
            memcpy(newBuffer + headerSize + (2 * sizeof(uint16_t) * i) + sizeof(uint16_t), &sizeOfValueCell, sizeof(uint16_t));

        } else {
            uint16_t zero = 0;
            memcpy(newBuffer + headerSize + (2 * sizeof(uint16_t) * i), &zero, sizeof(uint16_t));
            memcpy(newBuffer + headerSize + (2 * sizeof(uint16_t) * i) + sizeof(uint16_t), &zero, sizeof(uint16_t));
        }

        if (i == blockNumber) {
            continue;
        }
        // cout << "newBuff: " << (metadata.endOffset - sizeOfValueCell) << endl;
        memcpy(newBuffer + (metadata.endOffset - sizeOfValueCell), buffer + blockOffset, sizeOfValueCell);
        metadata.begOffset += sizeof(uint16_t);
        metadata.endOffset -= sizeOfValueCell;

        // memcpy(&nextBlockOffset, buffer + headerSize + (sizeof(uint16_t) * (blockNumber + 1)), sizeof(nextBlockOffset));
        // uint16_t sizeOfNextValue=blockOffset-nextBlockOffset;
        // memcpy(&buffer+prevBlockOffset-sizeOfNextValue,buffer+nextBlockOffset,sizeOfNextValue);

        // //moving value
        // memcpy(&buffer+blockNumber,buffer+nextBlockOffset,blockOffset-nextBlockOffset);

        // memcpy(&buffer+headerSize+(sizeof(uint16_t)*blockNumber), buffer + headerSize + (sizeof(uint16_t) * blockNumber+1), sizeof(blockOffset));
    }
    // cout << "no of blocks:" << metadata.noOfBlocks << " og blocks: " << ogNoOfBlocks << endl;
    memcpy(newBuffer, &metadata.noOfBlocks, sizeof(metadata.noOfBlocks));
    memcpy(newBuffer + sizeof(uint16_t), &metadata.begOffset, sizeof(metadata.begOffset));
    memcpy(newBuffer + sizeof(uint16_t) * 2, &metadata.endOffset, sizeof(metadata.endOffset));
    data_file->clear();
    data_file->seekp(pageOffset, ios::beg);
    data_file->write(newBuffer, PAGE_SIZE);
    data_file->flush();
    page_file->clear();
    // cout << "ACTUAL: " << metadata.endOffset - metadata.begOffset << endl;
    // page_file->seekg(0, ios::beg);
    // page_file->seekp(0, ios::beg);
    // // cout << "LAST PAGEEE: " << pageNumber << endl;
    // string line;

    // for (int i = 0; i <= pageNumber; ++i) {
    //     if (!getline(*page_file, line)) {
    //         // If we can't read a line, we've reached the end of the file
    //         // You might want to handle this case (e.g., by seeking to the end or throwing an exception)
    //         page_file->seekg(0, ios::end);
    //         break;
    //     }
    // }
    // std::istringstream iss(line);
    // std::vector<std::string> tokens;
    // std::string token;

    // while (iss >> token) {
    //     tokens.push_back(token);
    // }
    // if (tokens.size() < 3) {
    //     pageNumber = pageNumber;
    //     return;
    // }
    // page_file->seekp(0, ios::beg);

    // *page_file << string(to_string(pageNumber) + " " + to_string(metadata.endOffset - metadata.begOffset)) + " " +
    //                   to_string(stoi(tokens[3]) + sizeof(uint16_t))
    //            << endl;
    // page_file->flush();

    PageMap pageMap;
    pageMap.readFromFile(pageNumber, page_file);
    pageMap.availableSpace = metadata.endOffset - metadata.begOffset;
    pageMap.noOfBlocksAvailable += 2 * sizeof(uint16_t);
    pageMap.writeToFile(page_file);
}
// int updateValueToDataFile(string val, uint64_t pageNumber, fstream* data_file, bool newPage) {
//     int pageOffset = pageNumber * PAGE_SIZE;
//     char buffer[PAGE_SIZE] = {0};
//     uint32_t headerSize = 3 * sizeof(uint16_t);
//     MetadataDataPage metadata=MetadataDataPage{noOfBlocks:0,begOffset:headerSize,endOffset:PAGE_SIZE};
//     if (!newPage) {
//         data_file->seekg(pageOffset, ios::beg);
//         char readBuffer[PAGE_SIZE];
//         data_file->read(readBuffer,PAGE_SIZE);
//         metadata=readMetadata(readBuffer);

//     }

//     data_file->seekp(pageOffset, ios::beg);
//     uint16_t newBegOffset = metadata.begOffset+sizeof(uint16_t);
//     uint16_t newEndOffset = metadata.endOffset-val.size();

//     if (!newPage) {
//         memcpy(&newEndOffset, buffer + sizeof(uint16_t) * 2, sizeof(uint16_t));
//     }

//     uint16_t blockOffset = newEndOffset - val.size();

//     metadata.noOfBlocks++;
//     memcpy(buffer, &metadata.noOfBlocks, sizeof(metadata.noOfBlocks));
//     size_t offset=sizeof(metadata.noOfBlocks);
//     memcpy(buffer + sizeof(uint16_t), &metadata.begOffset, sizeof(metadata.begOffset));
//     offset+=sizeof(metadata.begOffset);
//     memcpy(buffer + sizeof(uint16_t), &metadata.endOffset, sizeof(metadata.endOffset));
//     offset+=sizeof(metadata.endOffset);

//     memcpy(buffer + newBegOffset, &blockOffset, sizeof(blockOffset));

//     memcpy(buffer + headerSize + (prevNumberOfNodes - 1) * sizeof(uint16_t), &blockOffset, sizeof(blockOffset));
//     memcpy(buffer + blockOffset, val.c_str(), val.size());

//     data_file->seekp(pageOffset, ios::beg);
//     data_file->write(buffer, PAGE_SIZE);

//     return 0;
// }

optional<string> Table::readValue(uint64_t pageNumber, uint16_t blockNumber) {
    char buffer[PAGE_SIZE];
    int pageOffset = pageNumber * PAGE_SIZE;
    data_file->seekg(pageOffset, ios::beg);
    data_file->read(buffer, PAGE_SIZE);
    MetadataDataPage metadata = readMetadata(buffer);
    int headerSize = metadata.size();
    // cout << "BLOCK NUMBER : " << blockNumber << " " << metadata.begOffset << " " << metadata.noOfBlocks << " " << metadata.endOffset << endl;
    if (blockNumber >= metadata.noOfBlocks) {
        return nullopt;  // Block number out of range
    }

    uint16_t blockOffset;
    memcpy(&blockOffset, buffer + headerSize + (2 * sizeof(uint16_t) * blockNumber), sizeof(blockOffset));
    // cout << "READ: BLOCK OFFSET block : " << blockNumber << " is : " << blockOffset << endl;
    if (blockOffset == 0) {
        return nullopt;
    }
    uint16_t sizeOfValueCell;

    memcpy(&sizeOfValueCell, buffer + headerSize + (2 * sizeof(uint16_t) * blockNumber) + sizeof(uint16_t), sizeof(uint16_t));

    string value(sizeOfValueCell, '\0');
    memcpy(&value[0], buffer + blockOffset, sizeOfValueCell);

    return value;
}

void Table::Print() { btree->printTree(this->rootPageNumber); }

void Table::RangeQuery(string key1, string key2) {
    pair<BTreeNode*, optional<Block>> SearchResult1 = btree->search(key1);
    BTreeNode* currentNode = SearchResult1.first;
    optional<Block> optData = SearchResult1.second;

    if (!optData.has_value()) {
        cout << "KEY NOT FOUND" << endl;
        return;
    }
    Block data = optData.value();
    string key = data.key;
    uint64_t pgNumber = data.pageNumber.value();
    uint16_t blNumber = data.blockNumber.value();
    int i = 0;
    while (i < currentNode->blocks.size() && currentNode->blocks[i].key < key1) i++;

    while (key <= key2) {
        blNumber = currentNode->blocks[i].blockNumber.value();
        // cout<<"BL NUMBER HEHEHE : "<<blNumber<<endl;
        pgNumber = currentNode->blocks[i].pageNumber.value();
        optional<string> foundValue = readValue(pgNumber, blNumber);
        cout << "FOUND VALUE :" << foundValue.value() << endl;

        if (key == key2) {
            return;
        }
        if ((i + 1) < currentNode->blocks.size()) {
            i = i + 1;
        } else {
            // cout<<"NEXT SIBLING: "<<currentNode->nextSibling<<endl;
            BTreeNode* nextNode = btree->readPage(currentNode->nextSibling);
            // cout<<"AVVVEEEE: "<<nextNode->blocks[0].key<<endl;
            i = 0;
            // cout<<"AVVEE BHAIII :"<<nextNode->pageNumber<<endl;
            currentNode = nextNode;
        }
        key = currentNode->blocks[i].key;
    }
}

string Table::Search(string key) {
    pair<BTreeNode*, optional<Block>> SearchData = btree->search(key);
    optional<Block> optData = SearchData.second;
    // cout<<"KEY: "<<key<<" PAGE NUMBER: " << optData.value().pageNumber.value()<<" BLOCK NUMBER: "<<optData.value().blockNumber.value()<<endl;
    if (optData.has_value()) {
        Block data = optData.value();
        cout << "STRANGER: " << data.pageNumber.value() << endl;

        // BTreeNode* startNode=btree->readPage(data.pageNumber.value());
        cout << "KEY:: " << key << "  MEDDIATORS:  " << SearchData.first->nextSibling << endl;

        // cout<<"key : "<<key<<data.pageNumber.value()<<data.blockNumber.value()<<endl;
        optional<string> foundValue = readValue(data.pageNumber.value(), data.blockNumber.value());
        if (foundValue.has_value()) {
            return foundValue.value();
        }

        return "KEY NOT FOUND";
    } else {
        return "KEY NOT FOUND";
    }
}

void Table::Update(vector<string> args) {
    deleteValue(args[0]);
    Insert(args);
}

void Table::deleteValue(string key) {
    optional<Block> deletedBlock = btree->deleteNode(key);

    if (deletedBlock.has_value()) {
        Block delBlock = deletedBlock.value();
        // cout << "deleted page Number: " << delBlock.pageNumber.value() << " deleted page Block Number: " << delBlock.blockNumber.value() << endl;
        deleteData(delBlock.pageNumber.value(), delBlock.blockNumber.value(), data_file, page_file);
    } else {
        cout << "Record Not found" << endl;
    }
}
