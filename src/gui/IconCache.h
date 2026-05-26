#pragma once

/*
 * IconCache.h - доступ к иконкам для UI таблицы и заголовков колонок.
 *
 * Инкапсулирует сопоставление внутренних имен колонок с wxArtProvider и
 * выдачу wxBitmap / wxBitmapBundle для единообразного вида на разных DPI.
 */

#include <wx/bitmap.h>
#include <wx/bmpbndl.h>
#include <wx/string.h>

class IconCache {
public:
    // Единая точка доступа к иконкам, чтобы не дублировать логику
    static IconCache& Instance();

    // Возвращает готовый bitmap фиксированного размера для простых контролов
    wxBitmap Bitmap(const wxString& name, const wxSize& size = wxSize(16, 16));

    // Возвращает bitmap bundle для корректного масштабирования на разных DPI
    wxBitmapBundle BitmapBundle(const wxString& name, const wxSize& size = wxSize(16, 16));

private:
    IconCache() = default;
};
