#pragma once

/*
 * AppDatabase.h - слой между GUI и ядром (RIA_DB + HTTP-парсер).
 *
 * Открытие sqlite БД, загрузка событий за дату с приоритетом
 * локального JSON в БД и fetch при пустом кэше.
 */

#include "core/Database.h"
#include "core/Parser.h"

#include <wx/string.h>

#include <memory>
#include <vector>

struct EventRow {
    int number = 0;
    wxString area;
    wxString name;
    wxString eventType;
};

class AppDatabase {
public:
    // Открыть существующий .db/.sqlite файл
    bool OpenExistingFile(const wxString& path, wxString& errorOut);
    
    // Открыть (или создать) БД в выбранной папке
    bool OpenFolder(const wxString& folderPath, wxString& errorOut);

    void Close();
    bool IsOpen() const { return db_ != nullptr; }

    // Возвращает данные за дату: сначала из БД, при отсутствии — из API с последующим сохранением
    bool LoadForDate(const wxString& dateStr, wxString& errorOut);
    const std::vector<EventRow>& Rows() const { return rows_; }

    wxString DisplayPath() const;

private:
    void FillRowsFromJson(const json& data);

    std::unique_ptr<RIA_DB> db_;
    std::vector<EventRow> rows_;
};
