/* #include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
// 1. Inclua o addon de imagem
#include <allegro5/allegro_image.h> 
#include <math.h>

// Definição dos Estados do Jogo (Enums)
typedef enum {
    TELA_MENU = 0,
    TELA_OPCOES = 1,
    TELA_FIM = 2
} EstadoDoJogo;

// Definições de Tela
#define LARGURA_TELA 1280
#define ALTURA_TELA 720

int main() {
    // --- Inicializações ---
    al_init();
    al_init_primitives_addon();
    al_install_keyboard();
    // 1. Inicialize o addon de imagem
    al_init_image_addon();
    // 2. Instale o mouse para os botões
    al_install_mouse();

    ALLEGRO_DISPLAY* janela = al_create_display(LARGURA_TELA, ALTURA_TELA);
    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);

    // Registra o mouse
    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));
    al_register_event_source(fila, al_get_mouse_event_source()); // Registra o mouse

    // Variável para controlar a tela atual
    EstadoDoJogo estado_atual = TELA_MENU;

    // Variáveis para as Imagens (Bitmaps)
    ALLEGRO_BITMAP* img_menu = NULL;
    ALLEGRO_BITMAP* img_opcoes = NULL;

    // --- 3. Carregamento de Imagens ---
    // Você DEVE ter estas imagens na mesma pasta do seu executável para este teste!
    // Se não tiver, o código abaixo vai SIMULAR uma imagem simples.

    // -------------------------------------------------------------------------
    // ATENÇÃO: SUBSTITUA os al_create_bitmap pelas suas imagens se elas existirem!
    // Ex: img_menu = al_load_bitmap("seu_menu.png");
    // -------------------------------------------------------------------------

    // Simulando a imagem do Menu
    img_menu = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
    if (img_menu) {
        al_set_target_bitmap(img_menu);
        al_clear_to_color(al_map_rgb(100, 100, 200)); // Fundo Azul (Menu)
        al_draw_filled_rectangle(450, 400, 850, 500, al_map_rgb(50, 200, 50)); // Botão
        al_set_target_bitmap(al_get_backbuffer(janela));
    }

    // Simulando a imagem de Opções
    img_opcoes = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
    if (img_opcoes) {
        al_set_target_bitmap(img_opcoes);
        al_clear_to_color(al_map_rgb(200, 100, 100)); // Fundo Vermelho (Opções)
        al_draw_filled_rectangle(450, 400, 850, 500, al_map_rgb(50, 200, 50)); // Botão
        al_set_target_bitmap(al_get_backbuffer(janela));
    }

    if (!img_menu || !img_opcoes) {
        fprintf(stderr, "Erro ao carregar ou criar bitmaps. Verifique os caminhos das imagens.\n");
        // Não é estritamente necessário, mas é bom ter uma verificação
    }

    // Variáveis do Botão (simples)
    int btn_x1 = 450;
    int btn_y1 = 400;
    int btn_x2 = 850;
    int btn_y2 = 500;

    bool rodando = true;
    al_start_timer(timer);

    // --- Loop Principal ---
    while (rodando) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(fila, &ev);

        // --- Lógica de Transição (Clique do Mouse) ---
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = false;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            // Verifica se o clique ocorreu DENTRO da área do botão
            if (ev.mouse.x >= btn_x1 && ev.mouse.x <= btn_x2 &&
                ev.mouse.y >= btn_y1 && ev.mouse.y <= btn_y2)
            {
                // Se o botão foi clicado, muda o estado
                switch (estado_atual) {
                case TELA_MENU:
                    estado_atual = TELA_OPCOES; // Vai para a tela de Opções
                    printf("Transição: Menu -> Opções\n");
                    break;
                case TELA_OPCOES:
                    estado_atual = TELA_FIM; // Vai para a tela de Fim/Sair
                    printf("Transição: Opções -> Fim\n");
                    break;
                case TELA_FIM:
                    rodando = false; // Fecha o jogo
                    break;
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_TIMER) {

            al_clear_to_color(al_map_rgb(0, 0, 0)); // Limpa a tela

            // --- 4. Lógica de Desenho por Estado ---
            switch (estado_atual) {
            case TELA_MENU:
                // Desenha a imagem de fundo do Menu
                if (img_menu) {
                    al_draw_bitmap(img_menu, 0, 0, 0);
                }
                // Desenha um texto (se você incluir o addon de fonte) ou apenas um retângulo para o botão
                al_draw_filled_rectangle(btn_x1, btn_y1, btn_x2, btn_y2, al_map_rgb(50, 200, 50));
                break;

            case TELA_OPCOES:
                // Desenha a imagem de fundo das Opções
                if (img_opcoes) {
                    al_draw_bitmap(img_opcoes, 0, 0, 0);
                }
                // Desenha o botão
                al_draw_filled_rectangle(btn_x1, btn_y1, btn_x2, btn_y2, al_map_rgb(200, 50, 50));
                break;

            case TELA_FIM:
                // Tela final: apenas texto simples e fecha o jogo na próxima iteração.
                al_clear_to_color(al_map_rgb(0, 0, 0));
                // Nota: para texto de verdade, você precisaria do 'font addon'
                // al_draw_text(font, al_map_rgb(255, 255, 255), 640, 360, ALLEGRO_ALIGN_CENTER, "FIM DO JOGO!");
                rodando = false; // Fecha na próxima atualização/evento
                break;
            }

            al_flip_display();
        }
    }

    // --- Limpeza ---
    if (img_menu) al_destroy_bitmap(img_menu);
    if (img_opcoes) al_destroy_bitmap(img_opcoes);
    al_destroy_event_queue(fila);
    al_destroy_display(janela);
    al_destroy_timer(timer);

    return 0;
} */

