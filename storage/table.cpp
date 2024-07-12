
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

Table::Table(string table_name, vector<string> types, vector<string> names, string database_name, fstream* data_file, fstream* page_file) {
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

void Table::Insert(vector<string> args) {
    if (args.size() != columns.size()) {
        cout << "ERROR : args size not equal to column data";
        return;
    }

    Block newData = writeData(args[0], args[1]);
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

    uint16_t newBegOffset = metadata.begOffset + sizeof(uint16_t);
    uint16_t newEndOffset = metadata.endOffset - val.size();

    if (newEndOffset <= newBegOffset) {
        return -1;  // Not enough space
    }

    uint16_t blockOffset = newEndOffset;
    // cout << "WRITE: BLOCK OFFSET block : " << metadata.noOfBlocks << " is : " << blockOffset << endl;

    metadata.noOfBlocks++;
    memcpy(buffer, &metadata.noOfBlocks, sizeof(metadata.noOfBlocks));
    memcpy(buffer + sizeof(uint16_t), &newBegOffset, sizeof(newBegOffset));
    memcpy(buffer + sizeof(uint16_t) * 2, &newEndOffset, sizeof(newEndOffset));

    // Copy the block offset for the new entry
    memcpy(buffer + headerSize + (metadata.noOfBlocks - 1) * sizeof(uint16_t), &blockOffset, sizeof(blockOffset));

    // Copy the actual value into the page
    memcpy(buffer + blockOffset, val.c_str(), val.size());

    // Write the updated page back to the file
    data_file->seekp(pageOffset, ios::beg);
    data_file->write(buffer, PAGE_SIZE);
    // cout<<string(to_string(pageNumber)+ " " + to_string(newEndOffset - newBegOffset))<< endl;;
    *page_file << string(to_string(pageNumber) + " " + to_string(newEndOffset - newBegOffset))<<endl;
    page_file->flush();
    return metadata.noOfBlocks - 1;
}

int updateValueInDataFile(string val, uint64_t pageNumber, uint16_t blockNumber, fstream* data_file, fstream* page_file) {
    int pageOffset = pageNumber * PAGE_SIZE;
    char buffer[PAGE_SIZE] = {0};

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
    while (getline(*page_file, page_data)) {
        // Process each line here (e.g., print it)
        // cout << "PAGE_DATA: " << page_data << endl;
        std::istringstream iss(page_data);
        std::vector<std::string> tokens;
        std::string token;

        while (iss >> token) {
            tokens.push_back(token);
        }
        if (tokens.size() < 2) {
            pageNumber = pageNumber ;
            continue;
        }

        uint64_t pagenumber = stoi(tokens[0]);
        int availableSpace = stoi(tokens[1]);
        if (availableSpace > sizeof(uint16_t) + value.size()) {
            pageNumber = pagenumber;
            newPage = false;
            // cout << "DAYUUMMM" << endl;
            break;
        } else {
            pageNumber = max(pagenumber + 1, pageNumber);
        }
    }
    page_file->clear();

    page_file->seekg(0, ios::beg);
    page_file->seekp(0, ios::beg);
    // cout << "LAST PAGEEE: " << pageNumber << endl;
    for (int i = 0; i < pageNumber; ++i) {
        string line;
        if (!getline(*page_file, line)) {
            // If we can't read a line, we've reached the end of the file
            // You might want to handle this case (e.g., by seeking to the end or throwing an exception)
            page_file->seekg(0, ios::end);
            break;
        }
    }

    int blockNumber = insertValueToDataFile(value, pageNumber, data_file, page_file, newPage);
    if (blockNumber == -1) {
        cout << "FFF" << endl;
        return Block{.key = "asdsdsd"};
    }
    return Block{
        .key = key,
        .pageNumber = pageNumber,
        .blockNumber = blockNumber,
    };
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
    memcpy(&blockOffset, buffer + headerSize + (sizeof(uint16_t) * blockNumber), sizeof(blockOffset));

    uint16_t nextBlockOffset;
    if (blockNumber == 0) {
        nextBlockOffset = PAGE_SIZE;
    } else {
        memcpy(&nextBlockOffset, buffer + headerSize + (sizeof(uint16_t) * (blockNumber - 1)), sizeof(nextBlockOffset));
    }
    // cout << "READ: BLOCK OFFSET block : " << blockNumber << " is : " << blockOffset << endl;

    uint16_t valueSize = nextBlockOffset - blockOffset;
    string value(valueSize, '\0');
    memcpy(&value[0], buffer + blockOffset, valueSize);

    return value;
}

void Table::Print() { btree->printTree(this->rootPageNumber); }

string Table::Search(string key) {
    optional<Block> optData = btree->search(key);
    // cout<<"KEY: "<<key<<" PAGE NUMBER: " << optData.value().pageNumber.value()<<" BLOCK NUMBER: "<<optData.value().blockNumber.value()<<endl;
    if (optData.has_value()) {
        Block data = optData.value();
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
