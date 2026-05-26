#pragma once

/*
 * Parser.h — загрузка JSON с CDN по дате (HTTP, без UI-зависимостей).
 *
 * ParseJsonRIA::fetchFromApi возвращает распарсенный nlohmann::json;
 * ошибки сети и HTTP-коды пробрасываются как исключения вызывающему коду.
 */

#include <iostream>
#include <string>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using string = std::string;
using json = nlohmann::json;

class ParseJsonRIA {

public:
    static json fetchFromApi(const std::string& date_str);
};
