#ifndef AUX_H
#define AUX_H

#define MAXTOPICS 1024

char *topicos[MAXTOPICS];

void toBit (char c, int v[]);

int calculaBit (int byte[], int limite);

int procuraTopico (char * topico, char *vetor[]);

int adicionaTopico (char *topico, char *vetor[]);


#endif