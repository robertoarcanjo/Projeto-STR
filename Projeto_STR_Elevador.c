#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <math.h>
#include <time.h>

#define NUM_ANDARES 5
#define TAMANHO_FILA 100
#define TEMPO_DESLOCAMENTO 500000 //micro segundos
#define TAXA_CHEGADA 1.25 // Controla as taxas de chegadas. Quanto maior, mais usuários são gerados. Deve ser maior que zero.


// Estrutura para representar uma chamada de elevador
typedef struct {
    int id_usuario;
    int andar_origem;
    int andar_destino;
} ChamadaElevador;

// Fila de chamadas
ChamadaElevador fila_chamadas[TAMANHO_FILA];
int tamanho_fila = 0;
int inicio_fila = 0;
int fim_fila = 0;

// Semáforos para controle de acesso à fila
sem_t mutex_fila;       // Controla o acesso à fila
sem_t fila_vazia;       // Indica se a fila está vazia
sem_t fila_cheia;       // Indica se a fila está cheia

// Posição atual do elevador
int andar_atual = 0;

// Variável global para controle de execução
bool executando = true;

// Função para tratar o sinal de interrupção (Ctrl + C)
void tratar_sinal(int sinal) {
    if (sinal == SIGINT) {
        printf("\nRecebido sinal de interrupção. Finalizando o programa...\n");
        executando = false;
    }
}

// Função para gerar um número aleatório com distribuição normal (gaussiana)
double gaussiana(double media, double desvio_padrao) {
    static int tem_numero_armazenado = 0;
    static double numero_armazenado;
    double u1, u2, w, mult;
    double x1, x2;

    if (tem_numero_armazenado) {
        tem_numero_armazenado = 0;
        return media + desvio_padrao * numero_armazenado;
    }

    do {
        u1 = -1 + ((double) rand() / RAND_MAX) * 2;
        u2 = -1 + ((double) rand() / RAND_MAX) * 2;
        w = u1 * u1 + u2 * u2;
    } while (w >= 1 || w == 0);

    mult = sqrt((-2 * log(w)) / w);
    x1 = u1 * mult;
    x2 = u2 * mult;

    tem_numero_armazenado = 1;
    numero_armazenado = x2;

    return media + desvio_padrao * x1;
}

// Função para adicionar uma chamada à fila
void adicionar_chamada(int id_usuario, int andar_origem, int andar_destino) {
    //DEGUBprintf("[Elevador] Tamanho da fila: %d\n", tamanho_fila);

    // Verifica se a fila está cheia
    int valor_fila_cheia;
    sem_getvalue(&fila_cheia, &valor_fila_cheia);
    if (valor_fila_cheia == 0) {
        //DEBUGprintf("[Elevador] Fila cheia. Usuário %d não será atendido.\n", id_usuario);
        printf("\033[0;31m[Elevador] Atenção! A fila está cheia e este usuário não será atendido.\033[0m\n");
        return;
    }

    sem_wait(&fila_cheia);  // Espera se a fila estiver cheia
    sem_wait(&mutex_fila);  // Bloqueia o acesso à fila

    // Mostrar a fila
    /*printf("[Elevador] Fila de chamadas: ");
    for (int i = inicio_fila; i != fim_fila; i = (i + 1) % TAMANHO_FILA) {
        printf("%d -> %d, ", fila_chamadas[i].andar_origem, fila_chamadas[i].andar_destino);
    }
    printf("\n");*/

    fila_chamadas[fim_fila].id_usuario = id_usuario;
    fila_chamadas[fim_fila].andar_origem = andar_origem;
    fila_chamadas[fim_fila].andar_destino = andar_destino;
    fim_fila = (fim_fila + 1) % TAMANHO_FILA;
    tamanho_fila++;

    sem_post(&mutex_fila);  // Libera o acesso à fila
    sem_post(&fila_vazia);  // Sinaliza que a fila não está mais vazia
}

// Função para remover uma chamada da fila
ChamadaElevador remover_chamada() {
    sem_wait(&fila_vazia);  // Espera se a fila estiver vazia
    sem_wait(&mutex_fila);  // Bloqueia o acesso à fila

    ChamadaElevador chamada = fila_chamadas[inicio_fila];
    inicio_fila = (inicio_fila + 1) % TAMANHO_FILA;
    tamanho_fila--;

    sem_post(&mutex_fila);  // Libera o acesso à fila
    sem_post(&fila_cheia);  // Sinaliza que a fila não está mais cheia

    return chamada;
}

// Função para simular o movimento do elevador
void mover_elevador(int andar_destino) {
    while (andar_atual != andar_destino) {
        if (andar_atual < andar_destino) {
            andar_atual++;
        } else {
            andar_atual--;
        }
        printf("[Elevador] Movendo-se para o andar %d...\n", andar_atual);
        usleep(TEMPO_DESLOCAMENTO);  // Simula o tempo de deslocamento entre os andares
    }
    printf("[Elevador] Chegou ao andar %d.\n", andar_atual);
}

