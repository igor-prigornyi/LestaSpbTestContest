#pragma once
#include <iostream>
#include <string>
#include <iterator>
#include <stdexcept>
#include <initializer_list>
#include "array_ptr.h"

namespace dynamic_ring_buffer_deque {

// Класс динамического дека на кольцевом буфере
template <typename Type>
class DynamicRingBufferDeque {
private:

    // Класс итератора/константного итератора для дека на кольцевом буфере
    template <typename IteratorAccessType>
    class BasicIterator {
    private:
        // Сделаем класс DynamicRingBufferDeque дружественным, чтобы иметь
        // доступ к полям size_, capacity_, buff_size_, begin_ и end_
        friend class DynamicRingBufferDeque;

        // Указатель на дек
        DynamicRingBufferDeque* deque_ = nullptr;

        // Индекс элемента в буфере данных, на который указывает итератор
        size_t index_ = 0u;

        // Конструктор, принимающий указатель на дек и индекс элемента
        explicit BasicIterator(DynamicRingBufferDeque* deque, size_t index) : deque_(deque), index_(index) { }

    public:
        // Тип итератора и тип данных для расстояния между итераторами
        // (нужно указать для использования стандартных алгоритмов STL,
        // которые принимают диапазоны итераторов в качестве аргументов)
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;

        using value_type = Type;        // Псевдоним для типа элементов  (Type  или const Type )
        using reference  = value_type&; // Псевдоним для типа ссылок     (Type& или const Type&)
        using pointer    = value_type*; // Псевдоним для типа указателей (Type* или const Type*)
        
        // Конструктор по умолчанию (нужен, чтобы создать поля begin_ и end_ в классе дека)
        BasicIterator() = default;

        // Конструктор копирования
        BasicIterator(const BasicIterator<value_type>& other) : deque_(other.deque_), index_(other.index_) { }

        // Оператор присвоения
        BasicIterator& operator = (const BasicIterator& rhs) {
            deque_ = rhs.deque_;
            index_ = rhs.index_;
            return *this;
        }

        // Функция обмена с другим итератором
        void swap(const BasicIterator& rhs) {
            std::swap(deque_, rhs.deque_);
            std::swap(index_, rhs.index_);
        }

        // Операторы сравнения итераторов
        bool operator == (const BasicIterator& rhs) const noexcept { return deque_ == rhs.deque_ && index_ == rhs.index_; }
        bool operator != (const BasicIterator& rhs) const noexcept { return !(*this == rhs); }
       
        // Оператор префиксного инкремента (++iter)
        // (при инкременте итератора, ссылающегося на последний элемент в буфере, он будет перескакиевать на начальный)
        BasicIterator& operator ++ () {
            index_ = (index_ + 1u) % deque_->buff_size_;
            return *this;
        }

        // Оператор постфиксного инкремента (iter++)
        // (при инкременте итератора, ссылающегося на последний элемент в буфере, он будет перескакиевать на начальный)
        BasicIterator operator ++ (int) {
            BasicIterator tmp(*this);
            ++(*this);
            return tmp;
        }

        // Оператор префиксного декремента (--iter)
        // (при декременте итератора, ссылающегося на начальный элемент в буфере, он будет перескакиевать на последний)
        BasicIterator& operator -- () {
            using namespace std;
            index_ = (index_ + (deque_->buff_size_ - 1)) % deque_->buff_size_;
            return *this;
        }

        // Оператор постфиксного декремента (iter--)
        // (при декременте итератора, ссылающегося на начальный элемент в буфере, он будет перескакиевать на последний)
        BasicIterator operator -- (int) {
            BasicIterator tmp(*this);
            --(*this);
            return tmp;
        }

        // Оператор разыменования элемента, на который указывает итератор
        reference operator * () const {
            using namespace std;
            return deque_->buff_[index_];
        }

        // Оператор доступа к членам (полям и методам) элемента, на который указывает итератор
        pointer operator -> () const {
            using namespace std;
            return deque_->buff_[index_];
        }
    };

public:
    // Псевдонимы для итератора и константного итератора
    using Iterator      = BasicIterator<Type>;
    using ConstIterator = BasicIterator<const Type>;

private:
    array_ptr::ArrayPtr<Type> buff_; // Умный указатель на буфер данных в heap'е

    size_t size_      = 0u; // Размер дека  (размер диапазона, занятого данными в буфере)
    size_t capacity_  = 0u; // Ёмкость дека (физический размер буфера, за исключением разделительной ячейки)
    size_t buff_size_ = 0u; // Физический размер буфера, включая разделительную ячейку

    Iterator begin_ = Iterator(this, 0u); // Итератор на начало диапазона данных в буфере
    Iterator end_   = Iterator(this, 0u); // Итератор на конец  диапазона данных в буфере

