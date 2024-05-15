#pragma once
#include <algorithm>
#include <numeric>
#include <vector>
#include <future>
#include <cmath>

namespace merge_sort {

// Параллельная функция сортировки слиянием для диапазона [begin; end) 
// (принимает максимальный уровень рекурсии, на котором всё ещё запускается параллельная сортировка левой и правой половинок)
template <typename RandomIt>
void MergeSort(RandomIt begin, RandomIt end, int max_async_depth, int depth) {
    using namespace std;

    // Расстояние между итераторами
    int range_length = distance(begin, end);

    // Если диапазон содержит меньше 2 элементов, выходим из функции
    if (range_length < 2) return;

    // Создаём вектор, содержащий все элементы текущего диапазона
    vector<typename iterator_traits<RandomIt>::value_type> elements(begin, end);

    // Разбиваем вектор на две равные части
    auto mid = elements.begin() + range_length / 2;
    
    // Задача (лямбда) по запуску MergeSort от каждой половинки вектора
    auto left_task  = [start = elements.begin(), mid, max_async_depth, depth] { MergeSort(start,  mid, max_async_depth, depth + 1); };
    auto right_task = [mid,  finish = elements.end(), max_async_depth, depth] { MergeSort(mid, finish, max_async_depth, depth + 1); };

    // Если текущий уровень рекурсии меньше, чем максимальный - запускаем задачи по
    // сортировки половинок параллельно с помощью std::async
    if (depth <= max_async_depth) {
        auto left_future = async(left_task);
        right_task();
        left_future.get();
    }
    // А иначе выполним их последовательно
    else {
        left_task();
        right_task();
    }

    // С помощью merge сливаем отсортированные половины в исходный диапазон
    merge(elements.begin(), mid, mid, elements.end(), begin);

    // Также можно подключить <execution> и запустить merge с параллельными политиками, доступными с C++17:
    // merge(execution::par, elements.begin(), mid, mid, elements.end(), begin);
}

// Параллельная функция сортировки слиянием для диапазона [begin; end) 
template <typename RandomIt>
void MergeSort(RandomIt begin, RandomIt end) {
    using namespace std;

    // Установим максимальную глубену рекурсии как O(log N)
    const int max_async_depth = static_cast<int>(log(static_cast<double>(end - begin)));

    // Запускаем сортировку слиянием
    MergeSort(begin, end, max_async_depth, 0);
}

}