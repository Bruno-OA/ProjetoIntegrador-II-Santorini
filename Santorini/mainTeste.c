// jogo_completo_integrado.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h> // Necessário se for usar o font addon
#include <allegro5/allegro_ttf.h>
#include <math.h>
#include <stdbool.h>
#include "botoes.h"
#include "npcs.h"

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
    //TELA_INTERACAO = 6,
    TELA_MERCADO = 6,     // NOVO: Tela específica para NPC 1
    TELA_CASSINO = 7,     // NOVO: Tela específica para NPC 2
    TELA_BANCO = 8,     // NOVO: Tela específica para NPC 3
    TELA_SAIR = 9
} EstadoDoJogo;

// Variáveis Globais
EstadoDoJogo estado_atual = TELA_MENU;
int current_npc_id = -1; // ID do NPC com o qual estamos interagindo (-1 = nenhum)
bool can_interact = false; // Flag para mostrar o prompt de confirmação

// Bitmaps do Menu/Mapa
ALLEGRO_BITMAP* img_menu_fundo = NULL;
ALLEGRO_BITMAP* img_mapa_fundo = NULL;
ALLEGRO_BITMAP* img_tutorial_fundo = NULL;
ALLEGRO_BITMAP* img_quarto_fundo = NULL;
ALLEGRO_BITMAP* img_mercado_fundo = NULL;
ALLEGRO_BITMAP* img_cassino_fundo = NULL;
ALLEGRO_BITMAP* img_banco_fundo = NULL;
ALLEGRO_BITMAP* img_fim_dia_fundo = NULL;

// ===================================
// NOVAS VARIÁVEIS DO JOGADOR (SPRITE)
// ===================================
ALLEGRO_BITMAP* img_player_sprite = NULL;

// Variáveis de Posição, Velocidade e Animação
float player_pos_x = LARGURA_TELA / 2.0;
float player_pos_y = ALTURA_TELA / 2.0;
float player_velocidade = 7.0; // Velocidade em pixels por frame (igual ao seu novo código)
int PLAYER_RAIO = 50;
float anim_frame = 0.f;          // Contador do frame atual
int anim_current_frame_y = 90 * 2; // Linha do sprite (Começa olhando para baixo)
const int FRAME_LARGURA = 95;
const int FRAME_ALTURA = 180;
int quarto_interact_id = -1;
// ===================================

// ======================= NPCs / Botões (mantidos) =======================
NPC NPC_LIST[MAX_NPCS] = {
    // ID, X, Y, RAIO
    { 1, 250, 100, 30 }, // NPC 1: João
    { 2, 650, 100, 30 }, // NPC 2: Maria
    { 3, 1050, 100, 30 }, // NPC 3: Pedro
    { 4, 2, 360, 40 }, // NPC 4: (Ocioso)
    { -1, 0, 0, 0 }      // Slot Vazio (ou use 0 na posição se não usar todos)
};

// mapeamento de botoes
CoordenadasBotao INICIAR_BTN = {
    .x1 = 480,
    .y1 = 420,
    .x2 = 848,
    .y2 = 510
};
CoordenadasBotao TUTORIAL_BTN = {
    .x1 = 480,
    .y1 = 545,
    .x2 = 848,
    .y2 = 630
};
CoordenadasBotao FECHAR_BTN = {
    .x1 = 1100, // Exemplo: Canto superior esquerdo
    .y1 = 50,
    .x2 = 1250,
    .y2 = 100
};
CoordenadasBotao CAMA_AREA = {
    .x1 = 350,
    .y1 = 430,
    .x2 = 570,
    .y2 = 560
};
CoordenadasBotao PORTA_AREA = {
    .x1 = 920,
    .y1 = 300,
    .x2 = 1000,
    .y2 = 550
};

bool check_collision(int x1, int y1, int r1, int x2, int y2, int r2) {
    // Distância euclidiana ao quadrado
    int dx = x1 - x2;
    int dy = y1 - y2;
    int dist_squared = dx * dx + dy * dy;

    // Raio de colisão somado ao quadrado
    int radii_sum_squared = (r1 + r2) * (r1 + r2);

    return dist_squared <= radii_sum_squared;
}

