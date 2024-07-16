#include <iostream>
#include <optional>
#include <memory>
#include "./tokenizer.cpp"
#include "./expr.cpp"

#include "./parser.cpp"
#include "./include/statement.hpp"
class Interpreter {
   public:
    Interpreter(Database *db) {

        // string query = "SELECT id,salary,age,name from test_table where index=\"key2\"";
        string query=R"(
        CREATE TABLE newtable (name STRING , age INT, salary INT)
        
        INSERT INTO newtable VALUES("key1" , 10 , 5000)
        INSERT INTO newtable VALUES("key2" , 20 , 2000)
        INSERT INTO newtable VALUES("key3" , 30 , 3000)
 
        SELECT age,name FROM newtable
        UPDATE newtable SET salary = 70000, age = 31 WHERE name = "key3";
        SELECT age,name,salary FROM newtable


        )";
        // cout<<"THE ROCK: "<<query<<endl;oo
        Tokenizer tokenizer(query);
        tokenizer.print();
        Parser parser(tokenizer.tokens);
        
        vector<unique_ptr<Statement>> stmts=parser.parse();

        if(stmts.size()>0){
            // stmt->print();
            // cout<<"LESS GO"<<endl;
            for(auto &stmt:stmts){
                stmt->execute(db);
            }

        }
      
        
        


    }
};