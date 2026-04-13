#include <iostream>
#include <vector>
#include <pthread.h>
#include <numeric>

// Estrutura passada a cada thread
struct ThreadArgs {
    const int* data;    // ponteiro
    int        size;    // tamanho do sub-vetor
    long long  result;  // resultado parcial
};

// Função executada por cada thread
void* map_sum(void* arg) {
    ThreadArgs* a = static_cast<ThreadArgs*>(arg);
    a->result = 0;
    for (int i = 0; i < a->size; ++i)
        a->result += a->data[i];
    return nullptr;
}

// Redução
long long reduce_sum(const std::vector<ThreadArgs>& args) {
    long long total = 0;
    for (const auto& a : args)
        total += a.result;
    return total;
}

// Map Reduce
long long map_reduce(const std::vector<int>& data, int num_threads) {
    int n         = static_cast<int>(data.size());
    int base_size = n / num_threads;
    int remainder = n % num_threads;

    std::vector<pthread_t>   threads(num_threads);
    std::vector<ThreadArgs>  args(num_threads);

    // Map
    int offset = 0;
    for (int t = 0; t < num_threads; ++t) {
        int chunk      = base_size + (t == num_threads - 1 ? remainder : 0);
        args[t].data   = data.data() + offset;
        args[t].size   = chunk;
        args[t].result = 0;
        offset        += chunk;

        pthread_create(&threads[t], nullptr, map_sum, &args[t]);
    }

    for (int t = 0; t < num_threads; ++t)
        pthread_join(threads[t], nullptr);

    return reduce_sum(args);
}

// Main
int main() {
    const int N = 1'000'000;
    const int T = 4;               // número de threads

    std::vector<int> data(N);
    std::iota(data.begin(), data.end(), 1);

    std::cout << "Elementos : " << N << "\n";
    std::cout << "Threads   : " << T << "\n";

    long long resultado = map_reduce(data, T);

    // Valor esperado: N*(N+1)/2
    long long esperado = static_cast<long long>(N) * (N + 1) / 2;

    std::cout << "Resultado : " << resultado << "\n";
    std::cout << "Esperado  : " << esperado  << "\n";
    std::cout << (resultado == esperado ? "[OK]" : "[ERRO]") << "\n";

    return 0;
}


/*
Debug e compilar:
g++ -O2 -pthread map_reduce.cpp -o map_reduce
./map_reduce
*/