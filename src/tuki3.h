#include "giocatore.h"
#define LARGHEZZA_T 140
#define ALTEZZA_T 30
#define LUNGHEZZA_CAMMINO 512 //Lunghezza totale del cammino
#define LUNGHEZZA_CAMMINO_VISIBILE 140 //Porzione visibile del cammino
#define Y_CAMMINO 15 
#define NUMERO_OGGETTI 10
#define GDELAY 40000
#define TUKI_DIM 8

#define GEMMA_TOX 0.1
#define POZIONE_TOX 0.7
#define MELANZANA_TOX 0.05
#define GEMMA_NTRL 0.2
#define POZIONE_NTRL 0.2
#define MELANZANA_NTRL 0.75
#define GEMMA_NRGY 0.7
#define POZIONE_NRGY 0.1
#define MELANZANA_NRGY 0.2
#define X0_TUKI 10

typedef struct{
  float nutrienti;
  float tossine;
  float energia;
  tipo_oggetto t;
  int pos_x;
  char pxl[DIM*DIM];
  char exists;
}oggetto;

typedef struct  {
  int id; 
  int pos_x; 
  int pos_xold;
  int pos_yold; 
  int pos_y; 
  int score;
  char name[10];
  char pxl[11][TUKI_DIM*TUKI_DIM];
  stato stato_giocatore;
 } Giocatore;
