#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>

#define EMPTY -2
#define BLACK -1
#define WHITE 0
#define MAX_ATTEMPTS 100000000

#define MIN_FIELD_SIZE 3
#define MAX_FIELD_SIZE 12
#define DEFAULT_FILENAME_LEN 64

typedef struct
{
    int x;
    int y;
} Point;

typedef struct
{
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

/**
* Главная функция программы
* Выполняет инициализацию, выводит шапку и запускает циклическое меню
* @return 0 при нормальном завершении программы
*/
int main()
{
    int is_running = 1;
    int menu_choice = 0;

    setlocale(LC_ALL, "RUS");
    system("chcp 1251");
    
    srand(time(NULL));

    printf("============================================================\n");
    printf("Вас приветствует программа-генератор игровых полей\n");
    printf("головоломки «Роза ветров»\n");
    printf("\n");
    printf("Назначение: генераировать 3 поля по заданным размерам,\n");
    printf("выводить их на экран и сохранять выбранные варианты в файл.\n");
    printf("\n");
    printf("Исполнитель: Григорян Э.Г.\n");
    printf("Группа: бТИИ-251\n");
    printf("Преподаватель: Минакова О.В.\n");
    printf("Год: 2025-2026\n");
    printf("============================================================\n\n");

    while (is_running)
    {
        menu_choice = show_menu();

        if (menu_choice == 1)
        {
            printf("\nВыход из программы.\n");
            is_running = 0;
        }
        else if (menu_choice == 2)
        {
            run_generator();
        }
        else
        {
            printf("\nОшибка: выберите пункт 1 или 2.\n");
        }
    }

    return 0;
}

/**
* Удаляет символы конца строки '\n' из строки
* Используется после fgets(), чтобы корректно обработать имя файла
* @param s строка для обработки
* @return 0
*/
int trim_newline(char* s)
{
    int i;

    i = 0;
    while (s[i] != '\0')
    {
        if (s[i] == '\n')
        {
            s[i] = '\0';
            break;
        }
        i++;
    }

    return 0;
}

/**
* Дочитывает ввод до символа '\n'
* Используется после scanf(), чтобы очистить буфер ввода
* @return 0
*/
int flush_line()
{
    int c;

    c = getchar();
    while (c != '\n')
    {
        c = getchar();
    }

    return 0;
}

/**
* Выводит главное меню и считывает выбор пользователя
* @return введённый пользователем пункт меню (ожидается 1 или 2)
*/
int show_menu()
{
    int choice;
    int scan_result;

    choice = 0;

    printf("\n");
    printf("Главное меню\n");
    printf("----------------------------------------\n");
    printf("1. Выйти из программы\n");
    printf("2. Генерировать игровые поля\n");
    printf("----------------------------------------\n");
    printf("Выберите пункт (1-2): ");

    scan_result = scanf("%d", &choice);
    flush_line();

    if (scan_result != 1)
    {
        choice = 0;
    }

    return choice;
}

/**
* Запускает режим генерации игровых полей
* Запрашивает размеры поля, затем формирует и сохраняет 3 поля
* После сохранения 3 полей выполняется возврат в меню
* @return 0
*/
int run_generator()
{
    int rows = 0;
    int cols = 0;
    int is_data_ok = 0;
    int generated = 0;
    int attempts = 0;

    printf("\nРежим: генерация игровых полей\n");
    printf("----------------------------------------\n");

    while (is_data_ok == 0)
    {
        printf("Введите размеры поля (строки и столбцы, от 3 до 12): ");
        scanf("%d %d", &rows, &cols);
        flush_line();

        if (rows < MIN_FIELD_SIZE || cols < MIN_FIELD_SIZE || rows > MAX_FIELD_SIZE || cols > MAX_FIELD_SIZE)
        {
            printf("Ошибка: размеры должны быть в диапазоне от 3 до 12.\n");
        }
        else
        {
            is_data_ok = 1;
        }
    }

    printf("\nПараметры приняты: %d x %d\n", rows, cols);
    printf("Начинается генерация 3 полей...\n");

    while (generated < 3 && attempts < MAX_ATTEMPTS)
    {
        int** puzzle;

        attempts++;
        puzzle = generate_puzzle(rows, cols);

        if (puzzle != NULL)
        {
            if (is_solvable(puzzle, rows, cols))
            {
                int accepted = 0;

                printf("\n========================================\n");
                printf("Поле %d из 3 (попытка %d)\n", generated + 1, attempts);
                printf("========================================\n");
                print_field(puzzle, rows, cols);

                while (accepted == 0)
                {
                    char yn;

                    printf("\nПоле подходит? (y/n): ");
                    scanf(" %c", &yn);
                    flush_line();

                    if (yn == 'n' || yn == 'N')
                    {
                        printf("Вариант отклонён. Генерация нового варианта...\n");
                        accepted = 1;
                    }
                    else if (yn == 'y' || yn == 'Y')
                    {
                        int saved = 0;

                        while (saved == 0)
                        {
                            char filename[DEFAULT_FILENAME_LEN];

                            printf("Введите имя файла (Enter — puzzle%d.txt): ", generated + 1);

                            if (fgets(filename, sizeof(filename), stdin) == NULL)
                            {
                                printf("Ошибка ввода имени файла. Попробуйте снова.\n");
                                continue;
                            }

                            if (filename[0] == '\n')
                            {
                                strcpy(filename, "puzzle0.txt");
                                filename[6] = (char)('0' + (generated + 1));
                            }
                            else
                            {
                                trim_newline(filename);
                            }

                            if (save_to_file(puzzle, rows, cols, filename) == 0)
                            {
                                saved = 1;
                            }
                            else
                            {
                                printf("Не удалось сохранить поле. Попробуйте другое имя файла.\n");
                            }
                        }

                        generated++;
                        accepted = 1;
                    }
                    else
                    {
                        printf("Ошибка: введите только y или n.\n");
                    }
                }
            }

            free_field(puzzle, rows);
        }
    }

    printf("\n----------------------------------------\n");
    printf("Сохранено полей: %d\n", generated);

    if (generated == 3)
    {
        printf("Успешно сформирован набор из 3 полей.\n");
    }
    else
    {
        printf("Сформирован неполный набор полей.\n");
    }

    printf("Возврат в меню...\n");

    return 0;
}

/**
* Создаёт динамическое поле (матрицу) заданного размера
* Все клетки инициализируются значением EMPTY
* @param rows количество строк
* @param cols количество столбцов
* @return указатель на поле (int**), либо NULL при ошибке выделения памяти
*/
int** create_field(int rows, int cols)
{
    int** field;
    int row_index;
    int col_index;

    field = (int**)malloc(rows * sizeof(int*));
    if (field == NULL)
    {
        printf("Ошибка выделения памяти для поля\n");
        return NULL;
    }

    for (row_index = 0; row_index < rows; row_index++)
    {
        field[row_index] = (int*)malloc(cols * sizeof(int));
        if (field[row_index] == NULL)
        {
            printf("Ошибка выделения памяти для строки поля\n");

            for (col_index = 0; col_index < row_index; col_index++)
            {
                free(field[col_index]);
            }

            free(field);
            return NULL;
        }

        for (col_index = 0; col_index < cols; col_index++)
        {
            field[row_index][col_index] = EMPTY;
        }
    }

    return field;
}

/**
* Освобождает память, выделенную под поле
* @param field указатель на поле
* @param rows количество строк (сколько строк освобождать)
* @return 0
*/
int free_field(int** field, int rows)
{
    if (field == NULL)
    {
        return 0;
    }

    for (int i = 0; i < rows; i++)
    {
        if (field[i] != NULL)
        {
            free(field[i]);
        }
    }

    free(field);
    return 0;
}

/**
* Проверяет, что координаты клетки находятся в пределах поля
* @param x индекс строки
* @param y индекс столбца
* @param rows количество строк
* @param cols количество столбцов
* @return 1 если координаты корректны, 0 если выходят за границы
*/
int is_valid(int x, int y, int rows, int cols)
{
    return (x >= 0 && x < rows && y >= 0 && y < cols);
}

/**
* Проверяет, свободна ли клетка для продолжения линии
* @param field игровое поле
* @param x индекс строки
* @param y индекс столбца
* @param rows количество строк
* @param cols количество столбцов
* @return 1 если клетка валидна и EMPTY, иначе 0
*/
int is_cell_available_for_line(int** field, int x, int y, int rows, int cols)
{
    if (!is_valid(x, y, rows, cols))
    {
        return 0;
    }

    if (field[x][y] == EMPTY)
    {
        return 1;
    }

    return 0;
}

/**
* Проводит линию от чёрной клетки в заданном направлении по пустым клеткам
* Длина линии выбирается случайно в пределах доступной длины
* Клетки линии помечаются значением id
* @param field игровое поле
* @param x индекс строки стартовой (чёрной) клетки
* @param y индекс столбца стартовой (чёрной) клетки
* @param dir направление (dx, dy)
* @param rows количество строк
* @param cols количество столбцов
* @param id идентификатор линии (число, которым помечаются клетки линии)
* @return длина проведённой линии (количество закрашенных клеток), либо 0 если провести линию нельзя
*/
int draw_line(int** field, int x, int y, Direction dir, int rows, int cols, int id)
{
    int len;
    int cx;
    int cy;
    int available_len;
    int max_len;
    
    len = 0;

    cx = x + dir.dx;
    cy = y + dir.dy;

    available_len = 0;

    while (is_cell_available_for_line(field, cx, cy, rows, cols))
    {
        available_len++;
        cx += dir.dx;
        cy += dir.dy;
    }

    if (available_len == 0)
    {
        return 0;
    }

    max_len = 1 + rand() % available_len;

    cx = x + dir.dx;
    cy = y + dir.dy;

    for (int i = 0; i < max_len; i++)
    {
        field[cx][cy] = id;
        len++;

        cx += dir.dx;
        cy += dir.dy;
    }

    return len;
}

/**
* Проверяет, что поле полностью покрыто (не осталось EMPTY клеток)
* @param field игровое поле
* @param rows количество строк
* @param cols количество столбцов
* @return 1 если пустых клеток нет, 0 если есть хотя бы одна EMPTY
*/
int is_fully_covered(int** field, int rows, int cols)
{
    for (int row_index = 0; row_index < rows; row_index++)
    {
        for (int col_index = 0; col_index < cols; col_index++)
        {
            if (field[row_index][col_index] == EMPTY)
            {
                return 0;
            }
        }
    }

    return 1;
}

/**
* Генерирует одно игровое поле головоломки «Роза ветров»
* 1) создаёт пустое поле
* 2) размещает чёрные клетки
* 3) пытается провести линии от каждой чёрной клетки
* 4) проверяет покрытие поля
* 5) преобразует поле в формат: WHITE (0) и числа в чёрных клетках
* @param rows количество строк
* @param cols количество столбцов
* @return указатель на сгенерированное поле, либо NULL если генерация не удалась
*/
int** generate_puzzle(int rows, int cols)
{
    Direction directions_local[4] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
    int** puzzle;
    int black_count;
    int area;
    Point* blacks;
    int* lengths;
    int placed;

    puzzle = create_field(rows, cols);
    if (puzzle == NULL)
    {
        return NULL;
    }

    black_count = 0;
    area = rows * cols;

    if (area <= 25)
    {
        black_count = 3 + rand() % 3;
    }
    else if (area <= 64)
    {
        black_count = 9 + rand() % 4;
    }
    else if (area <= 81)
    {
        black_count = 15 + rand() % 5;
    }
    else if (area <= 100)
    {
        black_count = 36 + rand() % 6;
    }
    else if (area <= 121)
    {
        black_count = 45 + rand() % 7;
    }
    else if (area <= 144)
    {
        black_count = 56 + rand() % 8;
    }
    else
    {
        black_count = 3;
    }

    blacks = (Point*)malloc((size_t)black_count * sizeof(Point));
    if (blacks == NULL)
    {
        printf("Ошибка выделения памяти для черных клеток\n");
        free_field(puzzle, rows);
        return NULL;
    }

    lengths = (int*)calloc((size_t)black_count, sizeof(int));
    if (lengths == NULL)
    {
        printf("Ошибка выделения памяти для длин линий\n");
        free(blacks);
        free_field(puzzle, rows);
        return NULL;
    }

    placed = 0;

    while (placed < black_count)
    {
        int x;
        int y;

        x = rand() % rows;
        y = rand() % cols;

        if (puzzle[x][y] == EMPTY)
        {
            puzzle[x][y] = BLACK;
            blacks[placed].x = x;
            blacks[placed].y = y;
            placed++;
        }
    }

    for (int i = 0; i < black_count; i++)
    {
        int dirs[4] = { 0, 1, 2, 3 };
        int temp;
        
        for (int j = 0; j < 4; j++)
        {
            int k = rand() % 4;
            temp = dirs[j];
            dirs[j] = dirs[k];
            dirs[k] = temp;
        }

        for (int d = 0; d < 4; d++)
        {
            Direction dir;
            int line_len;

            dir = directions_local[dirs[d]];
            line_len = draw_line(puzzle, blacks[i].x, blacks[i].y, dir, rows, cols, i + 1);
            lengths[i] += line_len;
        }

        if (lengths[i] <= 0)
        {
            free(blacks);
            free(lengths);
            free_field(puzzle, rows);
            return NULL;
        }
    }

    if (!is_fully_covered(puzzle, rows, cols))
    {
        free(blacks);
        free(lengths);
        free_field(puzzle, rows);
        return NULL;
    }

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (puzzle[i][j] == BLACK)
            {
                for (int k = 0; k < black_count; k++)
                {
                    if (blacks[k].x == i && blacks[k].y == j)
                    {
                        puzzle[i][j] = lengths[k];
                        break;
                    }
                }
            }
            else
            {
                puzzle[i][j] = WHITE;
            }
        }
    }

    free(blacks);
    free(lengths);

    return puzzle;
}

