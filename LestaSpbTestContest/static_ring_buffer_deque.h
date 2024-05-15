#pragma once
#include <iostream>
#include <array>
#include <string>
#include <stdexcept>

namespace static_ring_buffer_deque {

// Класс статического дека на кольцевом буфере
template <typename Type, size_t capacity_>
class StaticRingBufferDeque {
private:
    std::array<Type, capacity_> buff_ = {}; // Буфер данных

    size_t size_       = 0u; // Размер диапазона, занятого данными в буфере
    size_t head_index_ = 0u; // Индекс начала диапазона данных в буфере

    // Функция инкремента со взятием по модулю capacity_
    // (заменяет оператор ++ для итератора по контейнеру)
    size_t increment_cycle(size_t pos) const {

        // В случае, если будет инкрементирован индекс, указывающий на последний элемент
        // в буфере данных, он перескочит на начальный элемент в буфере
        return (pos + 1u) % capacity_;
    }

    // Функция декремента со взятием по модулю capacity_
    // (заменяет оператор -- для итератора по контейнеру)
    size_t decrement_cycle(size_t pos) const {

        // В случае, если будет декрементирован индекс, указывающий на начальный элемент
        // в буфере данных, он перескочит на последний элемент в буфере
        return (pos + (capacity_ - 1)) % capacity_;
    }

public:
    // Конструктор по умолчанию, сгенерированный автоматически, подойдёт
    // (по умолчанию генерируется пустой дек)
    explicit StaticRingBufferDeque() = default;

    // Конструктор копирования, сгенерированный автоматически, подойдёт, так как в heap'е мы ничего не храним
    StaticRingBufferDeque(const StaticRingBufferDeque<Type, capacity_>&) = default;

    // Конструктор перемещения, сгенерированный автоматически, подойдёт, так как в heap'е мы ничего не храним
    StaticRingBufferDeque(StaticRingBufferDeque<Type, capacity_>&&) noexcept = default;

    // Оператор присвоения с копированием, сгенерированный автоматически, подойдёт, так как в heap'е мы ничего не храним
    StaticRingBufferDeque<Type, capacity_>& operator=(const StaticRingBufferDeque<Type, capacity_>&) = default;

    // Оператор присвоения с перемещением, сгенерированный автоматически, подойдёт, так как в heap'е мы ничего не храним
    StaticRingBufferDeque<Type, capacity_>& operator=(StaticRingBufferDeque<Type, capacity_>&&) noexcept = default;

    // Нетривиальный деструктор не требуется, так как в heap'е мы ничего не храним
    ~StaticRingBufferDeque() = default;

    // Функция получения размера
    size_t size() const { return size_; }

    // Функция проверки на пустоту
    bool is_empty() const { return size_ == 0u; }

    // Функция проверки, хватает ли ещё места в буфере для нового элемента
    bool is_capacity_enough() const { return size_ != capacity_; }

    // Функция добавления в конец (копирование lvalue в конец)
    void push_back(const Type& lvalue) {
        using namespace std;

        // В случае отсутствия места выбразываем исключение
        // (также можно было бы поставить assert(is_capacity_enough()) для DEBUG-конфигурации,
        // возможно использование механизма исключений на таком уровне нецелесообразно)
        if (!is_capacity_enough()) throw overflow_error("static-ring-buffer-deque capacity is not enough for push_back() call"s);
             
        buff_[(head_index_ + size_) % capacity_] = lvalue; // Cначала копируем значение в конец диапазона 
        ++size_;                                           // Затем увеличиваем размер
    }

    // Функция перемещения в конец (перемещение rvalue в конец) 
    void push_back(Type&& rvalue) {
        using namespace std;

        // В случае отсутствия места выбразываем исключение
        // (также можно было бы поставить assert(is_capacity_enough()) для DEBUG-конфигурации,
        // возможно использование механизма исключений на таком уровне нецелесообразно)
        if (!is_capacity_enough()) throw overflow_error("static-ring-buffer-deque capacity is not enough for push_back() call"s);

        buff_[(head_index_ + size_) % capacity_] = move(rvalue); // Cначала перемещаем значение в конец диапазона 
        ++size_;                                                 // Затем увеличиваем размер
    }

