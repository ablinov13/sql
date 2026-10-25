// Лабораторная работа 4. Код лабы 3 (DAO, две таблицы) переведён на ORM (библиотека sqlite_orm).
// Как и раньше: создаём структуру базы, добавляем элементы, делаем 3 осмысленных запроса.

#include <windows.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Storage.h"

using namespace sqlite_orm;

// Дополняет строку пробелами до width символов (символы UTF-8, а не байты)
std::string pad(const std::string& s, size_t width)
{
    size_t chars = 0;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80) chars++;
    return chars >= width ? s : s + std::string(width - chars, ' ');
}

// Читает файл "значение,значение,..." в список строк, каждая строка — список полей
std::vector<std::vector<std::string>> readCsv(const std::string& path, size_t fieldCount)
{
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Не найден файл " + path + " (запускайте из папки решения)");

    std::vector<std::vector<std::string>> rows;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string part;
        while (std::getline(ss, part, ',')) fields.push_back(part);
        if (fields.size() != fieldCount)
            throw std::runtime_error("Неверная строка в " + path + ": " + line);
        rows.push_back(fields);
    }
    return rows;
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    try {
        // 1. Структура базы: удаляем старый файл, ORM создаёт таблицы по описанию из Storage.h
        std::remove("lab4.db");
        Storage storage = makeStorage("lab4.db");
        storage.sync_schema();
        std::cout << "База lab4.db создана, таблицы: authors, books\n";

        // 2. Добавление элементов: создаём объекты и передаём их в storage.insert
        storage.transaction([&] {
            for (const auto& f : readCsv("data/authors.txt", 2)) {
                Author author{ 0, f[0], f[1] };
                storage.insert(author);
            }
            for (const auto& f : readCsv("data/books.txt", 5)) {
                // ищем автора по имени запросом ORM, а не SQL-строкой
                auto found = storage.get_all<Author>(where(c(&Author::name) == f[4]));
                if (found.empty())
                    throw std::runtime_error("Автор не найден в authors.txt: " + f[4]);

                Book book{ 0, f[0], f[1], std::stoi(f[2]), std::stoi(f[3]), found[0].id };
                storage.insert(book);
            }
            return true;  // true — зафиксировать транзакцию (COMMIT)
        });
        std::cout << "Добавлено авторов: " << storage.count<Author>()
                  << ", книг: " << storage.count<Book>() << "\n";

        // 3. Запросы

        // Запрос 1: группировка по повторяющимся значениям (жанр)
        std::cout << "\nЗапрос 1. Количество книг и средняя цена по жанрам (GROUP BY):\n";
        auto byGenre = storage.select(
            columns(&Book::genre, count(&Book::id), avg(&Book::price)),
            group_by(&Book::genre),
            order_by(count(&Book::id)).desc());
        for (const auto& [genre, booksCount, avgPrice] : byGenre)
            std::cout << pad(genre, 14) << pad(std::to_string(booksCount) + " шт.", 8)
                      << "средняя цена " << std::lround(avgPrice) << " руб.\n";

        // Запрос 2: выборка по диапазону (цена от 500 до 700)
        std::cout << "\nЗапрос 2. Книги от 500 до 700 руб. (BETWEEN):\n";
        auto middle = storage.get_all<Book>(
            where(between(&Book::price, 500, 700)),
            order_by(&Book::price));
        for (const Book& b : middle)
            std::cout << pad(b.title, 34) << pad(b.genre, 12) << b.price << "\n";

        // Запрос 3: две таблицы через JOIN — книги британских авторов
        std::cout << "\nЗапрос 3. Книги авторов из Великобритании (JOIN authors):\n";
        auto british = storage.select(
            columns(&Author::name, &Book::title, &Book::year),
            inner_join<Author>(on(c(&Book::authorId) == &Author::id)),
            where(c(&Author::country) == "Великобритания"),
            multi_order_by(order_by(&Author::name), order_by(&Book::year)));
        for (const auto& [author, title, year] : british)
            std::cout << pad(author, 20) << pad(title, 34) << year << "\n";
    }
    catch (const std::system_error& e) {
        // sqlite_orm сообщает об ошибках SQLite через std::system_error
        std::cerr << "Ошибка базы данных: " << e.what() << "\n";
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
