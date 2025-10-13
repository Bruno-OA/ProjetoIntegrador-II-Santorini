#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h> // Necessário se for usar o font addon
#include <math.h>

// Definições de Tela
#define LARGURA_TELA 1280
#define ALTURA_TELA 720

// Definição dos Estados do Jogo
typedef enum {
    TELA_MENU = 0,
    TELA_JOGO = 1,
    TELA_SAIR = 2
} EstadoDoJogo;

// Variáveis Globais
EstadoDoJogo estado_atual = TELA_MENU;

// Bitmaps do Menu/Mapa
ALLEGRO_BITMAP* img_menu_fundo = NULL;
ALLEGRO_BITMAP* img_mapa_fundo = NULL;

// ===================================
// NOVAS VARIÁVEIS DO JOGADOR (SPRITE)
// ===================================
ALLEGRO_BITMAP* img_player_sprite = NULL;

// Variáveis de Posição, Velocidade e Animação
float player_pos_x = LARGURA_TELA / 2.0;
float player_pos_y = ALTURA_TELA / 2.0;
float player_velocidade = 7.0; // Velocidade em pixels por frame (igual ao seu novo código)

float anim_frame = 0.f;          // Contador do frame atual
int anim_current_frame_y = 63 * 2; // Linha do sprite (Começa olhando para baixo)
const int FRAME_LARGURA = 155;
const int FRAME_ALTURA = 134;
// ===================================

// Variáveis do Botão "Iniciar"
int btn_x1 = 540;
int btn_y1 = 550;
int btn_x2 = 740;
int btn_y2 = 600;

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

    return 1; // Sucesso
}

void limpar_recursos() {
    if (img_menu_fundo) al_destroy_bitmap(img_menu_fundo);
    if (img_mapa_fundo) al_destroy_bitmap(img_mapa_fundo);
    if (img_player_sprite) al_destroy_bitmap(img_player_sprite);
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
                if (ev.mouse.x >= btn_x1 && ev.mouse.x <= btn_x2 &&
                    ev.mouse.y >= btn_y1 && ev.mouse.y <= btn_y2)
                {
                    estado_atual = TELA_JOGO;
                    printf("Transição: Menu -> Jogo\n");
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (ev.keyboard.keycode < ALLEGRO_KEY_MAX) {
                key_down[ev.keyboard.keycode] = true;
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
            // Liberação da tecla
            if (ev.keyboard.keycode < ALLEGRO_KEY_MAX) {
                key_down[ev.keyboard.keycode] = false;
            }
        }

        else if (ev.type == ALLEGRO_EVENT_TIMER) {

            // --- Lógica de Atualização por Estado ---
            if (estado_atual == TELA_JOGO) {
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
            // --- Fim da Lógica de Atualização ---


            al_clear_to_color(al_map_rgb(0, 0, 0)); // Limpa a tela

            // --- Lógica de Desenho por Estado ---
            switch (estado_atual) {
            case TELA_MENU:
                al_draw_bitmap(img_menu_fundo, 0, 0, 0);
                al_draw_filled_rectangle(btn_x1, btn_y1, btn_x2, btn_y2, al_map_rgb(50, 200, 50));
                break;

            case TELA_JOGO:
                al_draw_bitmap(img_mapa_fundo, 0, 0, 0);

                // === DESENHO DO SPRITE ===
                if (img_player_sprite) {
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