    // Функция добавления в начало (копирование lvalue в начало)
    void push_front(const Type& lvalue) {
        using namespace std;

        // В случае отсутствия места выбразываем исключение
        // (также можно было бы поставить assert(is_capacity_enough()) для DEBUG-конфигурации,
        // возможно использование механизма исключений на таком уровне нецелесообразно)
        if (!is_capacity_enough()) throw overflow_error("static-ring-buffer-deque capacity is not enough for push_front() call"s);

        head_index_ = decrement_cycle(head_index_); // Сначала смещаем индекс начала диапазона и "вращаем барабан" (декремент + взятие по модулю)
        buff_[head_index_] = lvalue;                // Затем копируем значение в начало диапазона
        ++size_;                                    // Увеличиваем размер
    }

    // Функция перемещения в начало (перемещение rvalue в начало)
    void push_front(Type&& rvalue) {
        using namespace std;

        // В случае отсутствия места выбразываем исключение
        // (также можно было бы поставить assert(is_capacity_enough()) для DEBUG-конфигурации,
        // возможно использование механизма исключений на таком уровне нецелесообразно)
        if (!is_capacity_enough()) throw overflow_error("static-ring-buffer-deque capacity is not enough for push_front() call"s);

        head_index_ = decrement_cycle(head_index_); // Сначала смещаем индекс начала диапазона и "вращаем барабан" (декремент + взятие по модулю)
        buff_[head_index_] = move(rvalue);          // Затем перемещаем значение в начало диапазона
        ++size_;                                    // Увеличиваем размер
    }

    // Функция удаления из конца
    Type pop_back() {
        using namespace std;

        // В случае пустого дека выбразываем исключение
        // (также можно было бы поставить assert(!is_empty()) для DEBUG-конфигурации,
        // возможно использование механизма исключений на таком уровне нецелесообразно)
        if (is_empty()) throw out_of_range("pop_back() call from empty static-ring-buffer-deque"s);

        --size_;                                         // Сначала уменьшаем размер
        return buff_[(head_index_ + size_) % capacity_]; // Затем забираем и возвращаем значение из конца диапазона
    }

    // Функция удаления из начала
    Type pop_front() {
        using namespace std;

        // В случае пустого дека выбразываем исключение
        // (также можно было бы поставить assert(!is_empty()) для DEBUG-конфигурации,
        // возможно использование механизма исключений на таком уровне нецелесообразно)
        if (is_empty()) throw out_of_range("pop_front() call from empty static-ring-buffer-deque"s);

        Type value = buff_[head_index_];            // Сначала забираем значение из начала диапазона
        head_index_ = increment_cycle(head_index_); // Затем смещаем индекс начала диапазона и "вращаем барабан" (инкремент + взятие по модулю)
        --size_;                                    // Уменьшаем размер

        return value; // Возвращаем значение
    }

    // Функция получения ссылки на элемент с определённым индексом
    Type& operator [] (size_t index) {
        using namespace std;

        // В случае попытки получения ссылки на элемент с индексом за границей диапазона значений дека, выбразываем исключение
        // (также можно было бы поставить assert(0u <= index && index < size_) для DEBUG-конфигурации,
        // возможно использование механизма исключений на таком уровне нецелесообразно)
        if (index < 0u || index >= size_) throw out_of_range("operator [] call for out of range index"s);

        return buff_[(head_index_ + index) % capacity_]; // Возвращаем значение
    }

    // Функция получения константной ссылки на элемент с определённым индексом
    // (аналогична предыдущей, но для константных деков)
    const Type& operator [] (size_t index) const {
        using namespace std;

        // В случае попытки получения ссылки на элемент с индексом за границей диапазона значений дека, выбразываем исключение
        // (также можно было бы поставить assert(0u <= index && index < size_) для DEBUG-конфигурации,
        // возможно использование механизма исключений на таком уровне нецелесообразно)
        if (index < 0u || index >= size_) throw out_of_range("operator [] call for out of range index"s);

        return buff_[(head_index_ + index) % capacity_]; // Возвращаем значение
    }
 
};

// Перегрузка оператора "<<" для вывода элементов дека в поток
template <typename Type, size_t capacity_>
std::ostream& operator << (std::ostream& os, const StaticRingBufferDeque<Type, capacity_>& static_deque) {
    using namespace std;

    os << "["s;
    bool first = true;

    for (size_t index = 0; index < static_deque.size(); ++index) {
        if (first) first = false;
        else       os << ", "s;
        os << static_deque[index];
    }

    os << "]"s;

    return os;
}

}