// Vetor de estados das teclas (para movimento contínuo)
bool key_down[ALLEGRO_KEY_MAX] = { false };

// ==================== SISTEMA DE PROGRESSO ====================
// Dia, fome, energia, dinheiro, contadores do dia
int dia_atual = 1;
int fome = 100;       // 0 = faminto (morto), 100 = cheio
int energia = 100;    // 0 = exausto, 100 = descansado
int dinheiro = 500;   // saldo atual
int dias_sem_comer = 0;
int ganho_dia = 0;    // quanto entrou durante o dia atual
int gasto_dia = 0;    // quanto saiu durante o dia atual

// Fonte para HUD
ALLEGRO_FONT* fonte_hud = NULL;

// ==================== FUNÇÕES MODULARES ====================
void comer(int custo, int ganho_fome) {
    if (dinheiro >= custo) {
        dinheiro -= custo;
        gasto_dia += custo;
        fome += ganho_fome;
        if (fome > 100) fome = 100;
        printf("Comeu: -%d dinheiro, +%d fome (fome=%d)\n", custo, ganho_fome, fome);
    }
    else {
        printf("Dinheiro insuficiente para comer!\n");
    }
}

void trabalhar(int ganho, int custo_energia) {
    if (energia >= custo_energia) {
        energia -= custo_energia;
        dinheiro += ganho;
        ganho_dia += ganho;
        printf("Trabalhou: +%d dinheiro, -%d energia (energia=%d)\n", ganho, custo_energia, energia);
    }
    else {
        printf("Energia insuficiente para trabalhar!\n");
    }
}

void apostar(int valor) {
    if (dinheiro >= valor) {
        int resultado = rand() % 2; // 50% chance simples
        if (resultado == 1) {
            dinheiro += valor;
            ganho_dia += valor;
            printf("Ganhou aposta! +%d dinheiro (saldo=%d)\n", valor, dinheiro);
        }
        else {
            dinheiro -= valor;
            gasto_dia += valor;
            printf("Perdeu aposta! -%d dinheiro (saldo=%d)\n", valor, dinheiro);
        }
    }
    else {
        printf("Dinheiro insuficiente para apostar!\n");
    }
}

// ==================== HUD (canto superior esquerdo) ====================
void desenhar_hud_texto() {
    if (!fonte_hud) return;
    char buf[64];
    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 20, 20, 0, "Dia: %d", dia_atual);
    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 20, 50, 0, "Dinheiro: R$ %d", dinheiro);
    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 20, 80, 0, "Fome: %d%%", fome);
    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 20, 110, 0, "Energia: %d%%", energia);

    // Se quiser uma linha extra com dias sem comer:
    al_draw_textf(fonte_hud, al_map_rgb(255, 200, 200), 20, 140, 0, "Dias sem comer: %d", dias_sem_comer);
}

