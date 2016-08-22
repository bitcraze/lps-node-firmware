#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct queue_s {
  char *buffer;
  uint32_t head;
  uint32_t tail;
  uint32_t length;
} queue_t;

inline static bool queueIsEmpty(queue_t *queue)
{
  return queue->head == queue->tail;
}

inline static bool queueIsFull(queue_t *queue)
{
  uint32_t nhead = (queue->head+1)%queue->length;
  return nhead == queue->tail;
}

inline static uint32_t queueGetFreeSpace(queue_t *queue) {
  int freeSpace = queue->tail - queue->head;
  if (freeSpace == 0) {
    freeSpace = queue->length;
  } else if (freeSpace < 0) {
    freeSpace = queue->length + freeSpace;
  }

  return freeSpace - 1;
}

inline static void queuePush(queue_t *queue, char data)
{
  if (!queueIsFull(queue)) {
    queue->buffer[queue->head] = data;
    queue->head = (queue->head+1)%queue->length;
  }
}

inline static char queuePull(queue_t *queue) {
  char data = 0;
  if (!queueIsEmpty(queue)) {
    data = queue->buffer[queue->tail];
    queue->tail = (queue->tail+1)%queue->length;
  }

  return data;
}

#endif //__QUEUE_H__
