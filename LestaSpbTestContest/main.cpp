#include <iostream>
#include <vector>
#include <cassert>
#include <numeric>
#include <algorithm>

#include "is_even.h"                   // Задание 1
#include "static_ring_buffer_deque.h"  // Задание 2
#include "dynamic_ring_buffer_deque.h" // Задание 2
#include "merge_sort.h"                // Задание 3
#include "in_place_quick_sort.h"       // Задание 3

int main() {
	using namespace std;

	// Задание 1
	// Тестирование функции проверки на чётность
	{
		using namespace is_even;

		assert(!isEvenByModulo(41));
		assert(!isEvenByModulo(41));

		assert(!isEvenByModulo(-41));
		assert(!isEvenByModulo(-41));

		assert(isEvenByModulo(42));
		assert(isEvenByModulo(42));

		assert(isEvenByModulo(-42));
		assert(isEvenByModulo(-42));
	}

	// Задание 2
	// Тестирование статического дека на кольцевом буфере
	{
		using namespace static_ring_buffer_deque;

		cout << "StaticRingBufferDeque testing"s << endl;

		StaticRingBufferDeque<int, 5> ring;

		// Изначально дек должен быть пустым, а ёмкости должно хватать для добавления новый элементов
		assert(ring.is_empty());
		assert(ring.is_capacity_enough());

		cout << ring << endl;

		// Попытка вызова pop_back/pop_front из пустого дека должна привести к вызову исключения
		try { ring.pop_back(); assert(false); }
		catch (const out_of_range&) {}
		catch (...) { assert(false); }

		// Добавляем элементы
		ring.push_front(3);
		ring.push_back(4);
		ring.push_front(2);
		ring.push_back(5);
		ring.push_front(1);

		// Проверим, что элементы добавлены правильно
		assert(!ring.is_empty());
		assert(!ring.is_capacity_enough());
		assert(ring.size() == 5u);
		assert(ring[0] == 1 && ring[1] == 2 && ring[2] == 3 && ring[3] == 4 && ring[4] == 5);

		cout << ring << endl;

		// Удалим крайние элементы
		ring.pop_back();
		ring.pop_front();

		// Проверим, что элементы удалены правильно
		assert(ring.is_capacity_enough());
		assert(ring.size() == 3u);
		assert(ring[0] == 2 && ring[1] == 3 && ring[2] == 4);

		cout << ring << endl;

		// Добавим элемены после удаления
		ring.push_front(-1);
		ring.push_back(-5);

		// Проверим, что элементы добавлены правильно
		assert(ring.size() == 5u);
		assert(ring[0] == -1 && ring[1] == 2 && ring[2] == 3 && ring[3] == 4 && ring[4] == -5);

		cout << ring << endl;

		// Попытка добавить элемент, когда не осталось места, должна привести к вызову исключения
		try { ring.push_back(6); assert(false); }
		catch (const overflow_error&) { }
		catch (...) { assert(false); }

		// Попытка получить элемент по несуществующему индексу должна привести к вызову исключения
		try { ring[404]; assert(false); }
		catch (const out_of_range&) { }
		catch (...) { assert(false); }
	}

	// Задание 2
	// Тестирование динамического дека на кольцевом буфере
	{
		using namespace dynamic_ring_buffer_deque;

		cout << endl << "DynamicRingBufferDeque testing"s << endl;

		DynamicRingBufferDeque<int> ring;
		
		// Изначально дек должен быть пустым
		assert(ring.empty());

		cout << ring << endl;

		// Добавляем элементы
		ring.push_back(3);
		ring.push_front(2);
		ring.push_back(4);
		ring.push_front(1);
		ring.push_back(5);

		// Проверим, что элементы добавлены правильно
		assert(!ring.empty());
		assert(ring.size() == 5u);
		assert(ring[0] == 1 && ring[1] == 2 && ring[2] == 3 && ring[3] == 4 && ring[4] == 5);

		cout << ring << endl;

		// Удалим крайние элементы
		ring.pop_front();
		ring.pop_back();

		// Проверим, что элементы удалены правильно
		assert(ring.size() == 3u);
		assert(ring[0] == 2 && ring[1] == 3 && ring[2] == 4);

		cout << ring << endl;

		// Добавим элемены после удаления
		ring.push_front(11);
		ring.push_back(15);

		// Проверим, что элементы добавлены правильно
		assert(ring.size() == 5u);
		assert(ring[0] == 11 && ring[1] == 2 && ring[2] == 3 && ring[3] == 4 && ring[4] == 15);

		cout << ring << endl;

		// Посчитаем сумму элементов в деке
		int sum = accumulate(ring.begin(), ring.end(), 0);
		assert(sum == 35);

		// Очистим дек
		ring.clear();

		// Проверим, что он пуст
		assert(ring.empty());

		cout << ring << endl;

		// Вновь наполним дек данными
		ring.push_back(8);
		ring.push_back(9);
		ring.push_front(7);
		ring.push_front(6);
		ring.push_back(10);

		// Проверим, что элементы добавлены правильно
		assert(ring[0] == 6 && ring[1] == 7 && ring[2] == 8 && ring[3] == 9 && ring[4] == 10);

		cout << ring << endl;
	}

	// Задание 3
	// Тестирование параллельной версии merge sort
	{
		using namespace merge_sort;

		cout << endl << "MergeSort testing"s << endl;

		vector<int> values({ 42, -9, 15, 3, -21, 95, 38, 17, -30, 12, 19, 44, 0, 24, 15, 68, 21, -49, -51 });
		MergeSort(values.begin(), values.end());

		for (const auto& e : values) cout << e << " "s;
		cout << endl;
	}

	// Задание 3
	// Тестирование параллельной версии in-place quick sort
	{
		using namespace in_place_quick_sort;

		cout << endl << "InPlaceQuickSort testing"s << endl;

		vector<int> values({ 42, -9, 15, 3, -21, 95, 38, 17, -30, 12, 19, 44, 0, 24, 15, 68, 21, -49, -51 });
		InPlaceQuickSort(values.begin(), values.end());

		for (const auto& e : values) cout << e << " "s;
		cout << endl;
	}

	return 0;
}