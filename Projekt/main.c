#include "tree.h"
#include <stdio.h>
#include <string.h>

void Sort(int n, int freq[], char letters[])
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

void SaveCode(struct QueuePart* root, char* code[], char arr[], int top)
{
    if (root->left)
    {
        arr[top] = '0';
        SaveCode(root->left, code, arr, top + 1);
    }
    if (root->right)
    {
        arr[top] = '1';
        SaveCode(root->right, code, arr, top + 1);
    }
    else if (!root->left)
    {
        code[root->data] = calloc(top, sizeof(char));
        printf("'%c'     ", root->data);
        for (int i = 0;i < top;i++) {
            printf("%c", arr[i]);
            code[root->data][i] = arr[i];
        }
        code[root->data][top] = '\0';
        printf("\n");
    }
}

void Decode(struct QueuePart* root, int* index, char str_compressed[], FILE* file)
{
    if (!root->left && !root->right)
    {
        fprintf(file, "%c", root->data);
        return;
    }

    *index = *index + 1;
    if (root->left && str_compressed[*index] == '0') Decode(root->left, index, str_compressed, file);
    else if (root->right && str_compressed[*index] == '1') Decode(root->right, index, str_compressed, file);
    else return;
}

void* ReconstructTree(FILE* file)
{
    char letter, letter2, letters[256], count[8];
    int i = 0, n = 0, freq[256];

    printf("\nznak    czestotliwosc\n");
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

        printf("'%c'     %d\n",letters[n], freq[letters[n]]);
        n++;
    }
    Sort(n, freq, letters);
    return Tree(letters, freq, n);
}

void Decompress(char file_name[])
{
    //Rekonstrukcja drzewa binarnego
    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }
    struct QueuePart* root = ReconstructTree(file);

    //Dekompresja tekstu
    char* str_compressed = calloc(8, sizeof(char));
    char last_letter = '0', prev_letter, letter, temp[8];
    unsigned int mask = 1U << 7;
    int i, in, n = 0;

    while (!feof(file))
    {
        letter = fgetc(file);
        if (feof(file)) break;

        prev_letter = last_letter;
        last_letter = letter;

        in = (int)letter;
        for (i = 0; i < 8; i++)
        {
            str_compressed[n + i] = (in & mask) ? '1' : '0';
            in <<= 1;
        }
        n += 8;
        str_compressed = realloc(str_compressed, sizeof(char) * (n + 8));
    }
    n -= 8;

    //Nadpisanie przedostatniego bajtu na podstawie ilość bitów nadmiaru odczytanej z ostatniego bajtu
    if ((int)last_letter > 0)
    {
        n -= 8;
        in = (int)prev_letter;
        mask = 1U << (int)last_letter-1;
        for (i = 0; i < (int)last_letter; i++)
        {
            str_compressed[n + i] = (in & mask) ? '1' : '0';
            in <<= 1;
        }
        n += i;
    }
    str_compressed[n] = '\0';

    fclose(file);

    //Odkodowanie tekstu
    *strstr(file_name, ".huff") = '\0';
    file = fopen(file_name, "w");
    int index = -1;

    while (index < (int)(strlen(str_compressed) - 1)) Decode(root, &index, str_compressed, file);


    //Zwolnienie miejsca w pamięci
    free(str_compressed);
}

void Compress(char file_name[], char string[], int freq[], char letters[], int str_length, int letters_count)
{
    printf("\nDlugosc: %d\nUnikalnych znakow: %d\n\n", str_length, letters_count);

    //Sortowanie i tworzenie drzewa binarnego
    Sort(letters_count, freq, letters);
    struct QueuePart* root = Tree(letters, freq, letters_count);

    //Save code
    char* code[256];
    char arr[16];
    int top = 0;
    printf("znak    kod\n");
    SaveCode(root, code, arr, top);

    //Zapis drzewa binarnego
    strcat(file_name, ".huff");
    FILE* file = fopen(file_name, "w");
    for (int i = 0; i < letters_count; i++) {
        fprintf(file, "%c%d ", letters[i], freq[letters[i]]);
    }
    fprintf(file, "--");

    //Kompresja tekstu
    char temp[8];
    unsigned char bit;
    int j, n = 0;
    for (int i = 0;i < str_length;i++)
    {
        for (j = 0;j < strlen(code[string[i]]);j++)
        {
            temp[n++] = code[string[i]][j];
            if (n >= 8)
            {
                n = 0;
                bit = strtol(temp, 0, 2);
                fputc(bit, file);
            }
        }
    }
    if (n != 0) //Uzupełnianie niepełnego bajtu
    {
        temp[n] = '\0';
        bit = strtol(temp, 0, 2);
        fputc(bit, file);
        fputc(n, file);
    } else fputc(0, file);

    fclose(file);

}

void ReadString(char to_compress[])
{
    //Inicjacja zmiennych potrzebnych do odczytu tekstu
    int str_length = strlen(to_compress), letters_count = 0, i, freq[256];
    char letters[256];
    memset(freq, 0, sizeof freq);

    //Odczyt danych z argumentu komendy
    for (i = 0; i < str_length; i++)
    {
        if (freq[to_compress[i]] == 0)
        {
            letters[letters_count] = to_compress[i];
            letters_count++;
        }
        freq[to_compress[i]]++;
    }

    //Utworzenie drzewa binarnego i kompresja tekstu
    Compress("file",to_compress, freq, letters, str_length, letters_count);
}

void* InputFile(FILE* file, int freq[], char letters[], int* str_length, int* letters_count)
{
    char letter;
    char* str = calloc(1, sizeof(char));

    while ((letter = fgetc(file)) != EOF)
    {
        if (freq[letter] == 0)
        {
            letters[*letters_count] = letter;
            *letters_count = *letters_count + 1;
        }
        str[*str_length] = letter;
        freq[letter]++;
        *str_length = *str_length + 1;
        str = realloc(str, sizeof(char) * (*str_length + 1));
    }
    str[*str_length] = '\0';
    return realloc(str, sizeof(char) * (*str_length + 1));
    fclose(file);
}

void ReadFile(FILE* file, char file_name[])
{
    //Inicjacja zmiennych potrzebnych do odczytu tekstu
    int str_length = 0, letters_count = 0, i, freq[256];
    char letters[256];
    memset(freq, 0, sizeof freq);

    char* string = InputFile(file, freq, letters, &str_length, &letters_count);

    //Utworzenie drzewa binarnego i kompresja tekstu
    Compress(file_name, string, freq, letters, str_length, letters_count);

}

int main(int argc, char *argv[])
{
    //Obsluga lini komend
    if (argc >= 3 && !strcmp(argv[1], "compress")) {

        FILE* file = fopen(argv[2], "r");

        if (file == NULL) ReadString(argv[2]);
        else
        {
            char file_name[128];
            strcpy(file_name, argv[2]);
            ReadFile(file, file_name);
        }
    }
    else if (argc >= 3 && !strcmp(argv[1], "decompress")) {
        if (strstr(argv[2], "huff") == NULL) {
            printf("\nNiepoprawny format lub adres pliku\n");
            return 0;
        }
        Decompress(argv[2]);
    }
    else {
        printf("Lista dostepnych komend:\n\n huffman compress \"tekst\"\n huffman compress <adres pliku>\n huffman decompress <adres skompresowanego pliku>\n\n");
        return 1;
    }

    return 0;
}