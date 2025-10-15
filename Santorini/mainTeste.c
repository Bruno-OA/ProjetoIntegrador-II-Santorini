#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h> // Necessário se for usar o font addon
#include <math.h>
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

// ===================================
// NOVAS VARIÁVEIS DO JOGADOR (SPRITE)
// ===================================
ALLEGRO_BITMAP* img_player_sprite = NULL;

// Variáveis de Posição, Velocidade e Animação
float player_pos_x = LARGURA_TELA / 2.0;
float player_pos_y = ALTURA_TELA / 2.0;
float player_velocidade = 7.0; // Velocidade em pixels por frame (igual ao seu novo código)
int PLAYER_RAIO = 20;
float anim_frame = 0.f;          // Contador do frame atual
int anim_current_frame_y = 63 * 2; // Linha do sprite (Começa olhando para baixo)
const int FRAME_LARGURA = 155;
const int FRAME_ALTURA = 134;
// ===================================

NPC NPC_LIST[MAX_NPCS] = {
    // ID, X, Y, RAIO
    { 1, 300, 300, 25 }, // NPC 1: João
    { 2, 800, 200, 30 }, // NPC 2: Maria
    { 3, 500, 600, 20 }, // NPC 3: Pedro
    { 4, 100, 100, 25 }, // NPC 4: (Ocioso)
    { -1, 0, 0, 0 }      // Slot Vazio (ou use 0 na posição se não usar todos)
};

