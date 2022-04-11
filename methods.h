#ifndef methods_H
#define methods_H

void publish (/* char DUP, char Qos, char RETAIN, unsigned int MessageID, char *Topic, char *Message */);

void subscribe (/* char DUP, unsigned int MessageID, char *SubTopic, char SubQos */);

#endif