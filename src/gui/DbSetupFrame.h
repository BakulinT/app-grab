#pragma once

/*
 * DbSetupFrame.h - стартовый экран выбора источника SQLite.
 *
 * Два сценария: 
 *  - открыть существующий файл БД или указать папку для БД приложения;
 *  - По успеху передает управление главному окну через App.
 */

#include <wx/wx.h>

class DbSetupFrame : public wxFrame {
public:
    // Стартовое окно выбора источника данных (файл БД или папка)
    DbSetupFrame();

private:
    // Открытие существующей SQLite базы
    void OnOpenDatabase(wxCommandEvent& event);
    
    // Выбор папки для создания/использования рабочей БД
    void OnChooseFolder(wxCommandEvent& event);

    // Корректное завершение приложения при закрытии стартового окна
    void OnClose(wxCloseEvent& event);

    wxDECLARE_EVENT_TABLE();
};
