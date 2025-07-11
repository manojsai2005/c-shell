#ifndef QUE_H
#define QUE_H

#define MAX_COMMAND_SIZE 1024
#define QUEUE_CAPACITY 15

typedef struct {
    char info[QUEUE_CAPACITY][MAX_COMMAND_SIZE];
    int front;
    int rear;
    int filled;
} queue;

void init_que(queue *que);
void enque(queue *que, char *entry);
void deque(queue *que);
int size(queue *que);
void que_to_file(queue *que, char *file_name);
void queue_out(queue *que);
void file_to_que(queue *que, char *file_name);
int is_que_full(queue *que);
void print_que(queue* que);
void clear_que(queue *que);

#endif
