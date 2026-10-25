#pragma once
#include <string>
#include <sqlite_orm.h>
#include "Models.h"

// Описание структуры базы для ORM: какая структура какой таблице соответствует
// и какое поле какой колонке. SQL CREATE TABLE ORM строит сама по этому описанию.
inline auto makeStorage(const std::string& path)
{
    using namespace sqlite_orm;
    return make_storage(path,
        make_table("authors",
            make_column("id", &Author::id, primary_key().autoincrement()),
            make_column("name", &Author::name, unique()),
            make_column("country", &Author::country)),
        make_table("books",
            make_column("id", &Book::id, primary_key().autoincrement()),
            make_column("title", &Book::title),
            make_column("genre", &Book::genre),
            make_column("price", &Book::price),
            make_column("year", &Book::year),
            make_column("author_id", &Book::authorId),
            foreign_key(&Book::authorId).references(&Author::id)));
}

// Тип хранилища (длинный шаблонный тип, поэтому берём его через decltype)
using Storage = decltype(makeStorage(""));
