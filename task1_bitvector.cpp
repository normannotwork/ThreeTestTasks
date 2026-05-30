#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <algorithm>
#include <type_traits>

class BitVector {
private:
    size_t len;                  // Храним общую разрядность
    std::vector<uint8_t> bytes;  // Массив байт для хранения бит (1 байт = 8 бит)

    void mask_last_byte() {
        if (len == 0) return;
        size_t bits_in_last = len % 8;
        if (bits_in_last != 0) {
            bytes.back() &= (1 << bits_in_last) - 1;
        }
    }

public:
    // Конструктор для создания битвектора заданной длины (все биты по умолчанию 0)
    explicit BitVector(size_t bits_count = 0) 
        : len(bits_count), bytes((bits_count + 7) / 8, 0) {}


    template <typename T>
    explicit BitVector(size_t bits_count, const T* src_array)
        : len(bits_count), bytes((bits_count + 7) / 8, 0) 
    {
        // проверка типа с сообщением об ошибке
        static_assert(std::is_integral<T>::value, "BitVector constructor needs an integral type");
        
        // Проверяем входные данные
        if (src_array == nullptr || bits_count == 0) return;
        
        // Вычисляем, сколько бит в одном элементе типа T
        constexpr size_t bits_in_t = sizeof(T) * 8;

        using UnsignedT = typename std::make_unsigned<T>::type;

        // Проходим по каждому биту и устанавливаем его в битвекторе, если он установлен в исходном массиве
        for (size_t i = 0; i < bits_count; ++i) {
            size_t element_idx = i / bits_in_t;
            size_t bit_in_element = i % bits_in_t;
            
            // извлечение бита
            bool bit_val = (static_cast<UnsignedT>(src_array[element_idx]) >> bit_in_element) & 1;
            
            // Устанавливаем бит в битвекторе
            if (bit_val) {
                size_t byte_idx = i / 8;
                size_t bit_idx = i % 8;
                bytes[byte_idx] |= (1 << bit_idx);
            }
        }
        mask_last_byte();
    }

    BitVector(BitVector&&) noexcept = default;
    BitVector& operator=(BitVector&&) noexcept = default;

    BitVector(const BitVector&) = default;
    BitVector& operator=(const BitVector&) = default;


    // Метод получения разрядности
    size_t size() const {
        return len;
    }

    // Вспомогательный метод: прочитать 1 конкретный бит
    bool get_single_bit(size_t index) const {
        if (index >= len) return false;
        size_t byte_idx = index / 8;
        size_t bit_idx = index % 8;
        return (bytes[byte_idx] >> bit_idx) & 1;
    }

    // Вспомогательный метод: установить 1 конкретный бит
    void set_single_bit(size_t index, bool val) {
        if (index >= len) return;
        size_t byte_idx = index / 8;
        size_t bit_idx = index % 8;
        if (val) {
            bytes[byte_idx] |= (1 << bit_idx);
        } else {
            bytes[byte_idx] &= ~(1 << bit_idx);
        }
    }

    // Установить битовое поле из другого битвектора (произвольная длина)
    void set_field(size_t offset, const BitVector& bv) {
        for (size_t i = 0; i < bv.size(); ++i) {
            if (offset + i >= len) break; // Защита от выхода за границы
            set_single_bit(offset + i, bv.get_single_bit(i));
        }
    }

    // Получить битовое поле в виде нового битвектора
    BitVector get_field(size_t offset, size_t count) const {
        if (offset >= len) return BitVector(0);
        if (offset + count > len) {
            count = len - offset; // Обрезаем, если просят слишком много
        }
        
        BitVector result(count);
        for (size_t i = 0; i < count; ++i) {
            result.set_single_bit(i, get_single_bit(offset + i));
        }
        return result;
    }

