#pragma once
#include <string>

// Модели: обычные структуры, каждая соответствует строке таблицы

// Строка таблицы authors
struct Author {
    int id = 0;
    std::string name;
    std::string country;
};

// Строка таблицы books (author_id ссылается на authors.id)
struct Book {
    int id = 0;
    std::string title;
    std::string genre;
    int price = 0;
    int year = 0;
    int authorId = 0;
};

// Результат JOIN: книга + имя её автора
struct BookWithAuthor {
    Book book;
    std::string authorName;
};
