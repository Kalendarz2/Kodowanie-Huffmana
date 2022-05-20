#include <stdlib.h>
#include <stdio.h>

//Fragment kolejki zawiarający dane o znaku
struct QueuePart {
    char data;
    unsigned freq;
    struct QueuePart* left, * right;
};

//Kolejka
struct Queue {
    int front, back, capacity;
    struct QueuePart** array;
};

//Test, czy kolejka jest pusta
int isEmpty(struct Queue* queue)
{
    return (queue->front == -1);
}

//Pobieranie pierwszego elementu kolejki
struct QueuePart* getFront(struct Queue* queue)
{
    if (queue->front == -1) return NULL;
    return queue->array[queue->front];
}

//Utworzenie nowego elementu
struct QueuePart* CreatePart(char data, unsigned freq)
{
    struct QueuePart* temp = (struct QueuePart*)malloc(sizeof(struct QueuePart));
    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;
    return temp;
}

//Utworzenie nowej kolejki
struct Queue* CreateQueue(int capacity)
{
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    queue->front = queue->back = -1;
    queue->capacity = capacity;
    queue->array = (struct QueuePart**)malloc(queue->capacity * sizeof(struct QueuePart*));

    return queue;
}

//Dodanie elementu do kolejki
void AddToQueue(struct Queue* queue, struct QueuePart* item)
{
    if (queue->back == queue->capacity - 1) return;
    queue->array[++queue->back] = item;

    if (queue->front == -1) ++queue->front;
}

//Usunięcie elementu z kolejki
struct QueuePart* RemoveFromQueue(struct Queue* queue)
{
    if (isEmpty(queue)) return NULL;
    struct QueuePart* temp = queue->array[queue->front];
    if (queue->front == queue->back) queue->front = queue->back = -1;
    else ++queue->front;

    return temp;
}

//Transfer najmniejszej wartości pomiędzy kolejkami
struct QueuePart* MinFrom(struct Queue* Queue1, struct Queue* Queue2)
{
    if (isEmpty(Queue1)) return RemoveFromQueue(Queue2);
    if (isEmpty(Queue2)) return RemoveFromQueue(Queue1);
    if (getFront(Queue1)->freq < getFront(Queue2)->freq) return RemoveFromQueue(Queue1);
    return RemoveFromQueue(Queue2);
}

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

//Sortowanie wartości wejściowych drzewa
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

//Rekonstrukcja drzewa podczas dekompresji pliku
void* ReconstructTree(FILE* file)
{
    char letter, letter2, letters[256], count[8];
    int i = 0, n = 0, line = 0, freq[256];

    printf("znak    ilosc   znak    ilosc   znak    ilosc   znak    ilosc\n");
    while ((letter = (char)fgetc(file)) != -1)
    {

        letters[n] = letter;
        i = 0;
        letter2 = (char)fgetc(file);
        if (letter == '-' && letter2 == '-') //Koniec drzewa i miejsce rozpoczęcia skompresowanego pliku
        {
            break;
        }

        do {
            count[i++] = (char)letter2;
        } while ((letter2 = (char)fgetc(file)) != ' ');

        freq[letters[n]] = (int)atoi(count);

        //Wyświetlanie
        if (letters[n] == '\n') printf("'/n'    %d", letters[n], freq[letters[n]]);
        else if (letters[n] == 9) printf("'tab'   %d", letters[n], freq[letters[n]]);
        else printf("'%c'     %d", letters[n], freq[letters[n]]);

        line++;
        printf("\r\033[16C");
        if (line >= 2) printf("\033[16C");
        if (line == 3) printf("\033[16C");
        else if (line >= 4)
        {
            printf("\n");
            line = 0;
        }

        //printf("'%c'     %d\n", letters[n], freq[letters[n]]);
        n++;
    }
    SortInput(n, freq, letters);
    return Tree(letters, freq, n);
}