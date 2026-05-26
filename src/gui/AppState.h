#pragma once

/*
 * AppState.h - минимальное состояние выбора хранилища данных.
 *
 * Режим (файл / папка / не задано) и пути используются для подписей в UI
 * и не дублируют факт открытого соединения (см. AppDatabase::IsOpen).
 */

#include <wx/string.h>

enum class StorageMode {
    None,
    ExistingDatabase,
    NewDatabaseFolder
};

struct AppState {
    // Текущий режим хранения данных, выбранный на стартовом экране
    StorageMode mode = StorageMode::None;

    // Путь к выбранному существующему файлу БД
    wxString dbFilePath;
    
    // Папка, где создается/открывается рабочая БД приложения
    wxString dbFolderPath;

    bool HasOpenDatabase() const {
        return mode == StorageMode::ExistingDatabase && !dbFilePath.empty();
    }

    bool HasFolderTarget() const {
        return mode == StorageMode::NewDatabaseFolder && !dbFolderPath.empty();
    }

    void Reset() {
        mode = StorageMode::None;
        dbFilePath.clear();
        dbFolderPath.clear();
    }
};
