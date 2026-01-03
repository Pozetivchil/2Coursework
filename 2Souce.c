#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>

#define EMPTY 0
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

Direction directions[4] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

// Функция для создания поля (сетки) размером rows x cols
int** create_field(int rows, int cols) {
    int** field = (int**)malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        field[i] = (int*)malloc(cols * sizeof(int));
        for (int j = 0; j < cols; j++) {
            field[i][j] = EMPTY;
        }
    }
    return field;
}

// Функция для копирования поля
int** copy_field(int** source, int rows, int cols) {
    int** copy = create_field(rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            copy[i][j] = source[i][j];
        }
    }
    return copy;
}

// Функция для освобождения памяти поля
void free_field(int** field, int rows) {
    for (int i = 0; i < rows; i++) {
        free(field[i]);
    }
    free(field);
}

// Проверка, валидны ли координаты (в пределах поля)
int is_valid(int x, int y, int rows, int cols) {
    return (x >= 0 && x < rows && y >= 0 && y < cols);
}

// Улучшенная функция для проверки, свободна ли клетка для линии
int is_cell_available_for_line(int** field, int x, int y, int rows, int cols) {
    if (!is_valid(x, y, rows, cols)) return 0;
    return (field[x][y] == EMPTY); // Клетка должна быть абсолютно пустой
}

// Функция для рисования линии от чёрной клетки в направлении dir
// Возвращает фактическую длину нарисованной линии
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

    // Если нет доступных клеток, возвращаем 0
    if (available_len == 0) return 0;

    // Выбираем случайную длину от 1 до доступной
    int max_len = 1 + rand() % available_len;

    // Рисуем линию
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

// Проверка, что все белые клетки покрыты
int is_fully_covered(int** field, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (field[i][j] == EMPTY) {
                return 0; // Есть непокрытая клетка
            }
        }
    }
    return 1; // Всё покрыто
}

// Улучшенная проверка корректности чисел в черных клетках
int verify_black_cells_correct(int** solution, int rows, int cols, Point* blacks, int black_count, int* lengths) {
    // Создаем копию поля для подсчета
    int** temp = copy_field(solution, rows, cols);

    // Для каждой черной клетки считаем длину ее линий
    int* actual_lengths = (int*)calloc(black_count, sizeof(int));

    // Проходим по всему полю и считаем, сколько клеток принадлежит каждой линии
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (temp[i][j] > 0 && temp[i][j] != BLACK) { // Это часть линии
                int line_id = temp[i][j];
                // Ищем, какой черной клетке принадлежит эта линия
                for (int k = 0; k < black_count; k++) {
                    if (line_id == k + 1) { // ID линий начинаются с 1
                        actual_lengths[k]++;
                        break;
                    }
                }
            }
        }
    }

    // Сравниваем с ожидаемыми длинами
    int valid = 1;
    for (int i = 0; i < black_count; i++) {
        if (actual_lengths[i] != lengths[i]) {
            valid = 0;
            break;
        }
    }

    free(actual_lengths);
    free_field(temp, rows);
    return valid;
}

// Упрощенная генерация головоломки - более надежный алгоритм
int** generate_puzzle_simple(int rows, int cols) {
    int** puzzle = create_field(rows, cols);

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
        black_count = 45 + rand() % 6; 
    }
    else if (rows * cols <= 144) {
        black_count = 68 + rand() % 6; 
    }


    // Размещаем черные клетки
    Point* blacks = (Point*)malloc(black_count * sizeof(Point));
    int* lengths = (int*)calloc(black_count, sizeof(int));

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

        // Если совсем ничего не нарисовали, рисуем хотя бы одну короткую линию
        if (lengths[i] == 0) {
            Direction dir = directions[rand() % 4];
            int line_len = draw_line(puzzle, blacks[i].x, blacks[i].y, dir, rows, cols, i + 1);
            lengths[i] = line_len;
        }
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
            else if (puzzle[i][j] > 0) { // Часть линии
                puzzle[i][j] = WHITE;
            }
        }
    }

    free(blacks);
    free(lengths);

    return puzzle;
}

// Улучшенный вывод поля на экран с ASCII-арт
void print_field(int** field, int rows, int cols) {
    // Верхняя граница
    printf("+");
    for (int j = 0; j < cols; j++) {
        printf("---+");
    }
    printf("\n");

    for (int i = 0; i < rows; i++) {
        printf("|");
        for (int j = 0; j < cols; j++) {
            if (field[i][j] == WHITE) {
                printf(" * |");
            }
            else {
                printf(" %2d|", field[i][j]);
            }
        }
        printf("\n");
        // Граница между строками
        printf("+");
        for (int j = 0; j < cols; j++) {
            printf("---+");
        }
        printf("\n");
    }
}

// Функция: запись поля в файл
void save_to_file(int** field, int rows, int cols, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Ошибка открытия файла!\n");
        return;
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
}

// Проверка базовой решаемости
int is_basically_solvable(int** puzzle, int rows, int cols) {
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

    // Базовая проверка: сумма чисел должна быть >= количеству белых клеток
    return (total_black_numbers >= total_white);
}

int main() {
    setlocale(LC_ALL, "RUS");

    srand(time(NULL));
    int rows, cols;

    printf("Генератор головоломки 'Роза ветров'\n");
    printf("===================================\n\n");
    printf("Введите размеры поля (строки и столбцы, от 3 до 12): ");
    scanf("%d %d", &rows, &cols);

    if (rows < 3 || cols < 3) {
        printf("Ошибка: минимальный размер поля 3x3\n");
        return 1;
    }

    if (rows > 12 || cols > 12) {
        printf("Ошибка: максимальный размер поля 12x12\n");
        return 1;
    }

    printf("\nГенерация 3 полей для головоломки %dx%d...\n\n", rows, cols);

    int generated = 0;
    int attempts = 0;

    while (generated < 3 && attempts < MAX_ATTEMPTS) {
        attempts++;

        int** puzzle = generate_puzzle_simple(rows, cols);
        if (puzzle != NULL) {
            if (is_basically_solvable(puzzle, rows, cols)) {
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
                        filename[strcspn(filename, "\n")] = 0;
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
        printf("Файлы сохранены в текущей директории.\n");
    }
    else {
        printf("\nНе удалось сгенерировать ни одного поля за %d попыток.\n", attempts);
        printf("Попробуйте уменьшить размеры поля.\n");
    }

    return 0;
}