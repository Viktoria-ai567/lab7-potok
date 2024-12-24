#include <iostream>     
#include <thread>        // Для работы с потоками
#include <mutex>         // Для работы с мьютексами
#include <vector>        
#include <chrono>        // Для работы со временем
#include <functional>    // Для использования функций как объектов

using namespace std::chrono;

// Структура для хранения результата функции и затраченного времени
template<typename T>
struct FuncResult
{
    T _result;          // Результат функции
    long long _time;    // Время выполнения функции в микросекундах
// Конструктор для инициализации результата и времени выполнения
    FuncResult(T result, double time) :
        _result(result), _time(time)
    { }

    // Метод для отображения результатов в консоли
    void Print()
    {
        std::cout << "[val: " << _result
                  << "; time: " << _time << "]" << std::endl;
    }
};

// Функция для суммирования подмножества массива в отдельном потоке
template<typename T>
inline
void thread_sum(T* data, size_t indStart, size_t indEnd, T& sum, std::mutex& m)
{
    T local_sum = 0; // Локальная сумма для каждого потока

    // Суммируем элементы в заданном диапазоне
    for (size_t i = indStart; i <= indEnd; i++)
    {
        local_sum += data[i];
    }

    {
        std::lock_guard<std::mutex> lock(m); // Блокировка мьютекса для безопасного доступа к общей сумме
        sum += local_sum; // Добавляем локальную сумму к общей
    }
}

// Класс для работы с массивом в памяти
template<typename T>
class VectorRam
{
public:
    T* data;          // Указатель на массив
    size_t size;      // Размер массива

    // Конструктор для инициализации массива заданного размера
    VectorRam(size_t size) : size(size)
    {
        data = new T[size]; // Выделяем память под массив
    }

    // Деструктор для освобождения памяти при уничтожении объекта
    ~VectorRam()
    {
        delete[] data; // Освобождаем память
    }

    // Инициализация массива заданным значением
    void InitByVal(T val)
    {
        for (size_t i = 0; i < size; i++)
        {
            data[i] = val; // Заполняем массив
        }
    }

    // Вывод массива в консоль
    void PrintToConsole()
    {
        for (size_t i = 0; i < size; i++)
        {
            std::cout << data[i] << " "; // Выводим элемент
        }
        std::cout << std::endl;
    }

    // Суммирование элементов массива на заданном диапазоне
    T Sum(size_t indStart, size_t indEnd)
    {
        T result = 0;
        for (size_t i = indStart; i <= indEnd; i++)
        {
            result += data[i]; // Суммируем элементы
        }
        return result; // Возвращаем результат
    }

    // Суммирование всех элементов массива
    T Sum()
    {
        return Sum(0, size - 1); // Вызываем Sum с полным диапазоном
    }

    // Суммирование элементов с помощью многопоточности
    T Sum(size_t indStart, size_t indEnd, unsigned threadsNum)
    {
        std::mutex m; // Мьютекс для синхронизации
        T sum = 0;    // Объявление общей суммы
        size_t blockSize = indEnd - indStart + 1; // Размер блока
        std::vector<std::thread> threads; // Вектор потоков
        size_t thBlockSize = blockSize / threadsNum; // Размер блока для каждого потока
        
        // Запуск потоков
        for (size_t i = 0; i < threadsNum; i++)
        {
            size_t thIndStart = indStart + i * thBlockSize; // Начальный индекс для потока
            size_t thIndEnd = thIndStart + thBlockSize - 1; // Конечный индекс для потока
            if (i == threadsNum - 1) // Для последнего потока, чтобы охватить все элементы
                thIndEnd = indEnd;

            threads.push_back(std::thread(thread_sum<T>, data, thIndStart, thIndEnd, std::ref(sum), std::ref(m))); // Создаем новый поток
        }

        // Ожидание завершения всех потоков
        for (auto& th : threads)
        {
            th.join(); // Присоединяем поток
        }

        return sum; // Возвращаем общую сумму
    }

