#include <iostream>
#include <optional>
#include <memory>
#include "./tokenizer.cpp"
#include "./parser.cpp"
#include "./include/statement.hpp"
class Interpreter {
   public:
    Interpreter() {

        string query = "SELECT anurag,raut from anuar";
        // cout<<"THE ROCK: "<<query<<endl;
        Tokenizer tokenizer(query);
        tokenizer.print();
        Parser parser(tokenizer.tokens);
        
        unique_ptr<Statement> stmt=parser.parse();

        if(stmt){
            cout<<"LESS GO"<<endl;
            stmt->print();

        }
        else{
            cout<<"BRTHR"<<endl;
        }
        
        


    }
};