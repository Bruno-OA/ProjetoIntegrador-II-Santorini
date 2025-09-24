#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>

int main() {
    al_init();
    al_init_primitives_addon();
    al_install_keyboard();

    ALLEGRO_DISPLAY* janela = al_create_display(1280, 720);
    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);

    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));

    float x = 640, y = 360;
    float velocidade = 5;

    bool teclas[ALLEGRO_KEY_MAX] = { false };

    float bolinhas_x[] = { 100, 1180, 100, 1180 };
    float bolinhas_y[] = { 100, 100, 620, 620 };
    float raio_player = 100;
    float raio_bolinha = 50;

    bool rodando = true;
    al_start_timer(timer);

    while (rodando) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(fila, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = false;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            teclas[ev.keyboard.keycode] = true;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            teclas[ev.keyboard.keycode] = false;
        }
        else if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (teclas[ALLEGRO_KEY_UP])    y -= velocidade;
            if (teclas[ALLEGRO_KEY_DOWN])  y += velocidade;
            if (teclas[ALLEGRO_KEY_LEFT])  x -= velocidade;
            if (teclas[ALLEGRO_KEY_RIGHT]) x += velocidade;

            if (x - raio_player < 0) x = raio_player;
            if (x + raio_player > 1280) x = 1280 - raio_player;
            if (y - raio_player < 0) y = raio_player;
            if (y + raio_player > 720) y = 720 - raio_player;

            if (teclas[ALLEGRO_KEY_SPACE]) {
                for (int i = 0; i < 4; i++) {
                    float distancia = sqrt(pow(x - bolinhas_x[i], 2) + pow(y - bolinhas_y[i], 2));
                    if (distancia < raio_player + raio_bolinha) {
                        printf("Dialogo com NPC %d!\n", i + 1); 
                    }
                }
            }

            al_clear_to_color(al_map_rgb(255, 255, 255));

            for (int i = 0; i < 4; i++) {
                al_draw_filled_circle(bolinhas_x[i], bolinhas_y[i], raio_bolinha, al_map_rgb(255, 0, 0));
            }

            al_draw_filled_circle(x, y, raio_player, al_map_rgb(0, 0, 255));
            al_flip_display();
        }
    }

    al_destroy_event_queue(fila);
    al_destroy_display(janela);
    al_destroy_timer(timer);

    return 0;
}