#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h> // Adicionado para carregar imagens
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

// Variáveis Globais (para simplificar o exemplo)
EstadoDoJogo estado_atual = TELA_MENU;

// Bitmaps
ALLEGRO_BITMAP* img_menu_fundo = NULL;
ALLEGRO_BITMAP* img_mapa_fundo = NULL;

// Variáveis do Botão "Iniciar"
int btn_x1 = 540;
int btn_y1 = 550;
int btn_x2 = 740;
int btn_y2 = 600;

// Função para Carregar Imagens
int carregar_imagens() {
    // 1. Carrega o fundo do Menu
    img_menu_fundo = al_load_bitmap("menu_fundo.png");
    if (!img_menu_fundo) {
        fprintf(stderr, "ERRO: Não foi possível carregar menu_fundo.png. Use uma imagem valida!\n");
        // Se a imagem não carregar, tentamos criar um fundo simples para continuar o teste
        img_menu_fundo = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
        if (img_menu_fundo) {
            al_set_target_bitmap(img_menu_fundo);
            al_clear_to_color(al_map_rgb(100, 100, 200));
            al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
        }
        else return 0;
    }

    // 2. Carrega o fundo do Mapa (pode ser grande, ex: 2560x1440)
    img_mapa_fundo = al_load_bitmap("mapa_fundo.png");
    if (!img_mapa_fundo) {
        fprintf(stderr, "ERRO: Não foi possível carregar mapa_fundo.png. Usando cor simples.\n");
        // Cria um fundo simples para a tela de jogo
        img_mapa_fundo = al_create_bitmap(LARGURA_TELA, ALTURA_TELA);
        if (img_mapa_fundo) {
            al_set_target_bitmap(img_mapa_fundo);
            al_clear_to_color(al_map_rgb(100, 200, 100)); // Fundo Verde
            al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
        }
        else return 0;
    }

    return 1; // Sucesso
}

void limpar_recursos() {
    if (img_menu_fundo) al_destroy_bitmap(img_menu_fundo);
    if (img_mapa_fundo) al_destroy_bitmap(img_mapa_fundo);
}


int main() {
    // --- Inicializações ---
    al_init();
    al_init_primitives_addon();
    al_install_keyboard();
    al_install_mouse();
    al_init_image_addon(); // ESSENCIAL para ler PNG/JPG!

    ALLEGRO_DISPLAY* janela = al_create_display(LARGURA_TELA, ALTURA_TELA);
    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);

    // Carrega todas as imagens
    if (!carregar_imagens()) {
        fprintf(stderr, "Falha crítica ao inicializar imagens.\n");
        limpar_recursos();
        return -1;
    }

    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));
    al_register_event_source(fila, al_get_mouse_event_source());

    bool rodando = true;
    al_start_timer(timer);

    while (rodando) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(fila, &ev);

        // --- Lógica de Transição (Clique do Mouse) ---
        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = false;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            // Verifica o clique SOMENTE se estiver no MENU
            if (estado_atual == TELA_MENU) {
                // Verifica se o clique ocorreu DENTRO da área do botão "Iniciar"
                if (ev.mouse.x >= btn_x1 && ev.mouse.x <= btn_x2 &&
                    ev.mouse.y >= btn_y1 && ev.mouse.y <= btn_y2)
                {
                    estado_atual = TELA_JOGO; // Mudar para a tela de Jogo
                    printf("Transição: Menu -> Jogo\n");
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            // Adicionando uma tecla de escape para voltar ao menu ou sair do jogo
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                if (estado_atual == TELA_JOGO) {
                    estado_atual = TELA_MENU; // Volta para o menu
                    printf("Transição: Jogo -> Menu\n");
                }
                else if (estado_atual == TELA_MENU) {
                    rodando = false; // Fecha o jogo
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_TIMER) {

            al_clear_to_color(al_map_rgb(0, 0, 0)); // Limpa a tela

            // --- Lógica de Desenho por Estado ---
            switch (estado_atual) {
            case TELA_MENU:
                // Desenha o fundo da tela de menu (imagem)
                al_draw_bitmap(img_menu_fundo, 0, 0, 0);

                // Desenha o Botão "Iniciar" (retângulo verde)
                al_draw_filled_rectangle(btn_x1, btn_y1, btn_x2, btn_y2, al_map_rgb(50, 200, 50));
                // (Você pode adicionar texto ao botão depois com o font addon)
                break;

            case TELA_JOGO:
                // Desenha o mapa de fundo na tela de jogo
                // Usamos al_draw_bitmap, pois o mapa (por enquanto) é do tamanho da tela.
                al_draw_bitmap(img_mapa_fundo, 0, 0, 0);

                // Aqui você desenharia seu player e outros elementos
                // Exemplo de Player no centro:
                al_draw_filled_circle(LARGURA_TELA / 2, ALTURA_TELA / 2, 20, al_map_rgb(0, 0, 255));
                break;

            case TELA_SAIR:
                rodando = false; // O jogo fecha
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

//teste

