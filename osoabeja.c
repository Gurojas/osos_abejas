#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t lleno, vacio, mutex;
int recipiente = 0;
int in = 0;
int h;

void init(sem_t *sem, int valor_inicial) {
   if (sem_init(sem, 0, valor_inicial)) {
      printf("Error inicializando semaforo\n");
      exit(EXIT_FAILURE);
   }
}

//función encargada de bloquear un semáforo
//si ya estaba bloqueado, se queda a la espera de ser liberado para bloquearlo
void p(sem_t *sem) {
   if (sem_wait(sem)) {
      printf("Error fatal haciendo sem_wait()\n");
      exit(EXIT_FAILURE);
   }
}

//función encargada de liberar un semáforo
//en caso que el semáforo ya esté liberado, 
void v(sem_t *sem) {
   if (sem_post(sem)) {
      printf("Error fatal haciendo sem_post()\n");
      exit(EXIT_FAILURE);
   }
}

void *proceso_abeja(void *param){
   int *id = (int*)param;
   while(1){
      p(&mutex); // bloqueo para que UNA abeja llene el recipiente
      printf("abeja %d deposita miel\n",*id);
      recipiente = recipiente + 1;
      if (recipiente == h){
         sleep(rand()%5);
         v(&lleno); // despierto al oso
         /*
         Como el proceso de la abeja no va a pasar por el else, entonces no podra entrar 
         otra abeja a llenar el recipiente
         */
      }
      else{ 
        sleep(rand()%5);
        v(&mutex); // doy el paso a otra abeja para que llene el recipiente
         
      }
   }

}

void *proceso_oso(void *param){

   while(1){
      p(&lleno); //el oso comienza a comer miel
      recipiente = 0;
      printf("oso consumiendo recipiente con miel\n");
      sleep(rand()%5); 
      v(&mutex); // el oso termina de comer y desbloquea a las abejas para que empiezen a rellenar el pote
   }

}


int main(int argc, char *argv[]){
	if (argc != 3){
		printf("uso correcto:$ %s <numero_abejas> <capacidad_recipiente>\n",argv[0]);
		exit(EXIT_FAILURE);
	}

   int n = atoi(argv[1]);
   h = atoi(argv[2]);

   printf("Numero de abejas: %d\n",n);
   printf("Capacidad pote: %d\n",h);

   srand(time(NULL));

   
   init(&lleno,0);
   // semaforo inicializado con el numero de abejas (procesos) que acceden al recurso compartido 
   init(&mutex,1);

   pthread_t p_abeja[n],p_oso;
   pthread_attr_t attr;
   pthread_attr_init(&attr);

   int id_abejas[n];

   if(pthread_create(&p_oso,NULL,proceso_oso,NULL)){
      printf("Error creando thread oso\n");
      exit(EXIT_FAILURE);
   }

   for(int i = 0; i < n; i++){
      id_abejas[i] = i;
      if (pthread_create(&p_abeja[i],NULL,proceso_abeja,&id_abejas[i])){
         printf("Error creando thread abeja %d\n",i);
         exit(EXIT_FAILURE);
      }
      
   }
   
   
   

   pthread_exit(NULL);

}

