
#include <string>
#include <vector>
using namespace std;

struct Column {
    string name;
    string type;
};

class Table {
   private:
   //indexes
   public:
    vector<Column> columns;
    string table_name;
    Table(string table_name, vector<string> types,  vector<string> names);
};

Table::Table(string table_name,  vector<string> types,  vector<string> names) {
    if (types.size() != names.size()) {
        return;
    }

    for (size_t i = 0; i < types.size(); ++i) {
        columns.push_back({names[i], types[i]});
    }
}
