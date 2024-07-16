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

        string query = "SELECT id,salary,age,name from test_table where index=\"key2\"";
        // cout<<"THE ROCK: "<<query<<endl;oo
        Tokenizer tokenizer(query);
        tokenizer.print();
        Parser parser(tokenizer.tokens);
        
        unique_ptr<Statement> stmt=parser.parse();

        if(stmt){
            stmt->print();
            // cout<<"LESS GO"<<endl;
            stmt->execute(db);

        }
        else{
            cout<<"BRTHR"<<endl;
        }
        
        


    }
};