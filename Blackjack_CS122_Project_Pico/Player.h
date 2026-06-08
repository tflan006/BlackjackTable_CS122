#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"
#include "Card.h"

#define MAX_PLAYERS 8

class Player {
public:
    // Identity
    ip4_addr_t ip;
    struct tcp_pcb *sse_pcb = nullptr;
    bool active = false;

    // Game state
    int chips = 1000;
    Card cards[10] = {};
    int card_count = 0;
    int cards_value = 0;
    char status_msg[64] = "Place your bet!";
    char game_state[16] = "waiting";
    int bet_amount = 15;

    // Flags from HTTP server
    volatile bool start_requested = false;
    volatile bool place_bet_requested = false;
    volatile bool hit_requested = false;
    volatile bool stand_requested = false;
    volatile bool bet_100_requested = false;
    volatile bool bet_10_requested = false;
    volatile bool bet_5_requested = false;
    volatile bool bet_1_requested = false;
    volatile bool reset_bet_requested = false;

    // Methods
    int calculate_hand_value() {
        int value = 0;
        int ace_count = 0;
        
        for (int i = 0; i < card_count; i++) {
            value += cards[i].value;
            if (cards[i].value == 1) 
            {
                ace_count++;
            }
        }

        for (int i = 0; i < ace_count; i++) {
            if (value + 10 <= 21) {
                value += 10;
            }
        }
        
       return value;
    }

    void draw_card(Card card) {
        cards[card_count] = card;
        card_count++;
        cards_value = calculate_hand_value();
    }

    void reset_hand() {
        card_count = 0;
        cards_value = 0;
        memset(cards, 0, sizeof(cards));
    }

    bool is_bust() {
        return cards_value > 21;
    }

    void place_bet(int amount) {
        chips -= amount;
    }

};


#endif