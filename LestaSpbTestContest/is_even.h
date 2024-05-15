#pragma once

namespace is_even {

// Функция проверки числа на чётность с помощью операции остатка от деления (%)
bool isEvenByModulo(int value);

// Функция проверки числа на чётность с помощью операции побитового AND (&)
bool isEvenByBitwise(int value);

}