// jogo_final_integrado.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <math.h>
#include <stdbool.h>
#include <string.h> // Necessário para memset

// Definições de Tela
#define LARGURA_TELA 1280
#define ALTURA_TELA 720

// Definição dos Estados do Jogo
typedef enum {
    TELA_MENU = 0,
    TELA_JOGO = 1,
    TELA_TUTORIAL = 2,
    TELA_QUARTO = 3,
    TELA_FIM_DIA = 4,
    TELA_MERCADO = 6,
    TELA_CASSINO = 7,
    TELA_BANCO = 8,
    TELA_SAIR = 9
} EstadoDoJogo;

// =================================================================
// ESTRUTURAS DE COLISÃO / NPC
// =================================================================

#define MAX_NPCS 5
typedef struct {
    int id;
    int x;
    int y;
    int raio;
} NPC;
typedef struct {
    int x1, y1;
    int x2, y2;
} CoordenadasBotao;

// =================================================================
// ESTRUTURAS E DADOS DE INVESTIMENTO
// =================================================================
#define MAX_INVESTMENTS 3

typedef struct {
    int id;
    const char* nome;
    float taxa_juros;
    int dias_duracao;
    int custo_minimo;
} InvestmentOption;

typedef struct {
    int option_id;
    int principal;
    int start_day;
    bool active;
} ActiveInvestment;

InvestmentOption INVESTMENT_OPTIONS[] = {
    {1, "Curto Prazo (+20%% | 2 dias)", 0.20f, 2, 100},
    {2, "Medio Prazo (+40%% | 4 dias)", 0.40f, 4, 150},
    {3, "Alto Risco (+75%% | 7 dias)", 0.75f, 7, 200}
};
ActiveInvestment active_investments[MAX_INVESTMENTS] = { 0 };

// =================================================================
// ESTRUTURAS E DADOS DE COMIDA (NOVAS)
// =================================================================
#define MAX_FOOD_ITEMS 8

typedef struct {
    const char* nome;
    int custo;
    float ganho_fome;
} FoodItem;

FoodItem FOOD_OPTIONS[MAX_FOOD_ITEMS] = {
    {"Pães",          10,                 5.0f},
    {"Berinjela",     20,                 15.0f},
    {"Tomate",        30,                 10.0f},
    {"Uva",           40,                 20.0f},
    {"Queijo",        50,                 30.0f},
    {"Alcaparras",    70,                 25.0f},
    {"Peixe",         100,                50.0f},
    {"Figo",          120,                40.0f}
};

// Mapeamento dinâmico dos botões de comida (coordenadas serão calculadas no desenho)
CoordenadasBotao FOOD_BTN[MAX_FOOD_ITEMS];


// =================================================================
// VARIÁVEIS GLOBAIS
// =================================================================
EstadoDoJogo estado_atual = TELA_MENU;
int current_npc_id = -1;
bool can_interact = false;
int quarto_interact_id = -1;

// Bitmaps
ALLEGRO_BITMAP* img_menu_fundo = NULL;
ALLEGRO_BITMAP* img_mapa_fundo = NULL;
ALLEGRO_BITMAP* img_tutorial_fundo = NULL;
ALLEGRO_BITMAP* img_quarto_fundo = NULL;
ALLEGRO_BITMAP* img_mercado_fundo = NULL;
ALLEGRO_BITMAP* img_cassino_fundo = NULL;
ALLEGRO_BITMAP* img_banco_fundo = NULL;
ALLEGRO_BITMAP* img_fim_dia_fundo = NULL;
ALLEGRO_BITMAP* img_player_sprite = NULL;

// Variáveis do Jogador
float player_pos_x = LARGURA_TELA / 2.0;
float player_pos_y = ALTURA_TELA / 2.0;
float player_velocidade = 7.0f;
int PLAYER_RAIO = 50;
float anim_frame = 0.f;
int anim_current_frame_y = 90 * 2;
const int FRAME_LARGURA = 95;
const int FRAME_ALTURA = 180;

// NPCs
NPC NPC_LIST[MAX_NPCS] = {
    { 1, 250, 100, 30 },
    { 2, 650, 100, 30 },
    { 3, 1050, 100, 30 },
    { 4, 2, 360, 40 },
    { -1, 0, 0, 0 }
};

