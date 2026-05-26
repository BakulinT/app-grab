#include "MainFrame.h"
#include "App.h"
#include "IconCache.h"

#include <wx/aboutdlg.h>
#include <wx/dateevt.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace {
    constexpr int kMinWidth = 900;
    constexpr int kMinHeight = 600;

    enum {
        ID_LOAD_DATE = wxID_HIGHEST + 1,
        ID_MENU_OPEN_DB,
        ID_MENU_HELP,
    };

    const wxColour kColIdBg(230, 240, 255);
    const wxColour kColNameBg(235, 250, 235);
    const wxColour kColValueBg(255, 245, 230);
}

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)

EVT_MENU(ID_MENU_OPEN_DB, MainFrame::OnMenuOpenDb)
EVT_MENU(wxID_CLOSE, MainFrame::OnMenuClose)
EVT_MENU(ID_MENU_HELP, MainFrame::OnMenuHelp)
EVT_MENU(wxID_ABOUT, MainFrame::OnMenuAbout)

EVT_BUTTON(ID_LOAD_DATE, MainFrame::OnLoadDate)
EVT_DATE_CHANGED(wxID_ANY, MainFrame::OnDateChanged)
EVT_GRID_COL_SIZE(MainFrame::OnGridColSize)
EVT_SIZE(MainFrame::OnSize)
EVT_CLOSE(MainFrame::OnClose)
wxEND_EVENT_TABLE()

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, L"AppGrab - Оболочка парсера", wxDefaultPosition, wxSize(930, 700))
{
    // Базовая инициализация окна: стиль, меню и основные панели
    SetBackgroundColour(*wxWHITE);
    SetMinSize(wxSize(kMinWidth, kMinHeight));
    BuildMenu();
    BuildLayout();
}

void MainFrame::BuildMenu() {
    // Меню "Файл": сценарии переключения источника и закрытия окна
    auto* fileMenu = new wxMenu();

    fileMenu->Append(ID_MENU_OPEN_DB, L"Открыть БД...\tCtrl+O");
    fileMenu->Append(wxID_CLOSE, L"Закрыть\tCtrl+W");

    // Меню "Справка": краткая инструкция и информация о приложении
    auto* helpMenu = new wxMenu();

    helpMenu->Append(ID_MENU_HELP, L"Справка...\tF1");
    helpMenu->Append(wxID_ABOUT, L"О программе...");

    auto* bar = new wxMenuBar();
    bar->Append(fileMenu, L"&Файл");
    bar->Append(helpMenu, L"&Справка");

    SetMenuBar(bar);
    CreateStatusBar(1);
    SetStatusText(L"Готово");
}

wxPanel* MainFrame::CreateColumnHeader(
    wxWindow* parent, const wxString& iconName, const wxString& label, const wxColour& bg
) {
    auto* panel = new wxPanel(parent);
    panel->SetBackgroundColour(bg);

    auto* row = new wxBoxSizer(wxHORIZONTAL);
    const wxBitmapBundle bundle = IconCache::Instance().BitmapBundle(iconName);

    if (bundle.IsOk()) {
        auto* bmp = new wxStaticBitmap(panel, wxID_ANY, bundle.GetBitmapFor(panel));
        row->Add(bmp, 0, wxALL | wxALIGN_CENTER_VERTICAL, 6);
    }

    auto* text = new wxStaticText(panel, wxID_ANY, label);

    wxFont f = text->GetFont();
    f.SetWeight(wxFONTWEIGHT_BOLD);

    text->SetFont(f);
    row->Add(text, 1, wxALL | wxALIGN_CENTER_VERTICAL, 6);

    panel->SetSizer(row);
    panel->SetMinSize(wxSize(-1, 32));

    return panel;
}