// Thread do elevador
void* thread_elevador(void* arg) {
    while (executando) {
        sem_wait(&mutex_fila);  // Bloqueia o acesso à fila
        static bool mensagem_mostrada = false;
        if (tamanho_fila == 0) {
            sem_post(&mutex_fila);  // Libera o acesso à fila
            
            if (!mensagem_mostrada) {
            printf("\033[0;32m[Elevador] Nenhuma chamada pendente. Aguardando a próxima.\033[0m\n");
            mensagem_mostrada = true;
            }
        } else {
            mensagem_mostrada = false;
            sem_post(&mutex_fila);  // Libera o acesso à fila
            ChamadaElevador chamada = remover_chamada();
            printf("[Elevador] Nova chamada recebida: Usuário %d, Andar %d -> %d.\n", chamada.id_usuario, chamada.andar_origem, chamada.andar_destino);

            mover_elevador(chamada.andar_origem);
            printf("[Elevador] Usuário %d embarcou no andar %d.\n", chamada.id_usuario, chamada.andar_origem);

            mover_elevador(chamada.andar_destino);
            printf("[Elevador] Usuário %d desembarcou no andar %d.\n", chamada.id_usuario, chamada.andar_destino);
        }
    }
    printf("[Elevador] Programa finalizado.\n");
    return NULL;
}

// Thread do usuário
void* thread_usuario(void* arg) {
    int id_usuario = *((int*)arg);
    int andar_origem = rand() % NUM_ANDARES;
    int andar_destino = rand() % NUM_ANDARES;

    while (andar_destino == andar_origem) {
        andar_destino = rand() % NUM_ANDARES;
    }

    printf("\033[0;34m[Usuário %d] Chamando o elevador do andar %d para o andar %d.\033[0m\n", id_usuario, andar_origem, andar_destino);
    adicionar_chamada(id_usuario, andar_origem, andar_destino);

    return NULL;
}

void* thread_gerador_usuarios(void* arg) {
    int id_usuario = 0;
    double taxa_chegada = gaussiana(0.5 * TAXA_CHEGADA, 0.2 * TAXA_CHEGADA);  // Ajusta a média e o desvio padrão com a constante TAXA_CHEGADA
    if (taxa_chegada < 0.1 * TAXA_CHEGADA) {
        taxa_chegada = 0.1 * TAXA_CHEGADA;
    } else if (taxa_chegada > 1.0 * TAXA_CHEGADA) {
        taxa_chegada = 1.0 * TAXA_CHEGADA;
    }
    //DEBUGprintf("Taxa de chegada inicial: %.2f chamadas por segundo.\n", taxa_chegada);

    while (executando) {
        pthread_t usuario;
        pthread_create(&usuario, NULL, thread_usuario, &id_usuario);
        id_usuario++;

        double tempo_espera = 1.0 / taxa_chegada;  // Tempo de espera entre usuários

        // Garante que o tempo de espera esteja entre 1 e 10 segundos
        if (tempo_espera < 1.0) {
            tempo_espera = 1.0;
        } else if (tempo_espera > 10.0) {
            tempo_espera = 10.0;
        }
        //DEBUGprintf("Próximo usuário em %.2f segundos.\n", tempo_espera);
        sleep((int)tempo_espera);  // Converte para inteiro e espera

    
        static double tempo_decorrido = 0;
        tempo_decorrido += tempo_espera;
        if (tempo_decorrido >= (NUM_ANDARES*TEMPO_DESLOCAMENTO/1000000)) {
            taxa_chegada = gaussiana(0.5 * TAXA_CHEGADA, 0.2 * TAXA_CHEGADA);  // Ajusta a média e o desvio padrão com a constante TAXA_CHEGADA
            if (taxa_chegada < 0.1 * TAXA_CHEGADA) {
                taxa_chegada = 0.1 * TAXA_CHEGADA;
            } else if (taxa_chegada > 1.0 * TAXA_CHEGADA) {
                taxa_chegada = 1.0 * TAXA_CHEGADA;
            }
            //DEBUGprintf("Taxa de chegada atualizada: %.2f chamadas por segundo.\n", taxa_chegada);
            tempo_decorrido = 0;
        }
    }
    return NULL;
}


int main() {
    pthread_t elevador;
    pthread_t gerador_usuarios;

    // Semente para o gerador de números aleatórios
    srand(time(NULL));

    // Configura o tratamento do sinal SIGINT (Ctrl + C)
    signal(SIGINT, tratar_sinal);

    // Inicializa os semáforos
    sem_init(&mutex_fila, 0, 1);
    sem_init(&fila_vazia, 0, 0);
    sem_init(&fila_cheia, 0, TAMANHO_FILA);

    // Cria a thread do elevador
    pthread_create(&elevador, NULL, thread_elevador, NULL);

    // Cria a thread geradora de usuários
    pthread_create(&gerador_usuarios, NULL, thread_gerador_usuarios, NULL);

    // Aguarda a thread do elevador terminar
    pthread_join(elevador, NULL);

    // Destroi os semáforos
    sem_destroy(&mutex_fila);
    sem_destroy(&fila_vazia);
    sem_destroy(&fila_cheia);

    printf("Programa finalizado com sucesso.\n");
    return 0;
}
