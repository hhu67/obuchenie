#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include "json.hpp" // Скачайте с https://github.com/nlohmann/json

using json = nlohmann::json;
using namespace std;

// Функция для обработки ответа от сервера
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Универсальная функция для выполнения GET-запроса
string performRequest(string url) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

int main() {
    string apiKey = "cd3b386195c6857e09dbfde5";
    
    // 1. Получаем список всех кодов валют
    string codesUrl = "https://v6.exchangerate-api.com/v6/" + apiKey + "/codes";
    string codesResponse = performRequest(codesUrl);
    auto codesJson = json::parse(codesResponse);

    if (codesJson["result"] != "success") {
        cout << "Ошибка API при загрузке кодов!" << endl;
        return 1;
    }

    auto supportedCodes = codesJson["supported_codes"];

    auto getValidCurrency = [&](string prompt) {
        while (true) {
            cout << prompt;
            string input;
            cin >> input;
            for (auto& c : input) c = toupper(c);

            // Проверка на валидность
            for (auto& item : supportedCodes) {
                if (item[0] == input) return input;
            }

            // Если не найдено — поиск подсказок
            cout << "❌ Валюта '" << input << "' не найдена." << endl;
            string prefix = input.substr(0, 2);
            cout << "Возможно, вы имели в виду:" << endl;
            bool found = false;
            for (auto& item : supportedCodes) {
                string code = item[0];
                if (code.substr(0, 2) == prefix) {
                    cout << "  • " << code << " (" << item[1] << ")" << endl;
                    found = true;
                }
            }
            if (!found) cout << "Вариантов не найдено." << endl;
            cout << "--------------------" << endl;
        }
    };

    // 2. Логика конвертации
    string base = getValidCurrency("Введите исходную валюту (USD): ");
    string target = getValidCurrency("Введите целеную валюту (RUB): ");
    
    double amount;
    cout << "Введите сумму: ";
    cin >> amount;

    string pairUrl = "https://v6.exchangerate-api.com/v6/" + apiKey + "/pair/" + base + "/" + target;
    string pairResponse = performRequest(pairUrl);
    auto pairJson = json::parse(pairResponse);

    if (pairJson["result"] == "success") {
        double rate = pairJson["conversion_rate"];
        cout << "\n✅ Итог: " << amount << " " << base << " = " 
             << (amount * rate) << " " << target << " (Курс: " << rate << ")" << endl;
    }

    return 0;
}
