#!/bin/bash
echo "=== ФУНКЦИОНАЛЬНОЕ ТЕСТИРОВАНИЕ СЕРВЕРА ==="
echo ""

# Подготовка файлов
touch test1.log test2.log app.log custom.log

# Функция для теста
run_test() {
    local test_num=$1
    local test_name=$2
    local command=$3
    local expected=$4
    
    echo "========================================"
    echo "ТЕСТ $test_num: $test_name"
    echo "Команда: $command"
    echo "Ожидаем: $expected"
    echo "--- РЕЗУЛЬТАТ ---"
    
    # Для команд, которые не завершаются сами, добавляем таймаут
    if [[ $command != *"& sleep"* ]] && [[ $command != *"-h"* ]] && [[ $command != *"-v"* ]] && [[ $command != *"--help"* ]] && [[ $command != *"--unknown"* ]] && [[ $command != *"-p"* ]]; then
        # Это команда запуска сервера - запускаем в фоне и убиваем
        eval "$command &"
        local pid=$!
        sleep 2
        kill $pid 2>/dev/null
        wait $pid 2>/dev/null
    else
        # Это команда вывода справки или ошибки - выполняем напрямую
        eval "$command" 2>&1
    fi
    
    echo "--- КОНЕЦ ТЕСТА ---"
    echo ""
}

# Тесты
run_test "1/15" "Запуск без параметров" "./server" "Сервер запущен"
run_test "2/15" "Справка (-h)" "./server -h" "Обязательные опции:"
run_test "3/15" "Справка (--help)" "./server --help" "Дополнительные опции:"
run_test "4/15" "Версия программы (-v)" "./server -v" "версия"
run_test "5/15" "Кастомный конфиг" "./server -c test_db.txt -l test1.log -p 1024" "Загружен"
run_test "6/15" "Кастомный лог" "./server -c test_db.txt -l custom.log -p 1025" "custom.log"
run_test "7/15" "Валидный порт 1024" "./server -c test_db.txt -l test1.log -p 1024" "порт 1024"
run_test "8/15" "Невалидный порт 0" "./server -c test_db.txt -l test1.log -p 0" "ошибка"
run_test "9/15" "Системный порт 80" "./server -c test_db.txt -l test1.log -p 80" "ошибка"
run_test "10/15" "Порт >65535" "./server -c test_db.txt -l test1.log -p 70000" "ошибка"
run_test "11/15" "Все параметры" "./server --config test_db.txt --log app.log --port 22000" "app.log"
run_test "12/15" "Разный порядок аргументов" "./server -l test2.log -p 23000 -c test_db.txt" "test2.log"
run_test "13/15" "Неизвестный аргумент" "./server --unknown" "неизвестная"
run_test "14/15" "Отсутствие значения" "./server -p" "argument"
run_test "15/15" "Дублирование опций" "./server -p 24000 -p 25000 -c test_db.txt -l test1.log" "25000"

echo "========================================"
echo "ТЕСТИРОВАНИЕ ЗАВЕРШЕНО!"
echo "========================================"
