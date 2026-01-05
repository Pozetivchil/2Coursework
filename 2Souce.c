#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>

#define EMPTY -2
#define BLACK -1
#define WHITE 0
#define MAX_ATTEMPTS 100000000

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    int dx;
    int dy;
} Direction;

int trim_newline(char* s);
int flush_line();
int show_menu();
int run_generator();
int** create_field(int rows, int cols);
int free_field(int** field, int rows);
int is_valid(int x, int y, int rows, int cols);
int is_cell_available_for_line(int** field, int x, int y, int rows, int cols);
int draw_line(int** field, int x, int y, Direction dir, int rows, int cols, int id);
int is_fully_covered(int** field, int rows, int cols);
int** generate_puzzle(int rows, int cols);
int print_field(int** field, int rows, int cols);
int save_to_file(int** field, int rows, int cols, char* filename);
int is_solvable(int** puzzle, int rows, int cols);

int main() {
    setlocale(LC_ALL, "RUS");
    system("chcp 1251");

    srand(time(NULL));

    int running = 1;

    while (running) {
        int choice = show_menu();

        if (choice == 1) {
            printf("Выход из программы.\n");
            running = 0;
        }
        else if (choice == 2) {
            run_generator();
        }
        else {
            printf("Введите 1 или 2.\n");
        }
    }

    return 0;
}

int trim_newline(char* s) {
    int i = 0;
    while (s[i] != '\0') {
        if (s[i] == '\n' || s[i] == '\r') {
            s[i] = '\0';
            break;
        }
        i++;
    }
    return 0;
}

int flush_line() {
    int c;
    c = getchar();
    while (c != '\n')
    {
        c = getchar();
    }
    return 0;
}

int show_menu() {
    int choice = 0;

    printf("\n");
    printf("===================================\n");
    printf("Меню\n");
    printf("===================================\n");
    printf("1. Выйти из программы\n");
    printf("2. Генерировать игровые поля\n");
    printf("-----------------------------------\n");
    printf("Ваш выбор: ");

    scanf("%d", &choice);
    flush_line();

    return choice;
}

int run_generator() {
    int rows = 0, cols = 0;
    int ok = 0;

    printf("\nГенератор головоломки 'Роза ветров'\n");
    printf("===================================\n\n");

    while (ok == 0) {
        rows = 0;
        cols = 0;

        printf("Введите размеры поля (строки и столбцы, от 3 до 12): ");
        scanf("%d %d", &rows, &cols);
        flush_line();

        if (rows < 3 || cols < 3 || rows > 12 || cols > 12) {
            printf("Ошибка: размеры должны быть в диапазоне от 3 до 12.\n\n");
        }
        else {
            ok = 1;
        }
    }

    printf("\nГенерация 3 полей для головоломки %dx%d...\n\n", rows, cols);

    int generated = 0;
    int attempts = 0;

    while (generated < 3 && attempts < MAX_ATTEMPTS) {
        attempts++;

        int** puzzle = generate_puzzle(rows, cols);

        if (puzzle != NULL) {
            if (is_solvable(puzzle, rows, cols)) {

                printf("\nПоле %d (попытка %d):\n", generated + 1, attempts);
                print_field(puzzle, rows, cols);

                int accepted = 0;

                while (accepted == 0) {
                    char choice = 'n';

                    printf("\nПоле подходит? (y/n): ");
                    scanf(" %c", &choice);
                    flush_line();

                    if (choice == 'n' || choice == 'N') {
                        accepted = 1;
                    }
                    else if (choice == 'y' || choice == 'Y') {
                        char filename[50];

                        printf("Введите имя файла (или нажмите Enter для puzzle%d.txt): ", generated + 1);
                        fgets(filename, sizeof(filename), stdin);

                        if (filename[0] == '\n') {
                            sprintf(filename, "puzzle%d.txt", generated + 1);
                        }
                        else {
                            trim_newline(filename);
                        }

                        if (save_to_file(puzzle, rows, cols, filename) == 0) {
                            generated++;

                            if (generated < 3) {
                                printf("\n%s\n", "=======================================================");
                            }
                        }
                        else {
                            printf("Не удалось сохранить поле. Попробуйте другое имя файла.\n");
                        }

                        accepted = 1;
                    }
                    else {
                        printf("Введите только y или n.\n");
                    }
                }

                free_field(puzzle, rows);
            }
            else {
                free_field(puzzle, rows);
            }
        }
    }

    if (generated > 0) {
        printf("\n%s\n", "=======================================================");
        printf("Успешно сгенерировано %d полей!\n", generated);
        printf("Возврат в меню...\n");
        return 0;
    }

    printf("\nНе удалось сгенерировать ни одного поля.\nПопробуйте уменьшить размеры поля.\n");
    printf("Возврат в меню...\n");
    return -1;
}

int** create_field(int rows, int cols) {
    int** field = (int**)malloc(rows * sizeof(int*));
    if (field == NULL) {
        printf("Ошибка выделения памяти для поля\n");
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        field[i] = (int*)malloc(cols * sizeof(int));
        if (field[i] == NULL) {
            printf("Ошибка выделения памяти для строки поля\n");
            for (int j = 0; j < i; j++) {
                free(field[j]);
            }
            free(field);
            return NULL;
        }

        for (int j = 0; j < cols; j++) {
            field[i][j] = EMPTY;
        }
    }
    return field;
}

