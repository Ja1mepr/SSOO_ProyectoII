#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main(){
	 char *comando[LONGITUD_COMANDO];
	 char *orden[LONGITUD_COMANDO];
	 char *argumento1[LONGITUD_COMANDO];
	 char *argumento2[LONGITUD_COMANDO];
	 
	 int i,j;
	 unsigned long int m;
     EXT_SIMPLE_SUPERBLOCK ext_superblock;
     EXT_BYTE_MAPS ext_bytemaps;
     EXT_BLQ_INODOS ext_blq_inodos;
     EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
     EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
     int entradadir;
     int grabardatos;
     FILE *fent;
     
     // Lectura del fichero completo de una sola vez
     fent = fopen("particion.bin","r+b");
     fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
     
     
     memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
     memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
     memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
     memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
     memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
     // Buce de tratamiento de comandos
     for (;;){
		 do {
         strcpy(argumento1, "");
         strcpy(argumento2, "");
         printf (">> ");
         fflush(stdin);
         fgets(comando, LONGITUD_COMANDO, stdin);
		 } while (ComprobarComando(comando,orden,argumento1,argumento2)!=0);
	      if (strcmp(orden,"dir")==0) {
            Directorio(&directorio,&ext_blq_inodos);
            continue;
         }else if(strcmp(orden,"info")==0){
            LeeSuperBloque(&ext_superblock);
            continue;
         }else if(strcmp(orden,"bytemaps")==0){
            Printbytemaps(&ext_bytemaps);
            continue;
         }else if(strcmp(orden,"rename")==0){
            Renombrar(&directorio, &ext_blq_inodos, argumento1, argumento2);
            continue;
         }else if(strcmp(orden, "imprimir")==0){
            Imprimir(&directorio, &ext_blq_inodos, datosfich, argumento1);
            continue;
         }else if(strcmp(orden, "borrar")==0){
            Borrar(&directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1,  fent);
            continue;
         }else if(strcmp(orden, "copiar")==0){
            inCopiar(&directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, datosfich, argumento1, argumento2, fent);
            continue;
         }
         /*// Escritura de metadatos en comandos rename, remove, copy     
         Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
         GrabarByteMaps(&ext_bytemaps,fent);
         GrabarSuperBloque(&ext_superblock,fent);
         if (grabardatos)
           GrabarDatos(&memdatos,fent);
         grabardatos = 0;
         //Si el comando es salir se habrán escrito todos los metadatos
         //faltan los datos y cerrar
         if (strcmp(orden,"salir")==0){
            GrabarDatos(&memdatos,fent);
            fclose(fent);
            return 0;
         }*/
     }
}

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps){
   
   printf("\nInodos: ");
   for(int i=0; i<MAX_INODOS; i++){
      printf("%c ", ext_bytemaps->bmap_inodos[i]);
   }
   printf("\nBloques |0-25|: ");
   for(int i=0; i<25; i++){
      printf("%d ", ext_bytemaps->bmap_bloques[i]);
   }
}

//Comprobamos que el comando exista y en caso de existir que se le pasen los parametros necesarios para su funcionamiento
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2){

   int b=-1;
   //Utilizamos strtok para separar la cadena por espacios y obtener orden y los argumentos 
   char *token=strtok(strcomando, " ");
   if(token!=NULL){
      strcpy(orden, token);
      token=strtok(NULL, " ");
      b=0;
      if(token!=NULL){
         strcpy(argumento1, token);
         token=strtok(NULL, " ");
         if(token!=NULL)
            strcpy(argumento2, token);
      }
   }else
      printf("ERROR!! Los argumentos son invalidos\n");

   if(orden!="info" || orden!="bytemaps" || orden!="dir" || orden!="rename" || orden!="imprimir" || orden!="remove" || orden!="copy" || orden!="salir")
      printf("ERROR!! El comando introducido no existe\n");
   else if((orden=="rename" || orden=="copy") && (argumento1==NULL || argumento2==NULL))
      printf("ERROR!! El comando no ha recibido los parametros correspondientes\n");
   else if((orden=="imprimir" || orden=="remove") && argumento1==NULL)
      printf("ERROR!! El comando no ha recibido los parametros correspondientes\n");
   else
      b=0;
   return b;
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
   printf("Bloque %d Bytes", psup->s_block_size);
   printf("Inodos particion = %d", psup->s_inodes_count);
   printf("Inodos libres = %d", psup->s_free_inodes_count);
   printf("Bloques particion = %d", psup->s_blocks_count);
   printf("Bloques libres = %d", psup->s_free_blocks_count);
   printf("Primer bloque de datos = %d", psup->s_first_data_block);
}