// Mapeamento de Botões
CoordenadasBotao INICIAR_BTN = { .x1 = 480, .y1 = 420, .x2 = 848, .y2 = 510 };
CoordenadasBotao TUTORIAL_BTN = { .x1 = 480, .y1 = 545, .x2 = 848, .y2 = 630 };
CoordenadasBotao FECHAR_BTN = { .x1 = 1100, .y1 = 50, .x2 = 1250, .y2 = 100 };
CoordenadasBotao CAMA_AREA = { .x1 = 350, .y1 = 430, .x2 = 570, .y2 = 560 };
CoordenadasBotao PORTA_AREA = { .x1 = 920, .y1 = 300, .x2 = 1000, .y2 = 550 };
CoordenadasBotao INVEST_BTN_1 = { .x1 = 900, .y1 = 200, .x2 = 1200, .y2 = 250 };
CoordenadasBotao INVEST_BTN_2 = { .x1 = 900, .y1 = 270, .x2 = 1200, .y2 = 320 };
CoordenadasBotao INVEST_BTN_3 = { .x1 = 900, .y1 = 340, .x2 = 1200, .y2 = 390 };

// Variáveis de Jogo (Fome/Energia são float)
int dia_atual = 1;
float fome = 100.0f;
float energia = 100.0f;
int dinheiro = 500;
int dias_sem_comer = 0;
int ganho_dia = 0;
int gasto_dia = 0;
ALLEGRO_FONT* fonte_hud = NULL;
bool key_down[ALLEGRO_KEY_MAX] = { false };


// =================================================================
// FUNÇÕES DE LÓGICA E UTILIDADE
// =================================================================

bool check_collision(int x1, int y1, int r1, int x2, int y2, int r2) {
    int dx = x1 - x2;
    int dy = y1 - y2;
    int dist_squared = dx * dx + dy * dy;
    int radii_sum_squared = (r1 + r2) * (r1 + r2);
    return dist_squared <= radii_sum_squared;
}

void desenhar_hud_texto() {
    if (!fonte_hud) return;
    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 20, 20, 0, "Dia: %d", dia_atual);
    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 20, 50, 0, "Dinheiro: R$ %d", dinheiro);
    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 20, 80, 0, "Fome: %d%%", (int)fome);
    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 20, 110, 0, "Energia: %d%%", (int)energia);
    al_draw_textf(fonte_hud, al_map_rgb(255, 200, 200), 20, 140, 0, "Dias sem comer: %d", dias_sem_comer);
}

// ============== LÓGICA DE SIMULAÇÃO ==============

void comer(int custo, float ganho_fome) {
    if (dinheiro >= custo) {
        dinheiro -= custo;
        gasto_dia += custo;
        fome += ganho_fome;
        if (fome > 100.0f) fome = 100.0f;
        printf("Comeu: -%d dinheiro, +%.1f fome (fome=%.1f)\n", custo, ganho_fome, fome);
    }
    else {
        printf("Dinheiro insuficiente para comer!\n");
    }
}
void trabalhar(int ganho, float custo_energia) {
    if (energia >= custo_energia) {
        energia -= custo_energia;
        dinheiro += ganho;
        ganho_dia += ganho;
    }
}
void apostar(int valor) {
    if (dinheiro >= valor) {
        int resultado = rand() % 2;
        if (resultado == 1) dinheiro += valor; else dinheiro -= valor;
    }
}

// ============== LÓGICA DO BANCO ==============
int find_free_slot() {
    for (int i = 0; i < MAX_INVESTMENTS; i++) {
        if (!active_investments[i].active) { return i; }
    }
    return -1;
}

void investir(int option_id, int principal, int dia_atual, int* dinheiro_ptr, int* gasto_dia_ptr) {
    int slot = find_free_slot();
    if (slot == -1) { printf("Maximo de investimentos ativos alcancado!\n"); return; }
    InvestmentOption option = INVESTMENT_OPTIONS[option_id - 1];

    if (*dinheiro_ptr < principal || principal < option.custo_minimo) { printf("Investimento falhou: Saldo ou valor minimo (R$%d) insuficiente.\n", option.custo_minimo); return; }

    *dinheiro_ptr -= principal;
    *gasto_dia_ptr += principal;

    active_investments[slot].active = true;
    active_investments[slot].option_id = option_id;
    active_investments[slot].principal = principal;
    active_investments[slot].start_day = dia_atual;

    printf("Investimento iniciado: R$%d a %.0f%% por %d dias. Slot: %d\n",
        principal, option.taxa_juros * 100, option.dias_duracao, slot);
}