void MainFrame::BuildLayout() {
    // Вертикальный корневой контейнер: уведомления -> выбор даты -> таблица
    auto* root = new wxBoxSizer(wxVERTICAL);
    auto* notifyPanel = new wxPanel(this);

    notifyPanel->SetBackgroundColour(*wxWHITE);

    auto* notifySizer = new wxBoxSizer(wxHORIZONTAL);

    notificationText_ = new wxStaticText(notifyPanel, wxID_ANY, L"Добро пожаловать.");

    wxFont nf = notificationText_->GetFont();

    nf.SetPointSize(nf.GetPointSize() + 1);
    nf.SetWeight(wxFONTWEIGHT_BOLD);

    notificationText_->SetFont(nf);

    // Индикатор отображается только на время загрузки/парсинга данных
    loadingGauge_ = new wxGauge(notifyPanel, wxID_ANY, 100, wxDefaultPosition, wxSize(120, 16));
    loadingGauge_->Hide();
    
    notifySizer->Add(notificationText_, 1, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    notifySizer->Add(loadingGauge_, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);
    notifyPanel->SetSizer(notifySizer);

    root->Add(notifyPanel, 0, wxEXPAND);
    pathInfo_ = new wxStaticText(this, wxID_ANY, "");
    root->Add(pathInfo_, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);

    auto* dateBox = new wxStaticBoxSizer(wxVERTICAL, this, L"Выбор даты");
    dateBox->GetStaticBox()->SetBackgroundColour(*wxWHITE);
    auto* dateRow = new wxBoxSizer(wxHORIZONTAL);

    dateRow->Add(new wxStaticText(this, wxID_ANY, L"Дата:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    datePicker_ = new wxDatePickerCtrl(
        this, wxID_ANY, wxDateTime::Today(),
        wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT | wxDP_SHOWCENTURY
    );

    dateRow->Add(datePicker_, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);

    auto* loadBtn = new wxButton(this, ID_LOAD_DATE, L"Загрузить данные");

    dateRow->Add(loadBtn, 0, wxALIGN_CENTER_VERTICAL);
    dateBox->Add(dateRow, 0, wxALL, 10);

    root->Add(dateBox, 0, wxLEFT | wxRIGHT | wxEXPAND, 8);

    // Основная таблица данных с кастомной шапкой и отключенным редактированием
    auto* listBox = new wxStaticBoxSizer(wxVERTICAL, this, L"Данные");
    listBox->GetStaticBox()->SetBackgroundColour(*wxWHITE);
    auto* headerRow = new wxBoxSizer(wxHORIZONTAL);

    headerPanels_[0] = CreateColumnHeader(listBox->GetStaticBox(), "id", L"Номер", kColIdBg);
    headerPanels_[1] = CreateColumnHeader(listBox->GetStaticBox(), "name", L"Название", kColNameBg);
    headerPanels_[2] = CreateColumnHeader(listBox->GetStaticBox(), "value", L"Населенный пункт", kColValueBg);
    headerPanels_[3] = CreateColumnHeader(listBox->GetStaticBox(), "value", L"Событие", kColValueBg);

    headerRow->Add(headerPanels_[0], 0, wxEXPAND);
    headerRow->Add(headerPanels_[1], 0, wxEXPAND);
    headerRow->Add(headerPanels_[2], 0, wxEXPAND);
    headerRow->Add(headerPanels_[3], 0, wxEXPAND);

    listBox->Add(headerRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 6);

    dataGrid_ = new wxGrid(listBox->GetStaticBox(), wxID_ANY);
    dataGrid_->CreateGrid(0, 4);
    dataGrid_->SetColLabelSize(0);
    dataGrid_->SetRowLabelSize(0);
    dataGrid_->SetColSize(0, 90);
    dataGrid_->SetColSize(1, 220);
    dataGrid_->SetColSize(2, 330);
    dataGrid_->SetColSize(3, 190);
    dataGrid_->EnableEditing(false);
    dataGrid_->EnableGridLines(true);

    listBox->Add(dataGrid_, 1, wxEXPAND | wxALL, 6);
    root->Add(listBox, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 8);

    SetSizer(root);

    SyncHeaderWidthsToGrid();
}

void MainFrame::SyncHeaderWidthsToGrid() {
    if (!dataGrid_) return;

    for (int col = 0; col < 4; ++col) {
        if (!headerPanels_[col]) continue;
        const int w = dataGrid_->GetColSize(col);
        headerPanels_[col]->SetMinSize(wxSize(w, -1));
    }

    Layout();
    Refresh();
}

void MainFrame::ShowNotification(const wxString& message, bool isError, bool isWarning) {
    notificationText_->SetLabel(message);

    // Единый цветовой код статуса: ошибка / предупреждение / успех
    if (isError) {
        notificationText_->SetForegroundColour(wxColour(180, 30, 30));
    }
    else if (isWarning) {
        notificationText_->SetForegroundColour(wxColour(160, 100, 0));
    }
    else {
        notificationText_->SetForegroundColour(wxColour(20, 100, 40));
    }

    notificationText_->GetParent()->Layout();
}

void MainFrame::SetLoading(bool loading) {
    if (loading) {
        loadingGauge_->Show();
        loadingGauge_->Pulse();
    }
    else {
        loadingGauge_->Hide();
    }

    Layout();
}

void MainFrame::ReloadFromState() {
    auto& app = GetApp();

    const AppState& st = app.State();
    wxString pathLine;

    // Формируем текст с текущим источником данных для панели под уведомлением
    if (app.Database().IsOpen()) {
        pathLine = L"База данных: " + app.Database().DisplayPath();
    }
    else if (st.HasOpenDatabase()) {
        pathLine = L"Открыта БД: " + st.dbFilePath;
    }
    else if (st.HasFolderTarget()) {
        pathLine = L"Папка для БД: " + st.dbFolderPath;
    }
    else {
        pathLine = L"Источник данных не выбран.";
    }

    pathInfo_->SetLabel(pathLine);

    if (dataGrid_->GetNumberRows() > 0) {
        // Перед новой загрузкой очищаем таблицу, чтобы не оставлять старые строки
        dataGrid_->DeleteRows(0, dataGrid_->GetNumberRows());
    }

    if (!app.Database().IsOpen()) {
        ShowNotification(L"База данных не открыта.", true);
        return;
    }

    LoadDataForSelectedDate();
}

void MainFrame::LoadDataForSelectedDate() {
    auto& app = GetApp();

    SetLoading(true);
    ShowNotification(L"Загрузка данных...");
    SetStatusText(L"Загрузка...");

    wxString error;

    const wxString dateStr = datePicker_->GetValue().Format("%d.%m.%Y");

    if (!app.Database().LoadForDate(dateStr, error)) {
        SetLoading(false);
        ShowNotification(error, true);
        SetStatusText(L"Ошибка загрузки");
        return;
    }

    const auto& rows = app.Database().Rows();
    if (dataGrid_->GetNumberRows() > 0) {
        dataGrid_->DeleteRows(0, dataGrid_->GetNumberRows());
    }

    if (!rows.empty()) {
        // Подгоняем число строк под новый набор данных
        dataGrid_->AppendRows(static_cast<int>(rows.size()));
    }

    for (size_t i = 0; i < rows.size(); ++i) {
        const EventRow& r = rows[i];

        dataGrid_->SetCellValue(static_cast<int>(i), 0, wxString::Format("%d", r.number));
        dataGrid_->SetCellValue(static_cast<int>(i), 1, r.name);
        dataGrid_->SetCellValue(static_cast<int>(i), 2, r.area);
        dataGrid_->SetCellValue(static_cast<int>(i), 3, r.eventType);
        
        // Подсветка колонок делает большие списки визуально читаемее
        dataGrid_->SetCellBackgroundColour(static_cast<int>(i), 0, kColIdBg);
        dataGrid_->SetCellBackgroundColour(static_cast<int>(i), 1, kColNameBg);
        dataGrid_->SetCellBackgroundColour(static_cast<int>(i), 2, kColValueBg);
        dataGrid_->SetCellBackgroundColour(static_cast<int>(i), 3, kColValueBg);
    }

    SetLoading(false);

    if (rows.empty()) {
        ShowNotification(wxString::Format(L"Записей нет (дата: %s)", dateStr), false, true);
    }
    else {
        ShowNotification(wxString::Format(L"Загружено записей: %zu (дата: %s)", rows.size(), dateStr));
    }

    SetStatusText(wxString::Format(L"Записей: %zu", rows.size()));
}

void MainFrame::OnMenuOpenDb(wxCommandEvent&) {
    // Возвращаем пользовател к начальному экрану выбора БД
    GetApp().ShowDatabaseSetup();
}

void MainFrame::OnMenuClose(wxCommandEvent&) {
    Close(true);
}

void MainFrame::OnMenuHelp(wxCommandEvent&) {
    wxMessageBox(
        L"AppGrab - Оболочка парсера с SQLite.\n\n"

        L"Шаг 1:\n"
        L"  • Открыть .db - существующая база\n"
        L"  • Выбрать папку - путь для скрипта, создающего БД\n\n"

        L"Главный экран:\n"
        L"  • Уведомления и индикатор загрузки\n"
        L"  • Выбор даты и кнопка «Загрузить данные»\n"
        L"  • Таблица: номер, область, заголовок, событие\n\n"

        L"Если данных за дату нет в БД, они загружаются с API и сохраняются.\n\n"

        L"Меню «Файл» -> «Открыть БД» возвращает к шагу 1.\n",

        L"Справка", wxOK | wxICON_INFORMATION, this
    );
}

void MainFrame::OnMenuAbout(wxCommandEvent&) {
    wxAboutDialogInfo info;
    info.SetName("AppGrab");
    info.SetVersion("1.0");
    info.SetDescription(
        L"Графическая оболочка для парсера с сохранением результатов в SQLite.\n\n"
    );

    info.SetWebSite("https://github.com/BakulinT/app-grab");
    wxAboutBox(info, this);
}

void MainFrame::OnLoadDate(wxCommandEvent&) {
    LoadDataForSelectedDate();
}

void MainFrame::OnDateChanged(wxDateEvent&) {}

void MainFrame::OnGridColSize(wxGridSizeEvent& event) {
    event.Skip();
    // После ручного ресайза колонк обновляем ширины кастомной шапки
    SyncHeaderWidthsToGrid();
}

void MainFrame::OnSize(wxSizeEvent& event) {
    event.Skip();
    // При изменении размера окна поддерживаем выравнивание шапки и таблицы
    SyncHeaderWidthsToGrid();
}

void MainFrame::OnClose(wxCloseEvent& event) {
    // Явно закрываем БД
    GetApp().Database().Close();
    event.Skip();
    GetApp().ExitMainLoop();
}