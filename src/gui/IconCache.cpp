
#include "IconCache.h"

#include <wx/artprov.h>

namespace {
    // Привязка внутренних имен колонок к стандартным иконкам wxWidgets
    wxArtID ArtIdForName(const wxString& name) {
        if (name == "id") {
            return wxART_INFORMATION;
        }
        if (name == "name") {
            return wxART_FOLDER;
        }
        if (name == "value") {
            return wxART_TIP;
        }
        return wxART_NORMAL_FILE;
    }
}

IconCache& IconCache::Instance() {
    static IconCache cache;
    return cache;
}

wxBitmap IconCache::Bitmap(const wxString& name, const wxSize& size) {
    return wxArtProvider::GetBitmap(ArtIdForName(name), wxART_TOOLBAR, size);
}

wxBitmapBundle IconCache::BitmapBundle(const wxString& name, const wxSize& size) {
    return wxBitmapBundle::FromBitmap(Bitmap(name, size));
}
