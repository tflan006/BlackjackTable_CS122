#include "Card.h"
#include "pico/rand.h"

void Deck::build_deck() {
        int i = 0;
        for (int suit = 0; suit < 4; suit++) {
            for (int rank = 1; rank <= 13; rank++) {
                deck[i].suit = suit;
                deck[i].value = (rank > 10) ? 10 : rank;  // J/Q/K = 10
                i++;
            }
        }
    }

    void Deck::shuffle_deck() {
        for (int i = 51; i > 0; i--) {
            int j = get_rand_32() % (i + 1);
            Card tmp = deck[i];
            deck[i] = deck[j];
            deck[j] = tmp;
        }
        deck_index = 0;
    }

    Card Deck::deal_card() {
        return deck[deck_index++];
    }