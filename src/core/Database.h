#pragma once

/*
 * Database.h — обертка SQLite для хранения JSON событий по дате (RIA_DB).
 *
 * Два конструктора: путь к каталогу (файл db внутри) или путь к файлу БД;
 * connect открывает соединение, создает таблицу при необходимости.
 */

#include <filesystem>
#include <iostream>
#include <string>

#include <sqlite3.h>
#include <nlohmann/json.hpp>

using string = std::string;
using json = nlohmann::json;

namespace fs = std::filesystem;

class RIA_DB {
private:
    const string name_table_;

    sqlite3* db_ = nullptr;

    // Создает таблицу для хранения JSON по дате
    void createDB();

    // Выполняет SQL без результата
    bool exec(const char* sql);

    // Читает JSON для конкретной даты
    json select(const string& date_str);

    void closeConnection();

public:
    fs::path path_db;

    explicit RIA_DB(const fs::path& path_directory);

    RIA_DB(const fs::path& database_file, bool path_is_database_file);

    ~RIA_DB();

    bool connect();

    json getJsonByDate(const std::string& date_str);

    bool saveToDb(const std::string& date_str, const json& save_json);
};
