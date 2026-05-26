#include "DbSetupFrame.h"
#include "App.h"

#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace {
    constexpr int kMinWidth = 520;
    constexpr int kMinHeight = 320;
}

wxBEGIN_EVENT_TABLE(DbSetupFrame, wxFrame)

EVT_BUTTON(wxID_OPEN, DbSetupFrame::OnOpenDatabase)
EVT_BUTTON(wxID_ANY + 1, DbSetupFrame::OnChooseFolder)
EVT_CLOSE(DbSetupFrame::OnClose)

wxEND_EVENT_TABLE()

DbSetupFrame::DbSetupFrame()
    : wxFrame(nullptr, wxID_ANY, L"AppGrab Выбор базы данных", wxDefaultPosition, wxSize(560, 360)
) {
    // Небольшое стартовое окно-мастер для выбора сценария работы с БД
    SetBackgroundColour(*wxWHITE);
    SetMinSize(wxSize(kMinWidth, kMinHeight));

    auto* root = new wxBoxSizer(wxVERTICAL);
    auto* title = new wxStaticText(
        this, wxID_ANY,
        L"Выберите способ работы с базой данных SQLite"
    );

    wxFont titleFont = title->GetFont();

    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    root->Add(title, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 16);

    auto* hint = new wxStaticText(
        this, wxID_ANY,
        L"Откройте существующий файл .db или укажите папку."
    );

    hint->Wrap(480);
    root->Add(hint, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 16);

    auto* buttons = new wxBoxSizer(wxVERTICAL);
    // Сценарий 1: подключить уже существующий ффайл БД
    auto* openDb = new wxButton(this, wxID_OPEN, L"Открыть файл SQLite (.db)");

    openDb->SetMinSize(wxSize(400, 40));
    buttons->Add(openDb, 0, wxALL | wxEXPAND, 8);

    // Сценарий 2: выбрать папку, в которой будет создана/использована БД
    auto* pickFolder = new wxButton(this, wxID_ANY + 1, L"Выбрать папку для новой БД");

    pickFolder->SetMinSize(wxSize(400, 40));
    buttons->Add(pickFolder, 0, wxALL | wxEXPAND, 8);
    root->Add(buttons, 1, wxLEFT | wxRIGHT | wxEXPAND, 24);

    SetSizer(root);
}

void DbSetupFrame::OnOpenDatabase(wxCommandEvent&) {
    wxFileDialog dlg(
        this, L"Открыть базу SQLite", wxEmptyString, wxEmptyString,
        L"SQLite (*.db;*.sqlite;*.sqlite3)|*.db;*.sqlite;*.sqlite3|Все файлы (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    if (dlg.ShowModal() != wxID_OK) return;

    auto& app = GetApp();
    wxString error;
    if (!app.Database().OpenExistingFile(dlg.GetPath(), error)) {
        wxMessageBox(error, L"Ошибка открытия БД", wxOK | wxICON_ERROR, this);
        return;
    }

    // Фиксируем в состоянии путь к выбранному файлу для последующего отображения в UI
    app.State().mode = StorageMode::ExistingDatabase;
    app.State().dbFilePath = dlg.GetPath();
    app.State().dbFolderPath.clear();
    app.ShowMainWindow();
}

void DbSetupFrame::OnChooseFolder(wxCommandEvent&) {
    wxDirDialog dlg(this, L"Выберите папку для сохранения базы данных");

    if (dlg.ShowModal() != wxID_OK) return;

    auto& app = GetApp();
    wxString error;
    if (!app.Database().OpenFolder(dlg.GetPath(), error)) {
        wxMessageBox(error, L"Ошибка создания БД", wxOK | wxICON_ERROR, this);
        return;
    }

    // Сохраняем путь к папке и фактический путь к файлу БД, созданному Database-слоем
    app.State().mode = StorageMode::NewDatabaseFolder;
    app.State().dbFolderPath = dlg.GetPath();
    app.State().dbFilePath = app.Database().DisplayPath();
    app.ShowMainWindow();
}

void DbSetupFrame::OnClose(wxCloseEvent& event) {
    event.Skip();
    
    if (wxTheApp) {
        // Закрытие стартового окна трактуем как завершение приложения
        GetApp().ExitMainLoop();
    }
}