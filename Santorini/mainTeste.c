#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

int main() {
    al_init();
    al_init_primitives_addon();
    al_install_keyboard();

    ALLEGRO_DISPLAY* janela = al_create_display(1800, 900);
    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);

    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));

    float x = 800, y = 400;
    float velocidade = 5;

    // Array que guarda o estado de todas as teclas
    bool teclas[ALLEGRO_KEY_MAX] = { false };

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

        if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            teclas[ev.keyboard.keycode] = false;
        }
        else if (ev.type == ALLEGRO_EVENT_TIMER) {
            // Movimento baseado nas teclas pressionadas
            if (teclas[ALLEGRO_KEY_UP])    y -= velocidade;
            if (teclas[ALLEGRO_KEY_DOWN])  y += velocidade;
            if (teclas[ALLEGRO_KEY_LEFT])  x -= velocidade;
            if (teclas[ALLEGRO_KEY_RIGHT]) x += velocidade;
            // limites da janela
            if (x < 100) x = 100;
            if (x > 1800 - 100) x = 1800 - 100;
            if (y < 100) y = 100;
            if (y > 900 - 100) y = 900 - 100;


            // Redesenha
            al_clear_to_color(al_map_rgb(255, 255, 255));
            al_draw_filled_circle(x, y, 100, al_map_rgb(0, 0, 255));
            al_flip_display();
        }
    }

    al_destroy_event_queue(fila);
    al_destroy_display(janela);
    al_destroy_timer(timer);

    return 0;
}