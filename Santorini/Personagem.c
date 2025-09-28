#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/keyboard.h>

int main() {
    al_init();
    al_init_font_addon();
    al_init_image_addon();
    al_install_keyboard();

    ALLEGRO_DISPLAY* janela = al_create_display(1280, 720);
    ALLEGRO_EVENT_QUEUE* fila = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 25.0);
    ALLEGRO_BITMAP* sprite = al_load_bitmap("./sprite01.png");
    ALLEGRO_BITMAP* bg = al_load_bitmap("./bg_1.png");

    al_register_event_source(fila, al_get_display_event_source(janela));
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_timer_event_source(timer));
    al_start_timer(timer);

    // ESTADO DAS TECLAS
    bool teclas[ALLEGRO_KEY_MAX] = { false };

    // POSIÇÃO DO PERSONAGEM
    float frame = 0.f;
    float pos_x = 500, pos_y = 650;
    int current_frame_y = 63;

    bool rodando = true;

    // LOOP PRINCIPAL
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
            bool andando = false;
        

            // MOVIMENTAÇÃO
            if (teclas[ALLEGRO_KEY_UP]) { pos_y -= 10; current_frame_y = 0; andando = true;}
            if (teclas[ALLEGRO_KEY_DOWN]) { pos_y += 10; current_frame_y = 63 * 2; andando = true;}
            if (teclas[ALLEGRO_KEY_LEFT]) { pos_x -= 10; current_frame_y = 63 * 3; andando = true;}
            if (teclas[ALLEGRO_KEY_RIGHT]) { pos_x += 10; current_frame_y = 63; andando = true;}

        
            if (andando) {
                frame += 0.3f;
                if (frame > 3) frame = 0; // 4 frames (0,1,2,3)
            }
            else {
                frame = 0; // parado no frame inicial
            }


            // LIMITES DA JANELA
            if (pos_x < 0) pos_x = 0;
            if (pos_x > 1280 - 47) pos_x = 1280 - 47;  //47 = largura frame
            if (pos_y < 0) pos_y = 0;
            if (pos_y > 720 - 63) pos_y = 720 - 63;    //63 = altura frame

        }

            // DESENHA
            al_clear_to_color(al_map_rgb(255, 255, 255));
            al_draw_bitmap(bg, 0, 0, 0);
            al_draw_bitmap_region(sprite, 47 * (int)frame, current_frame_y, 47, 63, pos_x, pos_y, 0);
            al_flip_display();
            
    }

    // LIMPEZA 
    al_destroy_bitmap(bg);
    al_destroy_bitmap(sprite);
    al_destroy_event_queue(fila);
    al_destroy_display(janela);
    al_destroy_timer(timer);

    return 0;
}
