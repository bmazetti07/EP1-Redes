#ifndef AUX_H
#define AUX_H

#define MAXTOPICS 1024

char *topicos[MAXTOPICS];

void toBit (char c, int v[]);

int calculaBit (int byte[]);

int procuraTopico (char * topico, char *vetor[]);

void adicionaTopico (char *topico, char *vetor[]);


#endif