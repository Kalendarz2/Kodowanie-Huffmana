#include "tree.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

/// <summary>
/// Funkcja czyszcząca zawartość kompatybilnej konsoli 
/// </summary>
void ClearConsole()
{
    system("@cls||clear");
}

/// <summary>
/// Zapisanie kodu odczytu danych z drzewa na podstawie znaków stringu wejściowego (Funkcja optymalizacyjna)
/// </summary>
/// <param name="root">korzeń drzewa</param>
/// <param name="code">zapisywany kod</param>
/// <param name="arr">słowo bitowe</param>
/// <param name="top">długość słowa bitowego</param>
/// <param name="line">wyswietlanie w kolumnach w konsoli</param>
void SaveCode(struct QueuePart* root, char* code[], char arr[], int top, int* line)
{
    if (root->left)
    {
        arr[top] = '0';
        SaveCode(root->left, code, arr, top + 1, line);
    }
    if (root->right)
    {
        arr[top] = '1';
        SaveCode(root->right, code, arr, top + 1, line);
    }
    else if (!root->left)
    {
        code[root->data] = calloc(top, sizeof(char));
        if (root->data == '\n') printf("'/n'    ");
        else if (root->data == 9) printf("'tab'   ");
        else printf("'%c'     ", root->data);

        //Zapisanie kodu na literę
        for (int i = 0;i < top;i++) code[root->data][i] = arr[i];

        code[root->data][top] = '\0';
        printf("%s", code[root->data]);

        //Wyświetlanie 3 w 1 linijce
        printf("\r\033[32C");
        *line = *line + 1;
        if (*line == 2) printf("\033[32C");
        else if (*line >= 3)
        {
            printf("\n");
            *line = 0;
        }

    }
}

/// <summary>
/// Funkcja rekurencyjna odkodowująca tekst na podstawie drzewa
/// </summary>
/// <param name="root">korzeń drzewa</param>
/// <param name="index">indeks ciągu binarnego</param>
/// <param name="str_compressed">ciąg binarny</param>
/// <param name="file">plik wyjściowy</param>
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

/// <summary>
/// Rekonstrukcja drzewa podczas dekompresji pliku
/// </summary>
/// <param name="file">skompresowany plik</param>
/// <returns>korzeń drzewa</returns>
/// @see Decompress() Tree()
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