int free_field(int** field, int rows) {
    if (field == NULL) {
        return 0;
    }

    for (int i = 0; i < rows; i++) {
        if (field[i] != NULL) {
            free(field[i]);
        }
    }
    free(field);
    return 0;
}

int is_valid(int x, int y, int rows, int cols) {
    return (x >= 0 && x < rows && y >= 0 && y < cols);
}

int is_cell_available_for_line(int** field, int x, int y, int rows, int cols) {
    if (!is_valid(x, y, rows, cols)) return 0;
    return (field[x][y] == EMPTY);
}

int draw_line(int** field, int x, int y, Direction dir, int rows, int cols, int id) {
    int len = 0;

    int cx = x + dir.dx;
    int cy = y + dir.dy;

    int available_len = 0;
    while (is_cell_available_for_line(field, cx, cy, rows, cols)) {
        available_len++;
        cx += dir.dx;
        cy += dir.dy;
    }

    if (available_len == 0) return 0;

    int max_len = 1 + rand() % available_len;

    cx = x + dir.dx;
    cy = y + dir.dy;

    for (int i = 0; i < max_len; i++) {
        field[cx][cy] = id;
        len++;
        cx += dir.dx;
        cy += dir.dy;
    }

    return len;
}

int is_fully_covered(int** field, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (field[i][j] == EMPTY) {
                return 0;
            }
        }
    }
    return 1;
}

int** generate_puzzle(int rows, int cols) {
    Direction directions_local[4] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

    int** puzzle = create_field(rows, cols);
    if (puzzle == NULL) {
        return NULL;
    }

    int black_count = 0;
    int area = rows * cols;

    if (area <= 25) {
        black_count = 3 + rand() % 3;
    }
    else if (area <= 64) {
        black_count = 9 + rand() % 4;
    }
    else if (area <= 81) {
        black_count = 15 + rand() % 5;
    }
    else if (area <= 100) {
        black_count = 36 + rand() % 6;
    }
    else if (area <= 121) {
        black_count = 45 + rand() % 7;
    }
    else if (area <= 144) {
        black_count = 56 + rand() % 8;
    }
    else {
        black_count = 3;
    }

    Point* blacks = (Point*)malloc(black_count * sizeof(Point));
    if (blacks == NULL) {
        printf("Ошибка выделения памяти для черных клеток\n");
        free_field(puzzle, rows);
        return NULL;
    }

    int* lengths = (int*)calloc(black_count, sizeof(int));
    if (lengths == NULL) {
        printf("Ошибка выделения памяти для длин линий\n");
        free(blacks);
        free_field(puzzle, rows);
        return NULL;
    }

    int placed = 0;
    while (placed < black_count) {
        int x = rand() % rows;
        int y = rand() % cols;

        if (puzzle[x][y] == EMPTY) {
            puzzle[x][y] = BLACK;
            blacks[placed].x = x;
            blacks[placed].y = y;
            placed++;
        }
    }

    for (int i = 0; i < black_count; i++) {
        int dirs[4] = { 0, 1, 2, 3 };

        for (int j = 0; j < 4; j++) {
            int k = rand() % 4;
            int temp = dirs[j];
            dirs[j] = dirs[k];
            dirs[k] = temp;
        }

        for (int d = 0; d < 4; d++) {
            Direction dir = directions_local[dirs[d]];
            int line_len = draw_line(puzzle, blacks[i].x, blacks[i].y, dir, rows, cols, i + 1);
            lengths[i] += line_len;
        }

        if (lengths[i] <= 0) {
            free(blacks);
            free(lengths);
            free_field(puzzle, rows);
            return NULL;
        }
    }

    if (!is_fully_covered(puzzle, rows, cols)) {
        free(blacks);
        free(lengths);
        free_field(puzzle, rows);
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {

            if (puzzle[i][j] == BLACK) {
                for (int k = 0; k < black_count; k++) {
                    if (blacks[k].x == i && blacks[k].y == j) {
                        puzzle[i][j] = lengths[k];
                        break;
                    }
                }
            }
            else {
                puzzle[i][j] = WHITE;
            }
        }
    }

    free(blacks);
    free(lengths);

    return puzzle;
}

int print_field(int** field, int rows, int cols) {
    printf("+");
    for (int j = 0; j < cols; j++) {
        printf("-----+");
    }
    printf("\n");

    for (int i = 0; i < rows; i++) {
        printf("|");
        for (int j = 0; j < cols; j++) {
            if (field[i][j] == WHITE) {
                printf("     |");
            }
            else {
                printf(" %2d  |", field[i][j]);
            }
        }
        printf("\n");

        printf("+");
        for (int j = 0; j < cols; j++) {
            printf("-----+");
        }
        printf("\n");
    }

    return 0;
}

int save_to_file(int** field, int rows, int cols, char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Ошибка открытия файла!\n");
        return -4;
    }

    fprintf(file, "%d %d\n", rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(file, "%d ", field[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("Поле сохранено в %s\n", filename);
    return 0;
}

int is_solvable(int** puzzle, int rows, int cols) {
    int total_white = 0;
    int total_black_numbers = 0;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (puzzle[i][j] == WHITE) {
                total_white++;
            }
            else if (puzzle[i][j] > 0) {
                total_black_numbers += puzzle[i][j];
            }
        }
    }

    return (total_black_numbers == total_white);
}