    // Так как мы имеем дело с кольцевым буфером, при его полном заполнении итераторы
    // begin_ и end_ совместятся. Чтобы этого избежать, будем создавать буфер на 1 
    // элемент больше, чем необходимый размер. Итератор конца будет указывать на 
    // разделительную ячейку буфера [*]:
    // 
    //         0  1  2  3  4  5  6  7  8  9  <- индексы элементов в буфере данных
    //         |  |  |  |  |  |  |  |  |  |
    // buff_: [4][5][6][*][ ][ ][0][1][2][3] <- индекс элементов в деке
    //                  ^        ^
    //                 end_    begin_
    //
    // В случае на иллюстрации size_ = 7, capacity_ = 9, buff_size_ = 10
    // 
    // (очевидно, многие величины взаимосвязаны, например, buff_size_ = capacity_ + 1, а
    // size_ = std::distance(begin_, end_), можно было бы исключить некоторые переменные
    // ради экономии места, однако их использование делает код более наглядным)
    
public:
    // Конструктор по умолчанию создаёт пустой дек
    explicit DynamicRingBufferDeque() = default;

    // Конструктор, создающий дек из size элементов со значениями по умолчанию
    explicit DynamicRingBufferDeque(size_t size) : buff_(array_ptr::ArrayPtr<Type>(size + 1)),
                                                   size_(size),
                                                   capacity_(size),
                                                   buff_size_(size + 1),
                                                   begin_(Iterator(this, 0u)),
                                                   end_  (Iterator(this, size)) { }

    // Конструктор, создающий дек из size элементов со значениями value
    explicit DynamicRingBufferDeque(size_t size, const Type& value) : DynamicRingBufferDeque(size) {
        std::fill(begin_, end_, value);
    }

    // Конструктор, создающий дек из элементов std::initializer_list
    explicit DynamicRingBufferDeque(const std::initializer_list<Type>& values) {

        // Используем идеому copy-and-swap: создаём дек, инициализированный элементами из
        // std::initializer_list, а затем меняем его местами с текущим
        // (ранее зарезервированные ресурсы будут высвобождены после вызова деструктора для временного дека)
        CopyAndSwapFromIteratorRange(values.begin(), values.end());
    }

    // Конструктор копирования
    DynamicRingBufferDeque(const DynamicRingBufferDeque& lvalue) {

        // Используем идеому copy-and-swap: создаём дек, инициализированный элементами из
        // дека lvalue, а затем меняем его местами с текущим
        // (ранее зарезервированные ресурсы будут высвобождены после вызова деструктора для временного дека)
        CopyAndSwapFromIteratorRange(lvalue.begin(), lvalue.end());
    }

    // Конструктор перемещения
    DynamicRingBufferDeque(DynamicRingBufferDeque&& rvalue) noexcept {

        // Просто меняем местами деки, для rvalue сработает деструктор и ресурсы будут высвобождены
        swap(rvalue);
    }

    // Оператор присвоения с копированием
    DynamicRingBufferDeque& operator = (const DynamicRingBufferDeque& lvalue) {

        // В случае если не пытаемся присвоить объект самому себе
        if (this != &lvalue) {
            // Снова используем идеому copy-and-swap
            CopyAndSwapFromIteratorRange(lvalue.begin(), lvalue.end());
        }
        return *this;
    }

    // Оператор присвоения с перемещением
    DynamicRingBufferDeque& operator = (DynamicRingBufferDeque&& rvalue) noexcept {

        // Просто меняем местами деки, для rvalue сработает деструктор и ресурсы будут высвобождены
        swap(rvalue);
        return *this;
    }

    // Деструктор тривиальный, для умного указателя деструктор сработает
    // автоматически и ресурсы будут высвобождены
    ~DynamicRingBufferDeque() = default;

    // Функция резервирования места
    void reserve(size_t new_capacity) {

        // Если новая ёмкость больше предыдущей
        if (new_capacity > capacity_) {

            // Используем идеому copy-and-swap для буфера: создаём новый буфер нового размера
            array_ptr::ArrayPtr<Type> tmp_buff_(new_capacity + 1);

            // После чего перемещаем туда данные из старого буфера
            std::move(begin(), end(), tmp_buff_.get());

            // Меняем буферы местами (старые ресурсы будут высвобождены после вызова деструктора tmp_buff_)
            buff_.swap(tmp_buff_);

            // Обновляем ёмкость, размер буфера и итераторы на начало и конец диапазона данных в буфере
            capacity_  = new_capacity;
            buff_size_ = new_capacity + 1;
            begin_ = Iterator(this, 0u);
            end_   = Iterator(this, size_);
        }
    }

    // Функция очистки дека
    void clear() {

        // Просто сбрасываем размер на ноль, не трогая ресурсы
        size_ = 0u;
        begin_ = Iterator(this, 0u);
        end_   = Iterator(this, 0u);
    }

