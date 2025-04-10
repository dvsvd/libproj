#include "buf_queue.h"
#include "memfcn.h"

extern malloc_t real_malloc;
extern free_t real_free;
extern realloc_t real_realloc;

int buf_queue_init(buf_queue_t *q, int pshared) {
    int ret;
    pthread_condattr_t ca;
    pthread_mutexattr_t ma;
    if(q == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    q->front = NULL;
    q->rear = NULL;
    if( (ret = pthread_condattr_init(&ca)) ||
        (ret = pthread_mutexattr_init(&ma)) ||
        (ret = pthread_condattr_setpshared(&ca, pshared)) ||
        (ret = pthread_mutexattr_setpshared(&ma, pshared)) ||
        (ret = pthread_mutex_init(&q->lock, &ma)) ||
        (ret = pthread_cond_init(&q->cond, &ca)) ||
        (ret = pthread_condattr_destroy(&ca)) ||
        (ret = pthread_mutexattr_destroy(&ma))
    )
    {
        errno = ret;
        return -1;
    }
    return 0;
}

int push(buf_queue_t *q, shared_buf_t* data) {
    node_t* newnode;
    int ret;
    if(q == NULL || data == NULL)
    {
        errno = EINVAL;
        return -1;
    }
    newnode = (node_t*)real_malloc(sizeof(node_t));
    newnode->data = data;
    newnode->next = NULL;
    ret = pthread_mutex_lock(&q->lock);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    if (q->rear == NULL)
    {
        q->front = newnode;
        q->rear = newnode;
    }
    else
    {
        q->rear->next = newnode;
        q->rear = newnode;
    }
    if((ret = pthread_cond_signal(&q->cond)) || 
       (ret = pthread_mutex_unlock(&q->lock))
    )
    {
        errno = ret;
        return -1;
    }
}

shared_buf_t* pop(buf_queue_t *q) {
    int ret;
    if(q == NULL)
    {
        errno = EINVAL;
        return NULL;
    }
    ret = pthread_mutex_lock(&q->lock);
    if(ret)
    {
        errno = ret;
        return NULL;
    }
    while (q->front == NULL) {
        pthread_cond_wait(&q->cond, &q->lock);
    }

    node_t* temp = q->front;
    shared_buf_t* data = temp->data;
    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }

    free(temp);
    ret = pthread_mutex_unlock(&q->lock);
    if(ret)
    {
        errno = ret;
        return NULL;
    }
    return data;
}

int buf_queue_destroy(buf_queue_t *q) {
    int ret;
    ret = pthread_mutex_lock(&q->lock);
    if(ret)
    {
        errno = ret;
        return -1;
    }
    node_t *current = q->front;
    while (current != NULL) {
        node_t *temp = current;
        current = current->next;
        free(temp);
    }
    q->front = NULL;
    q->rear = NULL;
    if((ret = pthread_mutex_unlock(&q->lock)) ||
    (ret = pthread_mutex_destroy(&q->lock)) ||
    (ret = pthread_cond_destroy(&q->cond))
    )
    {
        errno = ret;
        return -1;
    }
    return 0;
}
