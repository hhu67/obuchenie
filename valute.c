#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include "cJSON.h"

// Структура для хранения ответа от сервера
struct Memory {
    char *response;
    size_t size;
};

// Функция записи данных от curl
size_t write_callback(void *data, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct Memory *mem = (struct Memory *)userp;
    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if (ptr == NULL) return 0;
    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;
    return realsize;
}

// Функция для выполнения HTTP запроса
char* perform_request(const char* url) {
    CURL *curl;
    CURLcode res;
    struct Memory chunk = {malloc(1), 0};

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return chunk.response;
}

void to_uppercase(char *str) {
    for (int i = 0; str[i]; i++) str[i] = toupper(str[i]);
}

int main() {
    const char *api_key = "cd3b386195c6857e09dbfde5";
    char url[256];
    char base[4], target[4];
    double amount;

    printf("--- Конвертер валют на C ---\n");

    // 1. Получаем список валют для проверки и подсказок
    sprintf(url, "https://v6.exchangerate-api.com/v6/%s/codes", api_key);
    char *json_data = perform_request(url);
    cJSON *root = cJSON_Parse(json_data);
    cJSON *codes = cJSON_GetObjectItem(root, "supported_codes");

    auto_input:
    printf("Введите исходную валюту (например, USD): ");
    scanf("%3s", base); to_uppercase(base);

    // Проверка валидности и поиск подсказок
    int found = 0;
    for (int i = 0; i < cJSON_GetArraySize(codes); i++) {
        cJSON *item = cJSON_GetArrayItem(codes, i);
        if (strcmp(cJSON_GetArrayItem(item, 0)->valuestring, base) == 0) {
            found = 1; break;
        }
    }

    if (!found) {
        printf("❌ Валюта %s не найдена. Подсказки:\n", base);
        for (int i = 0; i < cJSON_GetArraySize(codes); i++) {
            cJSON *item = cJSON_GetArrayItem(codes, i);
            const char *code = cJSON_GetArrayItem(item, 0)->valuestring;
            if (strncmp(code, base, 2) == 0) {
                printf("  • %s (%s)\n", code, cJSON_GetArrayItem(item, 1)->valuestring);
            }
        }
        goto auto_input;
    }

    printf("Введите целевую валюту: ");
    scanf("%3s", target); to_uppercase(target);
    printf("Введите сумму: ");
    scanf("%lf", &amount);

    // 2. Получаем курс
    sprintf(url, "https://v6.exchangerate-api.com/v6/%s/pair/%s/%s", api_key, base, target);
    char *pair_data = perform_request(url);
    cJSON *pair_root = cJSON_Parse(pair_data);
    
    if (strcmp(cJSON_GetObjectItem(pair_root, "result")->valuestring, "success") == 0) {
        double rate = cJSON_GetObjectItem(pair_root, "conversion_rate")->valuedouble;
        printf("\n✅ %f %s = %.2f %s (Курс: %f)\n", amount, base, amount * rate, target, rate);
    }

    // Очистка
    free(json_data); free(pair_data);
    cJSON_Delete(root); cJSON_Delete(pair_root);
    return 0;
}
