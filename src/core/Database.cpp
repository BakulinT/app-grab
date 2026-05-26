/*
 * Database.cpp — DDL, выборки и upsert JSON по уникальной дате.
 *
 * На Windows для путей с не-ASCII используется sqlite3_open16; на остальных
 * платформах — sqlite3_open_v2 с UTF-8 путем из std::filesystem.
 */

#include "Database.h"

#include <optional>
#include <stdexcept>

RIA_DB::RIA_DB(const fs::path& path_directory) : name_table_("RIA_SOP_DATE") {
    if (!fs::exists(path_directory) || !fs::is_directory(path_directory)) {
        throw std::runtime_error("Invalid directory path");
    }
    // При работе с папкой используем стандартное имя файла БД
    path_db = path_directory / "db_rio.db";
}

RIA_DB::RIA_DB(const fs::path& database_file, bool path_is_database_file) : name_table_("RIA_SOP_DATE") {
    if (!path_is_database_file) {
        throw std::runtime_error("Invalid database path mode");
    }

    if (!fs::exists(database_file) || !fs::is_regular_file(database_file)) {
        throw std::runtime_error("Invalid database file path");
    }
    path_db = database_file;
}

RIA_DB::~RIA_DB() {
    closeConnection();
}

void RIA_DB::createDB() {
    exec(
        "CREATE TABLE IF NOT EXISTS RIA_SOP_DATE ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "date_event TEXT UNIQUE NOT NULL,"
            "json_text TEXT NOT NULL"
        ");"
    );
}

bool RIA_DB::exec(const char* sql) {
    char* errMsg = nullptr;

    if (sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "[APP] SQL error: " << errMsg << "\n";
        sqlite3_free(errMsg);

        return false;
    }
    return true;
}

json RIA_DB::select(const string& date_str) {
    sqlite3_stmt* stmt = nullptr;

    const string sql = "SELECT date_event, json_text FROM " + name_table_ + " WHERE date_event = ?;";

    const int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr 
            << "[APP] Error preparing request: "
            << sqlite3_errmsg(db_) << '\n';
        return json{};
    }

    sqlite3_bind_text(stmt, 1, date_str.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<json> result;

    // Читаем максимум одну запись: date_event уникален в таблице
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 1);
        if (text) {
            try {
                result = json::parse(reinterpret_cast<const char*>(text));
            }
            catch (const std::exception& e) {
                std::cerr << "[APP] Error parsing JSON: " << e.what() << '\n';
            }
        }
    }
    sqlite3_finalize(stmt);

    return result.value_or(json::array());
}

void RIA_DB::closeConnection() {
    if (db_) {
        sqlite3_close(static_cast<sqlite3*>(db_));
        db_ = nullptr;
    }
}


bool RIA_DB::connect() {
    closeConnection();

    const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

    int rc = SQLITE_ERROR;
    #if defined(_WIN32)
        const std::wstring wpath = path_db.wstring();
        rc = sqlite3_open16(wpath.c_str(), &db_);
    #else
        const std::string utf8Path = path_db.u8string();
        rc = sqlite3_open_v2(utf8Path.c_str(), &db_, flags, nullptr);
    #endif

    if (rc != SQLITE_OK) {
        std::cerr
            << "[APP] Error open DB (" << path_db.u8string() << "): "
            << (db_ ? sqlite3_errmsg(db_) : "unknown") << "\n";

        closeConnection();
        return false;
    }

    // Гарантируем, что таблица существует перед первым использованием
    createDB();
    return true;
}

json RIA_DB::getJsonByDate(const std::string& date_str) {
    return select(date_str);
}

bool RIA_DB::saveToDb(const std::string& date_str, const json& save_json) {
    const string sql = 
        "INSERT INTO " + name_table_ + " (date_event, json_text) VALUES (?, ?) "
            "ON CONFLICT(date_event) DO UPDATE SET json_text = EXCLUDED.json_text;";

    sqlite3_stmt* stmt = nullptr;

    if (
        sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK
    ) {
        return false;
    }

    const std::string json_text = save_json.dump();

    // INSERT по date_event: новая запись или обновление существующей
    sqlite3_bind_text(stmt, 1, date_str.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, json_text.c_str(), -1, SQLITE_STATIC);

    const auto status = sqlite3_step(stmt);
    const bool isSuccess = (status == SQLITE_DONE);

    sqlite3_finalize(stmt);

    return isSuccess;
}