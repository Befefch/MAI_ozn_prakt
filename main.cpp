#include <iostream>
#include <fstream>
#include <string>
#include <sstream> // Для работы со строками как с потоками

using namespace std;

// Сама структура данных
struct Airplane {
    string mark;
    string boardNum;
    string timeStr;
    int timeMin; // минуты для удобной сортировки
    int airfield;
};

// Ручная проверка времени
bool checkTime(string t, int &mins) {
    if (t.length() != 5) return false;
    if (t[2] != ':') return false;
    
    // проверка, что это цифры
    for (int i = 0; i < 5; i++) {
        if (i == 2) continue; // пропускаем двоеточие
        if (t[i] < '0' || t[i] > '9') return false;
    }

    // перевод символов в числа
    int h = (t[0] - '0') * 10 + (t[1] - '0');
    int m = (t[3] - '0') * 10 + (t[4] - '0');

    if (h > 23 || m > 59) return false;
    mins = h * 60 + m;
    return true;
}

// Чтение и контроль (обмен данными только через параметры)
void readData(string filename, Airplane* planes, int &count, int max_size) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        cout << "Ошибка: невозможно открыть файл.\n";
        return;
    }

    string line;
    count = 0;

    // Читаем файл строго построчно
    while (getline(fin, line)) {
        if (count >= max_size) {
            cout << "Массив заполнен.\n";
            break;
        }

        if (line.empty()) continue; // Пропускаем пустые строки

        // Берем нашу строку и делаем из нее область для чтения слов
        stringstream ss(line);
        string mark, board, timeStr, air, extra;

        // Пытаемся считать 4 нужных слова
        if (!(ss >> mark >> board >> timeStr >> air)) {
            continue; // Если слов меньше 4-х - скип
        }

        // Пытаемся считать 5-е слово. Если оно есть - скип
        if (ss >> extra) {
            continue;
        }

        // Проверка бортового номера (скип все, что меньше 3 символов)
        if (board.length() < 3) continue;

        // Проверка времени
        int mins = 0;
        if (!checkTime(timeStr, mins)) {
            continue; // если кривое время, то пропускаем строку
        }

        // Проверка аэродрома
        int airNum = 0;
        if (air == "АП1" || air == "1") airNum = 1;
        else if (air == "АП2" || air == "2") airNum = 2;
        else if (air == "АП3" || air == "3") airNum = 3;
        else continue; // если непонятный аэродром скип

        // Если условия реализованы, то данные нормальные
        planes[count].mark = mark;
        planes[count].boardNum = board;
        planes[count].timeStr = timeStr;
        planes[count].timeMin = mins;
        planes[count].airfield = airNum;
        count++;
    }
    fin.close();
}

// Пузырек только для массива индексов
void sortIndices(Airplane* planes, int* indices, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            // Индексы элементов, которые будем сравнивать
            int id1 = indices[j];
            int id2 = indices[j + 1];

            bool needSwap = false;

            // Смотрим по номеру аэродрома
            if (planes[id1].airfield > planes[id2].airfield) {
                needSwap = true;
            } 
            // Если аэродромы одинаковые - смотрим по времени
            else if (planes[id1].airfield == planes[id2].airfield) {
                if (planes[id1].timeMin > planes[id2].timeMin) {
                    needSwap = true;
                }
            }

            // Меняем местами только числа в массиве индексов
            if (needSwap) {
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }
}

// Функция для правильного подсчета видимых символов (защита от кодировки UTF-8)
int getLength(string s) {
    int len = 0;
    for (int i = 0; i < s.length(); i++) {
        // Игнорируем лишние байты кириллицы
        if ((s[i] & 0xC0) != 0x80) len++; 
    }
    return len;
}

// Функция для ровной печати ячеек с добивкой пробелами
void printCell(string text, int width) {
    cout << text;
    int spaces = width - getLength(text);
    if (spaces > 0) {
        for (int i = 0; i < spaces; i++) cout << " ";
    }
}

// Вывод на экран
void printTable(Airplane* planes, int* indices, int count) {
    if (count == 0) {
        cout << "Нет верных данных для вывода.\n";
        return;
    }

    int currentAirfield = -1;
    int orderNum = 1; // Счетчик самолетов по порядку

    for (int i = 0; i < count; i++) {
        int idx = indices[i];

        // Если начался новый аэродром
        if (planes[idx].airfield != currentAirfield) {
            currentAirfield = planes[idx].airfield;
            orderNum = 1; // Обнуляем счетчик для нового аэродрома
            
            cout << "\n=================================================================\n";
            cout << "                       АЭРОДРОМ № " << currentAirfield << "\n";
            cout << "=================================================================\n";
            
            // Новая широкая шапка
            printCell("№", 5);
            printCell("ID", 5);
            printCell("Марка", 20);
            printCell("Бортовой", 20);
            cout << "Время\n";
            cout << "-----------------------------------------------------------------\n";
        }

        // Выводим самолет с новыми колонками (to_string переводит числа в текст для нашей функции)
        printCell(to_string(orderNum), 5);
        printCell(to_string(idx), 5); // Тот самый индекс из массива indices
        printCell(planes[idx].mark, 20);
        printCell(planes[idx].boardNum, 20);
        cout << planes[idx].timeStr << "\n";

        orderNum++;
    }
    cout << "=================================================================\n";
}

int main() {
    setlocale(LC_ALL, "Russian");

    int max_size = 100; // размер с запасом
    
    // Выделение памяти динамически
    Airplane* planes = new Airplane[max_size];
    int* indices = new int[max_size];
    int count = 0;

    string filename;
    cout << "Введите имя файла: ";
    cin >> filename;
    filename += ".txt";

    // Чтение и фильтрация данных
    readData(filename, planes, count, max_size);

    if (count > 0) {
        // Инициализация массива индексов: 0, 1, 2, 3, ...
        for (int i = 0; i < count; i++) {
            indices[i] = i;
        }

        // Сортировка индексов
        sortIndices(planes, indices, count);

        // Вывод результата
        printTable(planes, indices, count);
    }

    // Чистка памяти
    delete[] planes;
    delete[] indices;

    return 0;
}
