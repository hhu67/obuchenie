import requests

def get_supported_codes(api_key):
    url = f"https://v6.exchangerate-api.com/v6/{api_key}/codes"
    try:
        response = requests.get(url)
        data = response.json()
        if data['result'] == 'success':
            return data['supported_codes']
    except:
        return []
    return []
def find_suggestions(user_input, codes_list):
    prefix = user_input[:2].upper()
    suggestions = [f"{code} ({name})" for code, name in codes_list if code.startswith(prefix)]
    return suggestions
def convert_currency():
    api_key = "cd3b386195c6857e09dbfde5" 
    all_codes_data = get_supported_codes(api_key)
    valid_codes = [item[0] for item in all_codes_data]
    print("конвертер валют ")
    def get_valid_currency(prompt):
        while True:
            code = input(prompt).upper()
            if code in valid_codes:
                return code
            else:
                print(f"Валюта '{code}' не найдена")
                suggestions = find_suggestions(code, all_codes_data)
                if suggestions:
                    print("Возможно, вы имели в виду:")
                    for s in suggestions:
                        print(f"  • {s}")
                else:
                    print("подходящих вариантов не найдено")
                print("-" * 20)

    base = get_valid_currency("Введите исходную валюту (напр. USD): ")
    target = get_valid_currency("Введите целевую валюту (напр. RUB): ")
    try:
        amount = float(input(f"Сумма в {base}: "))
    except ValueError:
        print("Ошибка: введите число.")
        return
    url = f"https://v6.exchangerate-api.com/v6/{api_key}/pair/{base}/{target}"
    response = requests.get(url).json()
    if response['result'] == 'success':
        rate = response['conversion_rate']
        result = amount * rate
        print(f"\n✅ Результат: {amount} {base} = {result:.2f} {target}")
    else:
        print("Произошла ошибка при получении курса")
if __name__ == "__main__":
    convert_currency()
