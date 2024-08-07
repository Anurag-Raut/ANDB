#include <sys/un.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <optional>
#include <sstream>

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
        bool isUpdate = false;
        int n;
        char buffer[200000];
        while (true) {
            n = read(client_socket, buffer, sizeof(buffer) - 1);
            if (n == 0) {
                continue;
            }
            buffer[n] = '\0';
            query = buffer;
            std::vector<std::unique_ptr<Statement>> stmts;
            try {
                Tokenizer tokenizer(query);
                Parser parser(tokenizer.tokens, db);
                 stmts = parser.parse();
            } catch (const std::exception &e) {
                write(client_socket, e.what(), strlen(e.what()));
            }


            if (stmts.size() > 0) {
                for (auto &stmt : stmts) {
                    try {
                        if (isTransactionRunning && dynamic_cast<BeginStatement *>(stmt.get())) {
                            throw std::runtime_error("TRANSACTION ALREADY RUNNING");
                        } else if (dynamic_cast<BeginStatement *>(stmt.get())) {
                            isTransactionRunning = true;
                            tx = new Transaction(db);
                        }
                        if (dynamic_cast<UpdateStatement *>(stmt.get())) {
                            isUpdate = true;
                        }

                        if (!isTransactionRunning) {
                            tx = new Transaction(db);
                        }
                        string response = stmt->execute(tx);
                        write(client_socket, response.c_str(), response.length());
                        if (!isTransactionRunning) {
                            tx->Commit(false);
                            isUpdate = false;
                        } else if (dynamic_cast<CommitStatement *>(stmt.get())) {
                            tx->Commit(isUpdate);
                            isUpdate = false;
                            delete tx;
                            tx = nullptr;
                            isTransactionRunning = false;
                        } else if (dynamic_cast<RollbackStatement *>(stmt.get())) {
                            tx->Rollback();
                            isUpdate = false;
                            delete tx;
                            tx = nullptr;
                            isTransactionRunning = false;
                        }
                    } catch (const std::exception &e) {
                        write(client_socket, e.what(), strlen(e.what()));
                    }
                }
            }
        }
    }

    // Interpreter(Database *db, std::istream &input_stream, std::ostream &output_stream) {
    //     std::string query;
    //     Transaction *tx = nullptr;
    //     bool isTransactionRunning = false;

    //     while (std::getline(input_stream, query)) {
    //         Tokenizer tokenizer(query);
    //         Parser parser(tokenizer.tokens,db);
    //         std::vector<std::unique_ptr<Statement>> stmts = parser.parse();

    //         if (stmts.size() > 0) {
    //             for (auto &stmt : stmts) {
    //                 if (isTransactionRunning && dynamic_cast<BeginStatement *>(stmt.get())) {
    //                     throw std::runtime_error("TRANSACTION ALREADY RUNNING");
    //                 } else if (dynamic_cast<BeginStatement *>(stmt.get())) {
    //                     isTransactionRunning = true;
    //                     tx = new Transaction(db);
    //                 }

    //                 if (!isTransactionRunning) {
    //                     tx = new Transaction(db);
    //                 }

    //                 stmt->execute(tx);

    //                 if (!isTransactionRunning) {
    //                     tx->Commit();
    //                 } else if (dynamic_cast<CommitStatement *>(stmt.get())) {
    //                     tx->Commit();
    //                     delete tx;
    //                     tx = nullptr;
    //                     isTransactionRunning = false;
    //                 } else if (dynamic_cast<RollbackStatement *>(stmt.get())) {
    //                     tx->Rollback();
    //                     delete tx;
    //                     tx = nullptr;
    //                     isTransactionRunning = false;
    //                 }
    //             }
    //         }
    //     }
    // }
    // Interpreter(Database *db, string query) {
    //     // string query = "SELECT id,salary,age,name from test_table where index=\"key2\"";
    //     Tokenizer tokenizer(query);
    //     // tokenizer.print();
    //     Parser parser(tokenizer.tokens,db);

    //     vector<unique_ptr<Statement>> stmts = parser.parse();
    //     Transaction *tx = NULL;
    //     bool isTransactionRunning = false;

    //     if (stmts.size() > 0) {
    //         // stmt->print();
    //         for (auto &stmt : stmts) {

    //             if (isTransactionRunning && dynamic_cast<BeginStatement *>(stmt.get())) {
    //                 runtime_error("TRANSACTION ALREADY RUNNING");

    //             } else if (dynamic_cast<BeginStatement *>(stmt.get())) {
    //                 isTransactionRunning = true;
    //                 tx = new Transaction(db);
    //             }

    //             if (!isTransactionRunning) {
    //                 tx = new Transaction(db);
    //             }

    //             stmt->execute(tx);
    //             if (!isTransactionRunning) {
    //                 tx->Commit();
    //             } else if (dynamic_cast<CommitStatement *>(stmt.get())) {
    //                 tx->Commit(false);
    //                 tx = NULL;
    //                 isTransactionRunning = false;

    //             } else if (dynamic_cast<RollbackStatement *>(stmt.get())) {
    //                 tx->Rollback();
    //                 tx = NULL;
    //                 isTransactionRunning = false;
    //             }
    //         }
    //     }
    // }
};
