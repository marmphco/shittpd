/*
 sdcqueue.c
*/

#include "sdcqueue.h"
#include "sdutil.h"

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

static const char *SDLOG_NAME = "Connection Queue";

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

struct SDConnectionQueue {
    SDNodeRef front;
    SDNodeRef back;
    int length;
    pthread_mutex_t mutex;
    pthread_cond_t nonempty;
};

SDConnectionQueueRef sdConnectionQueueAlloc() {
    SDConnectionQueueRef queue = malloc(sizeof(struct SDConnectionQueue));
    queue->length = 0;
    queue->front = NULL;
    queue->back = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->nonempty, NULL);
    return queue;
}

void sdConnectionQueueDestroy(SDConnectionQueueRef *queue) {
    assert(queue != NULL);
    assert(*queue != NULL);
    pthread_mutex_destroy(&(*queue)->mutex);
    pthread_cond_destroy(&(*queue)->nonempty);
    free(*queue);
    *queue = NULL;
}

void sdConnectionQueuePut(SDConnectionQueueRef queue, int socket) {
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
    SDLOG(" %p: Length = %d", queue, queue->length);
    pthread_cond_broadcast(&queue->nonempty);
    pthread_mutex_unlock(&queue->mutex);
}

int sdConnectionQueueGet(SDConnectionQueueRef queue) {
    assert(queue != NULL);
    int socket;
    if (queue->length >= 1) {
        SDNodeRef node = queue->front;
        socket = node->socket;
        queue->front = node->next;
        queue->length -= 1;
        sdNodeDestroy(&node);
    } else {
        socket = -1;
    }
    return socket;
}

int sdConnectionQueueWaitGet(SDConnectionQueueRef queue) {
    /*assert(queue != NULL);

    pthread_mutex_lock(&queue->mutex);
    int socket = sdConnectionQueueGet(queue);
    if (socket == -1) {
        pthread_cond_wait(&queue->nonempty, &queue->mutex);
        socket = sdConnectionQueueGet(queue);
    }
    SDLOG(" %p: Length = %d", queue, queue->length);
    pthread_mutex_unlock(&queue->mutex);
    return socket;*/
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
    SDLOG(" %p: Length = %d", queue, queue->length);
    pthread_mutex_unlock(&queue->mutex);
    return socket;
}
