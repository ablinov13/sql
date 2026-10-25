// Лабораторная работа 2. Подключение к встраиваемой БД (SQLite) из программы на C++.
// Создаём таблицу books, загружаем data/books.txt и выполняем 3 запроса SELECT.

#include <windows.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "sqlite3.h"

// Дополняет строку пробелами до width символов (считаем символы, а не байты UTF-8)
std::string pad(const std::string& s, size_t width)
{
    size_t chars = 0;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80) chars++;
    return chars >= width ? s : s + std::string(width - chars, ' ');
}

// Выполняет SQL без результата (CREATE, DROP, BEGIN, COMMIT). При ошибке печатает её и возвращает false
bool execSql(sqlite3* db, const char* sql)
{
    char* error = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &error) != SQLITE_OK) {
        std::cerr << "Ошибка SQL: " << error << "\n";
        sqlite3_free(error);
        return false;
    }
    return true;
}

// Выполняет SELECT и печатает все строки результата в виде таблицы
bool printQuery(sqlite3* db, const std::string& title, const char* sql, const std::vector<size_t>& widths)
{
    std::cout << "\n" << title << "\n" << sql << "\n";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Ошибка подготовки запроса: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    int columns = sqlite3_column_count(stmt);
    for (int i = 0; i < columns; i++)
        std::cout << pad(sqlite3_column_name(stmt, i), widths[i]);
    size_t total = 0;
    for (size_t w : widths) total += w;
    std::cout << "\n" << std::string(total, '-') << "\n";

    // sqlite3_step возвращает SQLITE_ROW, пока есть строки
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        for (int i = 0; i < columns; i++) {
            const unsigned char* text = sqlite3_column_text(stmt, i);
            std::cout << pad(text ? reinterpret_cast<const char*>(text) : "NULL", widths[i]);
        }
        std::cout << "\n";
    }
    sqlite3_finalize(stmt);
    return true;
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    // 1. Подключение к базе (файл создаётся, если его нет)
    sqlite3* db = nullptr;
    if (sqlite3_open("lab2.db", &db) != SQLITE_OK) {
        std::cerr << "Не удалось открыть БД: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }
    std::cout << "Подключение к lab2.db выполнено (SQLite " << sqlite3_libversion() << ")\n";

    // 2. Создание таблицы
    if (!execSql(db, "DROP TABLE IF EXISTS books") ||
        !execSql(db, "CREATE TABLE books ("
                     " title TEXT NOT NULL,"
                     " genre TEXT NOT NULL,"
                     " price INTEGER NOT NULL,"
                     " year INTEGER NOT NULL,"
                     " author TEXT NOT NULL)")) {
        sqlite3_close(db);
        return 1;
    }

    // 3. Загрузка данных из текстового файла
    std::ifstream file("data/books.txt");
    if (!file) {
        std::cerr << "Не найден файл data/books.txt (запускайте из папки решения)\n";
        sqlite3_close(db);
        return 1;
    }

    // Подготовленный запрос: ? заменяются значениями через sqlite3_bind_*
    sqlite3_stmt* insert = nullptr;
    sqlite3_prepare_v2(db, "INSERT INTO books VALUES (?, ?, ?, ?, ?)", -1, &insert, nullptr);

    execSql(db, "BEGIN");  // все вставки в одной транзакции: быстрее и надёжнее
    int loaded = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        // разбиваем строку по запятым: название,жанр,цена,год,автор
        std::vector<std::string> f;
        std::stringstream ss(line);
        std::string part;
        while (std::getline(ss, part, ',')) f.push_back(part);
        if (f.size() != 5) {
            std::cerr << "Пропущена строка: " << line << "\n";
            continue;
        }

        sqlite3_bind_text(insert, 1, f[0].c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insert, 2, f[1].c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(insert, 3, std::stoi(f[2]));
        sqlite3_bind_int(insert, 4, std::stoi(f[3]));
        sqlite3_bind_text(insert, 5, f[4].c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(insert) != SQLITE_DONE)
            std::cerr << "Ошибка вставки: " << sqlite3_errmsg(db) << "\n";
        else
            loaded++;
        sqlite3_reset(insert);  // подготовить тот же запрос к следующей строке
    }
    execSql(db, "COMMIT");
    sqlite3_finalize(insert);
    std::cout << "Загружено строк: " << loaded << "\n";

    // 4. Запросы SELECT
    printQuery(db, "Запрос 1: детективы дешевле 450 руб.",
        "SELECT title, price FROM books WHERE genre = 'Детектив' AND price < 450",
        { 34, 8 });
    printQuery(db, "Запрос 2: 5 самых дорогих книг",
        "SELECT title, genre, price FROM books ORDER BY price DESC LIMIT 5",
        { 34, 14, 8 });
    printQuery(db, "Запрос 3: книги 2022 года",
        "SELECT title, author, year FROM books WHERE year = 2022 ORDER BY title",
        { 34, 28, 6 });

    // 5. Закрытие соединения
    sqlite3_close(db);
    return 0;
}
