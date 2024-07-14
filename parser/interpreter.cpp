

#include "./tokenizer.cpp"

class Interpreter {
   public:
    Interpreter() {

        string query = "CREATE TABLE tablename";
        // cout<<"THE ROCK: "<<query<<endl;
        Tokenizer tokenizer(string("CREATE TABLE tablename"));

        tokenizer.print();
    }
};