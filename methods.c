#include <stdio.h>
#include <stdlib.h>

void publish (char RETAIN, char *Topic, char *Message, int socketnmbr) {
    printf ("Estou na função publish\n");
    printf ("Cliente está tentando publicar\n");
    RETAIN = 'A';
    if (RETAIN == 'B')
        printf ("B");
    printf ("Número do socket: %d\n", socketnmbr);
    printf ("Tópico a publicar: %s\n", Topic);
    printf ("Mensagem a publicar: %s\n", Message);
}

void subscribe (/* char DUP, unsigned int MessageID, char *SubTopic, char SubQos */) {
    printf ("Estou na função subscribe\n");
    printf ("Cliente está tentando inscrever-se\n");
}
