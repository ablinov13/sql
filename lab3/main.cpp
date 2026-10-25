// Лабораторная работа 3.1. Код лабы 2 переведён на API DAO, две таблицы: authors и books.
// Правила: в DAO нет вывода и try/catch. Весь вывод и обработка ошибок только здесь, в main.

#include <windows.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "AuthorDao.h"
#include "BookDao.h"
#include "Database.h"

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

void printBook(const Book& b)
{
    std::cout << pad(std::to_string(b.id), 4) << pad(b.title, 34) << pad(b.genre, 12)
              << pad(std::to_string(b.price), 6) << b.year << "\n";
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    try {
        Database db("lab3.db");
        AuthorDao authorDao(db);
        BookDao bookDao(db);
        std::cout << "Подключение к lab3.db выполнено\n";

        // 1. Структура базы: сначала удаляем books (она ссылается на authors), потом authors
        bookDao.dropTable();
        authorDao.dropTable();
        authorDao.createTable();
        bookDao.createTable();

        // 2. Добавление элементов: создаём объект и помещаем в базу через XxxDao.insert
        for (const auto& f : readCsv("data/authors.txt", 2)) {
            Author author;
            author.name = f[0];
            author.country = f[1];
            authorDao.insert(author);
        }

        for (const auto& f : readCsv("data/books.txt", 5)) {
            auto author = authorDao.getByName(f[4]);
            if (!author)
                throw std::runtime_error("Автор не найден в authors.txt: " + f[4]);

            Book book;
            book.title = f[0];
            book.genre = f[1];
            book.price = std::stoi(f[2]);
            book.year = std::stoi(f[3]);
            book.authorId = author->id;
            bookDao.insert(book);
        }

        // 3. Выборка элементов: через XxxDao.getXxx, итерация по списку объектов
        std::vector<Author> authors = authorDao.getAll();
        std::vector<Book> books = bookDao.getAll();
        std::cout << "Загружено авторов: " << authors.size() << ", книг: " << books.size() << "\n";

        std::cout << "\nАвторы (authorDao.getAll):\n";
        for (const Author& a : authors)
            std::cout << pad(std::to_string(a.id), 4) << pad(a.name, 28) << a.country << "\n";

        std::cout << "\nЗапрос 1. Книги жанра «Фантастика» (bookDao.getByGenre):\n";
        for (const Book& b : bookDao.getByGenre("Фантастика"))
            printBook(b);

        std::cout << "\nЗапрос 2. Книги от 500 до 700 руб. (bookDao.getByPriceRange):\n";
        for (const Book& b : bookDao.getByPriceRange(500, 700))
            printBook(b);

        std::cout << "\nЗапрос 3. Книги с авторами, JOIN двух таблиц (bookDao.getAllWithAuthors):\n";
        for (const BookWithAuthor& item : bookDao.getAllWithAuthors())
            std::cout << pad(item.authorName, 28) << pad(item.book.title, 34) << item.book.year << "\n";
    }
    catch (const DbException& e) {
        std::cerr << "Ошибка базы данных: " << e.what() << "\n";
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
