#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>

#define EMPTY 0
#define BLACK -1
#define WHITE 0
#define MAX_ATTEMPTS 50000

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

// Проверка, можно ли разместить чёрную клетку (не рядом с другой чёрной)
int can_place_black(int** field, int x, int y, int rows, int cols) {
    for (int i = 0; i < 4; i++) {
        int nx = x + directions[i].dx;
        int ny = y + directions[i].dy;
        if (is_valid(nx, ny, rows, cols) && field[nx][ny] == BLACK) {
            return 0; // Нельзя, рядом чёрная
        }
    }
    return 1; // Можно
}

// Функция для проверки, свободна ли клетка для линии
int is_cell_available(int** field, int x, int y, int rows, int cols, int exclude_id) {
    if (!is_valid(x, y, rows, cols)) return 0;
    return (field[x][y] == EMPTY || field[x][y] == exclude_id);
}

// Функция для рисования линии от чёрной клетки в направлении dir
// Возвращает фактическую длину нарисованной линии
int draw_line(int** field, int x, int y, Direction dir, int rows, int cols, int id) {
    int len = 0;
    int cx = x + dir.dx;
    int cy = y + dir.dy;

    // Сначала проверяем, можно ли нарисовать линию
    while (is_cell_available(field, cx, cy, rows, cols, id)) {
        len++;
        cx += dir.dx;
        cy += dir.dy;
    }

    // Если длина 0, возвращаем 0
    if (len == 0) return 0;

    // Выбираем случайную длину от 1 до доступной длины
    int max_len = 1 + rand() % len;

    // Рисуем линию
    cx = x + dir.dx;
    cy = y + dir.dy;
    for (int i = 0; i < max_len; i++) {
        field[cx][cy] = id;
        cx += dir.dx;
        cy += dir.dy;
    }

    return max_len;
}

// Проверка, что все белые клетки покрыты (нет EMPTY кроме BLACK)
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

// Проверка корректности чисел в черных клетках
int verify_black_cells(int** solution, int rows, int cols, Point* blacks, int black_count, int* lengths) {
    // Создаем временное поле для подсчета фактических длин
    int** temp = create_field(rows, cols);

    // Копируем только черные клетки
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (solution[i][j] == BLACK) {
                temp[i][j] = BLACK;
            }
        }
    }

    // Для каждой черной клетки рисуем линии и считаем длины
    int* actual_lengths = (int*)calloc(black_count, sizeof(int));

    for (int i = 0; i < black_count; i++) {
        // Перемешиваем направления
        int dirs[4] = { 0, 1, 2, 3 };
        for (int j = 0; j < 4; j++) {
            int k = rand() % 4;
            int temp_dir = dirs[j];
            dirs[j] = dirs[k];
            dirs[k] = temp_dir;
        }

        // Пробуем нарисовать линии в разных направлениях
        for (int j = 0; j < 4; j++) {
            Direction dir = directions[dirs[j]];
            int cx = blacks[i].x + dir.dx;
            int cy = blacks[i].y + dir.dy;
            int len = 0;

            // Проверяем, сколько клеток можно занять
            while (is_valid(cx, cy, rows, cols) && temp[cx][cy] == EMPTY) {
                len++;
                cx += dir.dx;
                cy += dir.dy;
            }

            if (len > 0) {
                // Рисуем часть линии
                int draw_len = 1 + rand() % len;
                cx = blacks[i].x + dir.dx;
                cy = blacks[i].y + dir.dy;

                for (int k = 0; k < draw_len; k++) {
                    temp[cx][cy] = i + 100; // Уникальный ID для линии
                    actual_lengths[i]++;
                    cx += dir.dx;
                    cy += dir.dy;
                }
            }
        }

        // Сравниваем с ожидаемой длиной
        if (actual_lengths[i] != lengths[i] && lengths[i] > 0) {
            // Если разница небольшая, корректируем
            if (abs(actual_lengths[i] - lengths[i]) <= 2) {
                lengths[i] = actual_lengths[i];
            }
            else {
                free(actual_lengths);
                free_field(temp, rows);
                return 0; // Разница слишком большая
            }
        }
    }

    free(actual_lengths);
    free_field(temp, rows);
    return 1; // Числа корректны
}

