#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>


template <typename T, typename WeightType = double>
T get_weighted_element(const std::vector<T>& elements, 
                       const std::vector<WeightType>& weights, 
                       double x) {
    
    //Проверяем входные данные на валидность
    if (elements.empty() || weights.empty()) {
        throw std::invalid_argument("Error: empty input arrays");
    }
    
    if (elements.size() != weights.size()) {
        throw std::invalid_argument("Error: sizes of elements and weights do not match");
    }

    if(x < 0.0 || x > 1.0) {
        throw std::invalid_argument("Error: x must be in the range [0, 1]");
    }

    // Считаем общую сумму всех весов и проверяем на неотрицательность
    double total_weight_sum = 0.0;
    for (size_t i = 0; i < weights.size(); ++i) {
        if (weights[i] < 0) {
            throw std::invalid_argument("Error: weight cannot be negative");
        }
        total_weight_sum += static_cast<double>(weights[i]);
    }

    // Проверяем, что сумма весов больше 0, иначе мы не сможем корректно распределить x
    if (total_weight_sum <= 0.0) {
        throw std::invalid_argument("Error: sum of weights must be greater than 0");
    }

    // Вычисляем целевое значение
    double target = x * total_weight_sum;
    double current_sum = 0.0;

    // Ищем нужный элемент
    for (size_t i = 0; i < elements.size(); ++i) {
        current_sum += static_cast<double>(weights[i]);
        if (current_sum >= target) {
            return elements[i];
        }
    }
    // отдаем самый последний элемент.
    return elements.back();
}

// Пример использования
int main() {
    // Со строками и double весами
    std::vector<std::string> fruits = {"Apple", "Banana", "Cherry"};
    std::vector<double> fruit_weights = {0.1, 0.7, 0.2};
    
    std::cout << "Fruit:" << get_weighted_element(fruits, fruit_weights, 0.5) << std::endl;

    // С целыми числами элементов и int весами
    std::vector<int> numbers = {10, 20, 30};
    std::vector<int> num_weights = {1, 5, 4};
    
    std::cout << "Number:" << get_weighted_element(numbers, num_weights, 0.99) << std::endl;

    return 0;
}