#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>
#include <errno.h>

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
int save_to_file(int** field, int rows, int cols, const char* filename);
int is_solvable(int** puzzle, int rows, int cols);

/**
* Главная функция программы - генератор головоломок "Роза ветров"
* @return 0 при успешном выполнении, -1 при невозможности генерации полей,
*         -2 при превышении максимального размера поля, -3 при слишком малом размере поля
*/
int main() {
    setlocale(LC_ALL, "RUS");
    system("chcp 1251");

    srand(time(NULL));
    int rows, cols;

    printf("Генератор головоломки 'Роза ветров'\n");
    printf("===================================\n\n");

    char exit_choice = 'n';

    while (exit_choice == 'n' || exit_choice == 'n')
    {
        printf("Введите размеры поля (строки и столбцы, от 3 до 12): ");
        scanf("%d %d", &rows, &cols);

        if (rows < 3 || cols < 3) {
            perror("Ошибка: минимальный размер поля 3x3\n");
            return -3;
        }

        if (rows > 12 || cols > 12) {
            perror("Ошибка: максимальный размер поля 12x12\n");
            return -2;
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

                    printf("\nСохранить это поле? (y/n): ");
                    char choice;
                    scanf(" %c", &choice);

                    if (choice == 'y' || choice == 'Y') {
                        char filename[50];
                        printf("Введите имя файла (или нажмите Enter для puzzle%d.txt): ", generated + 1);
                        getchar(); // Считываем остаток строки
                        fgets(filename, sizeof(filename), stdin);

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

                        save_to_file(puzzle, rows, cols, filename);
                        generated++;
                    }

                    free_field(puzzle, rows);

                    if (generated < 3) {
                        printf("\n%s\n", "=======================================================");
                    }
                }
                else {
                    free_field(puzzle, rows);
                }
            }
        }

        if (generated > 0) {
            printf("\n%s\n", "=======================================================");
            printf("Успешно сгенерировано %d полей!\n", generated);
        }
        else {
            perror("\nНе удалось сгенерировать ни одного поля.\nПопробуйте уменьшить размеры поля.\n");
            return -1;
        }

        printf("Вы хотите продолжить генерировать игровые поля: введите Y/y, чтобы выйти, или N/n, чтобы продолжить \n");
        scanf("%c", &exit_choice);
    }

    return 0;
}

/**
* Создание динамического двумерного массива для игрового поля
* @param rows - количество строк в создаваемом поле
* @param cols - количество столбцов в создаваемом поле
* @return указатель на созданное поле или NULL при ошибке выделения памяти
*/
int** create_field(int rows, int cols) {
    int** field = (int**)malloc(rows * sizeof(int*));
    if (field == NULL) {
        perror("Ошибка выделения памяти для поля");
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        field[i] = (int*)malloc(cols * sizeof(int));
        if (field[i] == NULL) {
            perror("Ошибка выделения памяти для строки поля");
            // Освобождаем уже выделенную память перед возвратом NULL
            for (int j = 0; j < i; j++) {
                free(field[j]);
            }
            free(field);
            return NULL;
        }

        for (int j = 0; j < cols; j++) {
            field[i][j] = EMPTY;  // Теперь EMPTY = -2
        }
    }
    return field;
}


/**
* Освобождение памяти, занятой игровым полем
* @param field - указатель на поле для освобождения
* @param rows - количество строк в поле
* @return 0 при успешном освобождении памяти
*/
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

/**
* Проверка корректности координат на игровом поле
* @param x - координата строки для проверки
* @param y - координата столбца для проверки
* @param rows - общее количество строк в поле
* @param cols - общее количество столбцов в поле
* @return 1 если координаты находятся в пределах поля, 0 в противном случае
*/
int is_valid(int x, int y, int rows, int cols) {
    return (x >= 0 && x < rows && y >= 0 && y < cols);
}

/**
* Проверка возможности проведения линии через указанную клетку
* @param field - текущее состояние игрового поля
* @param x - координата строки проверяемой клетки
* @param y - координата столбца проверяемой клетки
* @param rows - количество строк в поле
* @param cols - количество столбцов в поле
* @return 1 если клетка пуста (значение EMPTY), 0 если занята или координаты невалидны
*/
int is_cell_available_for_line(int** field, int x, int y, int rows, int cols) {
    if (!is_valid(x, y, rows, cols)) return 0;
    return (field[x][y] == EMPTY); // Клетка должна быть пустой (-2)
}

/**
* Рисование линии от черной клетки в заданном направлении
* @param field - игровое поле для рисования линии
* @param x - координата строки черной клетки
* @param y - координата столбца черной клетки
* @param dir - направление рисования линии (структура Direction)
* @param rows - количество строк в поле
* @param cols - количество столбцов в поле
* @param id - уникальный идентификатор линии для отслеживания
* @return фактическая длина нарисованной линии (0 если не удалось нарисовать)
*/
int draw_line(int** field, int x, int y, Direction dir, int rows, int cols, int id) {
    int len = 0;
    int cx = x + dir.dx;
    int cy = y + dir.dy;

    // Сначала проверяем, сколько клеток можно занять
    int available_len = 0;
    while (is_cell_available_for_line(field, cx, cy, rows, cols)) {
        available_len++;
        cx += dir.dx;
        cy += dir.dy;
    }

    if (available_len == 0) return 0;

    // Выбираем случайную длину от 1 до доступной
    int max_len = 1 + rand() % available_len;

    // Рисуем линию
    cx = x + dir.dx;
    cy = y + dir.dy;
    for (int i = 0; i < max_len; i++) {
        field[cx][cy] = id;  // Помечаем клетку ID линии
        len++;
        cx += dir.dx;
        cy += dir.dy;
    }

    return len;
}

