#include <string>
#include <cassert>
#include <iostream>
#include "storage/database.cpp"
#include <thread>   // For std::this_thread::sleep_for
#include <chrono>   // For std::chrono::seconds
using namespace std;

// Test functions
// void testInsertAndSearch(Database* store) {
    
//     store->insert("key1", "value1");
//     store->insert("key3", "value3");
//     store->insert("key2", "value2");
//     store->insert("key4", "value4");
//     store->insert("key0", "value0");
//     store->insert("key7", "value7");
//     store->insert("key9", "value9");
//     store->insert("key8", "value8");


//     assert(store->search("key1") == "value1");
//     assert(store->search("key2") == "value2");
//     assert(store->search("key3") == "value3");
//     assert(store->search("key4") == "value4");

//     cout << "testInsertAndSearch passed!" << endl;
// }

// void testDuplicateKeys(Database* store) {
//     store->insert("key1", "value1");
//     store->insert("key1", "value2");

//     assert(store->search("key1") == "value2"); // Assuming the current implementation does not handle duplicates and keeps the first value

//     cout << "testDuplicateKeys passed!" << endl;
// }

void testTreeSplitting(Table* table) {

    // cout
    // for (int i =500 ; i <= 500000; ++i) {
    //     store->insert("key" + to_string(i), "value" + to_string(i));

    // }
    for (int i =0 ; i <= 1000007; ++i) {
                        //  this_thread::sleep_for(chrono::milliseconds(100));
            // cout<<"Brother "<<i<<endl;
        table->Insert({ to_string(i), to_string(i)});

    }
    cout<<"INSERTION DONE"<<endl;
    // table->Print();

            // cout<<"FUCKING HELL: "<<0<<endl;
            //  this_thread::sleep_for(chrono::milliseconds(10));
            //  cout<<"FIRST: "<<store->search("key" + to_string(0))<<endl;

    for (int i =0 ; i <= 1000007; ++i) {
        // cout<<"i "<<i<<endl;
        // cout<<table->Search(to_string(i) )<<endl;
        assert(table->Search(to_string(i) ) == to_string(i));

    }

    // assert(store->search("key100000") == "value100000");


    // assert(store->search("key1000000") == "Key not found");

    cout << "testTreeSplitting passed!" << endl;
}

// void testSearchNonExistentKeys(Database* store) {
//     store->insert("key1", "value1");
//     store->insert("key2", "value2");

//     assert(store->search("asdasd") == "Key not found");

//     cout << "testSearchNonExistentKeys passed!" << endl;
// }

void test() {

    Database* database = new Database("test");
    Table * table=database->CreateTable("test_table",{"int","string"},{"age","name"}) ;
    this_thread::sleep_for(chrono::milliseconds(1000));
    // table->Insert({"1","anurag"});
        this_thread::sleep_for(chrono::milliseconds(1000));
    
        // table->Insert({"2","jhf"});
        // table->Insert({"3","DFsdfsadf"});
        // table->Insert({"4","asfdsfd"});
        // table->Insert({"5","asfdsfd"});
        // table->Insert({"6","asdsfd"});
        // table->Insert({"7","asfd"});
        // table->Insert({"8","asfd"});
        // table->Insert({"9","asfd"});
        // table->Insert({"10","asfd"});
        // table->Insert({"11","asfd"});
        // table->Insert({"12","asfd"});
        // table->Insert({"13","asfd"});
        // table->Insert({"14","asfd"});
        //     table->Insert({"15","asfd"});
        // table->Insert({"16","asfd"});
        // table->Insert({"17","asfd"});
        // table->Insert({"18","asfd"});
        // table->Insert({"19","asfd"});
        // table->Insert({"20","asfd"});
        // cout<<table->Search("5")<<endl;;


        // table->Insert({"51","asfd"});
        




        // table->Print();

    // testInsertAndSearch(store);
    // testDuplicateKeys(store);
    testTreeSplitting(table);
    // testSearchNonExistentKeys(store);

    cout << "All tests passed!" << endl;
    
}


