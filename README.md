# Projeto STR
## Descrição geral 
Neste projeto, vamos desenvolver um simulador de elevador concorrente 
utilizando linguagem C e semáforos no Linux. O sistema contará com um único 
elevador que atende múltiplos usuários concorrentes em um prédio de 5 
andares. O objetivo é gerenciar chamadas de forma organizada, garantindo que o 
elevador atenda os usuários corretamente sem conflitos entre threads. 
#### Objetivos 
* Implementar threads concorrentes no Linux. 
* Utilizar semáforos para controle de acesso e sincronização. 
* Gerenciar uma fila de chamadas de maneira eficiente. 
* Simular movimentação do elevador baseada em requisições. 
#### Funcionamento do sistema 

Principais componentes 

1. Usuários (threads) fazem chamadas para o elevador de andares aleatórios. 
Não deve haver limite na quantidade de usuários do sistema, embora você 
possa gerar novos usuários a uma determinado fluxo de usuários. 

2. Cada chamada é armazenada em uma fila compartilhada controlada por 
semáforos. Você deve garantir que essa fila seja grande o suficiente para 
atender ao fluxo de usuários. 

3. O elevador (outra thread) verifica a fila e atende as chamadas na ordem de 
chegada (FIFO). 

4. Quando o elevador chega ao andar solicitado, ele aguarda alguns 
segundos para simular embarque/desembarque. 

5. A thread continua até que todas as chamadas tenham sido atendidas. 

#### Regras do elevador 
* O elevador começa no andar 0 e só se move quando há chamadas 
pendentes. 
* As chamadas são atendidas na ordem de chegada, independentemente da 
direção. 
* Durante o deslocamento, novas chamadas podem ser adicionadas à fila, 
exigindo que o elevador as processe assim que possível. 
* Um semáforo protege o acesso à fila, evitando que múltiplos usuários 
modifiquem os dados ao mesmo tempo. 
Exemplo de execução 
Um possível caso de uso é dado a seguir: 
* Usuário 1 chama o elevador do andar 2 para o andar 4. 
* Usuário 2 chama do andar 0 para o andar 3. 
* O elevador atende primeiro a chamada do andar 2 para o 4 e depois desce 
ao andar 0 para levar o usuário ao 3. 

O sistema deve continuar executando indefinidamente e sempre atender novas 
chamadas de usuários que chegam. 

#### Exemplo de saída 

A seguir temos um exemplo da saída esperada do seu sistema. 

[Elevador] Inicializado no andar 0. Aguardando chamadas... 

[Usuário 1] Chamando o elevador do andar 2 para o andar 4. 

[Usuário 2] Chamando o elevador do andar 0 para o andar 3. 

[Elevador] Nova chamada recebida de Usuário 1: Andar 2 -> 4. 

[Elevador] Movendo-se para o andar 2... 

[Elevador] Chegou ao andar 2. Usuário 1 embarcou. 

[Elevador] Movendo-se para o andar 3... 

[Elevador] Movendo-se para o andar 4... 

[Elevador] Chegou ao andar 4. Usuário 1 desembarcou. 

[Elevador] Nova chamada recebida de Usuário 2: Andar 0 -> 3. 

[Elevador] Movendo-se para o andar 3... 

[Elevador] Chegou ao andar 3. Usuário 2 desembarcou. 

[Elevador] Nenhuma chamada pendente. Retornando ao modo de espera.

# Vídeo de apresentação