/**
* Проверка полного покрытия поля линиями
* @param field - игровое поле для проверки
* @param rows - количество строк в поле
* @param cols - количество столбцов в поле
* @return 1 если все клетки поля покрыты (нет клеток со значением EMPTY), 0 в противном случае
*/
int is_fully_covered(int** field, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (field[i][j] == EMPTY) {  // Проверяем EMPTY = -2
                return 0; // Есть непокрытая клетка
            }
        }
    }
    return 1; // Всё покрыто
}


/**
* Генерация головоломки "Роза ветров"
* @param rows - количество строк в генерируемом поле
* @param cols - количество столбцов в генерируемом поле
* @return указатель на сгенерированное поле или NULL при неудачной генерации после всех попыток
*/
int** generate_puzzle(int rows, int cols) {
    int** puzzle = create_field(rows, cols);
    if (puzzle == NULL) {
        return NULL;
    }

    // Количество черных клеток - больше для больших полей
    int black_count;
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

    // Размещаем черные клетки
    Point* blacks = (Point*)malloc(black_count * sizeof(Point));
    if (blacks == NULL) {
        perror("Ошибка выделения памяти для черных клеток");
        free_field(puzzle, rows);
        return NULL;
    }

    int* lengths = (int*)calloc(black_count, sizeof(int));
    if (lengths == NULL) {
        perror("Ошибка выделения памяти для длин линий");
        free(blacks);
        free_field(puzzle, rows);
        return NULL;
    }

    int placed = 0;
    while (placed < black_count) {
        int x = rand() % rows;
        int y = rand() % cols;

        if (puzzle[x][y] == EMPTY) {  // Проверяем EMPTY = -2
            puzzle[x][y] = BLACK;
            blacks[placed].x = x;
            blacks[placed].y = y;
            placed++;
        }
    }

    // Для каждой черной клетки рисуем линии в случайных направлениях
    for (int i = 0; i < black_count; i++) {
        // Перемешиваем направления
        int dirs[4] = { 0, 1, 2, 3 };
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
            else if (puzzle[i][j] == EMPTY) {
                puzzle[i][j] = WHITE;
            }
            else if (puzzle[i][j] > 0) {
                puzzle[i][j] = WHITE;
            }
        }
    }

    free(blacks);
    free(lengths);

    return puzzle;
}

/**
* Визуализация игрового поля в консоли с использованием ASCII-графики
* @param field - поле для отображения
* @param rows - количество строк в поле
* @param cols - количество столбцов в поле
* @return 0 при успешном выводе
*/
int print_field(int** field, int rows, int cols) {
    // Верхняя граница
    printf("+");
    for (int j = 0; j < cols; j++) {
        printf("-----+");
    }
    printf("\n");

    for (int i = 0; i < rows; i++) {
        printf("|");
        for (int j = 0; j < cols; j++) {
            if (field[i][j] == WHITE) {  // WHITE = 0
                printf("     |");
            }
            else {
                printf(" %2d  |", field[i][j]);
            }
        }
        printf("\n");
        // Граница между строками
        printf("+");
        for (int j = 0; j < cols; j++) {
            printf("-----+");
        }
        printf("\n");
    }

    return 0;
}

/**
* Сохранение игрового поля в текстовый файл
* @param field - поле для сохранения
* @param rows - количество строк в поле
* @param cols - количество столбцов в поле
* @param filename - имя файла для сохранения
* @return 0 при успешном сохранении, -4 при ошибке открытия файла
*/
int save_to_file(int** field, int rows, int cols, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Ошибка открытия файла!\n");
        return -4;
    }
    // Записываем размеры
    fprintf(file, "%d %d\n", rows, cols);
    // Записываем поле: 0 для белых, число для чёрных
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

/**
* Базовая проверка решаемости головоломки
* @param puzzle - игровое поле для проверки
* @param rows - количество строк в поле
* @param cols - количество столбцов в поле
* @return 1 если сумма чисел в черных клетках равна количеству белых клеток, 0 в противном случае
*/
int is_solvable(int** puzzle, int rows, int cols) {
    int total_white = 0;
    int total_black_numbers = 0;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (puzzle[i][j] == WHITE) {  // WHITE = 0
                total_white++;
            }
            else if (puzzle[i][j] > 0) {
                total_black_numbers += puzzle[i][j];
            }
        }
    }

    // Базовая проверка: сумма чисел должна быть > количеству белых клеток
    return (total_black_numbers == total_white);
}