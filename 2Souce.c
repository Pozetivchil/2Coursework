#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>

#define EMPTY -2      // Пустая клетка
#define BLACK -1      // Черная клетка
#define WHITE 0       // Белая клетка
#define MAX_ATTEMPTS 100000000

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    int dx;
    int dy;
} Direction;

Direction directions[4] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

// Объявления функций (прототипы)
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

    srand((unsigned)time(NULL));

    printf("Генератор головоломки 'Роза ветров'\n");
    printf("===================================\n\n");

    {
        char exit_choice = 'n';

        while (exit_choice == 'n' || exit_choice == 'N') {

            int rows = 0, cols = 0;

            printf("Введите размеры поля (строки и столбцы, от 3 до 12): ");

            if (scanf("%d %d", &rows, &cols) != 2) {
                printf("Ошибка ввода. Нужно ввести два целых числа.\n");
                return -4;
            }

            if (rows < 3 || cols < 3) {
                printf("Ошибка: минимальный размер поля 3x3\n");
                return -3;
            }

            if (rows > 12 || cols > 12) {
                printf("Ошибка: максимальный размер поля 12x12\n");
                return -2;
            }

            printf("\nГенерация 3 полей для головоломки %dx%d...\n\n", rows, cols);

            {
                int generated = 0;
                int attempts = 0;

                while (generated < 3 && attempts < MAX_ATTEMPTS) {
                    attempts++;

                    {
                        int** puzzle = generate_puzzle(rows, cols);

                        if (puzzle != NULL) {

                            if (is_solvable(puzzle, rows, cols)) {

                                printf("\nПоле %d (попытка %d):\n", generated + 1, attempts);
                                print_field(puzzle, rows, cols);

                                {
                                    char choice = 'n';
                                    printf("\nПоле вам нравится? (y/n): ");
                                    scanf(" %c", &choice);

                                    if (choice == 'y' || choice == 'Y') {

                                        // Если поле понравилось — оно ДОЛЖНО быть сохранено,
                                        // иначе нельзя переходить к следующему полю.
                                        {
                                            int saved_ok = 0;

                                            while (!saved_ok) {

                                                char filename[50];

                                                printf("Введите имя файла (или нажмите Enter для puzzle%d.txt): ", generated + 1);

                                                // очищаем остаток строки после scanf
                                                getchar();

                                                if (fgets(filename, sizeof(filename), stdin) == NULL) {
                                                    printf("Ошибка чтения имени файла.\n");
                                                    free_field(puzzle, rows);
                                                    return -5;
                                                }

                                                // Если пользователь просто нажал Enter
                                                if (filename[0] == '\n') {
                                                    sprintf(filename, "puzzle%d.txt", generated + 1);
                                                }
                                                else {
                                                    // Убираем символ новой строки
                                                    int i = 0;
                                                    while (filename[i] != '\n' && filename[i] != '\0') {
                                                        i++;
                                                    }
                                                    filename[i] = '\0';
                                                }

                                                if (save_to_file(puzzle, rows, cols, filename) == 0) {
                                                    saved_ok = 1;
                                                    generated++;
                                                }
                                                else {
                                                    printf("Не удалось сохранить поле. Попробуйте другое имя файла.\n");
                                                }
                                            }
                                        }
                                    }
                                }

                                free_field(puzzle, rows);

                                if (generated > 0 && generated < 3) {
                                    printf("\n%s\n", "=======================================================");
                                }

                            }
                            else {
                                free_field(puzzle, rows);
                            }
                        }
                    }
                }

                if (generated > 0) {
                    printf("\n%s\n", "=======================================================");
                    printf("Успешно сгенерировано %d полей!\n", generated);
                }
                else {
                    printf("\nНе удалось сгенерировать ни одного поля.\n");
                    printf("Попробуйте уменьшить размеры поля.\n");
                    return -1;
                }
            }

            printf("\nХотите сгенерировать ещё? (Y/y - продолжить, N/n - выйти): ");
            scanf(" %c", &exit_choice);
            printf("\n");
        }
    }

    return 0;
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

    {
        int max_len = 1 + rand() % available_len;

        cx = x + dir.dx;
        cy = y + dir.dy;

        for (int i = 0; i < max_len; i++) {
            field[cx][cy] = id;
            len++;
            cx += dir.dx;
            cy += dir.dy;
        }
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
    int** puzzle = create_field(rows, cols);
    if (puzzle == NULL) {
        return NULL;
    }

    int black_count = 0;

    if (rows * cols <= 25) {
        black_count = 3 + rand() % 3;
    }
    else if (rows * cols <= 64) {
        black_count = 9 + rand() % 4;
    }
    else if (rows * cols <= 81) {
        black_count = 15 + rand() % 5;
    }
    else if (rows * cols <= 100) {
        black_count = 36 + rand() % 6;
    }
    else if (rows * cols <= 121) {
        black_count = 45 + rand() % 7;
    }
    else if (rows * cols <= 144) {
        black_count = 56 + rand() % 8;
    }
    else {
        // на всякий случай (хотя при 3..12 сюда не попадём)
        black_count = 3;
    }

    {
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

        // Размещаем черные клетки
        {
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
        }

        // Для каждой черной клетки рисуем линии в случайных направлениях
        for (int i = 0; i < black_count; i++) {
            int dirs[4] = { 0, 1, 2, 3 };

            // Перемешиваем направления
            for (int j = 0; j < 4; j++) {
                int k = rand() % 4;
                int temp = dirs[j];
                dirs[j] = dirs[k];
                dirs[k] = temp;
            }

            // Пробуем нарисовать линии в каждом направлении
            for (int d = 0; d < 4; d++) {
                Direction dir = directions[dirs[d]];
                int line_len = draw_line(puzzle, blacks[i].x, blacks[i].y, dir, rows, cols, i + 1);
                lengths[i] += line_len;
            }
        }

        // Важно: если есть черная клетка с длиной 0 — это плохо (она превратится в 0 и станет "белой")
        for (int i = 0; i < black_count; i++) {
            if (lengths[i] <= 0) {
                free(blacks);
                free(lengths);
                free_field(puzzle, rows);
                return NULL;
            }
        }

        // Проверяем, что все клетки покрыты
        if (!is_fully_covered(puzzle, rows, cols)) {
            free(blacks);
            free(lengths);
            free_field(puzzle, rows);
            return NULL;
        }

        // Заменяем черные клетки на числа и остальное на белые
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {

                if (puzzle[i][j] == BLACK) {
                    // Находим, какая это черная клетка
                    for (int k = 0; k < black_count; k++) {
                        if (blacks[k].x == i && blacks[k].y == j) {
                            puzzle[i][j] = lengths[k];
                            break;
                        }
                    }
                }
                else {
                    // Всё остальное делаем белым
                    puzzle[i][j] = WHITE;
                }
            }
        }

        free(blacks);
        free(lengths);
    }

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

    // Базовая проверка: сумма чисел в черных клетках равна количеству белых клеток
    return (total_black_numbers == total_white);
}