/// <summary>
/// Głowna funkcja dekompresji pliku. Odtwarza ona drzewo binarne na podstawie zapisanych danych, a następnie na jego podstawie dokompresuje tekst
/// </summary>
/// <param name="file">plik .huff do dekompresji</param>
/// <param name="file_name">nazwa pliku</param>
/// @see ReconstructTree() Decode()
void Decompress(FILE* file, char file_name[])
{
    //Rekonstrukcja drzewa binarnego
    struct QueuePart* root = ReconstructTree(file);

    //Inicjacja dekompresji tekstu
    char* str_compressed = calloc(8, sizeof(char));
    char last_letter = '0', prev_letter, letter, temp[8];
    unsigned int mask = 1U << 7;
    int i, in, n = 0;

    while ((letter = fgetc(file)) != EOF) //Odczytanie pliku
    {
        in = (int)letter;

        if (in == -2) //Odkodowanie znaku EOF
        {
            letter = fgetc(file);
            in = (int)letter;

            if (!(in & mask)) //2 bity kodujące eof, znak zastępujący eof lub wadliwy znak 26
            {
                in <<= 1;
                if (in & mask)
                {
                    for (i = 0; i < 7; i++) str_compressed[n + i] = '1';
                    str_compressed[n + 7] = '0';
                }
                else for (i = 0; i < 8; i++) str_compressed[n + i] = '1';
            }
            else
            {
                in <<= 1;
                for (i = 0; i < 8; i++) str_compressed[n + i] = '0';
                for (i = 3; i < 5; i++) str_compressed[n + i] = '1';
                str_compressed[n + 6] = '1';
            }
            n += 8;
            in <<= 1;

            //Pozostałe 6 bitów
            for (i = 0; i < 6; i++)
            {
                str_compressed[n + i] = (in & mask) ? '1' : '0';
                in <<= 1;
            }
            n += 6;
        }
        else
        {
            for (i = 0; i < 8; i++)
            {
                str_compressed[n + i] = (in & mask) ? '1' : '0';
                in <<= 1;
            }
            n += 8;
        }

        //Zapis ostatnich znaków
        prev_letter = last_letter;
        last_letter = letter;

        //Poszerzenie pamięci stringu
        str_compressed = realloc(str_compressed, sizeof(char) * (n + 16));

    }
    n -= 8;

    //Nadpisanie przedostatniego bajtu na podstawie ilość bitów nadmiaru odczytanej z ostatniego bajtu
    if ((int)last_letter > 0)
    {
        n -= 8;
        in = (int)prev_letter;
        mask = 1U << (int)last_letter - 1;
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
    fclose(file);

    //Zwolnienie miejsca w pamięci
    free(str_compressed);
}

/// <summary>
/// Głowna funkcja kompresji. Część wspólna ReadTXTFile() i ReadString()
/// </summary>
/// <param name="file_name">nazwa pliku docelowego</param>
/// <param name="string">tekst do kompresji</param>
/// <param name="freq">częstotliwość występowania znaków</param>
/// <param name="letters">lista znaków, które wystąpiły w tekście</param>
/// <param name="str_length">długość tekstu</param>
/// <param name="letters_count">ilość unikatowych znaków</param>
/// @see ReadTXTFile() Compress() SaveCode()
void Compress(char file_name[], char string[], int freq[], char letters[], int str_length, int letters_count)
{
    printf("Dlugosc: %d\nUnikalnych znakow: %d\n\n", str_length, letters_count);

    //Sortowanie i tworzenie drzewa binarnego
    SortInput(letters_count, freq, letters);
    struct QueuePart* root = Tree(letters, freq, letters_count);

    //Zapis kodu optymalizacji odczytu drzewa
    char* code[256];
    char arr[16];
    int top = 0, line = 0;
    printf("znak    kod                     znak    kod                     znak    kod\n");
    SaveCode(root, code, arr, top, &line);

    //Zapis drzewa binarnego
    strcat(file_name, ".huff");
    FILE* file = fopen(file_name, "w");
    for (int i = 0; i < letters_count; i++) {
        fprintf(file, "%c%d ", letters[i], freq[letters[i]]);
    }
    fprintf(file, "--");

    //Kompresja tekstu
    char temp[9];
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
                temp[8] = '\0';
                bit = strtol(temp, 0, 2);

                if (bit == 254)  //Przygotowanie zamiennika znaku EOF
                {
                    temp[n++] = '0';
                    temp[n++] = '1';
                }
                else if (bit == 255)  //Zastępowanie znaku EOF (255/-1)
                {
                    bit = 254;
                    temp[n++] = '0';
                    temp[n++] = '0';
                }
                else if (bit == 26)  //Zastępowanie z jakiegoś powodu wadliwego znaku
                {
                    bit = 254;
                    temp[n++] = '1';
                    temp[n++] = '0';
                }

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
    }
    else fputc(0, file);

    fclose(file);
}

/// <summary>
/// Funkcja odczytująca tekst podany przez użytkowanika i przygotowująca dane.
/// </summary>
/// <param name="to_compress">wprowadzony tekst</param>
/// @see ReadTXTFile() Compress()
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
    char file_name[16] = "file.txt";
    Compress(file_name, to_compress, freq, letters, str_length, letters_count);
}

/// <summary>
/// Funkcja
/// </summary>
/// <param name="file">plik do odczytu</param>
/// <param name="freq">częstotliwość występowaniu znaków</param>
/// <param name="letters">znaki występujące w tekście</param>
/// <param name="str_length">długość tekstu w pliku</param>
/// <param name="letters_count">ilość unikalnych znaków</param>
/// <returns>dynamiczny adres nowego ciągu znaków pobranych z pliku</returns>
/// @see ReadTXTFile()
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

/// <summary>
/// Alternatywa ReadString(). Odczytuje dane z pliku i wywołuje funkcje.
/// </summary>
/// <param name="file"></param>
/// <param name="file_name"></param>
/// @see ReadString() InputFile() Compress()
void ReadTXTFile(FILE* file, char file_name[])
{
    //Inicjacja zmiennych potrzebnych do odczytu tekstu
    int str_length = 0, letters_count = 0, i, freq[256];
    char letters[256];
    memset(freq, 0, sizeof freq);

    char* string = InputFile(file, freq, letters, &str_length, &letters_count);

    //Utworzenie drzewa binarnego i kompresja tekstu
    Compress(file_name, string, freq, letters, str_length, letters_count);

    //Zwolnienie miejsca w pamięci
    free(string);
}

