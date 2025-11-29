#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <windows.h>

#define MAP_SIZE 23
#define MAX_NAME 32
#define INITIAL_SHOP_ITEMS 5
#define EXTRA_JUNK 8
#define PANEL_WIDTH 36

typedef struct ItemNode {
    char name[MAX_NAME];
    struct ItemNode *next;
    int collected;
} ItemNode;

typedef struct WorldItem {
    char name[MAX_NAME];
    int x, y;
    int collected;
} WorldItem;

/* ---------- Lista encadeada ---------- */
ItemNode* add_item(ItemNode *head, const char *name) {
    ItemNode *node = malloc(sizeof(ItemNode));
    if (!node) { perror("malloc"); exit(1); }
    strncpy(node->name, name, MAX_NAME-1);
    node->name[MAX_NAME-1] = '\0';
    node->collected = 0;
    node->next = head;
    return node;
}

void mark_item_collected(ItemNode *head, const char *name) {
    ItemNode *cur = head;
    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            cur->collected = 1;
            break;
        }
        cur = cur->next;
    }
}

// NOVA FUNÇÃO: Verifica se o item já foi coletado
int is_item_already_collected(ItemNode *head, const char *name) {
    ItemNode *cur = head;
    while (cur) {
        if (strcmp(cur->name, name) == 0 && cur->collected) {
            return 1; // Já foi coletado
        }
        cur = cur->next;
    }
    return 0; // Não foi coletado ou não está na lista
}

int contains_item(ItemNode *head, const char *name) {
    ItemNode *cur = head;
    while (cur) {
        if (strcmp(cur->name, name) == 0) return 1;
        cur = cur->next;
    }
    return 0;
}

void print_list_console(ItemNode *head) {
    if (!head) {
        printf("(Lista vazia)\n");
        return;
    }
    printf("Lista de compras:\n");
    ItemNode *cur = head;
    int i = 1;
    while (cur) {
        if (cur->collected) {
            printf(" %d) [X] %s\n", i++, cur->name);
        } else {
            printf(" %d) [ ] %s\n", i++, cur->name);
        }
        cur = cur->next;
    }
}

void free_list(ItemNode *head) {
    ItemNode *cur = head;
    while (cur) {
        ItemNode *n = cur->next;
        free(cur);
        cur = n;
    }
}

/* ---------- itens ---------- */
const char *example_items[] = {
    "mosca enlatada", "sopa de inseto", "suco de flor",
    "pao de petala", "geleia de orvalho", "cha de folha",
    "bolacha de semente", "iogurte de nectar", "salada de grama",
    "agua de chuva", "mel de abelha", "fruta silvestre",
    "tempero de terra", "ervas finas", "raizes crocantes"
};
#define TOTAL_ITEMS (sizeof(example_items)/sizeof(example_items[0]))

/* ---------- utilitarios ---------- */
void clear_screen() { system("cls"); }

void wait_enter() {
    char buf[64];
    fgets(buf, sizeof(buf), stdin);
}

void centralizar_linha(const char *texto, int largura_total) {
    int len = (int)strlen(texto);
    int espacos = (largura_total - len) / 2;
    if (espacos < 0) espacos = 0;
    for (int i = 0; i < espacos; i++) printf(" ");
    printf("%s", texto);
}

void exibir_arquivo_centralizado(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("Arquivo %s nao encontrado.\n", filename);
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;
        centralizar_linha(line, 100);
        printf("\n");
    }
    fclose(f);
}

/* ---------- menu inicial ---------- */
int initMenu(int initialLives) {
    exibir_arquivo_centralizado("menu.txt");
    printf("\n\n");
    char buf[16];
    while (1) {
        if (!fgets(buf, sizeof(buf), stdin)) return 1;
        int key = atoi(buf);
        if (key == 1) {
            printf("\n");
            return 0;
        } else {
            printf("\n");
        }
    }
}

/* ---------- painel ---------- */
void place_items_on_map(char map[MAP_SIZE][MAP_SIZE], WorldItem items[], int item_count) {
    for (int i = 0; i < item_count; i++) {
        if (items[i].collected) continue;
        map[items[i].y][items[i].x] = 'A' + (i % 26);
    }
}

int world_item_at(WorldItem items[], int item_count, int x, int y) {
    for (int i = 0; i < item_count; i++) {
        if (!items[i].collected && items[i].x == x && items[i].y == y) return i;
    }
    return -1;
}

int is_shopping_done(ItemNode *head) {
    ItemNode *cur = head;
    while (cur) {
        if (!cur->collected) return 0;
        cur = cur->next;
    }
    return 1;
}

