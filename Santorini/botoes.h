// botoes.h
#ifndef BOTOES_H
#define BOTOES_H

// 1. Definição da Estrutura
// Essa estrutura representa as coordenadas de qualquer botão.
typedef struct {
    int x1, y1; // Coordenada superior esquerda
    int x2, y2; // Coordenada inferior direita
} CoordenadasBotao;


// 2. Declaração das Instâncias de Botão
// Usamos 'extern' para que o main.c saiba que essas estruturas existem,
// mas serão definidas (inicializadas com valores) no main.c.

extern CoordenadasBotao INICIAR_BTN;
extern CoordenadasBotao FECHAR_BTN;
extern CoordenadasBotao TUTORIAL_BTN;
extern CoordenadasBotao CAMA_AREA;
extern CoordenadasBotao PORTA_AREA;

#endif