    // Вариант для малых полей (до 64 бит) — Запись
    template <typename T>
    void set_field_small(size_t offset, size_t count, T value) {
        static_assert(std::is_integral<T>::value, "set_field_small requires integral type");

        constexpr size_t max_bits = sizeof(T) * 8;
        if (count > 64) count = 64;
        if (count > max_bits) count = max_bits; 
   
        using UnsignedT = typename std::make_unsigned<T>::type;
        UnsignedT uval = static_cast<UnsignedT>(value);
        
        for (size_t i = 0; i < count; ++i) {
            if (offset + i >= len) break;
            bool bit = (uval >> i) & 1;
            set_single_bit(offset + i, bit);
        }
    }

    // Вариант для малых полей (до 64 бит) — Чтение
    template <typename T>
    T get_field_small(size_t offset, size_t count) const {
        static_assert(std::is_integral<T>::value, "get_field_small requires integral type");

        constexpr size_t max_bits = sizeof(T) * 8;
        if (count > 64) count = 64;
        if (count > max_bits) count = max_bits; 
        
        using UnsignedT = typename std::make_unsigned<T>::type;
        UnsignedT result = 0;
        
        for (size_t i = 0; i < count; ++i) {
            if (offset + i >= len) break;
            if (get_single_bit(offset + i)) {
                result |= (static_cast<UnsignedT>(1) << i);
            }
        }
        return static_cast<T>(result);
    }
   
    template <typename T>
    size_t copy_to_array(T* dest_array, size_t dest_elements_count) const {
        // проверка типа с сообщением об ошибке
        static_assert(std::is_integral<T>::value, "copy_to_array requires integral type");
        
        // Проверяем входные данные
        if (dest_array == nullptr || dest_elements_count == 0 || len == 0) return 0;
        
        // Вычисляем, сколько бит в одном элементе типа T
        constexpr size_t bits_in_t = sizeof(T) * 8;
        size_t elements_needed = (len + bits_in_t - 1) / bits_in_t;
        size_t elements_to_copy = std::min(dest_elements_count, elements_needed);
        
        // Инициализируем выходной массив нулями (на случай, если битов меньше, чем размер массива)
        for (size_t i = 0; i < elements_to_copy; ++i) dest_array[i] = 0;
        
        // Проходим по каждому биту в битвекторе и устанавливаем его в выходном массиве, если он установлен в битвекторе
        using UnsignedT = typename std::make_unsigned<T>::type;
        for (size_t i = 0; i < len; ++i) {
            size_t element_idx = i / bits_in_t;
            if (element_idx >= elements_to_copy) break;
            // извлечение бита и установка его в выходном массиве
            if (get_single_bit(i)) {
                dest_array[element_idx] |= static_cast<T>(static_cast<UnsignedT>(1) << (i % bits_in_t));
            }
        }
        return elements_to_copy;
}

    // Вывод в 16-ричном формате (Старшие разряды СЛЕВА)
    std::string to_hex_string() const {
        if (len == 0) return "0";
        std::string res;
        int total_nibbles = (len + 3) / 4;  // количество hex-цифр
        res.reserve(total_nibbles);

        for (int i = total_nibbles - 1; i >= 0; --i) {
            int nibble = 0;
            for (int bit = 3; bit >= 0; --bit) {
                size_t bit_idx = i * 4 + bit;
                nibble <<= 1;
                if (bit_idx < len && get_single_bit(bit_idx))
                    nibble |= 1;
            }
            if (nibble < 10)
                res.push_back('0' + nibble);
            else
                res.push_back('A' + (nibble - 10));
        }
        // удаляем лидирующие нули
        size_t pos = res.find_first_not_of('0');
        return (pos == std::string::npos) ? "0" : res.substr(pos);
    }

    void clear() noexcept {
        std::fill(bytes.begin(), bytes.end(), 0);
        len = 0;
    }

    void flip() noexcept {
        for (auto& b : bytes) {
            b = ~b;
        }
        mask_last_byte();
    }

};