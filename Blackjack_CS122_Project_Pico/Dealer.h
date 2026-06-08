#ifndef DEALER_H
#define DEALER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"
#include "Card.h"
#include "pico/rand.h"

class Dealer {
public:
    Card cards[10] = {};
    int card_count = 0;
    int cards_value = 0;

    int calculate_hand_value() {
        int value = 0;
        
        for (int i = 0; i < card_count; i++) {
            value += cards[i].value;
            if (cards[i].value == 1) 
            {
                if (value + 10 <= 21) {
                    value += 10; // Count ace as 11 if it doesn't bust
                }
            }
        }
        
       return value;
    }
    
    void draw_card(Card card) {
        cards[card_count] = card;
        card_count++;
        cards_value = calculate_hand_value();
    }

    bool is_bust() {
        return cards_value > 21;
    }

    void reset_hand() {
        card_count = 0;
        cards_value = 0;
        memset(cards, 0, sizeof(cards));
    }
};


#endif