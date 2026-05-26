/*
 * Parser.cpp — реализация HTTP GET (CPR) и разбор тела ответа как JSON.
 *
 * URL строится из даты в формате DD.MM.YYYY; таймаут ограничивает зависание UI.
 */

#include "Parser.h"

using string = std::string;
using json = nlohmann::json;

json ParseJsonRIA::fetchFromApi(const std::string& date_str) {
    // Формат источника: data-DD.MM.YYYY.json
    const string url = "https://cdndc.img.ria.ru/dc/kay-n/2022/SOP-content/data/points/data-" + date_str + ".json";

    cpr::Response r = cpr::Get(
        cpr::Url{ url },
        cpr::Header{
            { "Accept", "*/*" },
            { "Content-Type", "application/json;charset=utf-8" }
        },
        cpr::Timeout{6000} 
    );

    if (r.status_code != 200) {
        throw std::runtime_error("HTTP-error:" + std::to_string(r.status_code));
    }

    return json::parse(r.text);
}