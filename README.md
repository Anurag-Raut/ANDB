# ANDB
ANDB is a relational database management system (RDBMS) designed to handle complex queries and manage data efficiently. It features an SQL parser and uses B+ trees for indexing to ensure fast data retrieval and organization.

# Features
Relational Database Management: Organize and manage data in a structured, tabular format.
SQL Parser: Parse and execute SQL statements for querying and manipulating data.
B+ Tree Indexes: Utilize B+ trees for efficient indexing and fast data access.
Thread Management: Handle multiple queries and operations simultaneously with a multi-threaded architecture.

# Getting Started
### Prerequisites
C++11 or later
CMake (for building the project)
Basic understanding of SQL and relational databases
### Installation
Clone the Repository:

```
git clone https://github.com/yourusername/MyRelationalDB.git
cd andb
```
Build the Project:

```
mkdir build
cmake -B build -S .
cmake --build ./build
```
Run the Daemon:

```
./build/andb
```
Use the CLI Tool:



# Usage
***Commands***

```CONNECT <database_name>```

Connects to the specified database. Creates the database if it does not exist.

```EXIT```

Terminates the current session 


# Architecture
***SQL Parser*** : Handles the parsing and execution of SQL queries.</br>
***B+ Tree Indexing*** : Provides efficient indexing and quick access to data.</br>
***Daemon Service*** : Runs in the background to handle database operations.</br>
***CLI Tool*** : Provides a command-line interface to interact with the database.</br>