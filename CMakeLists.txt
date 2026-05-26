cmake_minimum_required(VERSION 3.16)
project ("AppGrab" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# wxWidgets (vcpkg: wxwidgets)
find_package(wxWidgets CONFIG REQUIRED)

# SQLite (vcpkg: sqlite3)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

# CPR (vcpkg: cpr)
find_package(cpr CONFIG REQUIRED)

find_package(nlohmann_json CONFIG REQUIRED)

set(APPGRAB_SOURCES
    src/main.cpp

    src/gui/App.cpp
    src/gui/DbSetupFrame.cpp
    src/gui/MainFrame.cpp
    src/gui/AppDatabase.cpp
    src/gui/IconCache.cpp

    src/core/Database.cpp
    src/core/Parser.cpp
)

if(WIN32)
    add_executable(AppGrab WIN32 ${APPGRAB_SOURCES})
else()
    add_executable(AppGrab ${APPGRAB_SOURCES})
endif()

target_include_directories(AppGrab PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(MSVC)
    target_compile_options(AppGrab PRIVATE /utf-8)
endif()

target_link_libraries(AppGrab PRIVATE
    wx::core
    wx::base
    wx::adv
    unofficial::sqlite3::sqlite3
    cpr::cpr
    nlohmann_json::nlohmann_json
)