    // Функция обмена с другим деком
    void swap(DynamicRingBufferDeque& other) {
        buff_.swap(other.buff_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        std::swap(buff_size_, other.buff_size_);

        begin_.swap(other.begin_);
        end_.swap(other.end_);
    }

    // Функция получения размера
    size_t size() const { return size_; }

    // Функция проверки на пустоту
    bool empty() const { return size_ == 0u; }

    // Итераторы на начало и конец
    Iterator begin() { return begin_; }
    Iterator end()   { return end_; }

    // Константные итераторы на начало и конец
    ConstIterator cbegin() const { return begin_; }
    ConstIterator cend()   const { return end_; }

    ConstIterator begin()  const { return cbegin(); }
    ConstIterator end()    const { return cend(); }

    // Функция добавления в конец (копирование lvalue в конец)
    void push_back(const Type& lvalue) {

        // Если ёмкости дека не хватает для добавления нового элемента, то выделим место
        reserve_if_not_enough();

        // Добавляем элемент в конец
        *(end_++) = lvalue;
        ++size_;
    }

    // Функция перемещения в конец (перемещение rvalue в конец) 
    void push_back(Type&& rvalue) {

        // Если ёмкости дека не хватает для добавления нового элемента, то выделим место
        reserve_if_not_enough();

        // Перемещаем элемент в конец
        *(end_++) = std::move(rvalue);
        ++size_;
    }

    // Функция добавления в начало (копирование lvalue в начало)
    void push_front(const Type& lvalue) {

        // Если ёмкости дека не хватает для добавления нового элемента, то выделим место
        reserve_if_not_enough();

        // Добавляем элемент в начало
        *(--begin_) = lvalue;
        ++size_;
    }

    // Функция перемещения в начало (перемещение rvalue в начало)
    void push_front(Type&& rvalue) {

        // Если ёмкости дека не хватает для добавления нового элемента, то выделим место
        reserve_if_not_enough();

        // Добавляем элемент в начало
        *(--begin_) = std::move(rvalue);
        ++size_;
    }

    // Функция удаления из конца
    Type pop_back() {
        using namespace std;

        // В случае пустого дека выбразываем исключение
        // (также можно было бы поставить assert(!empty()) для DEBUG-конфигурации,
        // возможно использование механизма исключений на таком уровне нецелесообразно)
        if (empty()) throw out_of_range("pop_back() call from empty dynamic-ring-buffer-deque"s);

        --size_;
        return *(--end_);
    }

    // Функция удаления из начала
    Type pop_front() {
        using namespace std;

        // В случае пустого дека выбразываем исключение
        // (также можно было бы поставить assert(!empty()) для DEBUG-конфигурации,
        // возможно использование механизма исключений на таком уровне нецелесообразно)
        if (empty()) throw out_of_range("pop_front() call from empty dynamic-ring-buffer-deque"s);

        --size_;
        return *(begin_++);
    }

    // Функция получения ссылки на элемент с определённым индексом
    Type& operator [] (size_t index) {
        using namespace std;

        // В случае попытки получения ссылки на элемент с индексом за границей диапазона значений дека, выбразываем исключение
        // (также можно было бы поставить assert(0u <= index && index < size_) для DEBUG-конфигурации,
        // возможно использование механизма исключений на таком уровне нецелесообразно)
        if (index < 0u || index >= size_) throw out_of_range("operator [] call for out of range index"s);

        return buff_[(begin_.index_ + index) % buff_size_]; // Возвращаем значение
    }

    // Функция получения константной ссылки на элемент с определённым индексом
    // (аналогична предыдущей, но для константных деков)
    const Type& operator [] (size_t index) const {
        using namespace std;

        // В случае попытки получения ссылки на элемент с индексом за границей диапазона значений дека, выбразываем исключение
        // (также можно было бы поставить assert(0u <= index && index < size_) для DEBUG-конфигурации,
        // возможно использование механизма исключений на таком уровне нецелесообразно)
        if (index < 0u || index >= size_) throw out_of_range("operator [] call for out of range index"s);

        return buff_[(begin_.index_ + index) % buff_size_]; // Возвращаем значение
    }

private:
    // Функция для реализации идеомы copy-and-swap в конструкторе: создаёт дек, инициализированный
    // элементами в интервале [begin; end) и меняет его местами с текущим
    template <typename ContainerIterator>
    void CopyAndSwapFromIteratorRange(const ContainerIterator begin, const ContainerIterator end) {

        DynamicRingBufferDeque<Type> tmp_deque(std::distance(begin, end));
        std::copy(begin, end, tmp_deque.begin());
        swap(tmp_deque);
    }

    // Функция резервирования места под новые элементы, если его недостаточно
    void reserve_if_not_enough() {

        // Если ёмкости дека не хватает для добавления нового элемента
        if (size_ == capacity_) {

            // Новая ёмкость в два раза больше текущей (либо просто 1, если изначально дек был пустой)
            const size_t new_capacity = capacity_ ? capacity_ * 2 : 1u;

            // Резервируем память под новые элементы
            reserve(new_capacity);
        }
    }

};

// Перегрузка оператора "<<" для вывода элементов дека в поток
template <typename Type>
std::ostream& operator << (std::ostream& os, const DynamicRingBufferDeque<Type>& dynamic_deque) {
    using namespace std;

    os << "["s;
    bool first = true;

    for (const Type& element : dynamic_deque) {
        if (first) first = false;
        else os << ", "s;
        os << element;
    }

    os << "]"s;
    return os;
}

}