// Map-Reduce completo — g++ -O2 -pthread map_reduce_completo.cpp -o map_reduce
#include <iostream>
#include <vector>
#include <pthread.h>
#include <numeric>
#include <chrono>
#include <cstring>
#include <climits>

#ifdef __linux__
  #include <unistd.h>
  static int hw_threads() { return (int)sysconf(_SC_NPROCESSORS_ONLN); }
#elif defined(_WIN32)
  #include <windows.h>
  static int hw_threads() { SYSTEM_INFO i; GetSystemInfo(&i); return (int)i.dwNumberOfProcessors; }
#else
  static int hw_threads() { return 1; }
#endif

using MapFn    = long long (*)(const int*, int);
using ReduceFn = long long (*)(long long, long long);

struct ThreadArgs {
    const int* data;
    int        size;
    long long  result;
    MapFn      map_fn;
};

void* worker(void* arg) {
    auto* a   = static_cast<ThreadArgs*>(arg);
    a->result = a->map_fn(a->data, a->size);
    return nullptr;
}

long long fn_soma   (const int* d, int n) { long long s=0;    for(int i=0;i<n;i++) s+=d[i];           return s; }
long long fn_max    (const int* d, int n) { long long m=d[0]; for(int i=1;i<n;i++) if(d[i]>m) m=d[i]; return m; }
long long fn_contar (const int* d, int n) { long long c=0;    for(int i=0;i<n;i++) if(d[i]%2==0) c++; return c; }

long long red_soma (long long a, long long b) { return a + b;       }
long long red_max  (long long a, long long b) { return a > b ? a:b; }

long long map_reduce(const std::vector<int>& data, int T,
                     MapFn map_fn, ReduceFn reduce_fn, long long identity) {
    int n = (int)data.size(), base = n/T, rem = n%T;
    std::vector<pthread_t>  threads(T);
    std::vector<ThreadArgs> args(T);

    int off = 0;
    for (int t=0; t<T; t++) {
        args[t] = { data.data()+off, base+(t==T-1?rem:0), 0, map_fn };
        off += args[t].size;
        pthread_create(&threads[t], nullptr, worker, &args[t]);
    }
    for (int t=0; t<T; t++) pthread_join(threads[t], nullptr);

    long long result = identity;
    for (int t=0; t<T; t++) result = reduce_fn(result, args[t].result);
    return result;
}

struct Teste {
    const char* nome;
    MapFn       map_fn;
    ReduceFn    reduce_fn;
    long long   identity;
    long long (*esperado)(int);
};

static Teste testes[] = {
    { "Soma total  ", fn_soma,   red_soma, 0,        [](int N){ return (long long)N*(N+1)/2; } },
    { "Maximo      ", fn_max,    red_max,  LLONG_MIN, [](int N){ return (long long)N;         } },
    { "Contar pares", fn_contar, red_soma, 0,        [](int N){ return (long long)N/2;        } },
};

void rodar_testes(const std::vector<int>& data, int T) {
    printf("\n%-16s %-14s %-14s %s\n","Operacao","Resultado","Esperado","Status");
    printf("%s\n",std::string(55,'-').c_str());
    for (auto& t : testes) {
        long long res = map_reduce(data,T,t.map_fn,t.reduce_fn,t.identity);
        long long exp = t.esperado((int)data.size());
        printf("%-16s %-14lld %-14lld %s\n",t.nome,res,exp,res==exp?"[OK]":"[ERRO]");
    }
}

void benchmark(const std::vector<int>& data, int max_T) {
    printf("\nBenchmark (soma, N=%d)\n",(int)data.size());
    printf("%-10s %-12s %-8s\n","Threads","Tempo(ms)","Speedup");
    printf("%s\n",std::string(33,'-').c_str());
    double base = 0;
    for (int T=1; T<=max_T; T++) {
        auto t0 = std::chrono::high_resolution_clock::now();
        for (int r=0;r<5;r++) map_reduce(data,T,fn_soma,red_soma,0);
        double ms = std::chrono::duration<double,std::milli>(
            std::chrono::high_resolution_clock::now()-t0).count()/5.0;
        if (T==1) base=ms;
        printf("%-10d %-12.3f %.2fx\n",T,ms,base/ms);
    }
}

int main(int argc, char* argv[]) {
    int hw=hw_threads(), N=1000000, T=hw;
    bool do_bench=false;

    for (int i=1;i<argc;i++) {
        if      (!strcmp(argv[i],"-n")    && i+1<argc) N=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-t")    && i+1<argc) T=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-auto"))              T=hw;
        else if (!strcmp(argv[i],"-bench"))             do_bench=true;
        else {
            printf("Uso: %s [-n <N>] [-t <T>] [-auto] [-bench]\n"
                   "  -n      elementos (default 1.000.000)\n"
                   "  -t      threads   (default: auto)\n"
                   "  -auto   usa todas as threads da maquina\n"
                   "  -bench  mede speedup de 1 ate max threads\n",argv[0]);
            return 0;
        }
    }

    if (T<1||T>N) { fprintf(stderr,"Threads invalido.\n"); return 1; }

    std::vector<int> data(N);
    std::iota(data.begin(), data.end(), 1);

    printf("N=%d  T=%d  (hardware: %d threads)\n",N,T,hw);
    rodar_testes(data, T);
    if (do_bench) benchmark(data, hw);
    return 0;
}
/*
Execucão:
g++ -O2 -pthread Algoritmo_MapReduce.cpp -o Algoritmo_MapReduce

./Algoritmo_MapReduce                    # auto: usa todas as threads da máquina
./Algoritmo_MapReduce -n 5000000 -t 8   # 5M elementos, 8 threads
./Algoritmo_MapReduce -t 1               # força single-thread
./Algoritmo_MapReduce -bench             # benchmark 1..N threads com speedup
*/