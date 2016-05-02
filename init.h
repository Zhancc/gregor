#ifndef INIT_H
#define INIT_H

int NUM_PROCESSOR;
#define NUM_WORKER ((int)(NUM_PROCESSOR*1.4))

void init(void);
void fini(void);

#endif