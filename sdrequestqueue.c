/*
 sdrequestqueue.c
*/

#include "sdrequestqueue.h"
#include "sdutil.h"

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

typedef struct SDNode *SDNodeRef;
struct SDNode {
    SDNodeRef next;
    int socket;
};

SDNodeRef sdNodeAlloc(int socket) {
    SDNodeRef node = malloc(sizeof(struct SDNode));
    node->socket = socket;
    node->next = NULL;
    return node;
}

void sdNodeDestroy(SDNodeRef *node) {
    assert(node != NULL);
    assert(*node != NULL);
    free(*node);
    *node = NULL;
}

struct SDRequestQueue {
    SDNodeRef front;
    SDNodeRef back;
    int length;
    pthread_mutex_t mutex;
    pthread_cond_t nonempty;
};

SDRequestQueueRef sdRequestQueueAlloc() {
    SDRequestQueueRef queue = malloc(sizeof(struct SDRequestQueue));
    queue->length = 0;
    queue->front = NULL;
    queue->back = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->nonempty, NULL);
    return queue;
}

void sdRequestQueueDestroy(SDRequestQueueRef *queue) {
    assert(queue != NULL);
    assert(*queue != NULL);
    pthread_mutex_destroy(&(*queue)->mutex);
    pthread_cond_destroy(&(*queue)->nonempty);
    free(*queue);
}

void sdRequestQueuePut(SDRequestQueueRef queue, int socket) {
    assert(queue != NULL);
    pthread_mutex_lock(&queue->mutex);

    SDNodeRef node = sdNodeAlloc(socket);
    if (queue->front == NULL) {
        queue->front = node;
    } else {
        queue->back->next = node;
    }
    queue->back = node;
    queue->length++;
    SDLOG("Connection Queue %p: Length = %d", queue, queue->length);
    pthread_cond_broadcast(&queue->nonempty);
    pthread_mutex_unlock(&queue->mutex);
}

/*
 TODO This is really messy and needs to be cleaned up
*/
int sdRequestQueueGet(SDRequestQueueRef queue) {
    assert(queue != NULL);
    pthread_mutex_lock(&queue->mutex);
    int socket;
    if (queue->length >= 1) {
        SDNodeRef node = queue->front;
        socket = node->socket;
        queue->front = node->next;
        queue->length -= 1;
        sdNodeDestroy(&node);
    } else {
        pthread_cond_wait(&queue->nonempty, &queue->mutex);
        if (queue->length >= 1) {
            SDNodeRef node = queue->front;
            socket = node->socket;
            queue->front = node->next;
            queue->length -= 1;
            sdNodeDestroy(&node);
        } else {
            socket = -1;
        }
    }
    SDLOG("Connection Queue %p: Length = %d", queue, queue->length);
    pthread_mutex_unlock(&queue->mutex);
    return socket;
}
