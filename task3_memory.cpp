#include <iostream>
#include <vector>

// Структура пары условию
struct MemoryRequest {
    uint64_t address;
    uint64_t size;
};

// Вспомогательная функция для рекурсивного поиска путей до упора
void find_chains_dfs(size_t u, const std::vector<MemoryRequest>& reqs, 
                     std::vector<size_t>& current_path, 
                     std::vector<std::vector<size_t>>& result) {

    // Добавляем текущий элемент в путь           
    current_path.push_back(u);

    // Ищем всех преемников текущего элемента
    bool has_successor = false;
    for (size_t v = u + 1; v < reqs.size(); ++v) {
        if (reqs[u].address + reqs[u].size == reqs[v].address) {
            has_successor = true;
            find_chains_dfs(v, reqs, current_path, result);
        }
    }
    
    // Если дальше идти некуда — цепочка завершена и она максимальная
    if (!has_successor) {
        result.push_back(current_path);
    }
    
    // Убираем текущий элемент из пути при возврате из рекурсии
    current_path.pop_back();
}

// Формирование последовательностей
std::vector<std::vector<size_t>> build_sequences(const std::vector<MemoryRequest>& reqs) {
    // Результат — массив цепочек. каждая цепочка — массив индексов в исходном массиве
    std::vector<std::vector<size_t>> result;
    std::vector<size_t> current_path;

    // Запускаем DFS от каждого элемента, который не является продолжением другого элемента
    for (size_t i = 0; i < reqs.size(); ++i) {
        // Проверяем есть ли у элемента предшественник слева
        bool has_predecessor = false;
        for (size_t j = 0; j < i; ++j) {
            if (reqs[j].address + reqs[j].size == reqs[i].address) {
                has_predecessor = true; 
                break;
            }
        }
        
        // Запускаем поиск только от чистых источников (элементов без предшественников)
        if (!has_predecessor) {
            // Рекурсивный поиск всех цепочек от элемента i
            find_chains_dfs(i, reqs, current_path, result);
        }
    }
    return result;
}

// Печать последовательностей в требуемом формате
void print_sequences(const std::vector<MemoryRequest>& reqs, const std::vector<std::vector<size_t>>& sequences) {
    for (const auto& chain : sequences) {
        for (size_t i = 0; i < chain.size(); ++i) {
            size_t idx = chain[i];
            std::cout << "#" << idx << "[" << reqs[idx].address << "; " << reqs[idx].size << "]";
            if (i + 1 < chain.size()) std::cout << " ";
        }
        std::cout << "\n";
    }
}

int main() {
    std::vector<MemoryRequest> requests = {
        {100, 50},   // #0: [100-150]
        {150, 30},   // #1: [150-180] (стыкуется с #0)
        {200, 40},   // #2: [200-240]
        {180, 20}    // #3: [180-200] (стыкуется с #1)
    };
    
    auto sequences = build_sequences(requests);
    print_sequences(requests, sequences);
    
    return 0;
}