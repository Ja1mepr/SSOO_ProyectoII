#include<stdio.h>
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

int main()
{
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
     ...
     
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
            Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
            continue;
         }else if(strcmp(orden,"rename")==0){
            //Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo);
            continue;
         }
         ...
         // Escritura de metadatos en comandos rename, remove, copy     
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
         }
     }
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
   printf("Bloque %d Bytes", psup->s_block_size);
   printf("Inodos particion = %d", psup->s_inodes_count);
   printf("Inodos libres = ", psup->s_free_inodes_count);
   printf("Bloques particion = %d", psup->s_blocks_count);
   printf("Bloques libres = ", psup->s_free_blocks_count);
   printf("Primer bloque de datos = %d", psup->s_first_data_block);
}

//Funcion que comprueba que exista el fichero. Si existe devuelve 1, en caso contrario devuelve 0
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){
   int b=0;
   for(int i=0; i<MAX_FICHEROS; i++){
      if(strcmp(directorio[i].dir_nfich, nombre)==0){
         b=1;
      }
   }
   return b;
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

   int b=1;

   *orden=*strcomando;
   if(orden!="info" || orden!="bytemaps" || orden!="dir" || orden!="rename" || orden!="imprimir" || orden!="remove" || orden!="copy" || orden!="salir"){
      b=0;
      printf("El comando introducido no existe\n");
   }
   else if((orden=="rename" || orden=="copy") && (argumento1==NULL || argumento2==NULL)){
      b=0;
      printf("El comando no ha recibido los parametros correspondientes");
   }
   else if((orden=="imprimir" || orden=="remove") && argumento1==NULL){
      b=0;
      printf("El comando no ha recibido los parametros correspondientes");
   }
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
               printf("%d ", inodos->blq_inodos[directorio[i].dir_inodo[j]]);
            }
         }
      }
   }
   printf("\n");
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){

}