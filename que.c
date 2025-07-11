#include "que.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int is_que_full(queue *que)
{
    return (que->filled == QUEUE_CAPACITY);
}

void init_que(queue *que)
{
    que->front = -1;
    que->rear = -1;
    que->filled = 0;
}

void enque(queue *que, char *entry)
{
    if (is_que_full(que) == 1)
    {
        printf("que is full");
        return;
    }
    que->filled++;
    if (que->front == -1)
    {
        que->front = 0;
        que->rear = 0;
    }
    else
    {
        que->rear = (que->rear + 1) % 15;
    }
    strcpy(que->info[que->rear], entry);
}

void deque(queue *que)
{
    if (que->front == -1)
    {
        printf("Queue is empty\n");
        return;
    }

    if (que->front == que->rear)
    {
        que->front = -1;
        que->rear = -1;
    }
    else
    {
        que->front = (que->front + 1) % QUEUE_CAPACITY;
    }

    que->filled--;
}

int size(queue *que)
{
    return que->filled;
}

void que_to_file(queue *que, char *file_name)
{
    FILE *f = fopen(file_name, "w");
    if (f == NULL)
    {
        
        perror("unable to open file");
        return;
    }

    int i = que->front;
    while (1)
    {
        if (fprintf(f, "%s\n", que->info[i]) < 0)
        {
            perror("error writing to file");
            break;
        }
        fflush(f); 
        if (i == que->rear)
        {
            break;
        }
        i = (i + 1) % QUEUE_CAPACITY;
    }

    fclose(f); 
}

void queue_out(queue *que)
{
    if (que->filled == 0)
    {
        printf("Queue is empty\n");
        return;
    }

    int i = que->front;
    while (1)
    {
        printf("%s\n", que->info[i]);
        if (i == que->rear)
            break;

        i = (i + 1) % QUEUE_CAPACITY;
    }
}

void file_to_que(queue *que, char *file_name)
{
    FILE *f = fopen(file_name, "r");
    if (f == NULL)
    {
        
        return;
    }

    char buffer[1024];
    init_que(que);

    while (fgets(buffer, sizeof(buffer), f) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = '\0';
        enque(que, buffer);
        fflush(f);
    }

    if (ferror(f))
    {
        perror("error reading from file");
    }

    fclose(f);
}

void print_que(queue *que)
{
    int n = size(que);
    int i = que->front;
    while (1)
    {
        printf("%s\n", que->info[i]);
        if (i == que->rear)
        {
            break;
        }
        i = (i + 1) % QUEUE_CAPACITY;
    }
}

void clear_que(queue *que)
{
    que->front = -1;
    que->rear = -1;
    que->filled = 0;
}