void sacar(int slot_index, int dia_atual, int* dinheiro_ptr, int* ganho_dia_ptr) {
    ActiveInvestment* inv = &active_investments[slot_index];
    if (inv->option_id == 0 || !inv->active) { printf("Erro: Slot %d sem investimento ativo.\n", slot_index); return; }

    InvestmentOption option = INVESTMENT_OPTIONS[inv->option_id - 1];
    int maturity_day = inv->start_day + option.dias_duracao;

    if (dia_atual < maturity_day) { printf("Ainda nao e possivel sacar! Maturidade no Dia %d. (Dia atual: %d)\n", maturity_day, dia_atual); return; }

    int rendimento = (int)(inv->principal * (1.0f + option.taxa_juros));
    int juros = rendimento - inv->principal;

    *dinheiro_ptr += rendimento;
    *ganho_dia_ptr += juros;

    printf("SAQUE BEM SUCEDIDO! R$ %d principal + R$ %d juros. Total: R$%d. Slot: %d\n",
        inv->principal, juros, rendimento, slot_index);

    // Limpa o slot
    memset(inv, 0, sizeof(ActiveInvestment));
}


// ==================== Funções de carregamento/limpeza ====================
int carregar_imagens() {
    img_menu_fundo = al_load_bitmap("menu_fundo1.png");
    img_mapa_fundo = al_load_bitmap("mapa_fundo.png");
    img_player_sprite = al_load_bitmap("sprite_final.png");
    img_tutorial_fundo = al_load_bitmap("tutorial_fundo.png");
    img_quarto_fundo = al_load_bitmap("quarto_fundo1.png");
    img_mercado_fundo = al_load_bitmap("mercado_fundo.png");
    img_cassino_fundo = al_load_bitmap("cassino_fundo.png");
    img_banco_fundo = al_load_bitmap("banco_fundo.png");
    img_fim_dia_fundo = al_load_bitmap("fim_dia_fundo.png");

    if (!img_menu_fundo && !img_mapa_fundo) { return 0; }
    return 1;
}

void limpar_recursos() {
    if (img_menu_fundo) al_destroy_bitmap(img_menu_fundo);
    if (img_mapa_fundo) al_destroy_bitmap(img_mapa_fundo);
    if (img_player_sprite) al_destroy_bitmap(img_player_sprite);
    if (img_tutorial_fundo) al_destroy_bitmap(img_tutorial_fundo);
    if (img_quarto_fundo) al_destroy_bitmap(img_quarto_fundo);
    if (img_mercado_fundo) al_destroy_bitmap(img_mercado_fundo);
    if (img_cassino_fundo) al_destroy_bitmap(img_cassino_fundo);
    if (img_banco_fundo) al_destroy_bitmap(img_banco_fundo);
    if (img_fim_dia_fundo) al_destroy_bitmap(img_fim_dia_fundo);
}

