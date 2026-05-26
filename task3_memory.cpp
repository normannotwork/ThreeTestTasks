#include <iostream>
#include <vector>

struct MemoryRequest {
    int index;
    uint64_t address;
    uint64_t size;
};

// Находим и печатаем все max цепочки памяти
void build_sequences(const std::vector<MemoryRequest>& reqs) {
    
    // перебираем каждый запрос как потенциальное начало цепочки
    for (size_t start_idx = 0; start_idx < reqs.size(); ++start_idx) {
        
        // является ли этот запрос началом?
        bool is_start = true;
        for (size_t i = 0; i < start_idx; ++i) {
            if (reqs[i].address + reqs[i].size == reqs[start_idx].address) {
                is_start = false; // Если можно - значит не начало
                break;
            }
        }
        if (!is_start) continue; // пропускаем, не начало цепочки
        
        // Строим цепочку - идем вперёд
        std::vector<MemoryRequest> chain;
        chain.push_back(reqs[start_idx]);
        size_t current = start_idx;
        
        while (true) {
            bool found_next = false;
            for (size_t i = current + 1; i < reqs.size(); ++i) {
                uint64_t current_end = reqs[current].address + reqs[current].size;
                if (current_end == reqs[i].address) {
                    chain.push_back(reqs[i]);
                    current = i;
                    found_next = true;
                    break;
                }
            }
            if (!found_next) break; // цепочка закончилась
        }
        
        // Шаг 3: печатаем результат
        for (size_t i = 0; i < chain.size(); ++i) {
            std::cout << "#" << chain[i].index << "[" 
                      << chain[i].address << "; " << chain[i].size << "]";
            if (i + 1 < chain.size()) std::cout << " ";
        }
        std::cout << "\n";
    }
}

// Пример использования
int main() {
    std::vector<MemoryRequest> requests = {
        {0, 100, 50},   // 100-150
        {1, 150, 30},   // 150-180 (стыкуется с #0)
        {2, 200, 40},   // 200-240 (отдельная цепочка)
        {3, 180, 20}    // 180-200 (стыкуется с #1)
    };
    
    build_sequences(requests);
    return 0;
}