#pragma once

namespace array_ptr {

// Умный указатель на массив элементов типа Type
template <typename Type>
class ArrayPtr {
private:
    // Сырой указатель на массив элементов
    Type* raw_ptr_ = nullptr;

public:
    // Конструктор по умолчанию: raw_ptr_ = nullptr 
    ArrayPtr() = default;

    // Конструктор: создаёт по адресу raw_ptr_ size элементов типа Type, инициализированных значением по умолчанию
    explicit ArrayPtr(size_t size) {
        if (size) raw_ptr_ = new Type[size]{};
        else      raw_ptr_ = nullptr;
    }

    // Конструктор: начинает владение массивом по адресу raw_ptr
    explicit ArrayPtr(Type* raw_ptr) noexcept : raw_ptr_(raw_ptr) { }

    // Конструктор копирования запрещён
    ArrayPtr(const ArrayPtr&) = delete;

    // Конструктор перемещения
    ArrayPtr(ArrayPtr&& rvalue) { swap(rvalue); }

    // Операция пресваивания c копированием запрещена
    ArrayPtr& operator = (const ArrayPtr&) = delete;

    // Операция пресваивания c перемещением
    ArrayPtr& operator = (ArrayPtr&& rvalue) { swap(rvalue); return *this; }

    // Деструктор
    ~ArrayPtr() { delete[] raw_ptr_; }

    // Возврат сырого указателя raw_ptr_
    Type* get() const noexcept { return raw_ptr_; }

    // Прекращает владение массивом по адресу raw_ptr_
    Type* release() noexcept { return std::exchange(raw_ptr_, nullptr); }

    // Обмен с другим умным указателем
    void swap(ArrayPtr& other) noexcept { std::swap(raw_ptr_, other.raw_ptr_); }

    // Возврат ссылки на элемент массива с индексом index
    Type& operator [] (size_t index) noexcept { return raw_ptr_[index]; }

    // Возврат константной ссылки на элемент массива с индексом index
    const Type& operator [] (size_t index) const noexcept { return raw_ptr_[index]; }

    // Преведение к типу bool возвращает true, если raw_ptr_ не nullptr
    explicit operator bool() const { return static_cast<bool>(raw_ptr_); }
};

}