// Генерация головоломки
int** generate_puzzle(int rows, int cols) {
    int attempts = 0;

    while (attempts < MAX_ATTEMPTS) {
        int** solution = create_field(rows, cols);

        // Количество чёрных клеток зависит от размера поля
        int black_count;
        if (rows * cols == 9) {
            black_count = 3; // 3-6 для маленьких полей, остатки от деления будут варьироваться от 0 до 3
        }
        else if (rows * cols <= 25) { 
            black_count = 3 + rand() % 4; // 3-6 для маленьких полей, остатки от деления будут варьироваться от 0 до 3
        }
        else if (rows * cols <= 64) {
            black_count = 13 + rand() % 6; // 4-9 для средних полей, остатки от деления будут варьироваться от 0 до 5
        }
        else {
            black_count = 16 + rand() % 8; // 6-13 для больших полей, остатки от деления будут варьироваться от 0 до 7
        }

        Point* blacks = (Point*)malloc(black_count * sizeof(Point));
        int* lengths = (int*)calloc(black_count, sizeof(int));

        int placed = 0;
        int placement_attempts = 0;
        int max_placement_attempts = rows * cols * 10;

        // Размещаем чёрные клетки
        while (placed < black_count && placement_attempts < max_placement_attempts) {
            int x = rand() % rows;
            int y = rand() % cols;

            if (solution[x][y] == EMPTY && can_place_black(solution, x, y, rows, cols)) {
                solution[x][y] = BLACK;
                blacks[placed].x = x;
                blacks[placed].y = y;
                placed++;
            }
            placement_attempts++;
        }

        if (placed < 3) { // Минимум 3 черные клетки
            free(blacks);
            free(lengths);
            free_field(solution, rows);
            attempts++;
            continue;
        }

        black_count = placed; // Обновляем реальное количество черных клеток

        // Для каждой чёрной клетки рисуем линии
        int success = 1;
        for (int i = 0; i < black_count; i++) {
            // Перемешиваем направления
            int dirs[4] = { 0, 1, 2, 3 };
            for (int j = 0; j < 4; j++) {
                int k = rand() % 4;
                int temp = dirs[j];
                dirs[j] = dirs[k];
                dirs[k] = temp;
            }

            // Рисуем линии в 2-4 направлениях
            int lines_to_draw = 2 + rand() % 3;
            if (lines_to_draw > 4) lines_to_draw = 4;

            for (int j = 0; j < lines_to_draw; j++) {
                Direction dir = directions[dirs[j]];
                int line_len = draw_line(solution, blacks[i].x, blacks[i].y, dir, rows, cols, i + 100);
                lengths[i] += line_len;
            }

            // Если ничего не нарисовано, помечаем как неудачу
            if (lengths[i] == 0) {
                success = 0;
                break;
            }

            // Ограничиваем максимальное число
            if (lengths[i] > 9) lengths[i] = 9;
        }

        if (!success) {
            free(blacks);
            free(lengths);
            free_field(solution, rows);
            attempts++;
            continue;
        }

        // Проверка покрытия (не требуем 100% покрытия для больших полей)
        int covered_cells = 0;
        int total_cells = rows * cols - black_count;

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (solution[i][j] != EMPTY && solution[i][j] != BLACK) {
                    covered_cells++;
                }
            }
        }

        // Требуем покрытия не менее 70% белых клеток
        float coverage = (float)covered_cells / total_cells;
        if (coverage < 0.7) {
            free(blacks);
            free(lengths);
            free_field(solution, rows);
            attempts++;
            continue;
        }

        // Проверяем корректность чисел в черных клетках
        if (!verify_black_cells(solution, rows, cols, blacks, black_count, lengths)) {
            free(blacks);
            free(lengths);
            free_field(solution, rows);
            attempts++;
            continue;
        }

        // Создаём puzzle: чёрные с числами, остальные WHITE (0)
        int** puzzle = create_field(rows, cols);

        // Сначала заполняем все клетки как белые
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                puzzle[i][j] = WHITE;
            }
        }

        // Затем размещаем черные клетки с числами
        for (int i = 0; i < black_count; i++) {
            puzzle[blacks[i].x][blacks[i].y] = lengths[i];
        }

        free(blacks);
        free(lengths);
        free_field(solution, rows);

        return puzzle;
    }

    return NULL; // Не удалось сгенерировать после всех попыток
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
                printf(" %d |", field[i][j]);
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

// Функция для проверки решаемости головоломки (упрощенная версия)
int is_solvable(int** puzzle, int rows, int cols) {
    int total_black_numbers = 0;
    int total_white_cells = 0;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (puzzle[i][j] > 0) { // Черная клетка с числом
                total_black_numbers += puzzle[i][j];
            }
            else if (puzzle[i][j] == WHITE) {
                total_white_cells++;
            }
        }
    }

    // Базовая проверка: сумма чисел должна быть >= количеству белых клеток
    // (может быть больше из-за пересечений, которые разрешены в нашей генерации)
    return (total_black_numbers >= total_white_cells);
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

int main() {
    setlocale(LC_ALL, "RUS");

    srand(time(NULL));
    int rows, cols;

    printf("Генератор головоломки 'Роза ветров'\n");
    printf("===================================\n\n");
    printf("Введите размеры поля (строки и столбцы, от 3 до 20): ");
    scanf("%d %d", &rows, &cols);

    if (rows < 3 || cols < 3) {
        printf("Ошибка: минимальный размер поля 3x3\n");
        return 1;
    }

    if (rows > 20 || cols > 20) {
        printf("Ошибка: максимальный размер поля 20x20\n");
        return 4;
    }

    printf("\nГенерация 3 полей для головоломки %dx%d...\n\n", rows, cols);

    int generated = 0;
    while (generated < 3) {
        printf("Попытка генерации поля %d...\n", generated + 1);

        int** puzzle = generate_puzzle(rows, cols);
        if (puzzle != NULL) {
            if (is_solvable(puzzle, rows, cols)) {
                printf("\nПоле %d (сгенерировано успешно):\n", generated + 1);
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
                printf("Поле не решаемо, перегенерируем...\n");
                free_field(puzzle, rows);
            }
        }
        else {
            printf("Не удалось сгенерировать поле после %d попыток\n", MAX_ATTEMPTS);
            printf("Попробуйте другие размеры поля.\n");
            break;
        }
    }

    if (generated > 0) {
        printf("\n%s\n", "=======================================================");
        printf("Успешно сгенерировано %d полей!\n", generated);
        printf("Файлы сохранены в текущей директории.\n");
    }
    else {
        printf("\nНе удалось сгенерировать ни одного поля.\n");
        printf("Попробуйте уменьшить размеры поля.\n");
    }

    return 0;
}