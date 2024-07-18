#include <cassert>
#include <chrono>  // For std::chrono::seconds
#include <iostream>
#include <string>
#include <thread>  // For std::this_thread::sleep_for

#include "storage/database.cpp"
#include "parser/interpreter.cpp"

using namespace std;

void testInsertAndSearch(Table* table) {
    table->Insert({"key1", "1", "1000", "John"});
    table->Insert({"key2", "2", "2000", "Jane"});
    table->Insert({"key3", "3", "3000", "Jim"});
    table->Insert({"key4", "4", "4000", "Jack"});

    assert(table->Search("key1", table->columns[table->primary_key_index].name) == "key1,1,1000,John");
    assert(table->Search("key2", table->columns[table->primary_key_index].name) == "key2,2,2000,Jane");
    assert(table->Search("key3", table->columns[table->primary_key_index].name) == "key3,3,3000,Jim");
    assert(table->Search("key4", table->columns[table->primary_key_index].name) == "key4,4,4000,Jack");


    table->CreateIndex("age");

    cout << "testInsertAndSearch passed!" << endl;
}

void testDelete(Table* table) {
    table->Delete("key1");
    cout << "answer " << table->Search("key1", table->columns[table->primary_key_index].name) << endl;
    assert(table->Search("key1", table->columns[table->primary_key_index].name) == "KEY NOT FOUND");

    cout << "testDelete passed!" << endl;
}

void testUpdate(Table* table) {
    table->Insert({"key1", "1", "1000", "John"});
    table->Update({"key1", "1", "80", "Johnny"});

    assert(table->Search("key1", table->columns[table->primary_key_index].name) == "key1,1,80,Johnny");

    cout << "testUpdate passed!" << endl;
}

void testSearchNonExistentKeys(Table* table) {
    assert(table->Search("non_existent_key", table->columns[table->primary_key_index].name) == "KEY NOT FOUND");

    cout << "testSearchNonExistentKeys passed!" << endl;
}

void test() {
    Database* database = new Database("test");
    Table* table = database->CreateTable("test_table", {"string", "int", "int", "string"}, {"id", "age", "salary", "name"}, 0);

    testInsertAndSearch(table);
    // testDelete(table);
    // testUpdate(table);
    // testSearchNonExistentKeys(table);
    // table->Print(table->columns[table->primary_key_index].name);

    // Interpreter interpreter(database);

    cout << "All tests passed!" << endl;
}
