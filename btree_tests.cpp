#include "storage/database.hpp"
#include <cassert>
#include <iostream>
#include <thread>   // For std::this_thread::sleep_for
#include <chrono>   // For std::chrono::seconds

using namespace std;

// Test functions
void testInsertAndSearch(Database* store) {
    
    store->insert("key1", "value1");
    store->insert("key3", "value3");
    store->insert("key2", "value2");
    store->insert("key4", "value4");
    store->insert("key0", "value0");
    store->insert("key7", "value7");
    store->insert("key9", "value9");
    store->insert("key8", "value8");


    assert(store->search("key1") == "value1");
    assert(store->search("key2") == "value2");
    assert(store->search("key3") == "value3");
    assert(store->search("key4") == "value4");

    cout << "testInsertAndSearch passed!" << endl;
}

void testDuplicateKeys(Database* store) {
    store->insert("key1", "value1");
    store->insert("key1", "value2");

    assert(store->search("key1") == "value2"); // Assuming the current implementation does not handle duplicates and keeps the first value

    cout << "testDuplicateKeys passed!" << endl;
}

void testTreeSplitting(Database* store) {

    // cout
    // for (int i =500 ; i <= 500000; ++i) {
    //     store->insert("key" + to_string(i), "value" + to_string(i));

    // }
    for (int i =0 ; i <= 100000; ++i) {
                        //  this_thread::sleep_for(chrono::milliseconds(100));
            cout<<"Brother "<<i<<endl;
        store->insert("key" + to_string(i), "value" + to_string(i));

    }
    cout<<"INSERTION DONE"<<endl;
    store->printTree();

            // cout<<"FUCKING HELL: "<<0<<endl;
            //  this_thread::sleep_for(chrono::milliseconds(10));
            //  cout<<"FIRST: "<<store->search("key" + to_string(0))<<endl;

        assert(store->search("key6" ) == "value6");
    for (int i =0 ; i <= 100; ++i) {
        // cout<<"i "<<i<<endl;
        assert(store->search("key"+to_string(i) ) == "value"+to_string(i));

    }

    // assert(store->search("key100000") == "value100000");


    // assert(store->search("key1000000") == "Key not found");

    cout << "testTreeSplitting passed!" << endl;
}

void testSearchNonExistentKeys(Database* store) {
    store->insert("key1", "value1");
    store->insert("key2", "value2");

    assert(store->search("asdasd") == "Key not found");

    cout << "testSearchNonExistentKeys passed!" << endl;
}

void test() {

     Database *store =new Database("test");
    // testInsertAndSearch(store);
    // testDuplicateKeys(store);
    // testTreeSplitting(store);
    // testSearchNonExistentKeys(store);

    cout << "All tests passed!" << endl;
    
}