// mapeamento de botoes
CoordenadasBotao INICIAR_BTN = {
    .x1 = 540,
    .y1 = 450,
    .x2 = 740,
    .y2 = 500
};
CoordenadasBotao TUTORIAL_BTN = {
    .x1 = 540,
    .y1 = 550,
    .x2 = 740,
    .y2 = 600
};
CoordenadasBotao FECHAR_BTN = {
    .x1 = 100, // Exemplo: Canto superior esquerdo
    .y1 = 50,
    .x2 = 250,
    .y2 = 100
};
CoordenadasBotao CAMA_AREA = {
    .x1 = 900,
    .y1 = 550,
    .x2 = 1200,
    .y2 = 700
};
CoordenadasBotao PORTA_AREA = {
    .x1 = 600,
    .y1 = 0,
    .x2 = 700,
    .y2 = 150
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


// Função para Carregar Imagens
int carregar_imagens() {
    // 1. Carrega o fundo do Menu
    img_menu_fundo = al_load_bitmap("menu_fundo.png");
    // ... (Seu código de tratamento de erro para menu_fundo) ...
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
    // ... (Seu código de tratamento de erro para mapa_fundo) ...
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

    // 3. Carrega o Sprite do Jogador (Substitua "sprite01.png" pelo caminho correto)
    img_player_sprite = al_load_bitmap("sprite0.2.png");
    if (!img_player_sprite) {
        fprintf(stderr, "ERRO: Não foi possível carregar sprite01.png. O jogador será invisível!\n");
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
    img_quarto_fundo = al_load_bitmap("quarto_fundo.png");
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

    // ... (Repetir para img_cassino_fundo - Verde Claro 150, 255, 150)
    img_cassino_fundo = al_load_bitmap("cassino_fundo.png");
    if (!img_cassino_fundo) { /* ... lógica de erro com al_map_rgb(150, 255, 150) ... */
        fprintf(stderr, "ERRO: Não foi possível carregar cassino_fundo.png. Usando cor simples.\n");
        img_cassino_fundo = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
        if (img_cassino_fundo) {
            al_set_target_bitmap(img_cassino_fundo);
            al_clear_to_color(al_map_rgb(150, 255, 150));
            al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
        }
        else return 0;
    }

    // ... (Repetir para img_banco_fundo - Azul Claro 150, 150, 255)
    img_banco_fundo = al_load_bitmap("banco_fundo.png");
    if (!img_banco_fundo) { /* ... lógica de erro com al_map_rgb(150, 150, 255) ... */
        fprintf(stderr, "ERRO: Não foi possível carregar banco_fundo.png. Usando cor simples.\n");
        img_banco_fundo = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
        if (img_banco_fundo) {
            al_set_target_bitmap(img_banco_fundo);
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
}


int main() {
    // --- Inicializações ---
    al_init();
    al_init_primitives_addon();
    al_install_keyboard();
    al_install_mouse();
    al_init_image_addon(); // ESSENCIAL para ler PNG/JPG!
    al_init_font_addon();  // Incluído conforme seu novo código (necessário para fontes)

    ALLEGRO_DISPLAY* janela = al_create_display(LARGURA_TELA, ALTURA_TELA);
    // Ajustei o timer para 1.0/60.0 para manter a consistência com jogos,
    // mas você pode usar 1.0/25.0 se preferir o seu valor original.
    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);

    // Carrega todas as imagens, incluindo o sprite do jogador
    if (!carregar_imagens()) {
        fprintf(stderr, "Falha crítica ao inicializar imagens.\n");
        limpar_recursos();
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
                    printf("Transição: Menu -> Jogo\n");
                }
                // Botão TUTORIAL
                if (ev.mouse.x >= TUTORIAL_BTN.x1 && ev.mouse.x <= TUTORIAL_BTN.x2 &&
                    ev.mouse.y >= TUTORIAL_BTN.y1 && ev.mouse.y <= TUTORIAL_BTN.y2)
                {
                    estado_atual = TELA_TUTORIAL;
                    printf("Transição: Menu -> Tutorial\n");
                }
            }
            else if (estado_atual == TELA_QUARTO) {

                // Interação com a CAMA (Encerra o dia)
                if (ev.mouse.x >= CAMA_AREA.x1 && ev.mouse.x <= CAMA_AREA.x2 &&
                    ev.mouse.y >= CAMA_AREA.y1 && ev.mouse.y <= CAMA_AREA.y2)
                {
                    // A transição para a tela final do dia deve ocorrer se o personagem estiver perto da cama
                    // Se você só usa clique, use o clique:
                    estado_atual = TELA_FIM_DIA;
                    printf("Interação: Quarto -> Fim do Dia (Cama)\n");
                }

                // Interação com a PORTA (Sai para o mapa)
                else if (ev.mouse.x >= PORTA_AREA.x1 && ev.mouse.x <= PORTA_AREA.x2 &&
                    ev.mouse.y >= PORTA_AREA.y1 && ev.mouse.y <= PORTA_AREA.y2)
                {
                    // Transição para a TELA_JOGO (Mapa)
                    estado_atual = TELA_JOGO;
                    // Opcional: Colocar jogador na entrada do mapa
                    // jogador_x = 640; 
                    // jogador_y = 600; 
                    printf("Interação: Quarto -> Jogo (Porta)\n");
                }
            }

            // --- NOVO: Lógica no FIM DO DIA ---
            else if (estado_atual == TELA_FIM_DIA) {
                // Exemplo: Usar o botão FECHAR para voltar ao Menu principal e iniciar um novo dia
                if (ev.mouse.x >= FECHAR_BTN.x1 && ev.mouse.x <= FECHAR_BTN.x2 &&
                    ev.mouse.y >= FECHAR_BTN.y1 && ev.mouse.y <= FECHAR_BTN.y2)
                {
                    estado_atual = TELA_MENU; // Volta para o menu
                    printf("Botão Fechar Clicado: Fim do Dia -> Menu\n");
                }
            }
            if (estado_atual == TELA_TUTORIAL) {
                if (ev.mouse.x >= FECHAR_BTN.x1 && ev.mouse.x <= FECHAR_BTN.x2 &&
                    ev.mouse.y >= FECHAR_BTN.y1 && ev.mouse.y <= FECHAR_BTN.y2)
                {
                    estado_atual = TELA_MENU; // Volta para o menu
                    printf("Botão Fechar Clicado: Tutorial -> Menu\n");
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (ev.keyboard.keycode < ALLEGRO_KEY_MAX) {
                key_down[ev.keyboard.keycode] = true;
            }
            if (estado_atual == TELA_JOGO) {
                // Exemplo de tecla 'E' para interagir
                if (ev.keyboard.keycode == ALLEGRO_KEY_E) {
                    if (can_interact && current_npc_id != -1) {
                        // MUDANÇA: Usar o ID do NPC para definir o estado
                        switch (current_npc_id) {
                        case 1: estado_atual = TELA_MERCADO; break;
                        case 2: estado_atual = TELA_CASSINO; break;
                        case 3: estado_atual = TELA_BANCO; break;
                        default: estado_atual = TELA_JOGO; break; // Ignora se ID for desconhecido
                        }
                        printf("Iniciando dialogo com NPC ID: %d. Tela: %d\n", current_npc_id, estado_atual);
                    }
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            // Lógica do ESC
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                if (estado_atual == TELA_JOGO) {
                    estado_atual = TELA_MENU;
                    printf("Transição: Jogo -> Menu\n");
                }
                else if (estado_atual == TELA_MENU) {
                    rodando = false;
                }
            }
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                if (estado_atual == TELA_MERCADO || estado_atual == TELA_CASSINO || estado_atual == TELA_BANCO) { // MUDANÇA AQUI
                    estado_atual = TELA_JOGO; // Volta para o mapa
                    current_npc_id = -1;
                    printf("Saindo do dialogo. Voltando para o Jogo.\n");
                }
            }
            // Liberação da tecla
            if (ev.keyboard.keycode < ALLEGRO_KEY_MAX) {
                key_down[ev.keyboard.keycode] = false;
            }
        }

        else if (ev.type == ALLEGRO_EVENT_TIMER) {

            // --- Lógica de Atualização por Estado ---
            if (estado_atual == TELA_JOGO || estado_atual == TELA_QUARTO) {
                bool andando = false;

                // === LÓGICA DE MOVIMENTAÇÃO E ANIMAÇÃO (DO SEU NOVO CÓDIGO) ===
                if (key_down[ALLEGRO_KEY_UP] || key_down[ALLEGRO_KEY_W]) {
                    player_pos_y -= player_velocidade;
                    anim_current_frame_y = FRAME_ALTURA * 3; // Olhando para cima
                    andando = true;
                }
                if (key_down[ALLEGRO_KEY_DOWN] || key_down[ALLEGRO_KEY_S]) {
                    player_pos_y += player_velocidade;
                    anim_current_frame_y = FRAME_ALTURA * 0 ; // Olhando para baixo
                    andando = true;
                }
                if (key_down[ALLEGRO_KEY_LEFT] || key_down[ALLEGRO_KEY_A]) {
                    player_pos_x -= player_velocidade;
                    anim_current_frame_y = FRAME_ALTURA ; // Olhando para a esquerda
                    andando = true;
                }
                if (key_down[ALLEGRO_KEY_RIGHT] || key_down[ALLEGRO_KEY_D]) {
                    player_pos_x += player_velocidade;
                    anim_current_frame_y = FRAME_ALTURA * 2; // Olhando para a direita
                    andando = true;
                }

                if (andando) {
                    anim_frame += 0.3f;
                    if (anim_frame >= 3) anim_frame = 0; // 4 frames (0,1,2,3)
                }
                else {
                    anim_frame = 0; // Parado no frame inicial da linha
                }

                // LIMITES DA JANELA
                if (player_pos_x < 0) player_pos_x = 0;
                if (player_pos_x > LARGURA_TELA - FRAME_LARGURA) player_pos_x = LARGURA_TELA - FRAME_LARGURA;
                if (player_pos_y < 0) player_pos_y = 0;
                if (player_pos_y > ALTURA_TELA - FRAME_ALTURA) player_pos_y = ALTURA_TELA - FRAME_ALTURA;
            }
            // Reset das flags de interação
            can_interact = false;
            current_npc_id = -1;

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


            al_clear_to_color(al_map_rgb(0, 0, 0)); // Limpa a tela

            // --- Lógica de Desenho por Estado ---
            switch (estado_atual) {
            case TELA_MENU:
                al_draw_bitmap(img_menu_fundo, 0, 0, 0);
                al_draw_filled_rectangle(INICIAR_BTN.x1, INICIAR_BTN.y1, INICIAR_BTN.x2, INICIAR_BTN.y2, al_map_rgb(50, 200, 50));
                al_draw_filled_rectangle(TUTORIAL_BTN.x1, TUTORIAL_BTN.y1, TUTORIAL_BTN.x2, TUTORIAL_BTN.y2, al_map_rgb(50, 200, 50));
                break;

            case TELA_QUARTO: // NOVO
                // Desenhar fundo do quarto (img_quarto_fundo)
                al_draw_bitmap(img_quarto_fundo, 0, 0, 0);
                //al_clear_to_color(al_map_rgb(100, 50, 50)); // Fundo de Quarto Simples
                // Desenhar Cama (para debug visual)
                al_draw_filled_rectangle(CAMA_AREA.x1, CAMA_AREA.y1, CAMA_AREA.x2, CAMA_AREA.y2, al_map_rgb(150, 150, 150));
                // Desenhar Porta (para debug visual)
                al_draw_filled_rectangle(PORTA_AREA.x1, PORTA_AREA.y1, PORTA_AREA.x2, PORTA_AREA.y2, al_map_rgb(10, 10, 10));
                if (img_player_sprite) {
                    al_draw_circle(
                        player_pos_x + FRAME_LARGURA / 2,
                        player_pos_y + FRAME_ALTURA / 2,
                        PLAYER_RAIO,
                        al_map_rgb(255, 0, 255), // Cor Magenta
                        2 // Espessura
                    );
                    al_draw_bitmap_region(
                        img_player_sprite,
                        FRAME_LARGURA * (int)anim_frame, // Posição X na folha de sprites
                        anim_current_frame_y,            // Posição Y na folha de sprites
                        FRAME_LARGURA,                   // Largura do frame
                        FRAME_ALTURA,                    // Altura do frame
                        player_pos_x,                    // Posição X na tela
                        player_pos_y,                    // Posição Y na tela
                        0
                    );
                }
                else {
                    // Desenha um quadrado vermelho substituto se o sprite falhar
                    al_draw_filled_rectangle(player_pos_x, player_pos_y, player_pos_x + FRAME_LARGURA, player_pos_y + FRAME_ALTURA, al_map_rgb(255, 0, 0));
                }
                break;
                break;

            case TELA_JOGO:
                al_draw_bitmap(img_mapa_fundo, 0, 0, 0);
                for (int i = 0; i < MAX_NPCS; i++) {
                    if (NPC_LIST[i].id != -1) {
                        // Cor do NPC: Verde se em colisão, Cinza se não
                        ALLEGRO_COLOR cor_npc = al_map_rgb(150, 150, 150);
                        if (can_interact && current_npc_id == NPC_LIST[i].id) {
                            cor_npc = al_map_rgb(255, 0, 0); // Fica Vermelho ao colidir

                            // Desenha o prompt de interação (Ex: Aperte E)
                            // Você precisará de uma fonte para isso!
                            // al_draw_text(sua_fonte, al_map_rgb(255, 255, 255), jogador_x, jogador_y - 40, ALLEGRO_ALIGN_CENTER, "APERTAR E");
                        }
                        al_draw_filled_circle(NPC_LIST[i].x, NPC_LIST[i].y, NPC_LIST[i].raio, cor_npc);
                    }
                }
                // === DESENHO DO SPRITE ===
                if (img_player_sprite) {
                    al_draw_circle(
                        player_pos_x + FRAME_LARGURA / 2,
                        player_pos_y + FRAME_ALTURA / 2,
                        PLAYER_RAIO,
                        al_map_rgb(255, 0, 255), // Cor Magenta
                        2 // Espessura
                    );
                    al_draw_bitmap_region(
                        img_player_sprite,
                        FRAME_LARGURA * (int)anim_frame, // Posição X na folha de sprites
                        anim_current_frame_y,            // Posição Y na folha de sprites
                        FRAME_LARGURA,                   // Largura do frame
                        FRAME_ALTURA,                    // Altura do frame
                        player_pos_x,                    // Posição X na tela
                        player_pos_y,                    // Posição Y na tela
                        0
                    );
                }
                else {
                    // Desenha um quadrado vermelho substituto se o sprite falhar
                    al_draw_filled_rectangle(player_pos_x, player_pos_y, player_pos_x + FRAME_LARGURA, player_pos_y + FRAME_ALTURA, al_map_rgb(255, 0, 0));
                }
                break;

            case TELA_MERCADO: // TELA 1 (NPC João)
                al_draw_bitmap(img_mercado_fundo, 0, 0, 0);
                // Botão de Sair (FECHAR_BTN)
                al_draw_filled_rectangle(FECHAR_BTN.x1, FECHAR_BTN.y1, FECHAR_BTN.x2, FECHAR_BTN.y2, al_map_rgb(200, 50, 50));
                break;

            case TELA_CASSINO: // TELA 2 (NPC Maria)
                al_draw_bitmap(img_cassino_fundo, 0, 0, 0);
                // Botão de Sair (FECHAR_BTN)
                al_draw_filled_rectangle(FECHAR_BTN.x1, FECHAR_BTN.y1, FECHAR_BTN.x2, FECHAR_BTN.y2, al_map_rgb(200, 50, 50));
                break;

            case TELA_BANCO: // TELA 3 (NPC Pedro)
                al_draw_bitmap(img_banco_fundo, 0, 0, 0);
                // Botão de Sair (FECHAR_BTN)
                al_draw_filled_rectangle(FECHAR_BTN.x1, FECHAR_BTN.y1, FECHAR_BTN.x2, FECHAR_BTN.y2, al_map_rgb(200, 50, 50));
                break;

            case TELA_TUTORIAL:
                // Desenha o fundo da Tela de Tutorial
                al_draw_bitmap(img_tutorial_fundo, 0, 0, 0);

                // Desenha o botão FECHAR/VOLTAR para retornar ao menu
                al_draw_filled_rectangle(FECHAR_BTN.x1, FECHAR_BTN.y1, FECHAR_BTN.x2, FECHAR_BTN.y2, al_map_rgb(0, 0, 0));
                // Nota: Você pode precisar do addon de fonte para escrever "VOLTAR" no botão

                break;

            case TELA_FIM_DIA: // NOVO
                // Desenhar fundo da tela final
                // al_draw_bitmap(img_fim_dia_fundo, 0, 0, 0);
                al_clear_to_color(al_map_rgb(20, 20, 20)); // Fundo de Fim de Dia Escuro

                // Desenhar botão para continuar/voltar (usando o FECHAR_BTN)
                al_draw_filled_rectangle(FECHAR_BTN.x1, FECHAR_BTN.y1, FECHAR_BTN.x2, FECHAR_BTN.y2, al_map_rgb(255, 255, 0));
                // al_draw_text(seu_font, al_map_rgb(0,0,0), 640, 575, ALLEGRO_ALIGN_CENTER, "NOVO DIA");
                break;

            case TELA_SAIR:
                rodando = false;
                break;
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