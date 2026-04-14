# SO_RuanRamos_UFRR_MapReduce_2026

## Visão Geral

Framework Map-Reduce em C++ com pthreads para processamento paralelo de vetores
de larga escala. Suporta operações intercambiáveis via ponteiros de função,
auto-detecção de hardware, validação analítica e benchmarking de speedup.

## Arquitetura

A execução é dividida em três fases determinísticas:

### 1. Particionamento e MAP

O vetor de N elementos é dividido em T chunks. O tamanho base é `⌊N/T⌋`,
com o resíduo (`N % T`) absorvido pela última thread, garantindo cobertura total.

- **Isolamento de memória:** cada thread opera exclusivamente em seu sub-vetor
  via `ThreadArgs` — sem variáveis compartilhadas na fase MAP, sem necessidade
  de mutex.
- **Operação genérica:** o tipo `MapFn` permite passar qualquer função de
  mapeamento (soma, máximo, contagem de pares) sem alterar o motor de
  concorrência.

### 2. Barreira de Sincronização

`pthread_join` em loop suspende a thread principal até que todos os workers
concluam. Só após a barreira os resultados parciais são lidos — garantindo
ausência de race conditions na fase de redução.

### 3. REDUCE

Operação serial executada pela thread principal. A função `ReduceFn` itera
sobre o vetor de `ThreadArgs` acumulando os resultados parciais sobre um valor
de identidade neutro (`0` para soma, `LLONG_MIN` para máximo).

## Funcionalidades

- **Ponteiros de função (`MapFn` / `ReduceFn`):** troca a lógica de negócio
  sem modificar a infraestrutura paralela.
- **Auto-detecção de hardware:** `sysconf(_SC_NPROCESSORS_ONLN)` no Linux e
  `GetSystemInfo` no Windows — usado como padrão quando `-t` não é informado.
- **Módulo de benchmark:** média de 5 execuções por valor de T, com cálculo
  de speedup relativo ao caso single-thread.
- **Validação analítica:** resultados cruzados com fórmulas fechadas O(1),
  independentes da implementação testada.

| Operação       | Fórmula de validação |
| :------------- | :------------------- |
| Soma total     | S = N·(N+1)/2        |
| Máximo         | M = N                |
| Contar pares   | C = ⌊N/2⌋            |

## Interface de Linha de Comando

| Flag       | Descrição                              | Padrão           |
| :--------- | :------------------------------------- | :--------------- |
| `-n <N>`   | Número de elementos no vetor           | 1.000.000        |
| `-t <T>`   | Número de threads                      | Auto (hardware)  |
| `-auto`    | Usa todas as threads lógicas           | Ativado          |
| `-bench`   | Executa benchmark de speedup (1..hw)   | Desativado       |

## Compilação e Execução

```bash
# Compilação
g++ -O2 -pthread Algoritmo_MapReduce.cpp -o map_reduce

# Execução padrão (auto-detecta threads)
./map_reduce

# 5M elementos, 8 threads
./map_reduce -n 5000000 -t 8

# Benchmark completo
./map_reduce -bench
```

## Saída Esperada

```
N=1000000  T=12  (hardware: 12 threads)

Operacao         Resultado      Esperado       Status
-------------------------------------------------------
Soma total       500000500000   500000500000   [OK]
Maximo           1000000        1000000        [OK]
Contar pares     500000         500000         [OK]
```
# Modo Benchmark
./map_reduce -bench
