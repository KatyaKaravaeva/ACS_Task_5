#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>

// Массив для хранения информации о конкретном пациенте.
std::string *arr;
// Индекс массива arr.
int arr_ind = 0;
// Буфер.
int buf[2];
// Индекс для чтения.
int readIndex = 0;
// Индекс для записи.
int writeIndex = 0;
// Счетчик занятых элементов буфера.
int count_busy = 0;

pthread_mutex_t mutex;
pthread_cond_t notFull;
pthread_cond_t notEmpty;

// Метод для запуска доктора.
void *doctorMethod(void *param) {
    while (true) {
        int info = rand() % 3 + 1;
        pthread_mutex_lock(&mutex);

        // Обрабатываем случай, когда число занятых элементов в буфере равно максимальной его вместимости.
        while (count_busy == 2)
            pthread_cond_wait(&notFull, &mutex);
        // Записываем созданную info в буфер.
        buf[writeIndex] = info;
        writeIndex = (writeIndex + 1) % 2;
        count_busy++;

        pthread_mutex_unlock(&mutex);

        // Вызываем потоки, в выбранной модели это - потребители.
        pthread_cond_broadcast(&notEmpty);
        sleep(rand() % 2);
    }
}

void *patientMethod(void *param) {
    int number = *((int *) param);

    pthread_mutex_lock(&mutex);

    // Проверка на возможность засыпания потока.
    while (count_busy == 0)
        pthread_cond_wait(&notEmpty, &mutex);

    // Берем элемент из буфера.
    int result = buf[readIndex];
    readIndex = (readIndex + 1) % 2;

    // Элемент достали, поэтому уменьшаем счетчик.
    count_busy--;
    // В массив кладем информацию о пациенте.
    arr[arr_ind] = std::to_string(number) + " " + std::to_string(result) + " " + std::to_string(readIndex) + " ";
    // Увеличиваем счетчик массива, так как уже положили в него элемент.
    ++arr_ind;
    pthread_mutex_unlock(&mutex);

    // Пробуждаем потоки, в нашем случае - производителей.
    pthread_cond_broadcast(&notFull);
}

// Метод для считывания целого числа.
int ReadInt(const std::string &message = "") {
    std::cout << message << std::endl;
    while (true) // цикл продолжается до тех пор, пока пользователь не введет корректное значение
    {
        std::cout << "Enter int value: ";
        int a;
        std::cin >> a;

        // Проверка на предыдущее извлечение
        if (std::cin.fail()) // если предыдущее извлечение оказалось неудачным,
        {
            std::cin.clear(); // то возвращаем cin в 'обычный' режим работы
            std::cin.ignore(32767, '\n'); // и удаляем значения предыдущего ввода из входного буфера
            std::cout << "Oops, that input is invalid.  Please try again.\n";
        } else {
            std::cin.ignore(32767, '\n'); // удаляем лишние значения

            return a;
        }
    }
}

int main() {
    // Запрашиваем у пользователя ввод количества пациентов.
    int countPatients = ReadInt("Enter a number of patients:");
    if (countPatients < 0) {
        countPatients *= -1;
    }
    // Объявлем массив с введенным пользователем числом пациентов.
    arr = new std::string[countPatients];

    /// Инициализация.
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&notFull, nullptr);
    pthread_cond_init(&notEmpty, nullptr);
    int main_ind;

    pthread_t firstDoctorsThread[2];
    int fThread[2];
    // Запускаем потоки врачей.
    for (main_ind = 0; main_ind < 2; main_ind++) {
        fThread[main_ind] = main_ind + 1;
        pthread_create(&firstDoctorsThread[main_ind], nullptr, doctorMethod, (void *) (fThread + main_ind));
    }

    // Запуск потоков пациентов.
    auto *secondThreat = new pthread_t[countPatients];
    int *sThread = new int[countPatients];
    // Создаем потоки.
    for (main_ind = 0; main_ind < countPatients; ++main_ind) {
        sThread[main_ind] = main_ind + 1;
        pthread_create(&secondThreat[main_ind], nullptr, patientMethod, (void *) (sThread + main_ind));
    }
    for (main_ind = 0; main_ind < countPatients; ++main_ind) {
        sThread[main_ind] = main_ind + 1;
        pthread_join(secondThreat[main_ind], nullptr);
    }

    // Перебираем каждый элемент массива (кроме последнего, он уже будет отсортирован к тому времени, когда мы до него доберемся)
    for (int startIndex = 0; startIndex < countPatients - 1; ++startIndex) {
        // В переменной smallestIndex хранится индекс наименьшего значения, которое мы нашли в этой итерации.
        // Начинаем с того, что наименьший элемент в этой итерации - это первый элемент (индекс 0)
        int smallestIndex = startIndex;
        // Затем ищем элемент поменьше в остальной части массива
        for (int currentIndex = startIndex + 1; currentIndex < countPatients; ++currentIndex) {
            // Если мы нашли элемент, который меньше нашего наименьшего элемента,
            if (stoi(arr[currentIndex].substr(0, arr[currentIndex].find(' '))) <
                stoi(arr[smallestIndex].substr(0, arr[smallestIndex].find(' '))))
                // то запоминаем его
                smallestIndex = currentIndex;
        }

        // smallestIndex теперь наименьший элемент.
        // Меняем местами наше начальное наименьшее число с тем, которое мы обнаружили
        std::swap(arr[startIndex], arr[smallestIndex]);
    }

    // Теперь, когда весь массив отсортирован - выводим его на экран
    for (int index = 0; index < countPatients; ++index) {
        std::string space_delimiter = " ";
        std::vector<std::string> patient{};
        size_t pos = 0;
        while ((pos = arr[index].find(space_delimiter)) != std::string::npos) {
            patient.push_back(arr[index].substr(0, pos));
            arr[index].erase(0, pos + space_delimiter.length());
        }
        int ind_check = 0;
        for (const auto &str: patient) {
            if (ind_check % 3 == 0) {
                std::cout << "Patient number " << str << " ";
            } else if (ind_check % 3 == 1) {
                if (str == "1") {
                    std::cout << "was sent to the dentist";
                } else if (str == "2") {
                    std::cout << "was sent to the surgeon";
                } else {
                    std::cout << "was sent to the therapist";
                }
            } else {
                std::cout << " by doctor number " << std::stoi(str) + 1 << std::endl;
            }
            ++ind_check;
            sleep(1);
        }

    }
    // Очищаем память.
    delete[] secondThreat;
    delete[] sThread;
    delete[] arr;

    return 0;
}