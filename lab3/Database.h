#pragma once
#include <stdexcept>
#include <string>
#include "sqlite3.h"

class DbException : public std::runtime_error {
public:
    explicit DbException(const std::string& message) : std::runtime_error(message) {}
};

class Database {
public:
    explicit Database(const std::string& path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* handle() const { return db_; }

    void exec(const std::string& sql);

    int lastInsertId() const;

private:
    sqlite3* db_ = nullptr;
};

class Statement {
public:
    Statement(Database& db, const std::string& sql);
    ~Statement();

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    void bind(int index, const std::string& value);
    void bind(int index, int value);

    bool step();

    std::string columnText(int index) const;
    int columnInt(int index) const;

private:
    Database& db_;
    sqlite3_stmt* stmt_ = nullptr;
};