//Funcion que comprueba que exista el fichero. Si existe devuelve 1, en caso contrario devuelve 0
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){

   int b=-1;

   for(int i=0; i<MAX_FICHEROS; i++){
      if(strcmp(directorio[i].dir_nfich, nombre)==0){
         b=0;
      }
   }
   if(b==-1)
      printf("ERROR!! El fichero %s no existe\n", nombre);
   return b;
}

//Mostramos por consola los ficheros(nombre, tamaño, inodos y bloques que ocupa)
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){
   for(int i=0; i<MAX_FICHEROS; i++){
      if(directorio[i].dir_inodo!=NULL_INODO){
         printf("%s\ttamano:%d\tinodo:%d\tbloques:", directorio[i].dir_nfich, inodos->blq_inodos[directorio[i].dir_inodo].size_fichero, 
                     directorio[i].dir_inodo);
         for(int j=0;MAX_NUMS_BLOQUE_INODO; j++){
            if(inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]!=NULL_BLOQUE){
               printf("%d ", inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]);
            }
         }
      }
   }
   printf("\n");
}

//Funcion para cambiar el nombre del fichero
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){

   int b=-1;

   if((strcmp(nombreantiguo, "")==0) || (strcmp(nombrenuevo, "")==0))//Comprobamos que nombreantiguo y nombrenuevo no sean cadenas vacias
      printf("ERROR!! Los argumentos introducidos no son correctos");
   else{
      if((BuscaFich(directorio, inodos, nombreantiguo)==0) && (BuscaFich(directorio, inodos, nombrenuevo)==-1)){//Comprobamos que nombreantiguo sea un fichero existente y nombrenuevo no lo sea
         for(int i=0; i<MAX_FICHEROS; i++){
            if(strcmp(directorio[i].dir_nfich, nombreantiguo)==0)//Si el fichero tiene se llama como nombreantiguo le cambiamos el nombre por nombre nuevo
               strcpy(directorio[i].dir_nfich, nombrenuevo);
         }
         b=0;
      }else if((BuscaFich(directorio, inodos, nombreantiguo)==-1) && (BuscaFich(directorio, inodos, nombrenuevo)==0)){//Comprobamos si alguno de los nombres da algun tipo de error y lo indicamos
         printf("ERROR!! El fichero %s no existe\n", nombreantiguo);
         printf("%s ya es esta asociado a otro fichero\n", nombrenuevo);
      }else if(BuscaFich(directorio, inodos, nombreantiguo)==-1)
         printf("ERROR!! El fichero %s no existe\n", nombreantiguo);
      else
         printf("ERROR!! %s ya es esta asociado a otro fichero\n", nombrenuevo);
   }
   return b;
}

//Funcion para imprimi el contenido del fichero
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre){
   
   int b=0;
   int j=0;
   char *contFich=(char *)calloc(0, sizeof(char));

   if(strcmp(nombre, "")==0){//Comprobamos que nombre no sea una cadena vacia
      printf("ERROR!! El argumento introducido no es correcto\n");
      b=-1;
   }else{
      if(BuscaFich(directorio, inodos, nombre)!=0){//Comprobamos que el nombre este asignado a un fichero existente
         printf("ERROR!! El fichero %s no existe\n", nombre);
         b=-1;
      }else{
         for(int i=0; i<MAX_FICHEROS; i++){
            if(strcmp(directorio[i].dir_nfich, nombre)==0){
               while(inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]!=NULL_INODO){//Recorremos los bloques mientras no sean nulos
                  contFich=(char*)realloc(contFich, (j+1)*SIZE_BLOQUE);//Aumentamos el espacio un bloque
                  strcat(contFich, memdatos[inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]].dato);//Concatenamos el nuevo bloque
                  j++;
               }
            }
         }
         contFich[strlen(contFich-1)]="\0";
         printf("%s", contFich);
         printf("\n");
         free(contFich);
      }
   }
   return b;
}

