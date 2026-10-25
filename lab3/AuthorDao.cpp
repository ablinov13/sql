#include "AuthorDao.h"

void AuthorDao::dropTable()
{
    db_.exec("DROP TABLE IF EXISTS authors");
}

void AuthorDao::createTable()
{
    db_.exec("CREATE TABLE authors ("
             " id INTEGER PRIMARY KEY AUTOINCREMENT,"
             " name TEXT NOT NULL UNIQUE,"
             " country TEXT NOT NULL)");
}

int AuthorDao::insert(const Author& author)
{
    Statement st(db_, "INSERT INTO authors (name, country) VALUES (?, ?)");
    st.bind(1, author.name);
    st.bind(2, author.country);
    st.step();
    return db_.lastInsertId();
}

static Author readAuthor(const Statement& st)
{
    Author a;
    a.id = st.columnInt(0);
    a.name = st.columnText(1);
    a.country = st.columnText(2);
    return a;
}

std::vector<Author> AuthorDao::getAll()
{
    Statement st(db_, "SELECT id, name, country FROM authors ORDER BY id");
    std::vector<Author> result;
    while (st.step())
        result.push_back(readAuthor(st));
    return result;
}

std::optional<Author> AuthorDao::getByName(const std::string& name)
{
    Statement st(db_, "SELECT id, name, country FROM authors WHERE name = ?");
    st.bind(1, name);
    if (st.step())
        return readAuthor(st);
    return std::nullopt;
}
