#ifndef methods_H
#define methods_H

void publish (char RETAIN, char *Topic, char *Message, int socketnmbr);

void subscribe (/* char DUP, unsigned int MessageID, char *SubTopic, char SubQos */);

#endif