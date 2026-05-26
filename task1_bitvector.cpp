#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <algorithm>

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

    // Конструктор для создания битвектора из массива байт (длина в битах + массив байт)
    explicit BitVector(size_t bits_count, const uint8_t* src_array) 
        : len(bits_count), bytes((bits_count + 7) / 8, 0) 
    {
        if (src_array == nullptr) return;
        size_t byte_count = (bits_count + 7) / 8;
        for (size_t i = 0; i < byte_count; ++i) {
            bytes[i] = src_array[i];
        } // или же std::memcpy(bytes.data(), src_array, byte_count); 
        mask_last_byte(); // Убедимся, что лишние биты в последнем байте обнулены
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
    void set_field_small(size_t offset, size_t count, uint64_t value) {
        if (count > 64) count = 64;
        
        // Побитово вытаскиваем из value и пишем в вектор
        for (size_t i = 0; i < count; ++i) {
            if (offset + i >= len) break;
            bool bit = (value >> i) & 1;
            set_single_bit(offset + i, bit);
        }
    }

    // Вариант для малых полей (до 64 бит) — Чтение
    uint64_t get_field_small(size_t offset, size_t count) const {
        if (count > 64) count = 64;
        uint64_t result = 0;
        
        for (size_t i = 0; i < count; ++i) {
            if (offset + i >= len) break;
            if (get_single_bit(offset + i)) {
                result |= (1ULL << i); // Собираем число обратно по битам
            }
        }
        return result;
    }

    // Скопировать в массив
    void copy_to_array(uint8_t* dest_array) const {
        if (dest_array) {
            std::memcpy(dest_array, bytes.data(), bytes.size());
        }
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

    void resize(size_t new_bits_count, bool fill_val = false) {
        size_t new_byte_count = (new_bits_count + 7) / 8;
        size_t old_byte_count = bytes.size();
        bytes.resize(new_byte_count, fill_val ? 0xFF : 0x00);
        len = new_bits_count;

        if (new_bits_count > 0 && new_bits_count % 8 != 0)
            mask_last_byte();
    }


};