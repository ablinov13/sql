#pragma once
#include <stdexcept>
#include <string>
#include "sqlite3.h"

// Исключение для всех ошибок работы с БД. Бросается внутри DAO, ловится в main
class DbException : public std::runtime_error {
public:
    explicit DbException(const std::string& message) : std::runtime_error(message) {}
};

// Подключение к БД. Открывается в конструкторе, закрывается в деструкторе (RAII)
class Database {
public:
    explicit Database(const std::string& path);
    ~Database();

    // подключение нельзя копировать: иначе sqlite3_close вызовется дважды
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* handle() const { return db_; }

    // выполнить SQL без результата (CREATE, DROP, BEGIN, COMMIT)
    void exec(const std::string& sql);

    // id последней вставленной строки
    int lastInsertId() const;

private:
    sqlite3* db_ = nullptr;
};

// Подготовленный запрос. sqlite3_finalize вызывается в деструкторе,
// поэтому запрос освобождается даже если вылетело исключение
class Statement {
public:
    Statement(Database& db, const std::string& sql);
    ~Statement();

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    void bind(int index, const std::string& value);
    void bind(int index, int value);

    // выполнить шаг: true — получена строка результата, false — запрос завершён
    bool step();

    std::string columnText(int index) const;
    int columnInt(int index) const;

private:
    Database& db_;
    sqlite3_stmt* stmt_ = nullptr;
};