/**
* Выводит игровое поле в консоль в виде ASCII-таблицы
* WHITE выводится как пустая клетка, числа выводятся в чёрных клетках
* @param field игровое поле
* @param rows количество строк
* @param cols количество столбцов
* @return 0
*/
int print_field(int** field, int rows, int cols)
{
    printf("+");
    for (int j = 0; j < cols; j++)
    {
        printf("-----+");
    }
    printf("\n");

    for (int i = 0; i < rows; i++)
    {
        printf("|");
        for (int j = 0; j < cols; j++)
        {
            if (field[i][j] == WHITE)
            {
                printf("     |");
            }
            else
            {
                printf(" %2d  |", field[i][j]);
            }
        }
        printf("\n");

        printf("+");
        for (int j = 0; j < cols; j++)
        {
            printf("-----+");
        }
        printf("\n");
    }

    return 0;
}

/**
* Сохраняет поле в текстовый файл
* Формат: первая строка "rows cols", далее rows строк по cols чисел
* @param field игровое поле
* @param rows количество строк
* @param cols количество столбцов
* @param filename имя файла
* @return 0 при успешном сохранении, -4 если файл открыть не удалось
*/
int save_to_file(int** field, int rows, int cols, char* filename)
{
    FILE* file;

    file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Ошибка открытия файла!\n");
        return -4;
    }

    fprintf(file, "%d %d\n", rows, cols);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            fprintf(file, "%d ", field[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);

    printf("Поле сохранено в %s\n", filename);
    return 0;
}

/**
* Выполняет проверку верности поля
* Проверка: сумма чисел в чёрных клетках равна количеству белых клеток (WHITE)
* @param puzzle игровое поле
* @param rows количество строк
* @param cols количество столбцов
* @return 1 если проверка пройдена, 0 если проверка не пройдена
*/
int is_solvable(int** puzzle, int rows, int cols)
{
    int total_white;
    int total_black_numbers;

    total_white = 0;
    total_black_numbers = 0;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (puzzle[i][j] == WHITE)
            {
                total_white++;
            }
            else if (puzzle[i][j] > 0)
            {
                total_black_numbers += puzzle[i][j];
            }
        }
    }

    if (total_black_numbers == total_white)
    {
        return 1;
    }

    return 0;
}
