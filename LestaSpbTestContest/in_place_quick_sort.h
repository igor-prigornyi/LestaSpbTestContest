#pragma once

namespace in_place_quick_sort {

// Функция поиска опорного элемента и упорядочивания относительно него
template <typename RandomAccessIterator, typename Comparator>
RandomAccessIterator InPlaceQuickSortPartition(RandomAccessIterator begin, RandomAccessIterator end, Comparator comparator) {
    using namespace std;

    RandomAccessIterator left  = begin;     // Итератор на левый  крайний элемент в полуинтервале [begin;end)
    RandomAccessIterator right = prev(end); // Итератор на правый крайний элемент в полуинтервале [begin;end)

    // Итератор на опорный элемент (средний)
    RandomAccessIterator pivot = begin; advance(pivot, distance(begin, end) / 2);
    
    // Значение опорного элемента
    const auto pivot_value = *pivot;

    // Покуда итератор левого края левее итератора правого края
    while (true) {
        while (comparator(*left,  pivot_value)) ++left;  // Продвигаем итератор левого  края до первого элемента, большего опорного
        while (comparator(pivot_value, *right)) --right; // Продвигаем итератор правого края до первого элемента, меньшего опорного

        if (distance(left, right) > 0) { swap(*left, *right); ++left; --right; } // Если итератор левого края левее итератора правого края, меняем эти элементы местами
        else break;                                                              // А иначе выходим из цикла
    }
    
    // Теперь опорным элементом будет тот, на который указывает итератор левого края
    return left;
}

// Функция эффективной быстрой сортировки
template <typename RandomAccessIterator, typename Comparator>
void InPlaceQuickSort(RandomAccessIterator begin, RandomAccessIterator end, Comparator comparator, int max_async_depth, int depth) {
    using namespace std;

    // Покуда в полуинтервале [begin;end) более, чем 1 элемент:
    if (distance(begin, end) > 1)
    {
        // Находим опорный элемент
        RandomAccessIterator pivot = InPlaceQuickSortPartition(begin, end, comparator);

        // Задачи (лямбды) для сортировки полуинтервалов [begin;pivot) и [pivot;end)
        auto left_task  = [begin, pivot, comparator, max_async_depth, depth] { InPlaceQuickSort(begin, pivot, comparator, max_async_depth, depth + 1); };
        auto right_task = [pivot, end,   comparator, max_async_depth, depth] { InPlaceQuickSort(pivot, end,   comparator, max_async_depth, depth + 1); };

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
    }
}

// Перегрузка InPlaceQuickSort со стандартным компаратором
template <typename RandomAccessIterator>
void InPlaceQuickSort(RandomAccessIterator begin, RandomAccessIterator end) {
    using namespace std;

    // Установим максимальную глубену рекурсии как O(log N)
    const int max_async_depth = static_cast<int>(log(static_cast<double>(end - begin)));

    // Запускаем эффективную быструю сортировку
    InPlaceQuickSort(begin, end, [](const typename RandomAccessIterator::value_type& lhs,
                                    const typename RandomAccessIterator::value_type& rhs) { return lhs < rhs; }, max_async_depth, 0);
}

}