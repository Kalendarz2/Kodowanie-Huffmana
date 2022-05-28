#include <stdlib.h>
#include <stdio.h>

/// Fragment drzewa zawiarający dane o znaku
struct QueuePart {
    /// Znak przechowywany w strukturze
    char data;
    /// Ilość wystąpień znaku
    unsigned freq;
    /// Wskaźniki na elementy drzewa
    struct QueuePart* left, * right;
};

/// Kolejka znaków
struct Queue {
    int front, back, capacity;
    /// lista wskaźników na części drzewa (QueuePart)
    struct QueuePart** array;
};

/// <summary>
/// Test, czy kolejka jest pusta
/// </summary>
/// <param name="queue">kolejka znaków</param>
/// <returns> 1 - kolejka jest pusta, 0 - nie jest pusta</returns>
int isEmpty(struct Queue* queue)
{
    return (queue->front == -1);
}

/// <summary>
/// Pobieranie pierwszego elementu kolejki
/// </summary>
/// <returns> pierwszy element</returns>
struct QueuePart* getFront(struct Queue* queue)
{
    if (queue->front == -1) return NULL;
    return queue->array[queue->front];
}

/// <summary>
/// Utworzenie nowego elementu drzewa
/// </summary>
/// <returns> nowy element drzewa</returns>
struct QueuePart* CreatePart(char data, unsigned freq)
{
    struct QueuePart* temp = (struct QueuePart*)malloc(sizeof(struct QueuePart));
    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;
    return temp;
}

/// <summary>
/// Utworzenie nowej kolejki
/// </summary>
/// <returns> nową kolejkę</returns>
struct Queue* CreateQueue(int capacity)
{
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    queue->front = queue->back = -1;
    queue->capacity = capacity;
    queue->array = (struct QueuePart**)malloc(queue->capacity * sizeof(struct QueuePart*));

    return queue;
}

/// <summary>
/// Dodanie elementu do kolejki
/// </summary>
/// <param name="queue">kolejka, do której zostanie dodany element</param>
/// <param name="item">element do dodania</param>
void AddToQueue(struct Queue* queue, struct QueuePart* item)
{
    if (queue->back == queue->capacity - 1) return;
    queue->array[++queue->back] = item;

    if (queue->front == -1) ++queue->front;
}

/// <summary>
/// Usunięcie elementu z kolejki
/// </summary>
/// <returns> usunięty element</returns>
struct QueuePart* RemoveFromQueue(struct Queue* queue)
{
    if (isEmpty(queue)) return NULL;
    struct QueuePart* temp = queue->array[queue->front];
    if (queue->front == queue->back) queue->front = queue->back = -1;
    else ++queue->front;

    return temp;
}

/// <summary>
/// Transfer najmniejszej wartości pomiędzy kolejkami
/// </summary>
struct QueuePart* MinFrom(struct Queue* Queue1, struct Queue* Queue2)
{
    if (isEmpty(Queue1)) return RemoveFromQueue(Queue2);
    if (isEmpty(Queue2)) return RemoveFromQueue(Queue1);
    if (getFront(Queue1)->freq < getFront(Queue2)->freq) return RemoveFromQueue(Queue1);
    return RemoveFromQueue(Queue2);
}

/// <summary>
/// Głowna funkcja drzewa. Tworzy 2 kolejki, wypełanie pierwszą z nich elementami, a następnie na podstawie algorytmu Huffmana wypełnia drugą kolejkę.
/// </summary>
/// <returns> korzeń drzewa</returns>
/// @see CreateQueue() CreatePart() AddToQueue() isEmpty() AddToQueue() RemoveFromQueue()
struct QueuePart* Tree(char data[], int freq[], int size)
{
    struct QueuePart* left, * right, * top;

    struct Queue* Queue1 = CreateQueue(size);
    struct Queue* Queue2 = CreateQueue(size);

    //Wypełnienie kolejki danymi
    for (int i = 0; i < size; ++i) AddToQueue(Queue1, CreatePart(data[i], freq[data[i]]));

    while (!(isEmpty(Queue1) && Queue2->front == Queue2->back && Queue2->front != -1))
    {
        left = MinFrom(Queue1, Queue2);
        right = MinFrom(Queue1, Queue2);

        top = CreatePart('$', left->freq + right->freq);
        top->left = left;
        top->right = right;
        AddToQueue(Queue2, top);
    }
    return RemoveFromQueue(Queue2);
}

/// <summary>
/// Sortowanie wartości wejściowych drzewa
/// </summary>
/// <param name="n">ilość znaków</param>
/// <param name="freq">ilość wystąpień znaków</param>
/// <param name="letters">znaki występujące w tekście</param>
void SortInput(int n, int freq[], char letters[])
{
    char temp;
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (freq[letters[j]] > freq[letters[j + 1]])
            {
                temp = letters[j];
                letters[j] = letters[j + 1];
                letters[j + 1] = temp;
            }
}