// =====================================================================
// MAIN
// =====================================================================
int main() {
    srand((unsigned int)time(NULL));

    // --- Inicializações ---
    if (!al_init()) { fprintf(stderr, "Falha ao inicializar Allegro.\n"); return -1; }
    al_init_primitives_addon();
    al_install_keyboard(); al_install_mouse();
    al_init_image_addon(); al_init_font_addon(); al_init_ttf_addon();

    ALLEGRO_DISPLAY* janela = al_create_display(LARGURA_TELA, ALTURA_TELA);
    if (!janela) { fprintf(stderr, "Falha ao criar janela.\n"); return -1; }

    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);

    fonte_hud = al_load_ttf_font("arial.ttf", 18, 0);
    if (!fonte_hud) { fonte_hud = al_create_builtin_font(); }

    if (!carregar_imagens()) {
        fprintf(stderr, "Falha crítica ao inicializar imagens.\n");
        limpar_recursos(); al_destroy_display(janela); return -1;
    }

    player_pos_x = (LARGURA_TELA - FRAME_LARGURA) / 2.0;
    player_pos_y = (ALTURA_TELA - FRAME_ALTURA) / 2.0;

    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));
    al_register_event_source(fila, al_get_mouse_event_source());

    bool rodando = true;
    al_start_timer(timer);

    while (rodando) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(fila, &ev);

        // --- Lógica de Eventos e Teclas ---
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { rodando = false; }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            if (estado_atual == TELA_MENU) {
                if (ev.mouse.x >= INICIAR_BTN.x1 && ev.mouse.x <= INICIAR_BTN.x2 && ev.mouse.y >= INICIAR_BTN.y1 && ev.mouse.y <= INICIAR_BTN.y2)
                {
                    estado_atual = TELA_QUARTO; player_pos_x = (LARGURA_TELA - FRAME_LARGURA) / 2.0; player_pos_y = (ALTURA_TELA - FRAME_ALTURA) / 2.0; printf("Transição: Menu -> Quarto\n");
                }
                if (ev.mouse.x >= TUTORIAL_BTN.x1 && ev.mouse.x <= TUTORIAL_BTN.x2 && ev.mouse.y >= TUTORIAL_BTN.y1 && ev.mouse.y <= TUTORIAL_BTN.y2)
                {
                    estado_atual = TELA_TUTORIAL; printf("Transição: Menu -> Tutorial\n");
                }
            }
            else if (estado_atual == TELA_FIM_DIA) {
                if (ev.mouse.x >= FECHAR_BTN.x1 && ev.mouse.x <= FECHAR_BTN.x2 && ev.mouse.y >= FECHAR_BTN.y1 && ev.mouse.y <= FECHAR_BTN.y2)
                {
                    estado_atual = TELA_MENU; printf("Botão Fechar Clicado: Fim do Dia -> Menu\n");
                }
            }
            else if (estado_atual == TELA_TUTORIAL) {
                if (ev.mouse.x >= FECHAR_BTN.x1 && ev.mouse.x <= FECHAR_BTN.x2 && ev.mouse.y >= FECHAR_BTN.y1 && ev.mouse.y <= FECHAR_BTN.y2)
                {
                    estado_atual = TELA_MENU; printf("Botão Fechar Clicado: Tutorial -> Menu\n");
                }
            }

            // ===================================
            // LÓGICA DE COMPRA DO MERCADINHO (NOVA)
            // ===================================
            else if (estado_atual == TELA_MERCADO) {
                for (int i = 0; i < MAX_FOOD_ITEMS; i++) {
                    if (ev.mouse.x >= FOOD_BTN[i].x1 && ev.mouse.x <= FOOD_BTN[i].x2 &&
                        ev.mouse.y >= FOOD_BTN[i].y1 && ev.mouse.y <= FOOD_BTN[i].y2) {

                        comer(FOOD_OPTIONS[i].custo, FOOD_OPTIONS[i].ganho_fome);
                        break;
                    }
                }
            }

            // ===================================
            // LÓGICA DE BOTÕES DO BANCO
            // ===================================
            else if (estado_atual == TELA_BANCO) {
                // 1. Tentar INVESTIR nas Opções 1, 2, 3
                if (ev.mouse.x >= INVEST_BTN_1.x1 && ev.mouse.x <= INVEST_BTN_1.x2 && ev.mouse.y >= INVEST_BTN_1.y1 && ev.mouse.y <= INVEST_BTN_1.y2) {
                    investir(1, INVESTMENT_OPTIONS[0].custo_minimo, dia_atual, &dinheiro, &gasto_dia);
                }
                else if (ev.mouse.x >= INVEST_BTN_2.x1 && ev.mouse.x <= INVEST_BTN_2.x2 && ev.mouse.y >= INVEST_BTN_2.y1 && ev.mouse.y <= INVEST_BTN_2.y2) {
                    investir(2, INVESTMENT_OPTIONS[1].custo_minimo, dia_atual, &dinheiro, &gasto_dia);
                }
                else if (ev.mouse.x >= INVEST_BTN_3.x1 && ev.mouse.x <= INVEST_BTN_3.x2 && ev.mouse.y >= INVEST_BTN_3.y1 && ev.mouse.y <= INVEST_BTN_3.y2) {
                    investir(3, INVESTMENT_OPTIONS[2].custo_minimo, dia_atual, &dinheiro, &gasto_dia);
                }

                // 2. Botão GLOBAL: Sacar Todos Prontos
                else if (ev.mouse.x >= 50 && ev.mouse.x <= 250 && ev.mouse.y >= 650 && ev.mouse.y <= 700) {
                    int sacados = 0;
                    for (int i = 0; i < MAX_INVESTMENTS; i++) {
                        ActiveInvestment* inv = &active_investments[i];
                        if (inv->active && dia_atual >= (inv->start_day + INVESTMENT_OPTIONS[inv->option_id - 1].dias_duracao)) {
                            sacar(i, dia_atual, &dinheiro, &ganho_dia);
                            sacados++;
                        }
                    }
                    if (sacados > 0) {
                        printf("Saque global efetuado: %d investimento(s) sacado(s).\n", sacados);
                    }
                    else {
                        printf("Nenhum investimento pronto para saque.\n");
                    }
                }

                // 3. Botões de SAQUE INDIVIDUAL (Mapeamento dinâmico)
                else {
                    for (int i = 0; i < MAX_INVESTMENTS; i++) {
                        if (active_investments[i].active) {
                            ActiveInvestment* inv = &active_investments[i];
                            int maturity_day = inv->start_day + INVESTMENT_OPTIONS[inv->option_id - 1].dias_duracao;
                            bool ready_to_withdraw = (dia_atual >= maturity_day);

                            if (ready_to_withdraw) {
                                int btn_x1 = 900;
                                int btn_y1 = 450 + (i * 60);
                                int btn_x2 = 1200;
                                int btn_y2 = btn_y1 + 40;

                                if (ev.mouse.x >= btn_x1 && ev.mouse.x <= btn_x2 && ev.mouse.y >= btn_y1 && ev.mouse.y <= btn_y2) {
                                    sacar(i, dia_atual, &dinheiro, &ganho_dia);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (ev.keyboard.keycode < ALLEGRO_KEY_MAX) { key_down[ev.keyboard.keycode] = true; }

            if (estado_atual == TELA_SAIR) {
                if (ev.keyboard.keycode == ALLEGRO_KEY_R) {
                    dia_atual = 1; fome = 100.0f; energia = 100.0f; dinheiro = 500;
                    dias_sem_comer = 0; ganho_dia = 0; gasto_dia = 0;
                    memset(active_investments, 0, sizeof(active_investments));
                    estado_atual = TELA_MENU; printf("Jogo reiniciado (R).\n");
                }
            }

            if (estado_atual == TELA_JOGO || estado_atual == TELA_QUARTO) {
                if (ev.keyboard.keycode == ALLEGRO_KEY_E) {
                    if (can_interact) {
                        if (estado_atual == TELA_JOGO && current_npc_id != -1) {
                            switch (current_npc_id) {
                            case 1: estado_atual = TELA_MERCADO; break;
                            case 2: estado_atual = TELA_CASSINO; break;
                            case 3: estado_atual = TELA_BANCO; break;
                            case 4: estado_atual = TELA_QUARTO; break;
                            default: estado_atual = TELA_JOGO; break;
                            }
                            printf("Iniciando dialogo com NPC ID: %d. Tela: %d\n", current_npc_id, estado_atual);
                        }
                        else if (estado_atual == TELA_QUARTO) {
                            if (quarto_interact_id == 1) {
                                dia_atual += 1; energia = 100.0f; fome -= 20.0f; if (fome < 0.0f) fome = 0.0f;
                                if (fome <= 20.0f) dias_sem_comer++; else dias_sem_comer = 0;
                                printf("=== FIM DO DIA ===\nDia %d\n", dia_atual - 1);
                                ganho_dia = 0; gasto_dia = 0;
                                estado_atual = TELA_FIM_DIA;
                                if (fome <= 0.0f || dias_sem_comer >= 3) { estado_atual = TELA_SAIR; }
                            }
                            else if (quarto_interact_id == 2) {
                                estado_atual = TELA_JOGO; printf("Interação (E): Quarto -> Jogo (Porta)\n");
                            }
                        }
                    }
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                switch (estado_atual) {
                case TELA_JOGO: case TELA_QUARTO: case TELA_TUTORIAL: case TELA_FIM_DIA:
                    estado_atual = TELA_MENU; break;
                case TELA_MERCADO: case TELA_CASSINO: case TELA_BANCO:
                    estado_atual = TELA_JOGO; current_npc_id = -1; break;
                case TELA_MENU:
                    rodando = false; break;
                default: break;
                }
            }
            if (ev.keyboard.keycode < ALLEGRO_KEY_MAX) { key_down[ev.keyboard.keycode] = false; }
        }

        else if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (fome <= 0.0f || dias_sem_comer >= 3) { estado_atual = TELA_SAIR; }

            if (estado_atual == TELA_JOGO || estado_atual == TELA_QUARTO) {
                bool andando_local = false;
                if (key_down[ALLEGRO_KEY_UP] || key_down[ALLEGRO_KEY_W]) { player_pos_y -= player_velocidade; anim_current_frame_y = FRAME_ALTURA * 2; andando_local = true; }
                if (key_down[ALLEGRO_KEY_DOWN] || key_down[ALLEGRO_KEY_S]) { player_pos_y += player_velocidade; anim_current_frame_y = FRAME_ALTURA * 0; andando_local = true; }
                if (key_down[ALLEGRO_KEY_LEFT] || key_down[ALLEGRO_KEY_A]) { player_pos_x -= player_velocidade; anim_current_frame_y = FRAME_ALTURA * 3; andando_local = true; }
                if (key_down[ALLEGRO_KEY_RIGHT] || key_down[ALLEGRO_KEY_D]) { player_pos_x += player_velocidade; anim_current_frame_y = FRAME_ALTURA; andando_local = true; }

                if (andando_local) {
                    anim_frame += 0.3f; if (anim_frame >= 3) anim_frame = 0;
                    energia -= 0.05f; if (energia < 0.0f) energia = 0.0f;
                    fome -= 0.02f; if (fome < 0.0f) fome = 0.0f;
                }
                else { anim_frame = 0; }

                if (energia < 10.0f) player_velocidade = 3.0f; else player_velocidade = 7.0f;

                if (player_pos_x < 0) player_pos_x = 0; if (player_pos_x > LARGURA_TELA - FRAME_LARGURA) player_pos_x = LARGURA_TELA - FRAME_LARGURA;
                if (player_pos_y < 0) player_pos_y = 0; if (player_pos_y > ALTURA_TELA - FRAME_ALTURA) player_pos_y = ALTURA_TELA - FRAME_ALTURA;
            }

            can_interact = false; current_npc_id = -1; quarto_interact_id = -1;

            if (estado_atual == TELA_QUARTO) {
                int player_center_x = player_pos_x + FRAME_LARGURA / 2; int player_center_y = player_pos_y + FRAME_ALTURA / 2;
                int CAMA_RAIO = 150; int cama_center_x = (CAMA_AREA.x1 + CAMA_AREA.x2) / 2; int cama_center_y = (CAMA_AREA.y1 + CAMA_AREA.y2) / 2;
                if (check_collision(player_center_x, player_center_y, PLAYER_RAIO, cama_center_x, cama_center_y, CAMA_RAIO)) { can_interact = true; quarto_interact_id = 1; }
                int PORTA_RAIO = 100; int porta_center_x = (PORTA_AREA.x1 + PORTA_AREA.x2) / 2; int porta_center_y = (PORTA_AREA.y1 + PORTA_AREA.y2) / 2;
                if (check_collision(player_center_x, player_center_y, PLAYER_RAIO, porta_center_x, porta_center_y, PORTA_RAIO)) { can_interact = true; quarto_interact_id = 2; }
            }
            if (estado_atual == TELA_JOGO) {
                for (int i = 0; i < MAX_NPCS; i++) {
                    if (NPC_LIST[i].id != -1) {
                        int player_center_x = player_pos_x + FRAME_LARGURA / 2; int player_center_y = player_pos_y + FRAME_ALTURA / 2;
                        if (check_collision(player_center_x, player_center_y, PLAYER_RAIO, NPC_LIST[i].x, NPC_LIST[i].y, NPC_LIST[i].raio)) {
                            can_interact = true; current_npc_id = NPC_LIST[i].id; break;
                        }
                    }
                }
            }

            al_clear_to_color(al_map_rgb(0, 0, 0));

            // --- Lógica de Desenho por Estado ---
            switch (estado_atual) {
            case TELA_MENU:
                al_draw_bitmap(img_menu_fundo, 0, 0, 0);
                al_draw_filled_rectangle(INICIAR_BTN.x1, INICIAR_BTN.y1, INICIAR_BTN.x2, INICIAR_BTN.y2, al_map_rgb(50, 200, 50));
                al_draw_filled_rectangle(TUTORIAL_BTN.x1, TUTORIAL_BTN.y1, TUTORIAL_BTN.x2, TUTORIAL_BTN.y2, al_map_rgb(50, 200, 50));
                break;
            case TELA_QUARTO:
            case TELA_JOGO:
                if (estado_atual == TELA_QUARTO) al_draw_bitmap(img_quarto_fundo, 0, 0, 0);
                if (estado_atual == TELA_JOGO) al_draw_bitmap(img_mapa_fundo, 0, 0, 0);
                if (estado_atual == TELA_JOGO) {
                    for (int i = 0; i < MAX_NPCS; i++) {
                        if (NPC_LIST[i].id != -1) {
                            ALLEGRO_COLOR cor_npc = (can_interact && current_npc_id == NPC_LIST[i].id) ? al_map_rgb(255, 0, 0) : al_map_rgb(150, 150, 150);
                            al_draw_filled_circle(NPC_LIST[i].x, NPC_LIST[i].y, NPC_LIST[i].raio, cor_npc);
                        }
                    }
                }
                if (img_player_sprite) {
                    al_draw_bitmap_region(img_player_sprite, FRAME_LARGURA * (int)anim_frame, anim_current_frame_y, FRAME_LARGURA, FRAME_ALTURA, player_pos_x, player_pos_y, 0);
                }
                desenhar_hud_texto();
                break;

            case TELA_MERCADO:
            {
                ALLEGRO_COLOR COR_TEXTO = al_map_rgb(0, 0, 0);

                al_draw_bitmap(img_mercado_fundo, 0, 0, 0);

                // Título
                al_draw_textf(fonte_hud, COR_TEXTO, 50, 50, 0, "Mercadinho: Saldo R$ %d", dinheiro);
                al_draw_textf(fonte_hud, COR_TEXTO, 50, 80, 0, "Clique para Comprar:");

                int start_x = 50;
                int start_y = 120;
                int btn_largura = 180;
                int btn_altura = 60;
                int colunas = 4;
                int espacamento_x = 20;
                int espacamento_y = 20;

                for (int i = 0; i < MAX_FOOD_ITEMS; i++) {
                    int col = i % colunas;
                    int row = i / colunas;

                    int x1 = start_x + (col * (btn_largura + espacamento_x));
                    int y1 = start_y + (row * (btn_altura + espacamento_y));
                    int x2 = x1 + btn_largura;
                    int y2 = y1 + btn_altura;

                    // Mapeia a coordenada do botão (para ser usada no clique)
                    FOOD_BTN[i].x1 = x1;
                    FOOD_BTN[i].y1 = y1;
                    FOOD_BTN[i].x2 = x2;
                    FOOD_BTN[i].y2 = y2;

                    bool tem_dinheiro = (dinheiro >= FOOD_OPTIONS[i].custo);
                    ALLEGRO_COLOR cor_fundo = tem_dinheiro ? al_map_rgb(0, 100, 0) : al_map_rgb(50, 50, 50);
                    ALLEGRO_COLOR cor_alerta = tem_dinheiro ? al_map_rgb(255, 255, 255) : al_map_rgb(255, 100, 100);

                    // Desenha o botão
                    al_draw_filled_rectangle(x1, y1, x2, y2, cor_fundo);

                    // Desenha o Nome
                    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), x1 + 10, y1 + 5, 0, "%s", FOOD_OPTIONS[i].nome);

                    // Desenha o Preço e Ganho de Fome
                    al_draw_textf(fonte_hud, cor_alerta, x1 + 10, y1 + 25, 0,
                        "R$%d | Fome: +%.0f%%",
                        FOOD_OPTIONS[i].custo,
                        FOOD_OPTIONS[i].ganho_fome);
                }

                desenhar_hud_texto();
                break;
            }

            case TELA_CASSINO:
                al_draw_bitmap(img_cassino_fundo, 0, 0, 0); desenhar_hud_texto(); break;

            case TELA_BANCO:
            {
                ALLEGRO_COLOR COR_TEXTO = al_map_rgb(0, 0, 0);
                al_draw_bitmap(img_banco_fundo, 0, 0, 0);

                // 1. Informações Principais
                al_draw_textf(fonte_hud, COR_TEXTO, 50, 50, 0, "Saldo Disponível: R$ %d | Dia Atual: %d", dinheiro, dia_atual);
                al_draw_textf(fonte_hud, COR_TEXTO, 900, 170, ALLEGRO_ALIGN_LEFT, "Opções de Investimento");

                // 2. Desenhar Opções de Investimento (Botões e Status de Saldo)
                for (int i = 0; i < 3; i++) {
                    CoordenadasBotao btn;
                    if (i == 0) btn = INVEST_BTN_1;
                    else if (i == 1) btn = INVEST_BTN_2;
                    else btn = INVEST_BTN_3;

                    int custo = INVESTMENT_OPTIONS[i].custo_minimo;
                    bool tem_dinheiro = (dinheiro >= custo);

                    ALLEGRO_COLOR cor_fundo = tem_dinheiro ? al_map_rgb(0, 150, 0) : al_map_rgb(50, 50, 50);
                    ALLEGRO_COLOR cor_alerta = tem_dinheiro ? al_map_rgb(0, 0, 0) : al_map_rgb(255, 100, 100);

                    al_draw_filled_rectangle(btn.x1, btn.y1, btn.x2, btn.y2, cor_fundo);
                    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), btn.x1 + 10, btn.y1 + 5, 0, "%s", INVESTMENT_OPTIONS[i].nome);
                    al_draw_textf(fonte_hud, cor_alerta, btn.x1 + 10, btn.y1 + 25, 0, "Custo: R$%d", custo);
                }

                // 3. Desenhar Investimentos Ativos
                al_draw_textf(fonte_hud, COR_TEXTO, 50, 420, 0, "Investimentos Ativos (Max: %d):", MAX_INVESTMENTS);

                int y_start = 450;
                for (int i = 0; i < MAX_INVESTMENTS; i++) {
                    if (active_investments[i].active) {
                        ActiveInvestment* inv = &active_investments[i];
                        InvestmentOption option = INVESTMENT_OPTIONS[inv->option_id - 1];

                        int maturity_day = inv->start_day + option.dias_duracao;
                        bool ready_to_withdraw = (dia_atual >= maturity_day);

                        int y_offset = y_start + (i * 60);

                        // Coluna 1: Status do Investimento (Lista)
                        al_draw_textf(fonte_hud, COR_TEXTO, 50, y_offset, 0,
                            "[SLOT %d] %s", i + 1, option.nome);
                        al_draw_textf(fonte_hud, COR_TEXTO, 50, y_offset + 20, 0,
                            "   Principal: R$%d | Start: Dia %d", inv->principal, inv->start_day);

                        // Coluna 2: Status de Saque
                        ALLEGRO_COLOR status_cor = ready_to_withdraw ? al_map_rgb(0, 255, 0) : al_map_rgb(255, 255, 0);
                        al_draw_textf(fonte_hud, status_cor, 450, y_offset + 10, 0, ready_to_withdraw ? "PRONTO!" : "Maturidade: Dia %d", maturity_day);

                        // Coluna 3: Botão de Saque (Dinâmico)
                        int btn_x1 = 900;
                        int btn_y1 = y_offset;
                        int btn_x2 = 1200;
                        int btn_y2 = btn_y1 + 40;

                        ALLEGRO_COLOR saque_cor = ready_to_withdraw ? al_map_rgb(0, 100, 200) : al_map_rgb(50, 50, 50);
                        al_draw_filled_rectangle(btn_x1, btn_y1, btn_x2, btn_y2, saque_cor);
                        al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), btn_x1 + 10, btn_y1 + 10, 0, ready_to_withdraw ? "SACAR INDIVIDUAL" : "Aguardando...");
                    }
                }

                // 4. Botão Sacar Tudo (Posição fixa)
                al_draw_filled_rectangle(50, 650, 250, 700, al_map_rgb(150, 0, 150));
                al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 60, 665, 0, "SACAR TUDO");

                desenhar_hud_texto();
                break;
            }

            case TELA_TUTORIAL: al_draw_bitmap(img_tutorial_fundo, 0, 0, 0); break;
            case TELA_FIM_DIA:
                if (img_fim_dia_fundo) { al_draw_bitmap(img_fim_dia_fundo, 0, 0, 0); }
                else { al_draw_filled_rectangle(0, 0, LARGURA_TELA, ALTURA_TELA, al_map_rgb(20, 20, 20)); }
                if (fonte_hud) {
                    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 520, 180, ALLEGRO_ALIGN_CENTER, "Fim do Dia %d", dia_atual - 1);
                    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 520, 300, ALLEGRO_ALIGN_CENTER, "Saldo atual: R$ %d", dinheiro);
                    al_draw_textf(fonte_hud, al_map_rgb(200, 200, 200), 520, 360, ALLEGRO_ALIGN_CENTER, "Clique no botão FECHAR para voltar ao menu.");
                }
                break;
            case TELA_SAIR:
                al_clear_to_color(al_map_rgb(10, 10, 10));
                if (fonte_hud) {
                    if (fome <= 0.0f || dias_sem_comer >= 3) {
                        al_draw_textf(fonte_hud, al_map_rgb(255, 80, 80), 400, 260, 0, "GAME OVER - Você morreu de fome.");
                        al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 400, 340, 0, "Pressione R para reiniciar ou ESC para sair.");
                    }
                    else { al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 400, 300, 0, "Saindo do jogo..."); }
                }
                break;
            default: break;
            }

            if (estado_atual == TELA_TUTORIAL || estado_atual == TELA_FIM_DIA || estado_atual == TELA_BANCO) {
                al_draw_filled_rectangle(FECHAR_BTN.x1, FECHAR_BTN.y1, FECHAR_BTN.x2, FECHAR_BTN.y2, al_map_rgb(200, 50, 50));
                al_draw_text(fonte_hud, al_map_rgb(255, 255, 255), FECHAR_BTN.x1 + 10, FECHAR_BTN.y1 + 15, 0, "SAIR (ESC)");
            }

            al_flip_display();
        }
    }

    // --- Limpeza ---
    limpar_recursos();
    al_destroy_event_queue(fila);
    al_destroy_display(janela);
    al_destroy_timer(timer);

    return 0;
}