// npcs.h
#ifndef NPCS_H
#define NPCS_H

#define MAX_NPCS 5 // Limite máximo de NPCs no mapa

// Estrutura para mapear um NPC e seu raio de colisão
typedef struct {
    int id;
    int x;
    int y;
    int raio;
    // ... (pode adicionar nome, sprite_id, etc.)
} NPC;

// Declaração da lista de NPCs (será definida em main.c)
extern NPC NPC_LIST[MAX_NPCS];

// Variável para a colisão
extern int PLAYER_RAIO;

#endif