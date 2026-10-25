#include "Database.h"

Database::Database(const std::string& path)
{
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        std::string message = "Не удалось открыть БД " + path + ": " + sqlite3_errmsg(db_);
        sqlite3_close(db_);
        throw DbException(message);
    }
    exec("PRAGMA foreign_keys = ON");  // включить проверку внешних ключей
}

Database::~Database()
{
    sqlite3_close(db_);
}

void Database::exec(const std::string& sql)
{
    char* error = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
        std::string message = "Ошибка SQL: " + std::string(error) + " [" + sql + "]";
        sqlite3_free(error);
        throw DbException(message);
    }
}

int Database::lastInsertId() const
{
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

Statement::Statement(Database& db, const std::string& sql) : db_(db)
{
    if (sqlite3_prepare_v2(db_.handle(), sql.c_str(), -1, &stmt_, nullptr) != SQLITE_OK)
        throw DbException("Ошибка подготовки запроса: " + std::string(sqlite3_errmsg(db_.handle())) + " [" + sql + "]");
}

Statement::~Statement()
{
    sqlite3_finalize(stmt_);
}

void Statement::bind(int index, const std::string& value)
{
    if (sqlite3_bind_text(stmt_, index, value.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
        throw DbException("Ошибка bind: " + std::string(sqlite3_errmsg(db_.handle())));
}

void Statement::bind(int index, int value)
{
    if (sqlite3_bind_int(stmt_, index, value) != SQLITE_OK)
        throw DbException("Ошибка bind: " + std::string(sqlite3_errmsg(db_.handle())));
}

bool Statement::step()
{
    int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    throw DbException("Ошибка выполнения запроса: " + std::string(sqlite3_errmsg(db_.handle())));
}

std::string Statement::columnText(int index) const
{
    const unsigned char* text = sqlite3_column_text(stmt_, index);
    return text ? reinterpret_cast<const char*>(text) : "";
}

int Statement::columnInt(int index) const
{
    return sqlite3_column_int(stmt_, index);
}
