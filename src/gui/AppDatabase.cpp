#include "AppDatabase.h"

#include <filesystem>
#include <wx/translation.h>

namespace {
    bool JsonHasEvents(const json& data) {
        return data.is_array() && !data.empty();
    }

    std::filesystem::path PathFromWx(const wxString& path) {
    #if defined(_WIN32)
        return std::filesystem::path(std::wstring(path.wc_str()));
    #else
        return std::filesystem::path(std::string(path.utf8_str()));
    #endif
    }

    wxString PathToWx(const std::filesystem::path& path) {
        #if defined(_WIN32)
            return wxString(path.wstring());
        #else
            return wxString::FromUTF8(path.u8string());
        #endif
    }
}

bool AppDatabase::OpenExistingFile(const wxString& path, wxString& errorOut) {
    Close();

    try {
        db_ = std::make_unique<RIA_DB>(PathFromWx(path), true);
    }
    catch (const std::exception& e) {
        errorOut = wxString::FromUTF8(e.what());
        return false;
    }

    if (!db_->connect()) {
        errorOut = L"Не удалось открыть базу данных.";
        db_.reset();
        return false;
    }

    rows_.clear();
    return true;
}

bool AppDatabase::OpenFolder(const wxString& folderPath, wxString& errorOut) {
    Close();

    try {
        db_ = std::make_unique<RIA_DB>(PathFromWx(folderPath));
    }
    catch (const std::exception& e) {
        errorOut = wxString::FromUTF8(e.what());
        return false;
    }

    if (!db_->connect()) {
        errorOut = L"Не удалось создать или открыть базу данных в выбранной папке.";
        db_.reset();
        return false;
    }

    rows_.clear();
    return true;
}

void AppDatabase::Close() {
    db_.reset();
    rows_.clear();
}

wxString AppDatabase::DisplayPath() const {
    if (!db_) {
        return {};
    }
    return PathToWx(db_->path_db);
}

void AppDatabase::FillRowsFromJson(const json& events) {
    rows_.clear();
    rows_.reserve(events.size());

    int index = 1;
    for (const auto& item : events) {
        if (!item.is_object()) {
            continue;
        }

        EventRow row;
        row.number = index++;
        row.area = wxString::FromUTF8(item.value("area", ""));
        row.name = wxString::FromUTF8(item.value("name", ""));
        row.eventType = wxString::FromUTF8(item.value("icon", ""));
        rows_.push_back(std::move(row));
    }
}

bool AppDatabase::LoadForDate(const wxString& dateStr, wxString& errorOut) {
    if (!db_) {
        errorOut = L"База данных не открыта.";
        return false;
    }

    const std::string dateUtf8(dateStr.utf8_str());
    // 1) Пробуем достать данные локально
    json stored = db_->getJsonByDate(dateUtf8);

    if (!JsonHasEvents(stored)) {
        try {
            // 2) Если в БД пусто — загружаем с API
            const json events = ParseJsonRIA::fetchFromApi(dateUtf8);

            if (!JsonHasEvents(events)) {
                errorOut = L"Парсер не вернул события для выбранной даты.";
                rows_.clear();
                return false;
            }

            if (!db_->saveToDb(dateUtf8, events)) {
                errorOut = L"Не удалось сохранить данные в базу.";
                return false;
            }

            // 3) Используем только что загруженные данные.=
            stored = events;
        }
        catch (const std::exception& e) {
            errorOut = wxString::Format(
                L"Ошибка загрузки данных: %s",
                wxString::FromUTF8(e.what())
            );
            return false;
        }
    }

    FillRowsFromJson(stored);
    return true;
}
