#ifndef LVGL_BLACKJACKAPP_H
#define LVGL_BLACKJACKAPP_H

#include "cs122_app.h"
#include "spi_display.h"
#include "Player.h"
#include "Dealer.h"

extern Player players[MAX_PLAYERS];


class BlackjackApp : public ucr::bcoe::cs::cs122::CS122_App {
public:
    BlackjackApp(ucr::bcoe::SPIDisplay *display);
    static void game_tick(lv_timer_t *t);
    uint32_t run() override;

private:
    void updateGame();
    void buildUI();
    void updateDealerCards(bool reveal);
    lv_obj_t* createCard(lv_obj_t *parent, int value, int suit, int x, int y);
    void clearDealerCards();

    lv_display_t *disp = lv_display_get_default();
    lv_obj_t *screen = lv_display_get_screen_active(disp);

    lv_obj_t *title;
    lv_obj_t *status_label;
    lv_obj_t *dealer_hand_label;
    lv_obj_t *dealer_cards_label;
    lv_obj_t *dealer_value_label;
    lv_obj_t *deck_container1 = nullptr;
    lv_obj_t *deck_container2 = nullptr;
    lv_obj_t *deck_container3 = nullptr;
    lv_obj_t *dealer_card_containers[10] = {};
    lv_obj_t *dealer_card_imgs[10] = {};
    lv_obj_t *hidden_container = nullptr;

    bool d1_started = false;
    bool d2_started = false;
    bool s1_started = false;
    int showing_card_num = 1;
    static BlackjackApp *instance;
};

#endif