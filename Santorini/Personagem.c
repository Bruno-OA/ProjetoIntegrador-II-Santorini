
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

int main() {
    al_init();
    al_init_font_addon();
    al_init_image_addon();
    al_install_keyboard();

    ALLEGRO_DISPLAY* display = al_create_display(1280, 720);
    al_set_window_title(display, "Santorinni!");

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 30.0);
    ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
    ALLEGRO_BITMAP* sprite = al_load_bitmap("./sprite_principal.jpg");

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_start_timer(timer);

    float frame = 0.f;
    int pos_x = 100, pos_y = 100;
    int current_row = 0;   // linha do sprite (0=cima,1=direita,2=baixo,3=esquerda)

    const int FRAME_W = 191; // 573 / 3
    const int FRAME_H = 161; // 644 / 4

    while (true) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN) { 
            if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                current_row = 1;
                pos_x += 20;
            }
            else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                current_row = 3;
                pos_x -= 20;
            }
            else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
                current_row = 2;
                pos_y += 20;
            }
            else if (event.keyboard.keycode == ALLEGRO_KEY_UP) {
                current_row = 0;
                pos_y -= 20;
            }
        }

        if (event.type == ALLEGRO_EVENT_TIMER) {
            frame += 0.3f;
            if (frame > 2.9f) frame = 0;

            al_clear_to_color(al_map_rgb(255, 255, 255));

            int src_x = (int)frame * FRAME_W;
            int src_y = current_row * FRAME_H;

            al_draw_bitmap_region(sprite, src_x, src_y, FRAME_W, FRAME_H,
                pos_x, pos_y, 0);

            al_flip_display();
        }
    }

    al_destroy_bitmap(sprite);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    al_destroy_timer(timer);

    return 0;
}