void build_panel_lines(char panel_lines[MAP_SIZE][PANEL_WIDTH+1], WorldItem world[], int world_count,
                       ItemNode *shopping, int lives, int moves, int score) {
    for (int i = 0; i < MAP_SIZE; i++) panel_lines[i][0] = '\0';

    int line = 0;
    snprintf(panel_lines[line++], PANEL_WIDTH, "  Legenda:");

    // Mostrar apenas os itens que estão no mundo atual
    for (int i = 0; i < world_count && line < MAP_SIZE - 7; i++) {
        char mark = 'A' + (i % 26);

        if (world[i].collected) {
            snprintf(panel_lines[line++], PANEL_WIDTH, "  %c => (coletado)", mark);
        } else {
            snprintf(panel_lines[line++], PANEL_WIDTH, "  %c => %s", mark, world[i].name);
        }
    }

    if (line < MAP_SIZE) snprintf(panel_lines[line++], PANEL_WIDTH, " ");
    if (line < MAP_SIZE) snprintf(panel_lines[line++], PANEL_WIDTH, "  Vidas: %d", lives);
    if (line < MAP_SIZE) snprintf(panel_lines[line++], PANEL_WIDTH, " ");
    if (line < MAP_SIZE) snprintf(panel_lines[line++], PANEL_WIDTH, "  Movimentos: %d", moves);
    if (line < MAP_SIZE) snprintf(panel_lines[line++], PANEL_WIDTH, "  Itens coletados: %d", score);

    while (line < MAP_SIZE) panel_lines[line++][0] = '\0';
}

void print_map_with_panel(char map[MAP_SIZE][MAP_SIZE], WorldItem world[], int world_count,
                          ItemNode *shopping, int lives, int moves, int score) {
    char panel_lines[MAP_SIZE][PANEL_WIDTH+1];
    build_panel_lines(panel_lines, world, world_count, shopping, lives, moves, score);

    int margem_esquerda = 10;

    for (int y = 0; y < MAP_SIZE; y++) {
        for (int i = 0; i < margem_esquerda; i++) printf(" ");

        for (int x = 0; x < MAP_SIZE; x++) {
            printf("%c ", map[y][x]);
        }

        printf("   ");
        printf("%s", panel_lines[y]);
        printf("\n");
    }
}

