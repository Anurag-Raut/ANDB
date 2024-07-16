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
        CREATE TABLE anurag (name STRING , age INT, salary INT)
        
        INSERT
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