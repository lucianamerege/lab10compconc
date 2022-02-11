#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <semaphore.h>

#define N 5 //tamanho do buffer
int P, C; //qtde de threads produtoras e consumidoras respectivamente

//variaveis do problema
int Buffer[N]; //espaco de dados compartilhados
int cheio = 0;
int out=0; //variaveis de estado
sem_t emp, emc;     // semaforo para exclusao mutua entre as threads produtoras e consumidoras respectivamente
sem_t condp, condc;

//inicializa o buffer
void IniciaBuffer(int n) {
    for(int i=0; i<n; i++)
        Buffer[i] = 0;
}

//imprime o buffer
void ImprimeBuffer(int n) {

    for(int i=0; i<n; i++)
        printf("%d ", Buffer[i]);
    printf("\n");
}

//insere elementos no Buffer ou bloqueia a thread caso o Buffer tenha algum elemento
void Insere (int item, int id) {
    sem_wait(&emp); //entrada na secao critica

    while(cheio!=0){
        sem_wait(&condc);
    }

    printf("P[%d] quer inserir\n", id);
    for(int i=0; i<N; i++)
        Buffer[i] = item+i;
    
    printf("P[%d] inseriu\n", id);
    cheio = N;
    ImprimeBuffer(N);
    sem_post(&condp);
    sem_post(&emp);//saida da secao critica
}

//retira um elemento no Buffer ou bloqueia a thread caso o Buffer esteja vazio
int Retira (int id) {
    int item;
    sem_wait(&emc); //entrada na secao critica
    printf("C[%d] quer consumir\n", id);
    while(cheio == 0) {
        printf("C[%d] bloqueou\n", id);
        sem_post(&condc);
        sem_wait(&condp); //espera o sinal da produtora de que pode voltar a retirar elementos do buffer
        printf("C[%d] desbloqueou\n", id);
    }
    item = Buffer[out];
    Buffer[out] = 0;
    out = (out + 1)%N;
    cheio--;
    printf("C[%d] consumiu %d\n", id, item);
    ImprimeBuffer(N);
    sem_post(&emc);//saida da secao critica
    return item;
}

//thread produtora
void * produtor(void * arg) {
    int *id = (int *) arg;
    printf("Sou a thread produtora %d\n", *id);
    while(1) {
        //produzindo o item
        Insere(*id, *id);
        sleep(1);
    } 
    free(arg);
    pthread_exit(NULL);
}

//thread consumidora
void * consumidor(void * arg) {
    int *id = (int *) arg;
    int item;
    printf("Sou a thread consumidora %d\n", *id);
    while(1) {
        item = Retira(*id);
        sleep(1); //faz o processamento do item 
    } 
    free(arg);
    pthread_exit(NULL);
}


//funcao principal
int main(int argc, char *argv[]) {

    if(argc<3) {
        printf("Digite: %s <numero de produtoras> <numero de consumidoras>\n", argv[0]);
        return 1;
    }
    P = atoi(argv[1]);
    C = atoi(argv[2]);

    sem_init(&emp, 0, 1);
    sem_init(&emc, 0, 1);
    sem_init(&condp, 0, 0);
    sem_init(&condc, 0, 0);

    //variaveis auxiliares
    int i;
    
    //identificadores das threads
    pthread_t tid[P+C];
    int *id[P+C];

    //aloca espaco para os IDs das threads
    for(i=0; i<P+C;i++) {
        id[i] = malloc(sizeof(int));
        if(id[i] == NULL) exit(-1);
        *id[i] = i+1;
    }

    //inicializa o Buffer
    IniciaBuffer(N);  


    //cria as threads produtoras
    for(i=0; i<P; i++) {
        if(pthread_create(&tid[i], NULL, produtor, (void *) id[i])) exit(-1);
    } 
    
    //cria as threads consumidoras
    for(i=0; i<C; i++) {
        if(pthread_create(&tid[i+P], NULL, consumidor, (void *) id[i+P])) exit(-1);
    } 

    pthread_exit(NULL);
    return 1;
}
