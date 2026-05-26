#pragma once

/*
 * App.h - инициализация wxWidgets-приложения и навигация между экранами
 */

#include "AppDatabase.h"
#include "AppState.h"

#include <wx/wx.h>

class DbSetupFrame;
class MainFrame;

class App : public wxApp {
public:
    bool OnInit() override;

    AppState& State() { return state_; }
    AppDatabase& Database() { return database_; }

    void ShowDatabaseSetup();
    void ShowMainWindow();

private:
    wxLocale locale_;
    AppState state_;
    AppDatabase database_;
    DbSetupFrame* setupFrame_ = nullptr;
    MainFrame* mainFrame_ = nullptr;
};

wxDECLARE_APP(App);

inline App& GetApp() {
    return *static_cast<App*>(wxApp::GetInstance());
}
