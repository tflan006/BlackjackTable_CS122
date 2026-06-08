#ifndef CARD_H
#define CARD_H

struct Card {
    int value;  // 1-10 (face cards = 10, ace = 1 or 11)
    int suit;   // 0-3 (hearts, diamonds, clubs, spades)
};

class Deck {
public:
    Card deck[52];
    int deck_index = 0;

    void build_deck();

    void shuffle_deck();

    Card deal_card();
};

#endif