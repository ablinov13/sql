#pragma once
#include <optional>
#include <string>
#include <vector>
#include "Database.h"
#include "Models.h"

class AuthorDao {
public:
    explicit AuthorDao(Database& db) : db_(db) {}

    void dropTable();
    void createTable();
    int insert(const Author& author);
    std::vector<Author> getAll();
    std::optional<Author> getByName(const std::string& name);

private:
    Database& db_;
};