// ==================== Funções de carregamento/limpeza (mantidas) ====================
int carregar_imagens() {
    // 1. Carrega o fundo do Menu
    img_menu_fundo = al_load_bitmap("menu_fundo1.png");
    if (!img_menu_fundo) {
        fprintf(stderr, "ERRO: Não foi possível carregar menu_fundo.png. Usando cor simples.\n");
        img_menu_fundo = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
        if (img_menu_fundo) {
            al_set_target_bitmap(img_menu_fundo);
            al_clear_to_color(al_map_rgb(100, 100, 200));
            al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
        }
        else return 0;
    }

    // 2. Carrega o fundo do Mapa
    img_mapa_fundo = al_load_bitmap("mapa_fundo.png");
    if (!img_mapa_fundo) {
        fprintf(stderr, "ERRO: Não foi possível carregar mapa_fundo.png. Usando cor simples.\n");
        img_mapa_fundo = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
        if (img_mapa_fundo) {
            al_set_target_bitmap(img_mapa_fundo);
            al_clear_to_color(al_map_rgb(100, 200, 100));
            al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
        }
        else return 0;
    }

    // 3. Carrega o Sprite do Jogador (Substitua "sprite_final.png" pelo caminho correto)
    img_player_sprite = al_load_bitmap("sprite_final.png");
    if (!img_player_sprite) {
        fprintf(stderr, "ERRO: Não foi possível carregar sprite_final.png. O jogador será invisível!\n");
    }

    //Carrega o fundo do Tutorial
    img_tutorial_fundo = al_load_bitmap("tutorial_fundo.png");
    if (!img_tutorial_fundo) {
        fprintf(stderr, "ERRO: Nao foi possivel carregar tutorial_fundo.png. Usando cor simples.\n");
        img_tutorial_fundo = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
        if (img_tutorial_fundo) {
            al_set_target_bitmap(img_tutorial_fundo);
            al_clear_to_color(al_map_rgb(200, 200, 100)); // Fundo Amarelo Claro
            al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
        }
        else return 0;
    }

    //Carrega o fundo do quarto
    img_quarto_fundo = al_load_bitmap("quarto_fundo1.png");
    if (!img_quarto_fundo) {
        fprintf(stderr, "ERRO: Nao foi possivel carregar quarto_fundo.png. Usando cor simples.\n");
        img_quarto_fundo = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
        if (img_quarto_fundo) {
            al_set_target_bitmap(img_quarto_fundo);
            al_clear_to_color(al_map_rgb(100, 50, 50)); // Fundo Marrom/Quarto Simples
            al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
        }
        else return 0;
    }

    // Carrega fundos de NPC (Simulação de tela 1, 2 e 3)
    img_mercado_fundo = al_load_bitmap("mercado_fundo.png");
    if (!img_mercado_fundo) {
        fprintf(stderr, "ERRO: Não foi possível carregar mercado_fundo.png. Usando cor simples.\n");
        img_mercado_fundo = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
        if (img_mercado_fundo) {
            al_set_target_bitmap(img_mercado_fundo);
            al_clear_to_color(al_map_rgb(255, 150, 150)); // Vermelho Claro
            al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
        }
        else return 0;
    }

    img_cassino_fundo = al_load_bitmap("cassino_fundo.png");
    if (!img_cassino_fundo) {
        fprintf(stderr, "ERRO: Não foi possível carregar cassino_fundo.png. Usando cor simples.\n");
        img_cassino_fundo = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
        if (img_cassino_fundo) {
            al_set_target_bitmap(img_cassino_fundo);
            al_clear_to_color(al_map_rgb(150, 255, 150));
            al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
        }
        else return 0;
    }

    img_banco_fundo = al_load_bitmap("banco_fundo.png");
    if (!img_banco_fundo) {
        fprintf(stderr, "ERRO: Não foi possível carregar banco_fundo.png. Usando cor simples.\n");
        img_banco_fundo = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
        if (img_banco_fundo) {
            al_set_target_bitmap(img_banco_fundo);
            al_clear_to_color(al_map_rgb(150, 150, 255));
            al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
        }
        else return 0;
    }

    img_fim_dia_fundo = al_load_bitmap("fim_dia_fundo.png");
    if (!img_fim_dia_fundo) {
        fprintf(stderr, "ERRO: Não foi possível carregar fim_dia_fundo.png. Usando cor simples.\n");
        img_fim_dia_fundo = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
        if (img_fim_dia_fundo) {
            al_set_target_bitmap(img_fim_dia_fundo);
            al_clear_to_color(al_map_rgb(150, 150, 255));
            al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
        }
        else return 0;
    }

    return 1; // Sucesso
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
    if (!al_init()) {
        fprintf(stderr, "Falha ao inicializar Allegro.\n");
        return -1;
    }
    al_init_primitives_addon();
    al_install_keyboard();
    al_install_mouse();
    al_init_image_addon(); // ESSENCIAL para ler PNG/JPG!
    al_init_font_addon();  // necessário para fontes
    al_init_ttf_addon();   // TTF fonts

    ALLEGRO_DISPLAY* janela = al_create_display(LARGURA_TELA, ALTURA_TELA);
    if (!janela) {
        fprintf(stderr, "Falha ao criar janela.\n");
        return -1;
    }

    // Timer & fila
    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);

    // Fonte HUD (tenta TTF, senão builtin)
    fonte_hud = al_load_ttf_font("arial.ttf", 18, 0);
    if (!fonte_hud) {
        fonte_hud = al_create_builtin_font();
        if (!fonte_hud) {
            fprintf(stderr, "Falha ao carregar fonte (ttf e builtin falharam).\n");
            // não é fatal: ainda podemos continuar sem texto
        }
    }

    // Carrega todas as imagens, incluindo o sprite do jogador
    if (!carregar_imagens()) {
        fprintf(stderr, "Falha crítica ao inicializar imagens.\n");
        limpar_recursos();
        al_destroy_display(janela);
        return -1;
    }

    // Posição inicial do jogador (centralizada)
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
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = false;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            if (estado_atual == TELA_MENU) {
                // Botão INICIAR
                if (ev.mouse.x >= INICIAR_BTN.x1 && ev.mouse.x <= INICIAR_BTN.x2 &&
                    ev.mouse.y >= INICIAR_BTN.y1 && ev.mouse.y <= INICIAR_BTN.y2)
                {
                    estado_atual = TELA_QUARTO;

                    player_pos_x = LARGURA_TELA / 2.0;
                    player_pos_y = ALTURA_TELA / 2.0;
                    printf("Transição: Menu -> Quarto\n");
                }
                // Botão TUTORIAL
                if (ev.mouse.x >= TUTORIAL_BTN.x1 && ev.mouse.x <= TUTORIAL_BTN.x2 &&
                    ev.mouse.y >= TUTORIAL_BTN.y1 && ev.mouse.y <= TUTORIAL_BTN.y2)
                {
                    estado_atual = TELA_TUTORIAL;
                    printf("Transição: Menu -> Tutorial\n");
                }
            }

            // Fim do dia: botão FECHAR volta ao menu
            else if (estado_atual == TELA_FIM_DIA) {
                if (ev.mouse.x >= FECHAR_BTN.x1 && ev.mouse.x <= FECHAR_BTN.x2 &&
                    ev.mouse.y >= FECHAR_BTN.y1 && ev.mouse.y <= FECHAR_BTN.y2)
                {
                    estado_atual = TELA_MENU; // Volta para o menu
                    printf("Botão Fechar Clicado: Fim do Dia -> Menu\n");
                }
            }

            else if (estado_atual == TELA_TUTORIAL) {
                if (ev.mouse.x >= FECHAR_BTN.x1 && ev.mouse.x <= FECHAR_BTN.x2 &&
                    ev.mouse.y >= FECHAR_BTN.y1 && ev.mouse.y <= FECHAR_BTN.y2)
                {
                    estado_atual = TELA_MENU; // Volta para o menu
                    printf("Botão Fechar Clicado: Tutorial -> Menu\n");
                }
            }

            // Interações de botões nas telas do NPC (ex.: mercado) podem usar aqui as coordenadas
            // (modular: quando quiser adicionar botões, insira checagens aqui e chame comer/trabalhar/apostar)
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (ev.keyboard.keycode < ALLEGRO_KEY_MAX) {
                key_down[ev.keyboard.keycode] = true;
            }

            // Reiniciar após Game Over (TELA_SAIR usada como Game Over)
            if (estado_atual == TELA_SAIR) {
                if (ev.keyboard.keycode == ALLEGRO_KEY_R) {
                    // Reinicia parâmetros básicos
                    dia_atual = 1;
                    fome = 100;
                    energia = 100;
                    dinheiro = 500;
                    dias_sem_comer = 0;
                    ganho_dia = 0;
                    gasto_dia = 0;
                    estado_atual = TELA_MENU;
                    printf("Jogo reiniciado (R).\n");
                }
            }

            if (estado_atual == TELA_JOGO || estado_atual == TELA_QUARTO) {
                if (ev.keyboard.keycode == ALLEGRO_KEY_E) {
                    if (can_interact) {
                        if (estado_atual == TELA_JOGO && current_npc_id != -1) {
                            // Abre tela do NPC com base no ID
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
                                // Dormir: avança o dia e mostra tela de fim do dia
                                // Lógica de fim do dia:
                                dia_atual += 1;
                                energia = 100;
                                fome -= 20;
                                if (fome < 0) fome = 0;

                                if (fome <= 20) dias_sem_comer++;
                                else dias_sem_comer = 0;

                                // Exibe resumo no console
                                printf("=== FIM DO DIA ===\nDia %d\nGanho no dia: %d\nGasto no dia: %d\nSaldo: %d\n",
                                    dia_atual - 1, ganho_dia, gasto_dia, dinheiro);

                                // Reset contabilizadores diários
                                ganho_dia = 0;
                                gasto_dia = 0;

                                // Vai para a tela de fim do dia (onde mostramos resumo)
                                estado_atual = TELA_FIM_DIA;
                                printf("Interação (E): Quarto -> Fim do Dia (Cama). Dia agora: %d. Fome=%d, Energia=%d, DiasSemComer=%d\n",
                                    dia_atual, fome, energia, dias_sem_comer);

                                // Checa Game Over imediato
                                if (fome <= 0 || dias_sem_comer >= 3) {
                                    estado_atual = TELA_SAIR;
                                    printf("GAME OVER: Morreu de fome (fome=%d, dias_sem_comer=%d)\n", fome, dias_sem_comer);
                                }
                            }
                            else if (quarto_interact_id == 2) {
                                // Porta -> sair para mapa
                                estado_atual = TELA_JOGO;
                                printf("Interação (E): Quarto -> Jogo (Porta)\n");
                            }
                        }
                    }
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            // Lógica do ESC
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                switch (estado_atual) {
                case TELA_JOGO:
                case TELA_QUARTO:
                case TELA_TUTORIAL:
                case TELA_FIM_DIA:
                    estado_atual = TELA_MENU;
                    printf("Transição: Jogo/Tutorial/FimDia -> Menu\n");
                    break;

                case TELA_MERCADO:
                case TELA_CASSINO:
                case TELA_BANCO:
                    estado_atual = TELA_JOGO; // Volta para o mapa
                    current_npc_id = -1;
                    printf("Saindo do dialogo. Voltando para o Jogo.\n");
                    break;

                case TELA_MENU:
                    rodando = false;
                    break;

                default:
                    break;
                }
            }
            // Liberação da tecla
            if (ev.keyboard.keycode < ALLEGRO_KEY_MAX) {
                key_down[ev.keyboard.keycode] = false;
            }
        }

        else if (ev.type == ALLEGRO_EVENT_TIMER) {

            // Checagem de Game Over antes das atualizações (por segurança)
            if (fome <= 0 || dias_sem_comer >= 3) {
                estado_atual = TELA_SAIR;
            }

            // --- Lógica de Atualização por Estado ---
            if (estado_atual == TELA_JOGO || estado_atual == TELA_QUARTO) {
                bool andando_local = false;

                // === LÓGICA DE MOVIMENTAÇÃO E ANIMAÇÃO (DO SEU NOVO CÓDIGO) ===
                if (key_down[ALLEGRO_KEY_UP] || key_down[ALLEGRO_KEY_W]) {
                    player_pos_y -= player_velocidade;
                    anim_current_frame_y = FRAME_ALTURA * 2; // Olhando para cima
                    andando_local = true;
                }
                if (key_down[ALLEGRO_KEY_DOWN] || key_down[ALLEGRO_KEY_S]) {
                    player_pos_y += player_velocidade;
                    anim_current_frame_y = FRAME_ALTURA * 0; // Olhando para baixo
                    andando_local = true;
                }
                if (key_down[ALLEGRO_KEY_LEFT] || key_down[ALLEGRO_KEY_A]) {
                    player_pos_x -= player_velocidade;
                    anim_current_frame_y = FRAME_ALTURA * 3; // Olhando para a esquerda
                    andando_local = true;
                }
                if (key_down[ALLEGRO_KEY_RIGHT] || key_down[ALLEGRO_KEY_D]) {
                    player_pos_x += player_velocidade;
                    anim_current_frame_y = FRAME_ALTURA; // Olhando para a direita
                    andando_local = true;
                }

                if (andando_local) {
                    anim_frame += 0.3f;
                    if (anim_frame >= 3) anim_frame = 0; // 4 frames (0,1,2,3)

                    // Consome energia e aumenta fome levemente enquanto anda
                    energia -= 0.05f;
                    if (energia < 0) energia = 0;

                    fome -= 0.02f; // interpretamos "andar" como gastar reservas -> diminuir fome
                    // Aqui mantemos a convenção: fome=100 é cheio; ao diminuir, aproxima de 0 (morte)
                    if (fome < 0) fome = 0;
                }
                else {
                    anim_frame = 0; // Parado no frame inicial da linha
                }

                // Penalidade: jogador anda mais devagar se estiver cansado
                if (energia < 10.0f)
                    player_velocidade = 3.0f;
                else
                    player_velocidade = 7.0f;

                // LIMITES DA JANELA
                if (player_pos_x < 0) player_pos_x = 0;
                if (player_pos_x > LARGURA_TELA - FRAME_LARGURA) player_pos_x = LARGURA_TELA - FRAME_LARGURA;
                if (player_pos_y < 0) player_pos_y = 0;
                if (player_pos_y > ALTURA_TELA - FRAME_ALTURA) player_pos_y = ALTURA_TELA - FRAME_ALTURA;
            }

            // Reset das flags de interação (será setado se houver colisão)
            can_interact = false;
            current_npc_id = -1;
            quarto_interact_id = -1;

            if (estado_atual == TELA_QUARTO) {
                int player_center_x = player_pos_x + FRAME_LARGURA / 2;
                int player_center_y = player_pos_y + FRAME_ALTURA / 2;
                int CAMA_RAIO = 150;
                int cama_center_x = (CAMA_AREA.x1 + CAMA_AREA.x2) / 2;
                int cama_center_y = (CAMA_AREA.y1 + CAMA_AREA.y2) / 2;

                if (check_collision(player_center_x, player_center_y, PLAYER_RAIO,
                    cama_center_x, cama_center_y, CAMA_RAIO)) {
                    can_interact = true;
                    quarto_interact_id = 1; // Cama
                }
                int PORTA_RAIO = 100;
                int porta_center_x = (PORTA_AREA.x1 + PORTA_AREA.x2) / 2;
                int porta_center_y = (PORTA_AREA.y1 + PORTA_AREA.y2) / 2;

                if (check_collision(player_center_x, player_center_y, PLAYER_RAIO,
                    porta_center_x, porta_center_y, PORTA_RAIO)) {
                    can_interact = true;
                    quarto_interact_id = 2; // Porta
                }
            }

            // --- Lógica de Interação/Colisão de NPCs (APENAS em TELA_JOGO) ---
            if (estado_atual == TELA_JOGO) {
                for (int i = 0; i < MAX_NPCS; i++) {
                    if (NPC_LIST[i].id != -1) { // Checa se o slot está em uso

                        int player_center_x = player_pos_x + FRAME_LARGURA / 2;
                        int player_center_y = player_pos_y + FRAME_ALTURA / 2;

                        if (check_collision(player_center_x, player_center_y, PLAYER_RAIO,
                            NPC_LIST[i].x, NPC_LIST[i].y, NPC_LIST[i].raio))
                        {
                            // Colisão detectada!
                            can_interact = true;
                            current_npc_id = NPC_LIST[i].id;
                            break;
                        }
                    }
                }
            }

            // --- Fim da Lógica de Atualização ---

            // --- Desenho ---
            al_clear_to_color(al_map_rgb(0, 0, 0)); // Limpa a tela

            switch (estado_atual) {
            case TELA_MENU:
                al_draw_bitmap(img_menu_fundo, 0, 0, 0);
                break;

            case TELA_QUARTO:
                al_draw_bitmap(img_quarto_fundo, 0, 0, 0);
                if (img_player_sprite) {
                    al_draw_bitmap_region(
                        img_player_sprite,
                        FRAME_LARGURA * (int)anim_frame,
                        anim_current_frame_y,
                        FRAME_LARGURA,
                        FRAME_ALTURA,
                        player_pos_x,
                        player_pos_y,
                        0
                    );
                }
                // desenha HUD
                desenhar_hud_texto();
                break;

            case TELA_JOGO:
                al_draw_bitmap(img_mapa_fundo, 0, 0, 0);
                for (int i = 0; i < MAX_NPCS; i++) {
                    if (NPC_LIST[i].id != -1) {
                        // (Opcional) desenhe NPCs aqui
                        // ALLEGRO_COLOR cor_npc = al_map_rgb(150, 150, 150);
                        // if (can_interact && current_npc_id == NPC_LIST[i].id) {
                        //     cor_npc = al_map_rgb(255, 0, 0);
                        // }
                        // al_draw_filled_circle(NPC_LIST[i].x, NPC_LIST[i].y, NPC_LIST[i].raio, cor_npc);
                    }
                }
                if (img_player_sprite) {
                    al_draw_bitmap_region(
                        img_player_sprite,
                        FRAME_LARGURA * (int)anim_frame,
                        anim_current_frame_y,
                        FRAME_LARGURA,
                        FRAME_ALTURA,
                        player_pos_x,
                        player_pos_y,
                        0
                    );
                }
                // desenha HUD
                desenhar_hud_texto();
                break;

            case TELA_MERCADO:
                al_draw_bitmap(img_mercado_fundo, 0, 0, 0);
                // EXEMPLO: Se quiser botões aqui, adicione detecção de mouse e chame comer(...)
                // Por exemplo: se clicar em uma área -> comer(30, 30);
                // desenhar HUD também
                desenhar_hud_texto();
                break;

            case TELA_CASSINO:
                al_draw_bitmap(img_cassino_fundo, 0, 0, 0);
                desenhar_hud_texto();
                break;

            case TELA_BANCO:
                al_draw_bitmap(img_banco_fundo, 0, 0, 0);
                desenhar_hud_texto();
                break;

            case TELA_TUTORIAL:
                al_draw_bitmap(img_tutorial_fundo, 0, 0, 0);
                // botão fechar desenhado pelo fundo ou debug
                break;

            case TELA_FIM_DIA:
                // Mostra resumo do dia e os valores
                al_draw_bitmap(img_fim_dia_fundo, 0, 0, 0);
                if (fonte_hud) {
                    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 520, 180, ALLEGRO_ALIGN_CENTER, "Fim do Dia %d", dia_atual - 1);
                    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 520, 220, ALLEGRO_ALIGN_CENTER, "Ganho no dia: R$ %d", ganho_dia);
                    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 520, 260, ALLEGRO_ALIGN_CENTER, "Gasto no dia: R$ %d", gasto_dia);
                    al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 520, 300, ALLEGRO_ALIGN_CENTER, "Saldo atual: R$ %d", dinheiro);
                    al_draw_textf(fonte_hud, al_map_rgb(200, 200, 200), 520, 360, ALLEGRO_ALIGN_CENTER, "Clique no botão FECHAR para voltar ao menu.");
                }
                break;

            case TELA_SAIR:
                // Usamos TELA_SAIR para Game Over também.
                al_clear_to_color(al_map_rgb(10, 10, 10));
                if (fonte_hud) {
                    if (fome <= 0 || dias_sem_comer >= 3) {
                        al_draw_textf(fonte_hud, al_map_rgb(255, 80, 80), 400, 260, 0, "GAME OVER - Você morreu de fome.");
                        al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 400, 300, 0, "Fome: %d%% | Dias sem comer: %d", fome, dias_sem_comer);
                        al_draw_textf(fonte_hud, al_map_rgb(200, 200, 200), 400, 340, 0, "Pressione R para reiniciar ou ESC para sair.");
                    }
                    else {
                        al_draw_textf(fonte_hud, al_map_rgb(255, 255, 255), 400, 300, 0, "Saindo do jogo...");
                    }
                }
                break;

            default:
                break;
            }

            al_flip_display();
        }
    }

    // --- Limpeza ---
    limpar_recursos();
    if (fonte_hud) al_destroy_font(fonte_hud);
    al_destroy_event_queue(fila);
    al_destroy_display(janela);
    al_destroy_timer(timer);

    return 0;
}
