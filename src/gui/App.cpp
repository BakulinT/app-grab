#include "App.h"

#include "DbSetupFrame.h"
#include "MainFrame.h"

#include <wx/intl.h>

wxIMPLEMENT_APP(App);

bool App::OnInit() {
    if (!wxApp::OnInit()) return false;

    if (!locale_.Init(wxLANGUAGE_RUSSIAN)) {
        locale_.Init(wxLANGUAGE_DEFAULT);
    }

    SetAppName("AppGrab");
    ShowDatabaseSetup();
    return true;
}

void App::ShowDatabaseSetup() {
    if (mainFrame_) {
        mainFrame_->Hide();
    }

    // Возврат к шагу выбора источника всегда начинает новую сессию
    database_.Close();
    state_.Reset();

    if (!setupFrame_) {
        setupFrame_ = new DbSetupFrame();
    }
    setupFrame_->CentreOnScreen();
    setupFrame_->Show(true);
    setupFrame_->Raise();
}

void App::ShowMainWindow() {
    if (setupFrame_) {
        setupFrame_->Hide();
    }

    if (!mainFrame_) {
        mainFrame_ = new MainFrame();
    }
    
    mainFrame_->CentreOnScreen();
    mainFrame_->Show(true);
    mainFrame_->Raise();
    
    // Перечитываем состояние и сразу инициируем загрузку на выбранную дату
    mainFrame_->ReloadFromState();
}
