#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <sys/un.h>
#include <unistd.h>
#include "./expr.cpp"
#include "./include/statement.hpp"
#include "./parser.cpp"
#include "./tokenizer.cpp"

class Interpreter {
   public:
    Interpreter(Database *db, int client_socket) {
        std::string query;
        Transaction *tx = nullptr;
        bool isTransactionRunning = false;
        int n;
        char buffer[256];
        while ( true) {
            n = read(client_socket, buffer, sizeof(buffer) - 1);
            if(n==0){
                continue;
            }
            buffer[n] = '\0';
            query=buffer;
            cout<<"QUERY : "<<query<<endl;
            Tokenizer tokenizer(query);
            Parser parser(tokenizer.tokens);
            std::vector<std::unique_ptr<Statement>> stmts = parser.parse();

            if (stmts.size() > 0) {
                for (auto &stmt : stmts) {
                    if (isTransactionRunning && dynamic_cast<BeginStatement *>(stmt.get())) {
                        throw std::runtime_error("TRANSACTION ALREADY RUNNING");
                    } else if (dynamic_cast<BeginStatement *>(stmt.get())) {
                        isTransactionRunning = true;
                        tx = new Transaction(db);
                    }

                    if (!isTransactionRunning) {
                        tx = new Transaction(db);
                    }
                cout<<"BBSSSSS"<<endl;
                    stmt->execute(tx);
                    write(client_socket, query.c_str(), query.length());
                    if (!isTransactionRunning) {
                        tx->Commit();
                    } else if (dynamic_cast<CommitStatement *>(stmt.get())) {
                        // std::cout << "OH YEAHH BABBY" << std::endl;
                        tx->Commit();
                        delete tx;
                        tx = nullptr;
                        isTransactionRunning = false;
                    } else if (dynamic_cast<RollbackStatement *>(stmt.get())) {
                        // std::cout << "OH YEAHH ROLLBACK" << std::endl;
                        tx->Rollback();
                        delete tx;
                        tx = nullptr;
                        isTransactionRunning = false;
                    }
                }
            }
        }
    }

    Interpreter(Database *db, std::istream &input_stream, std::ostream &output_stream) {
        std::string query;
        Transaction *tx = nullptr;
        bool isTransactionRunning = false;

        while (std::getline(input_stream, query)) {
            Tokenizer tokenizer(query);
            Parser parser(tokenizer.tokens);
            std::vector<std::unique_ptr<Statement>> stmts = parser.parse();

            if (stmts.size() > 0) {
                for (auto &stmt : stmts) {
                    if (isTransactionRunning && dynamic_cast<BeginStatement *>(stmt.get())) {
                        throw std::runtime_error("TRANSACTION ALREADY RUNNING");
                    } else if (dynamic_cast<BeginStatement *>(stmt.get())) {
                        isTransactionRunning = true;
                        tx = new Transaction(db);
                    }

                    if (!isTransactionRunning) {
                        tx = new Transaction(db);
                    }

                    stmt->execute(tx);

                    if (!isTransactionRunning) {
                        tx->Commit();
                    } else if (dynamic_cast<CommitStatement *>(stmt.get())) {
                        // std::cout << "OH YEAHH BABBY" << std::endl;
                        tx->Commit();
                        delete tx;
                        tx = nullptr;
                        isTransactionRunning = false;
                    } else if (dynamic_cast<RollbackStatement *>(stmt.get())) {
                        // std::cout << "OH YEAHH ROLLBACK" << std::endl;
                        tx->Rollback();
                        delete tx;
                        tx = nullptr;
                        isTransactionRunning = false;
                    }
                }
            }
        }
    }
    Interpreter(Database *db, string query) {
        // string query = "SELECT id,salary,age,name from test_table where index=\"key2\"";
        // cout<<"THE ROCK: "<<query<<endl;oo
        Tokenizer tokenizer(query);
        // tokenizer.print();
        Parser parser(tokenizer.tokens);

        vector<unique_ptr<Statement>> stmts = parser.parse();
        Transaction *tx = NULL;
        bool isTransactionRunning = false;

        if (stmts.size() > 0) {
            // stmt->print();
            // cout<<"LESS GO"<<endl;
            for (auto &stmt : stmts) {
                // cout<<"ARE WE GOOD"<<endl;

                if (isTransactionRunning && dynamic_cast<BeginStatement *>(stmt.get())) {
                    runtime_error("TRANSACTION ALREADY RUNNING");

                } else if (dynamic_cast<BeginStatement *>(stmt.get())) {
                    isTransactionRunning = true;
                    tx = new Transaction(db);
                }

                if (!isTransactionRunning) {
                    tx = new Transaction(db);
                }

                stmt->execute(tx);
                if (!isTransactionRunning) {
                    tx->Commit();
                } else if (dynamic_cast<CommitStatement *>(stmt.get())) {
                    cout << "OH YEAHH BABBY" << endl;
                    tx->Commit();
                    tx = NULL;
                    isTransactionRunning = false;

                } else if (dynamic_cast<RollbackStatement *>(stmt.get())) {
                    cout << "OH YEAHH ROLLBACK" << endl;
                    tx->Rollback();
                    tx = NULL;
                    isTransactionRunning = false;
                }
            }
        }
    }
};
