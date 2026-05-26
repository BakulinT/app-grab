#pragma once

/*
 * MainFrame.h - главное окно.
 *
 * Отвечает за разметку UI, обработку меню и событий, отображение строк из AppDatabase.
 */


#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/gauge.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>

class MainFrame : public wxFrame {
public:
    MainFrame();

    // Обновляет экран в соответствии с текущим состоянием приложения/БД
    void ReloadFromState();
    // Показывает сообщение пользователю (цвет зависит от типа сообщения)
    void ShowNotification(const wxString& message, bool isError = false, bool isWarning = false);
    // Управляет видимостью индикатора загрузки
    void SetLoading(bool loading);

private:
    // Сборка верхнего меню "Файл/Справка"
    void BuildMenu();
    // Сборка всех секций главного окна
    void BuildLayout();
    // Создает визуальную шапку колонки (иконка + подпись + фон)
    wxPanel* CreateColumnHeader(
        wxWindow* parent, const wxString& iconName, const wxString& label, const wxColour& bg
    );
    // Загружает данные за выбранную дату и заполняет таблицу
    void LoadDataForSelectedDate();
    // Поддерживает ширину кастомной шапки синхронно с wxGrid
    void SyncHeaderWidthsToGrid();

    void OnMenuOpenDb(wxCommandEvent& event);
    void OnMenuClose(wxCommandEvent& event);
    void OnMenuHelp(wxCommandEvent& event);
    void OnMenuAbout(wxCommandEvent& event);
    void OnLoadDate(wxCommandEvent& event);
    void OnDateChanged(wxDateEvent& event);
    void OnGridColSize(wxGridSizeEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnClose(wxCloseEvent& event);

    wxStaticText* notificationText_ = nullptr;
    wxGauge* loadingGauge_ = nullptr;
    wxDatePickerCtrl* datePicker_ = nullptr;
    wxGrid* dataGrid_ = nullptr;
    wxStaticText* pathInfo_ = nullptr;
    wxPanel* headerPanels_[4] = { nullptr, nullptr, nullptr, nullptr };

    wxDECLARE_EVENT_TABLE();
};
