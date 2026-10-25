#pragma once
#include <string>
#include <vector>
#include "Database.h"
#include "Models.h"

class BookDao {
public:
    explicit BookDao(Database& db) : db_(db) {}

    void dropTable();
    void createTable();
    int insert(const Book& book);
    std::vector<Book> getAll();
    std::vector<Book> getByGenre(const std::string& genre);
    std::vector<Book> getByPriceRange(int from, int to);
    std::vector<BookWithAuthor> getAllWithAuthors();

private:
    Database& db_;
};
