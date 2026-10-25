#include "BookDao.h"

static const char* BOOK_COLUMNS = "b.id, b.title, b.genre, b.price, b.year, b.author_id";

void BookDao::dropTable()
{
    db_.exec("DROP TABLE IF EXISTS books");
}

void BookDao::createTable()
{
    db_.exec("CREATE TABLE books ("
             " id INTEGER PRIMARY KEY AUTOINCREMENT,"
             " title TEXT NOT NULL,"
             " genre TEXT NOT NULL,"
             " price INTEGER NOT NULL,"
             " year INTEGER NOT NULL,"
             " author_id INTEGER NOT NULL REFERENCES authors(id))");
}

int BookDao::insert(const Book& book)
{
    Statement st(db_, "INSERT INTO books (title, genre, price, year, author_id) VALUES (?, ?, ?, ?, ?)");
    st.bind(1, book.title);
    st.bind(2, book.genre);
    st.bind(3, book.price);
    st.bind(4, book.year);
    st.bind(5, book.authorId);
    st.step();
    return db_.lastInsertId();
}

// Собирает объект Book из текущей строки результата (колонки BOOK_COLUMNS)
static Book readBook(const Statement& st)
{
    Book b;
    b.id = st.columnInt(0);
    b.title = st.columnText(1);
    b.genre = st.columnText(2);
    b.price = st.columnInt(3);
    b.year = st.columnInt(4);
    b.authorId = st.columnInt(5);
    return b;
}

std::vector<Book> BookDao::getAll()
{
    Statement st(db_, std::string("SELECT ") + BOOK_COLUMNS + " FROM books b ORDER BY b.id");
    std::vector<Book> result;
    while (st.step())
        result.push_back(readBook(st));
    return result;
}

std::vector<Book> BookDao::getByGenre(const std::string& genre)
{
    Statement st(db_, std::string("SELECT ") + BOOK_COLUMNS + " FROM books b WHERE b.genre = ? ORDER BY b.title");
    st.bind(1, genre);
    std::vector<Book> result;
    while (st.step())
        result.push_back(readBook(st));
    return result;
}

std::vector<Book> BookDao::getByPriceRange(int from, int to)
{
    Statement st(db_, std::string("SELECT ") + BOOK_COLUMNS +
                      " FROM books b WHERE b.price BETWEEN ? AND ? ORDER BY b.price");
    st.bind(1, from);
    st.bind(2, to);
    std::vector<Book> result;
    while (st.step())
        result.push_back(readBook(st));
    return result;
}

std::vector<BookWithAuthor> BookDao::getAllWithAuthors()
{
    // JOIN соединяет каждую книгу с её автором по author_id
    Statement st(db_, std::string("SELECT ") + BOOK_COLUMNS + ", a.name"
                      " FROM books b JOIN authors a ON a.id = b.author_id"
                      " ORDER BY a.name, b.year");
    std::vector<BookWithAuthor> result;
    while (st.step())
        result.push_back({ readBook(st), st.columnText(6) });
    return result;
}
