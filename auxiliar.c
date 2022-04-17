#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

static int nextTopic = 0;

void toBit (char c, int v[]) {
    int i;
    for (i = 0; i < 8; i ++){
        v[i] = !!((c << i) & 0x80);
    }
}

int calculaBit (int byte[]) {
    int x = 0, i;
    for (i = 0; i < 8; i ++) 
        x += byte[i] * pow (2, 8 - i - 1);
    
    return x;
}

void adicionaTopico (char *topico, char *vetor[]) {
    vetor[nextTopic++] = topico;
}

int procuraTopico (char * topico, char *vetor[]) {
    int i;

    for (i = 0; i < nextTopic; i ++) 
        if (strcmp (topico, vetor[i]) == 0)
            return i;

    adicionaTopico (topico, vetor);

    return -1;
}