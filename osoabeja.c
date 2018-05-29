#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define MAXDELAY 3

sem_t lleno, mutex;
int porcion = 0;
int *pote;
int num_porciones;

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
      //recipiente = recipiente + 1;
      pote[porcion] = 1;
      porcion = porcion + 1;
      if (porcion == num_porciones){
         printf("abeja %d deposita la porcion %d y avisa al oso que el pote esta lleno\n",*id,porcion);
         sleep(rand()%MAXDELAY);
         v(&lleno); // despierto al oso
         /*
         Como el proceso de la abeja no va a pasar por el else, entonces no podra entrar 
         otra abeja a llenar el recipiente
         */
      }
      else{
        printf("abeja %d deposita la porcion %d de miel\n",*id,porcion); 
        sleep(rand()%MAXDELAY);
        v(&mutex); // doy el paso a otra abeja para que llene el recipiente
        usleep(200); // duermo un tiempo muy pequeño el proceso para que otro pueda entrar
         
      }
   }

}

void *proceso_oso(void *param){
   while(1){
      p(&lleno); //el oso comienza a comer miel
      printf("oso despierta y consume recipiente con miel\n");
      porcion = 0;
      for (int i = 0; i < num_porciones; i++){
         pote[i] = 0;
      }
      sleep(rand()%MAXDELAY); // dejo un tiempo para que el oso termine de comer
      printf("oso comio toda la miel.. ahora se va a dormir\n");
      sleep(rand()%MAXDELAY); 
      v(&mutex); // el oso termina de comer y desbloquea a las abejas para que empiezen a rellenar el pote
   }

}


int main(int argc, char *argv[]){
	if (argc != 3){
		printf("uso correcto:$ %s <numero_abejas> <capacidad_recipiente>\n",argv[0]);
		exit(EXIT_FAILURE);
	}

   int num_abejas = atoi(argv[1]);
   num_porciones = atoi(argv[2]);

   printf("Numero de abejas: %d\n",num_abejas);
   printf("Capacidad pote: %d\n",num_porciones);

   srand(time(NULL));

   // inicializo el pote 
   pote = malloc(num_porciones*sizeof(*pote));
   for (int i = 0; i < num_abejas; i++){
      pote[i] = 0;
   }
   
   init(&lleno,0);
   init(&mutex,1);

   pthread_t p_abeja[num_abejas],p_oso;

   int id_abejas[num_abejas];

   printf("oso dormiendo zZzzZz\n");
   if(pthread_create(&p_oso,NULL,proceso_oso,NULL)){
      printf("Error creando thread oso\n");
      exit(EXIT_FAILURE);
   }

   for(int i = 0; i < num_abejas; i++){
      id_abejas[i] = i;
      if (pthread_create(&p_abeja[i],NULL,proceso_abeja,&id_abejas[i])){
         printf("Error creando thread abeja %d\n",i);
         exit(EXIT_FAILURE);
      }
      
   }
   
   pthread_exit(NULL);

}