    // Суммирование всех элементов с использованием указанного количества потоков
    T Sum(unsigned threadsNum)
    {
        return Sum(0, size - 1, threadsNum); // Вызываем Sum с полным диапазоном
    }

    // Суммирование с записью результата и времени выполнения
    FuncResult<T> SumFR(size_t indStart, size_t indEnd)
    {
        auto start = high_resolution_clock::now(); // Запоминаем время начала
        T result = Sum(indStart, indEnd); // Вызываем суммирование
        auto stop = high_resolution_clock::now(); // Запоминаем время окончания

        auto duration = duration_cast<microseconds>(stop - start); // Расчет времени выполнения
        auto t = duration.count(); // Получаем время в микросекундах

        return FuncResult<T>(result, t); // Возвращаем результат в виде FuncResult
    }

    // Суммирование всех элементов с записью результата и времени выполнения
    FuncResult<T> SumFR()
    {
        return SumFR(0, size - 1); // Вызываем SumFR с полным диапазоном
    }

    // Суммирование в многопоточном режиме с записью результата и времени выполнения
    FuncResult<T> SumFR(size_t indStart, size_t indEnd, unsigned threadsNum)
    {
        auto start = high_resolution_clock::now(); // Запоминаем время начала
        T result = Sum(indStart, indEnd, threadsNum); // Вызываем многопоточное суммирование
        auto stop = high_resolution_clock::now(); // Запоминаем время окончания

        auto duration = duration_cast<microseconds>(stop - start); // Расчет времени выполнения
        auto t = duration.count(); // Получаем время в микросекундах

        return FuncResult<T>(result, t); // Возвращаем результат в виде FuncResult
    }

    // Суммирование всех элементов с использованием указанного количества потоков с записью результата и времени выполнения
    FuncResult<T> SumFR(unsigned threadsNum)
    {
        return SumFR(0, size - 1, threadsNum); // Вызываем SumFR с полным диапазоном
    }
};

int main()
{
    unsigned Nthreads = 4; // Количество потоков
    size_t size = 500000000; // Размер массива
    double elVal = 0.001; // Значение для заполнения массива
    
    VectorRam<double> v(size); // Создание объекта VectorRam
    v.InitByVal(elVal); // Инициализация массива значением elVal

    std::cout << "sum must be equal " << size * elVal << std::endl; // Ожидаемая сумма
    auto sum_seq = v.Sum(); // Последовательное суммирование
    std::cout << "sum_seq = " << sum_seq << std::endl; // Вывод результата

    // Суммирование половины массива
    std::cout << "sum_seq_half must be equal " << (size / 2) * elVal << std::endl;
    auto sum_seq_half = v.Sum(0, size / 2); // Суммирование от 0 до size/2
    std::cout << "sum_seq_half = " << sum_seq_half << std::endl; // Вывод результата

    auto sum_par = v.Sum(Nthreads);
    std::cout << "sum_par = " << sum_par << std::endl;

    auto sum_par_half = v.Sum(0, size / 2, Nthreads);
    std::cout << "sum_par_half = " << sum_par_half << std::endl;

    auto sumFR = v.SumFR();
    std::cout << "sumFR: ";
    sumFR.Print();

    auto sumFR_half = v.SumFR(0, size / 2);
    std::cout << "sumFR_half: ";
    sumFR_half.Print();

    auto sumFR_par = v.SumFR(Nthreads);
    std::cout << "sumFR_par: ";
    sumFR_par.Print();

    auto sumFR_par_half = v.SumFR(0, size / 2, Nthreads);
    std::cout << "sumFR_par_half: ";
    sumFR_par_half.Print();

    auto S = (double)sumFR._time / sumFR_par._time;
    std::cout << "S = " << S << std::endl;

    auto S_half = (double)sumFR_half._time / sumFR_par_half._time;
    std::cout << "S_half = " << S_half << std::endl;

    auto E = S / Nthreads;
    std::cout << "E = " << E << std::endl;

    auto E_half = S_half / Nthreads;
    std::cout << "E_half = " << E_half << std::endl;
    /
}