//Funcion para eliminar un fichero del directorio
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich){
   
   int b=0;

   if(fich==NULL || strcmp(nombre, "")==0){
      b=-1;
      printf("ERROR!! El argumento introducido no es correcto\n");
   }else{
      if(BuscaFich(directorio, inodos, nombre)==-1){
         b=-1;
         printf("ERROR!! El fichero introducido no existe\n");
      }else{
         for(int i=0; i<MAX_FICHEROS; i++){
            if(strcmp(directorio[i].dir_nfich, nombre)==0){
               for(int j=0; i<MAX_NUMS_BLOQUE_INODO; j++){
                  if(inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]!=NULL_INODO){
                     ext_bytemaps->bmap_bloques[inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]]=0;//Damos valor 0 a los inodos liberados
                     inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]=NULL_BLOQUE;//Para cada bloque damos el valor FFFH(NULL_BLOQUE)
                     ext_superblock->s_free_blocks_count++;//Aumentamos la cuenta de bloques libres
                  }
               }
               ext_bytemaps->bmap_inodos[directorio[i].dir_inodo]=0;//Indicamos en el bytemap que queda liberado
               ext_superblock->s_free_inodes_count++;//Aumentamos la cuenta de inodos libres libres

               strcpy(directorio[i].dir_nfich, "");//Ponemos un string vacio en el nombre
               directorio[i].dir_inodo = NULL_INODO;//Damos el valor FFFH(NULL_INODO) en el numero del inodo
            }
         }
      }
   }
   return b;
}

//Funcion para copiar el contenido de un fichero en uno nuevo
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich) {
    int b = -1;

    if (fich == NULL || strcmp(nombreorigen, "") == 0 || strcmp(nombredestino, "") == 0) {
        printf("ERROR!! Los argumentos introducidos no son correctos\n");
    } else {
        if (BuscaFich(directorio, inodos, nombreorigen) == -1 || BuscaFich(directorio, inodos, nombredestino) == -1) {
            printf("ERROR!! El fichero introducido no existe\n");
        } else {
            int bloquesnecesarios = 0;

            for (int i = 1; i < MAX_FICHEROS; i++) {// Bucle para encontrar el fichero
                if (strcmp(nombreorigen, directorio[i].dir_nfich) == 0) {
                    
                    for (int j = 1; j < MAX_FICHEROS; j++) {// Bucle para encontrar un inodo libre
                        if (ext_bytemaps->bmap_inodos[j] == 0) {
                            ext_bytemaps->bmap_inodos[j] = 1;
                            ext_superblock->s_free_inodes_count--;

                            for (int k = 0; k < MAX_FICHEROS; k++) {// Bucle para encontrar una entrada del directorio libre
                                if (directorio[k].dir_inodo == NULL_INODO) {
                                    directorio[k].dir_inodo = j;
                                    inodos->blq_inodos[directorio[k].dir_inodo].size_fichero = inodos->blq_inodos[directorio[i].dir_inodo].size_fichero;

                                    for (int t = 0; t < MAX_NUMS_BLOQUE_INODO; t++) {// Contamos bloques necesarios
                                        if (inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[t] != NULL_BLOQUE)
                                            bloquesnecesarios++;
                                    }

                                    strcpy(directorio[k].dir_nfich, nombredestino);// Actualizamos entrada de directorio

                                    if (ext_superblock->s_free_blocks_count < bloquesnecesarios) {// Comprobamos el espacio libre
                                        printf("ERROR: No hay suficientes bloques libres para copiar el fichero\n");
                                        ext_bytemaps->bmap_inodos[j] = 0;
                                        ext_superblock->s_free_inodes_count++;
                                        directorio[k].dir_inodo = NULL_INODO;
                                        break;
                                    } else {
                                        b = 0;

                                        for (int p = 0; p < MAX_NUMS_BLOQUE_INODO; p++) {// Copiamos los bloques de datos
                                            if (inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[p] != NULL_BLOQUE) {
                                                for (int l = 0; l < MAX_BLOQUES_PARTICION; l++) {
                                                    if (ext_bytemaps->bmap_bloques[l] == 0) {
                                                        ext_bytemaps->bmap_bloques[l] = 1;
                                                        ext_superblock->s_free_blocks_count--;
                                                        inodos->blq_inodos[directorio[k].dir_inodo].i_nbloque[p] = l;
                                                        memcpy(memdatos[inodos->blq_inodos[directorio[k].dir_inodo].i_nbloque[p]].dato, memdatos[inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[p]].dato, SIZE_BLOQUE);
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    return b;
}

