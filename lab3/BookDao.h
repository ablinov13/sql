#pragma once
#include <string>
#include <vector>
#include "Database.h"
#include "Models.h"

// DAO для таблицы books. Как и AuthorDao: без вывода и без перехвата исключений
class BookDao {
public:
    explicit BookDao(Database& db) : db_(db) {}

    void dropTable();
    void createTable();
    int insert(const Book& book);  // возвращает id новой записи
    std::vector<Book> getAll();
    std::vector<Book> getByGenre(const std::string& genre);
    std::vector<Book> getByPriceRange(int from, int to);
    std::vector<BookWithAuthor> getAllWithAuthors();

private:
    Database& db_;
};
