#pragma once
#include <string>

// Те же модели, что в лабе 3. ORM сама сопоставляет поля структур с колонками таблиц

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