/// <summary>
/// Fukcja formatująca adres pliku w przypadku umieszczenia go w cudzysłowie
/// </summary>
/// <param name="file_name">adres pliku</param>
/// <returns>Otwarty plik ze sformatowanego adresu</returns>
FILE* FileFormat(char file_name[])
{
    ClearConsole();
    printf("Podaj sciezke pliku: ");

    gets(file_name);
    if (file_name[0] == '"' && strlen(file_name) > 2)
    {
        for (int i = 0; i < strlen(file_name) - 2; i++) file_name[i] = file_name[i + 1];
        file_name[strlen(file_name) - 2] = '\0';
    }
    return fopen(file_name, "r");
}

/// <summary>
/// Funkcja oczekująca na wciśnięcie dowolnego klawisza przez użytkownika
/// </summary>
void Wait()
{
    printf("\n\nWcisnij dowolny klawisz, aby kontynuowac...");
    getch();
}

/// <summary>
/// Głowna funkcja wykonywana w przypadku włączenia programu z poziomu pliku .exe lub wpisaniu komendy bez argumentów
/// </summary>
/// @see ClearConsole() FileFormat() ReadTXTFile() ReadString() Decompress()
void FromEXE()
{
    char ch = '1';
    char file_name[128];
    while (ch != '0')
    {
        ClearConsole();
        printf("###################################################################\n#                  Kompresja algorytmem Huffmana                  #\n#                                                                 #\n# 1 - kompresuj plik | 2 - kompresuj tekst | 3 - dekompresuj plik #\n###################################################################\n");

        ch = getch();

        if (ch == '1') { // Kompresja pliku
            FILE* file = FileFormat(file_name);

            if (file == NULL)
            {
                printf("\rNiepoprawny adres pliku");
                Sleep(2000);
            }
            else
            {
                ClearConsole();
                ReadTXTFile(file, file_name);
                fclose(file);
                Wait();
            }
        }
        else if (ch == '2') { // Kompresja tekstu
            ClearConsole();
            printf("Podaj tekst do kompresji: ");
            char text[256];
            gets(text);

            ReadString(text);
            Wait();
        }
        else if (ch == '3') { //Dekompresja pliku
            FILE* file = FileFormat(file_name);

            if (file == NULL)
            {
                printf("\rNiepoprawny adres pliku");
                Sleep(2000);
            }
            else if (strstr(file_name, "huff") == NULL)
            {
                fclose(file);
                printf("\nNiepoprawny format pliku. Akceptowane tylko .huff");
                Sleep(3000);
            }
            else
            {
                ClearConsole();
                Decompress(file, file_name);
                fclose(file);
                Wait();
            }
        }
    }
}

/// <summary>
/// Głowna funkcja programu z możliwością obsługi z poziomu linii komend
/// </summary>
/// <param name="argc">Ilość argumentów komendy</param>
/// <param name="argv">Treść argumentów</param>
int main(int argc, char* argv[])
{
    SetConsoleTitle(L"Kompresja Huffmana");

    if (argc == 1) FromEXE();
    else if (argc >= 3 && !strcmp(argv[1], "compress")) { //Obsluga lini komend

        FILE* file = fopen(argv[2], "r");

        if (file == NULL) ReadString(argv[2]);
        else
        {
            char file_name[128];
            strcpy(file_name, argv[2]);
            ReadTXTFile(file, file_name);
        }
    }
    else if (argc >= 3 && !strcmp(argv[1], "decompress")) {
        if (strstr(argv[2], "huff") == NULL) {
            printf("\nNiepoprawny format lub adres pliku\n");
            return 0;
        }

        FILE* file = fopen(argv[2], "r");
        if (file == NULL) {
            perror("Unable to open file");
            exit(EXIT_FAILURE);
        }
        Decompress(file, argv[2]);
    }
    else {
        printf("Lista dostepnych komend:\n\n Huff compress \"tekst\"\n Huff compress <adres pliku>\n Huff decompress <adres skompresowanego pliku>\n\n");
        system("pause");
        return 1;
    }

    return 0;
}