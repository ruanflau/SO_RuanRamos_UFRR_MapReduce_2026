# SO_RuanRamos_UFRR_MapReduce_2026

## Visão Geral
Este framework implementa o paradigma Map-Reduce em C++ utilizando a biblioteca `pthread` para processamento paralelo de alto desempenho. O sistema foi projetado para escalabilidade, permitindo a execução de operações genéricas sobre grandes volumes de dados com suporte nativo a benchmarking e auto-detecção de hardware.

## Arquitetura do Sistema

A execução é dividida em três fases principais, coordenadas por uma estrutura de dados central:

### 1. Fase de Particionamento e MAP
O vetor de entrada é segmentado em *chunks*. O sistema calcula o tamanho base de cada segmento e distribui o resto da divisão (N % T) integralmente para a última thread, garantindo cobertura total.
* **Isolamento de Dados:** Cada thread opera exclusivamente em seu sub-vetor via estrutura `ThreadArgs`.
* **Execução Paralela:** A função de mapeamento (`MapFn`) é injetada dinamicamente, permitindo operações como soma, busca de máximo ou contagens customizadas.

### 2. Barreira de Sincronização
Utiliza `pthread_join` como barreira implícita. A thread principal suspende a execução até que todos os *workers* tenham depositado os resultados parciais na memória segura (`result`).

### 3. Fase de REDUCE
Operação serial que consolida os resultados parciais. A função de redução (`ReduceFn`) garante que a lógica de agregação seja aplicada corretamente sobre os dados processados.

## Funcionalidades e Melhorias

* **Abstração Genérica:** Tipos `MapFn` e `ReduceFn` permitem trocar a lógica de negócio sem alterar o núcleo de concorrência.
* **Gestão Dinâmica de Recursos:**
    * **Auto-detecção:** Identificação automática de threads lógicas (Linux/Windows) via `sysconf` ou `GetSystemInfo`.
    * **Controle CLI:** Parâmetros para definir o número de elementos (N) e threads (T).
* **Módulo de Benchmark:** Executa baterias de testes (média de 5 execuções) e calcula o *speedup* real em relação à execução sequencial.
* **Validação de Integridade:** Cruza resultados com fórmulas matemáticas analíticas para garantir que nenhum dado foi perdido ou sobreposto.

## Interface de Linha de Comando (CLI)

| Flag | Descrição | Padrão |
| :--- | :--- | :--- |
| `-n <N>` | Número total de elementos no vetor | 1.000.000 |
| `-t <T>` | Quantidade fixa de threads | Hardware threads |
| `-auto` | Utiliza a capacidade máxima de hardware | Ativo |
| `-bench` | Executa análise de speedup (1 a T_max) | Desativado |

## Compilação e Execução

```bash
# Compilação otimizada
g++ -O2 -pthread Algoritmo_MapReduce.cpp -o map_reduce

# Execução com 5M elementos e 8 threads
./map_reduce -n 5000000 -t 8

# Modo Benchmark
./map_reduce -bench