/* ---------- main ---------- */
int main(void) {
    setlocale(LC_ALL, "Portuguese_Brazil.1252");
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    const int initialLives = 2;
    initMenu(initialLives);

    srand((unsigned)time(NULL));

    ItemNode *shopping = NULL;

    // Criar lista de compras aleatória SEM REPETIÇÕES
    int used_indices[TOTAL_ITEMS] = {0};
    char *shopping_items[INITIAL_SHOP_ITEMS]; // Array para armazenar os itens da lista

    for (int i = 0; i < INITIAL_SHOP_ITEMS; i++) {
        int index;
        do {
            index = rand() % TOTAL_ITEMS;
        } while (used_indices[index]); // Evitar repetições na lista

        used_indices[index] = 1;
        shopping_items[i] = (char*)example_items[index];
        shopping = add_item(shopping, example_items[index]);
    }

    int world_count = INITIAL_SHOP_ITEMS + EXTRA_JUNK;
    WorldItem *world = malloc(sizeof(WorldItem) * world_count);
    if (!world) { perror("malloc"); exit(1); }

    // Distribuir os itens da lista aleatoriamente no mundo
    int world_index = 0;

    // 1. Primeiro colocar todos os itens da lista de compras no mundo em posições aleatórias
    int shopping_positions[INITIAL_SHOP_ITEMS];
    for (int i = 0; i < INITIAL_SHOP_ITEMS; i++) {
        shopping_positions[i] = -1; // Inicializar como não definido
    }

    // Escolher posições aleatórias no mundo para os itens da lista
    for (int i = 0; i < INITIAL_SHOP_ITEMS; i++) {
        int pos;
        do {
            pos = rand() % world_count;
        } while (shopping_positions[pos % INITIAL_SHOP_ITEMS] != -1);

        shopping_positions[pos % INITIAL_SHOP_ITEMS] = pos;
    }

    // Colocar os itens da lista nas posições escolhidas
    for (int i = 0; i < INITIAL_SHOP_ITEMS; i++) {
        if (shopping_positions[i] != -1) {
            strncpy(world[shopping_positions[i]].name, shopping_items[i], MAX_NAME-1);
            world[shopping_positions[i]].name[MAX_NAME-1] = '\0';
            world[shopping_positions[i]].collected = 0;
        }
    }

    // 2. Preencher o resto do mundo com itens extras aleatórios (PERMITINDO REPETIDOS)
    for (int i = 0; i < world_count; i++) {
        // Se esta posição não foi preenchida com item da lista, preencher com item extra
        int is_shopping_item = 0;
        for (int j = 0; j < INITIAL_SHOP_ITEMS; j++) {
            if (shopping_positions[j] == i) {
                is_shopping_item = 1;
                break;
            }
        }

        if (!is_shopping_item) {
            // PERMITE REPETIÇÕES - escolhe qualquer item aleatório
            const char *nm = example_items[rand() % TOTAL_ITEMS];
            strncpy(world[i].name, nm, MAX_NAME-1);
            world[i].name[MAX_NAME-1] = '\0';
            world[i].collected = 0;
        }
    }

    char map[MAP_SIZE][MAP_SIZE];
    for (int y=0;y<MAP_SIZE;y++) for (int x=0;x<MAP_SIZE;x++) map[y][x] = '.';

    for (int i = 0; i < world_count; i++) {
        int x,y;
        do {
            x = rand() % MAP_SIZE;
            y = rand() % MAP_SIZE;
        } while (map[y][x] != '.');
        world[i].x = x;
        world[i].y = y;
        map[y][x] = '?';
    }

    // Resto do código do jogo
    int sx = MAP_SIZE/2, sy = MAP_SIZE/2;
    char input[64];
    int moves = 0;
    int score = 0;
    int lives = initialLives;

    while (1) {
        for (int y=0;y<MAP_SIZE;y++) for (int x=0;x<MAP_SIZE;x++) map[y][x] = '.';
        for (int i = 0; i < world_count; i++)
            if (world[i].collected) map[world[i].y][world[i].x] = 'X';

        place_items_on_map(map, world, world_count);
        map[sy][sx] = '@';

        clear_screen();

        centralizar_linha("=== Aranha Compradeira ===", 100); printf("\n");
        centralizar_linha("Objetivo: pegar todos os itens da lista de compras.", 100); printf("\n");
        centralizar_linha("Controles: w/a/s/d + Enter (mover), p = lista, q = sair", 100); printf("\n\n");

        print_map_with_panel(map, world, world_count, shopping, lives, moves, score);

        if (lives <= 0) {
            printf("\n");
            clear_screen();
            exibir_arquivo_centralizado("gameover.txt");
            printf("\n");
            centralizar_linha("Voce perdeu todas as suas vidas!", 100);
            printf("\n");
            break;
        }

        printf("\n");
        centralizar_linha("Digite comando: ", 100);
        if (!fgets(input, sizeof(input), stdin)) break;
        char cmd = input[0];

        if (cmd == 'q') {
            centralizar_linha("Saindo... ate a proxima!", 100); printf("\n");
            break;
        } else if (cmd == 'p') {
            printf("\n");
            print_list_console(shopping);
            printf("\nPressione Enter para continuar...");
            wait_enter();
            continue;
        } else if (cmd == 'w' || cmd == 'a' || cmd == 's' || cmd == 'd') {
            int nx = sx, ny = sy;
            if (cmd == 'w' && sy > 0) ny--;
            if (cmd == 's' && sy < MAP_SIZE-1) ny++;
            if (cmd == 'a' && sx > 0) nx--;
            if (cmd == 'd' && sx < MAP_SIZE-1) nx++;

            if (nx == sx && ny == sy) { moves++; continue; }

            sx = nx; sy = ny;
            moves++;

            int wi = world_item_at(world, world_count, sx, sy);
            if (wi >= 0) {
                printf("\n");
                centralizar_linha("Voce encontrou:", 100); printf("\n");
                centralizar_linha(world[wi].name, 100); printf("\n");

                // CORREÇÃO: Verificar primeiro se o item JÁ FOI COLETADO
                if (contains_item(shopping, world[wi].name)) {
                    if (is_item_already_collected(shopping, world[wi].name)) {
                        centralizar_linha("-> VOCE JA COLETOU ESTE ITEM! Perdeu 1 vida.", 100); printf("\n");
                        lives--;
                        world[wi].collected = 1;
                        printf("\n");
                        centralizar_linha("Vidas restantes: ", 100); printf("%d\n", lives);
                    } else {
                        mark_item_collected(shopping, world[wi].name);
                        centralizar_linha("-> Item da lista! Coletado com sucesso!", 100); printf("\n");
                        score++;
                        world[wi].collected = 1;
                    }
                } else {
                    centralizar_linha("-> Nao estava na lista. Voce perdeu 1 vida.", 100); printf("\n");
                    lives--;
                    world[wi].collected = 1;
                    printf("\n");
                    centralizar_linha("Vidas restantes: ", 100); printf("%d\n", lives);
                }
                centralizar_linha("Pressione Enter para continuar...", 100);
                wait_enter();
            }

            if (is_shopping_done(shopping)) {
                clear_screen();
                exibir_arquivo_centralizado("win.txt");
                printf("\n");
                centralizar_linha("PARABENS! Voce coletou todos os itens da lista!", 100); printf("\n");
                centralizar_linha("Movimentos: ", 100); printf("%d\n", moves);
                centralizar_linha("Itens coletados: ", 100); printf("%d\n", score);
                centralizar_linha("Vidas restantes: ", 100); printf("%d\n", lives);
                break;
            }
        } else {
            printf("\n");
            centralizar_linha("Comando desconhecido. Use w/a/s/d, p, q.", 100); printf("\n");
            centralizar_linha("Pressione Enter...", 100);
            wait_enter();
            continue;
        }
    }

    free_list(shopping);
    free(world);
    return 0;
}
