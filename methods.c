#include <stdio.h>
#include <stdlib.h>

void publish (/* char DUP, char Qos, char RETAIN, unsigned int MessageID, char *Topic, char *Message */) {
    printf ("Estou na função publish\n");
    printf ("Cliente está tentando publicar\n");
}

void subscribe (/* char DUP, unsigned int MessageID, char *SubTopic, char SubQos */) {
    printf ("Estou na função subscribe\n");
    printf ("Cliente está tentando inscrever-se\n");
}
