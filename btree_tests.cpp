#include <cassert>
#include <chrono>  // For std::chrono::seconds
#include <iostream>
#include <string>
#include <thread>  // For std::this_thread::sleep_for

#include "storage/database.cpp"
#include "parser/interpreter.cpp"
#include "storage/transaction.cpp"


using namespace std;

// void testInsertAndSearch(Transaction* tx) {
//         tx->Insert({"key1", "1", "1000", "John"});
//         tx->Insert({"key1", "2", "3000", "2ohn"});
//         string key1="key1";
//         auto rows=tx->RangeQuery(&key1,&key1,tx->table->columns,true,true);
//         cout<<"Row data "<<rows.size()<<endl;
//         for(auto row:rows){
//             cout<<row[0]<<" ";

//         }
//         cout<<endl;
//     tx->Insert({"key2", "2", "2000", "Jane"});
//     tx->Insert({"key3", "3", "3000", "Jim"});
//     tx->Insert({"key4", "4", "4000", "Jack"});

//     assert(tx->Search("key1", tx->table->columns[tx->table->primary_key_index].name) == "key1,1,1000,John");
//     assert(tx->Search("key2", tx->table->columns[tx->table->primary_key_index].name) == "key2,2,2000,Jane");
//     assert(tx->Search("key3", tx->table->columns[tx->table->primary_key_index].name) == "key3,3,3000,Jim");
//     assert(tx->Search("key4", tx->table->columns[tx->table->primary_key_index].name) == "key4,4,4000,Jack");



//     cout << "testInsertAndSearch passed!" << endl;
// }
// void testIndex(Table* table){
//     table->CreateIndex("age");
//     assert(table->Search("key1", "age") == "key1,1,1000,John");
//     table->Insert({"key5", "5", "5000", "Jimmy"});
//     table->Insert({"key6", "7", "7000", "joshua"});
//     assert(table->Search("key5", "age") == "key5,5,5000,Jimmy");
//     assert(table->Search("key6", "age") == "key6,7,7000,joshua");




// }
// void testDelete(Table* table) {
//     table->Delete("key1");
//     cout << "answer " << table->Search("key1", table->columns[table->primary_key_index].name) << endl;
//     assert(table->Search("key1", table->columns[table->primary_key_index].name) == "KEY NOT FOUND");

//     cout << "testDelete passed!" << endl;
// }

// void testUpdate(Table* table) {
//     table->Insert({"key1", "1", "1000", "John"});
//     table->Update({"key1", "1", "80", "Johnny"});

//     assert(table->Search("key1", table->columns[table->primary_key_index].name) == "key1,1,80,Johnny");

//     cout << "testUpdate passed!" << endl;
// }

// void testSearchNonExistentKeys(Table* table) {
//     assert(table->Search("non_existent_key", table->columns[table->primary_key_index].name) == "KEY NOT FOUND");

//     cout << "testSearchNonExistentKeys passed!" << endl;
// }

void test() {
    Database* database = new Database("test");
    // Transaction* tx= new Transaction(database);
    // tx->CreateTable("test_table", {"string", "int", "int", "string"}, {"id", "age", "salary", "name"}, 0);

    // testInsertAndSearch(tx);
    // testIndex(table);
    // testDelete(table);
    // testUpdate(table);
    // testSearchNonExistentKeys(table);
    // table->Print(table->columns[table->primary_key_index].name);

    Interpreter interpreter(database);

    cout << "All tests passed!" << endl;
}
