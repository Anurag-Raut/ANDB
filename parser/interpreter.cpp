#include <iostream>
#include <memory>
#include <optional>

#include "./expr.cpp"
#include "./include/statement.hpp"
#include "./parser.cpp"
#include "./tokenizer.cpp"
class Interpreter {
   public:
    Interpreter(Database *db, string query) {
        // string query = "SELECT id,salary,age,name from test_table where index=\"key2\"";
        // cout<<"THE ROCK: "<<query<<endl;oo
        Tokenizer tokenizer(query);
        // tokenizer.print();
        Parser parser(tokenizer.tokens);

        vector<unique_ptr<Statement>> stmts = parser.parse();
                Transaction *tx = NULL;
                bool isTransactionRunning=false;

        if (stmts.size() > 0) {
            // stmt->print();
            // cout<<"LESS GO"<<endl;
            for (auto &stmt : stmts) {
                        // cout<<"ARE WE GOOD"<<endl;

                if(isTransactionRunning && dynamic_cast<BeginStatement *>(stmt.get())){
                    runtime_error("TRANSACTION ALREADY RUNNING");

                }
                else if (dynamic_cast<BeginStatement*>(stmt.get())) {
                    isTransactionRunning=true;
                    tx=new Transaction(db);
                }  
 
                if(!isTransactionRunning){
                    tx=new Transaction(db);
                }
              
               

                stmt->execute(tx);
                if(!isTransactionRunning){
                    tx->Commit();
                }
                else if (dynamic_cast<CommitStatement *>(stmt.get())) {
                    cout<<"OH YEAHH BABBY"<<endl;
                    tx->Commit();
                    tx=NULL;
                    isTransactionRunning=false;

                }
                else if (dynamic_cast<RollbackStatement *>(stmt.get())) {
                    cout<<"OH YEAHH ROLLBACK"<<endl;
                    tx->Rollback();
                    tx=NULL;
                    isTransactionRunning=false;

                }
            }
        }
    }
};