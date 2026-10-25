#pragma once
#include <string>


struct Author {
    int id = 0;
    std::string name;
    std::string country;
};

struct Book {
    int id = 0;
    std::string title;
    std::string genre;
    int price = 0;
    int year = 0;
    int authorId = 0;
};

struct BookWithAuthor {
    Book book;
    std::string authorName;
};