[Link para o vídeo no YouTube](https://www.youtube.com/watch?v=GRmsM8l9Q2U)

# Versão do sistema operacional

Este projeto foi desenvolvido e testado em um sistema operacional Linux, versão Ubuntu 20.04.

# Como executar

* `Compilar`: 
    ```shell
    gcc -o Projeto_STR_Elevador Projeto_STR_Elevador.c -lpthread -lm
    ```
- `-lpthread`: Vincula a biblioteca de threads POSIX (pthread).
- `-lm`: Vincula a biblioteca matemática (libm), necessária para funções como sqrt e log.

Sem essas flags, o linker não conseguirá resolver as referências às funções fornecidas por essas bibliotecas, resultando em erros de compilação.
* `Executar`: 
    ```shell
    ./Projeto_STR_Elevador
    ```

# Resumo das bibliotecas e descrição

## Biblioteca `pthread`

A biblioteca `pthread` (POSIX threads) é usada para criar e gerenciar threads em programas C.

- `pthread_t`: Tipo de dado usado para identificar uma thread.
- `pthread_create`: Cria uma nova thread.
  ```c
  int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
  ```
- `pthread_join`: Aguarda a terminação de uma thread.
  ```c
  int pthread_join(pthread_t thread, void **retval);
  ```

## Biblioteca `semaphore`

A biblioteca `semaphore` é usada para controlar o acesso a recursos compartilhados em ambientes de programação concorrente.

- `sem_t`: Tipo de dado usado para representar um semáforo.
- `sem_init`: Inicializa um semáforo.
    ```c
    int sem_init(sem_t *sem, int pshared, unsigned int value);
    ```
- `sem_wait`: Decrementa (trava) o semáforo. Se o valor do semáforo é zero, a chamada bloqueia até que o semáforo seja maior que zero.
    ```c
    int sem_wait(sem_t *sem);
    ```
- `sem_post`: Incrementa (libera) o semáforo. Se havia threads bloqueadas esperando pelo semáforo, uma delas é desbloqueada.
    ```c
    int sem_post(sem_t *sem);
    ```
- `sem_destroy`: Destroi um semáforo.
    ```c
    int sem_destroy(sem_t *sem);
    ```

# Utilização no Código

## `pthread`

- **Criação de Threads**:
    ```c
    pthread_create(&elevador, NULL, thread_elevador, NULL);
    pthread_create(&gerador_usuarios, NULL, thread_gerador_usuarios, NULL);
    ```
- **Aguardar Terminação de Threads**:
    ```c
    pthread_join(elevador, NULL);
    pthread_join(gerador_usuarios, NULL);
    ```

## `semaphore`

- **Inicialização de Semáforos**:
    ```c
    sem_init(&mutex_fila, 0, 1);
    sem_init(&fila_vazia, 0, 0);
    sem_init(&fila_cheia, 0, TAMANHO_FILA);
    ```
- **Controle de Acesso à Fila**:

    - **Adicionar Chamada**:
    ```c
    sem_wait(&fila_cheia);
    sem_wait(&mutex_fila);
    // Adiciona chamada à fila
    sem_post(&mutex_fila);
    sem_post(&fila_vazia);
    ```

    - **Remover Chamada**:
    ```c
    sem_wait(&fila_vazia);
    sem_wait(&mutex_fila);
    // Remove chamada da fila
    sem_post(&mutex_fila);
    sem_post(&fila_cheia);
    ```

- **Destruição de Semáforos**:
    ```c
    sem_destroy(&mutex_fila);
    sem_destroy(&fila_vazia);
    sem_destroy(&fila_cheia);
    ```

# Detalhamento das Funções

## `void adicionar_chamada(int id_usuario, int andar_origem, int andar_destino)`

Adiciona uma chamada à fila de chamadas do elevador. Verifica se a fila está cheia antes de adicionar a chamada.

## `ChamadaElevador remover_chamada()`

Remove uma chamada da fila de chamadas do elevador. Espera se a fila estiver vazia.

## `void mover_elevador(int andar_destino)`

Simula o movimento do elevador até o andar de destino. Usa `usleep` para controlar o tempo de deslocamento.

## `void* thread_elevador(void* arg)`

Função executada pela thread do elevador. Processa chamadas na fila e move o elevador conforme necessário.

## `void* thread_usuario(void* arg)`

Função executada pelas threads dos usuários. Gera chamadas de elevador aleatórias.

## `void* thread_gerador_usuarios(void* arg)`

Função executada pela thread geradora de usuários. Cria novas threads de usuários em intervalos aleatórios.

# Explicação de Semáforos e Threads

## Semáforos

Os semáforos são usados para gerenciar o acesso à fila de chamadas do elevador, garantindo que múltiplas threads possam adicionar e remover chamadas de forma segura.

## Threads

As threads são usadas para simular o comportamento do elevador e dos usuários que fazem chamadas ao elevador. A biblioteca `pthread` é utilizada para criar e gerenciar essas threads.

### Contexto do Projeto

- **Thread do Elevador**: Processa as chamadas na fila e move o elevador conforme necessário.
- **Threads dos Usuários**: Cada usuário é representado por uma thread que gera chamadas de elevador aleatórias.
- **Thread Geradora de Usuários**: Cria novas threads de usuários em intervalos aleatórios, simulando a chegada de novos usuários ao sistema.

### Referências

1. Notas de aula da disciplina: Sistemas em tempo real:
   - Kyller Costa Gorgônio. (2025). Notas de Aula da disciplina: Sistemas em tempo real. Universidade Federal de Campina Grande.
