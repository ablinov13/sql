#pragma once
#include <optional>
#include <string>
#include <vector>
#include "Database.h"
#include "Models.h"

// DAO для таблицы authors: весь SQL про авторов собран здесь.
// Нет вывода на экран и нет try/catch: ошибки уходят исключением DbException в main
class AuthorDao {
public:
    explicit AuthorDao(Database& db) : db_(db) {}

    void dropTable();
    void createTable();
    int insert(const Author& author);  // возвращает id новой записи
    std::vector<Author> getAll();
    std::optional<Author> getByName(const std::string& name);

private:
    Database& db_;
};
