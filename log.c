#include "headers.h"
#include "que.h"

void logg(char *token, char *file, queue *que, char **saveptr, char *curr, char *prev, char *home)
{
    if (token == NULL)
    {
        queue_out(que);
    }
    else if (strcmp(token, "purge") == 0)
    {
        clear_que(que);
        que_to_file(que, file);
    }
    else if (strcmp(token, "execute") == 0)
    {
        token = strtok_r(NULL, " ", saveptr);
        int n = atoi(token);
        int r = que->filled - n;
        int want = (r + que->front) % QUEUE_CAPACITY;
        // split_tokens(que->info[want], curr, prev, home, que, file);
        initial_tokenize(que->info[want], curr, prev, home, que, file,0);
    }
}