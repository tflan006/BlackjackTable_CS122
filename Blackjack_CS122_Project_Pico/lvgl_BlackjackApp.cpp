#include "lvgl_BlackjackApp.h"
#include "pico/time.h"
#include "Card.h"
#include "card_back.h"
#include "card_font.h"
#include "card_font_small.h"
#include "chips.h"
#include "http_server.h"
#include <string>


void cs122_flush_cb_partial(lv_display_t * disp, const lv_area_t * area, uint8_t * px_buf) {
    ucr::bcoe::SPIDisplay *spi_display = 
        reinterpret_cast<ucr::bcoe::SPIDisplay *>(lv_display_get_user_data(disp));
    spi_display->drawBitmap(2 * area->x1, area->y1, 2 * area->x2 + 1, area->y2, px_buf);
    lv_display_flush_ready(disp);
}

static uint32_t my_tick_cb() {
    return to_ms_since_boot(get_absolute_time());
}

BlackjackApp *BlackjackApp::instance = nullptr;
BlackjackApp::BlackjackApp(ucr::bcoe::SPIDisplay *display)
    : CS122_App(display, cs122_flush_cb_partial, my_tick_cb) {instance = this;}

void BlackjackApp::game_tick(lv_timer_t *t) {
    BlackjackApp *app = (BlackjackApp *)lv_timer_get_user_data(t);
    app->updateGame();
}

uint32_t BlackjackApp::run() {
    buildUI();
    lv_timer_handler();
    spi_set_baudrate(spi_default, 4000000);
    lv_timer_create(game_tick, 100, this);  // runs every 100ms
    return loop();
}

Dealer dealer;
Deck deck;

void BlackjackApp::buildUI() {
   
    // Set background color
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x005500), 0);

    // Title
    this->title = lv_label_create(screen);
    lv_label_set_text(this->title, "Blackjack Table");
    lv_obj_set_style_text_color(this->title, lv_color_hex(0xFFCC00), 0);
    lv_obj_set_style_text_font(this->title, &lv_font_montserrat_24, 0);
    lv_obj_align(this->title, LV_ALIGN_TOP_LEFT, 10, 10);

    // Status 
    lv_obj_t *status_block = lv_obj_create(screen);
    lv_obj_set_size(status_block, 175, 35);
    lv_obj_align(status_block, LV_ALIGN_TOP_LEFT, 5, 47);
    lv_obj_set_style_radius(status_block, 8, 0);
    lv_obj_set_style_bg_color(status_block, lv_color_hex(0x004100), 0);
    lv_obj_set_style_bg_opa(status_block, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(status_block, lv_color_hex(0xAA0000), 0);
    lv_obj_set_style_border_width(status_block, 1, 0);
    lv_obj_set_style_pad_all(status_block, 0, 0);
    lv_obj_set_style_clip_corner(status_block, true, 0);

    this->status_label = lv_label_create(screen);
    lv_label_set_text(this->status_label, "Waiting to start...");
    lv_obj_set_style_text_color(this->status_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(this->status_label, &lv_font_montserrat_16, 0);
    lv_obj_align(this->status_label, LV_ALIGN_TOP_LEFT, 15, 55);
    

    // Dealer hand
    lv_obj_t *dealer_block = lv_obj_create(screen);
    lv_obj_set_size(dealer_block, 340, 170);
    lv_obj_align(dealer_block, LV_ALIGN_TOP_LEFT, 5, 92);
    lv_obj_set_style_radius(dealer_block, 8, 0);
    lv_obj_set_style_bg_color(dealer_block, lv_color_hex(0x004100), 0);
    lv_obj_set_style_bg_opa(dealer_block, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(dealer_block, lv_color_hex(0xAA0000), 0);
    lv_obj_set_style_border_width(dealer_block, 1, 0);
    lv_obj_set_style_pad_all(dealer_block, 0, 0);
    lv_obj_set_style_clip_corner(dealer_block, true, 0);

    this->dealer_hand_label = lv_label_create(screen);
    lv_label_set_text(this->dealer_hand_label, "Dealer's Hand:");
    lv_obj_set_style_text_color(this->dealer_hand_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(this->dealer_hand_label, &lv_font_montserrat_16, 0);
    lv_obj_align(this->dealer_hand_label, LV_ALIGN_TOP_LEFT, 15, 100);

    // Dealer cards
    this->dealer_value_label = lv_label_create(screen);
    lv_label_set_text(this->dealer_value_label, "");
    lv_obj_set_style_text_color(this->dealer_value_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(this->dealer_value_label, &lv_font_montserrat_16, 0);
    lv_obj_align(this->dealer_value_label, LV_ALIGN_TOP_LEFT, 15, 125);

    //deck block
    lv_obj_t *deck_block = lv_obj_create(screen);
    lv_obj_set_size(deck_block, 103, 140);
    lv_obj_set_pos(deck_block, 361, 110);
    lv_obj_set_style_radius(deck_block, 8, 0);
    lv_obj_set_style_bg_color(deck_block, lv_color_hex(0x004100), 0);
    lv_obj_set_style_bg_opa(deck_block, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(deck_block, lv_color_hex(0xAA0000), 0);
    lv_obj_set_style_border_width(deck_block, 1, 0);
    lv_obj_set_style_pad_all(deck_block, 0, 0);
    lv_obj_set_style_clip_corner(deck_block, true, 0);

    //deck container 1
    this->deck_container1 = lv_obj_create(screen);
    lv_obj_set_size(this->deck_container1, 70, 121);
    lv_obj_set_style_radius(this->deck_container1, 8, 0);
    lv_obj_set_style_clip_corner(this->deck_container1, true, 0);
    lv_obj_set_style_pad_all(this->deck_container1, 0, 0);
    lv_obj_set_style_border_width(this->deck_container1, 0, 0);
    lv_obj_set_style_bg_opa(this->deck_container1, LV_OPA_TRANSP, 0);
    lv_obj_set_pos(this->deck_container1, 385, 120);//378 20

    //Image inside the container
    lv_obj_t *deck_img1 = lv_image_create(this->deck_container1);
    lv_image_set_src(deck_img1, &card);
    lv_obj_center(deck_img1);

    //deck container 2
    this->deck_container2 = lv_obj_create(screen);
    lv_obj_set_size(this->deck_container2, 70, 121);
    lv_obj_set_style_radius(this->deck_container2, 8, 0);
    lv_obj_set_style_clip_corner(this->deck_container2, true, 0);
    lv_obj_set_style_pad_all(this->deck_container2, 0, 0);
    lv_obj_set_style_border_width(this->deck_container2, 0, 0);
    lv_obj_set_style_bg_opa(this->deck_container2, LV_OPA_TRANSP, 0);
    lv_obj_set_pos(this->deck_container2, 379, 120);

    //Image inside the container
    lv_obj_t *deck_img2 = lv_image_create(this->deck_container2);
    lv_image_set_src(deck_img2, &card);
    lv_obj_center(deck_img2);
    
    //deck container 3
    this->deck_container3 = lv_obj_create(screen);
    lv_obj_set_size(this->deck_container3, 70, 121);
    lv_obj_set_style_radius(this->deck_container3, 8, 0);
    lv_obj_set_style_clip_corner(this->deck_container3, true, 0);
    lv_obj_set_style_pad_all(this->deck_container3, 0, 0);
    lv_obj_set_style_border_width(this->deck_container3, 0, 0);
    lv_obj_set_style_bg_opa(this->deck_container3, LV_OPA_TRANSP, 0);
    lv_obj_set_pos(this->deck_container3, 371, 120);

    //Image inside the container
    lv_obj_t *deck_img3 = lv_image_create(this->deck_container3);
    lv_image_set_src(deck_img3, &card);
    lv_obj_center(deck_img3);

    //chip block
    lv_obj_t *chip_block = lv_obj_create(screen);
    lv_obj_set_size(chip_block, 200, 70);
    lv_obj_set_pos(chip_block, 240, 10);
    lv_obj_set_style_radius(chip_block, 8, 0);
    lv_obj_set_style_bg_color(chip_block, lv_color_hex(0x004100), 0);
    lv_obj_set_style_bg_opa(chip_block, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(chip_block, lv_color_hex(0xAA0000), 0);
    lv_obj_set_style_border_width(chip_block, 1, 0);
    lv_obj_set_style_pad_all(chip_block, 0, 0);
    lv_obj_set_style_clip_corner(chip_block, true, 0);

    //Image inside the container
    lv_obj_t *chips_img = lv_image_create(chip_block);
    lv_image_set_src(chips_img, &chips1);
    lv_obj_center(chips_img);
}

struct AnimDoneData {
    lv_obj_t *obj_to_delete;
    lv_anim_completed_cb_t user_cb;
};

void animate_deal(int32_t start_x, int32_t start_y, int32_t end_x, int32_t end_y, uint32_t duration_ms, lv_anim_completed_cb_t on_complete = nullptr) 
{
    lv_obj_t *screen = lv_display_get_screen_active(lv_display_get_default());

    lv_obj_t *container = lv_obj_create(screen);
    lv_obj_set_size(container, 70, 121);
    lv_obj_set_style_radius(container, 8, 0);
    lv_obj_set_style_clip_corner(container, true, 0);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_pos(container, start_x, start_y);

    lv_obj_t *img = lv_image_create(container);
    lv_image_set_src(img, &card);
    lv_obj_center(img);

    lv_anim_t anim_x;
    lv_anim_init(&anim_x);
    lv_anim_set_var(&anim_x, container);
    lv_anim_set_exec_cb(&anim_x, [](void *obj, int32_t val) {
        lv_obj_set_x((lv_obj_t *)obj, val);
    });
    lv_anim_set_values(&anim_x, start_x, end_x);
    lv_anim_set_time(&anim_x, duration_ms);
    lv_anim_start(&anim_x);

    lv_anim_t anim_y;
    lv_anim_init(&anim_y);
    lv_anim_set_var(&anim_y, container);
    lv_anim_set_exec_cb(&anim_y, [](void *obj, int32_t val) {
        lv_obj_set_y((lv_obj_t *)obj, val);
    });
    lv_anim_set_values(&anim_y, start_y, end_y);
    lv_anim_set_time(&anim_y, duration_ms);
    lv_anim_set_completed_cb(&anim_y, [](lv_anim_t *a) {
    AnimDoneData *data = (AnimDoneData *)lv_anim_get_user_data(a);
    lv_obj_delete(data->obj_to_delete);
    if (data->user_cb) data->user_cb(a);
    delete data;
    });
    lv_anim_set_user_data(&anim_y, new AnimDoneData{container, on_complete});
    lv_anim_start(&anim_y);
}

 

lv_obj_t* BlackjackApp::createCard(lv_obj_t *parent, int value, int suit, int x, int y) {
    // suit symbols
    const char* suit_symbol;
    lv_color_t suit_color;
    switch(suit) {
        case 0: suit_symbol = "♥"; suit_color = lv_color_hex(0xFF0000); break;
        case 1: suit_symbol = "♦"; suit_color = lv_color_hex(0xFF0000); break;
        case 2: suit_symbol = "♠"; suit_color = lv_color_hex(0x000000); break;
        case 3: suit_symbol = "♣"; suit_color = lv_color_hex(0x000000); break;
        default: suit_symbol = "?"; suit_color = lv_color_hex(0x000000); break;
    }

    // value string
    char val_str[4];
    switch(value) {
        case 1:  strcpy(val_str, "A");  break;
        case 11: strcpy(val_str, "J");  break;
        case 12: strcpy(val_str, "Q");  break;
        case 13: strcpy(val_str, "K");  break;
        default: snprintf(val_str, sizeof(val_str), "%d", value); break;
    }

    // card container
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, 70, 121);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_style_radius(card, 8, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_set_style_clip_corner(card, true, 0);

    // top-left value
    lv_obj_t *top_val = lv_label_create(card);
    lv_label_set_text(top_val, val_str);
    lv_obj_set_style_text_color(top_val, suit_color, 0);
    lv_obj_set_style_text_font(top_val, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(top_val, 4, 2);

    // top-left suit 
    lv_obj_t *top_suit = lv_label_create(card);
    lv_label_set_text(top_suit, suit_symbol);
    lv_obj_set_style_text_color(top_suit, suit_color, 0);
    lv_obj_set_style_text_font(top_suit, &card_font_small, 0);
    lv_obj_set_pos(top_suit, 4, 23);

    // center suit symbol 
    lv_obj_t *center_suit = lv_label_create(card);
    lv_label_set_text(center_suit, suit_symbol);
    lv_obj_set_style_text_color(center_suit, suit_color, 0);
    lv_obj_set_style_text_font(center_suit, &card_font, 0);
    lv_obj_align(center_suit, LV_ALIGN_CENTER, 0, 0);

    // bottom-right value 
    lv_obj_t *bot_val = lv_label_create(card);
    lv_label_set_text(bot_val, val_str);
    lv_obj_set_style_text_color(bot_val, suit_color, 0);
    lv_obj_set_style_text_font(bot_val, &lv_font_montserrat_20, 0);
    lv_obj_align(bot_val, LV_ALIGN_BOTTOM_RIGHT, -4, -16);

    // bottom-right suit
    lv_obj_t *bot_suit = lv_label_create(card);
    lv_label_set_text(bot_suit, suit_symbol);
    lv_obj_set_style_text_color(bot_suit, suit_color, 0);
    lv_obj_set_style_text_font(bot_suit, &card_font_small, 0);
    lv_obj_align(bot_suit, LV_ALIGN_BOTTOM_RIGHT, -4, -5);

    return card;
}

void BlackjackApp::updateDealerCards(bool reveal) {
    // first card 
    if (this->dealer_card_containers[0] == nullptr) {
        this->dealer_card_containers[0] = createCard(
            screen,
            dealer.cards[0].value,
            dealer.cards[0].suit,
            150, 130
        );
    }

    // second card, show back or face depending on reveal
    if (dealer.card_count >= 2) {
        if (!reveal) {
            // show card back for second card
            if (this->hidden_container == nullptr) {
                this->hidden_container = lv_obj_create(screen);
                lv_obj_set_size(this->hidden_container, 70, 121);
                lv_obj_set_style_radius(this->hidden_container, 8, 0);
                lv_obj_set_style_clip_corner(this->hidden_container, true, 0);
                lv_obj_set_style_pad_all(this->hidden_container, 0, 0);
                lv_obj_set_style_border_width(this->hidden_container, 0, 0);
                lv_obj_set_style_bg_opa(this->hidden_container, LV_OPA_TRANSP, 0);
                lv_obj_set_pos(this->hidden_container, 175, 130);

                lv_obj_t *deck_img = lv_image_create(this->hidden_container);
                lv_image_set_src(deck_img, &card);
                lv_obj_center(deck_img);
            }
        } else {
            // delete the card back and show all cards face up
            if (this->hidden_container != nullptr) {
                lv_obj_delete(this->hidden_container);
                this->hidden_container = nullptr;
            }
            for (int i = 1; i < dealer.card_count; i++) {
                if (this->dealer_card_containers[i] != nullptr) continue;
                this->dealer_card_containers[i] = createCard(
                    screen,
                    dealer.cards[i].value,
                    dealer.cards[i].suit,
                    150 + (i * 25), 130
                );
            }
        }
    }
}

void BlackjackApp::clearDealerCards() {
    if (this->hidden_container != nullptr) {
        lv_obj_delete(this->hidden_container);
        this->hidden_container = nullptr;
    }
    for (int i = 0; i < 10; i++) {
        if (this->dealer_card_containers[i] != nullptr) {
            lv_obj_delete(this->dealer_card_containers[i]);
            this->dealer_card_containers[i] = nullptr;
            this->dealer_card_imgs[i] = nullptr;
        }
    }
}

enum States {Start, waiting, betting, dealing1, dealing2, dealing3, playing, showing1, showing2, done} state = Start;

bool shuffled = false;
void BlackjackApp::updateGame() {
    switch(state)
    {
        case Start:
            state = waiting;
            break;

        case waiting:
            lv_label_set_text(this->status_label, "Waiting to start...");
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (!players[i].active) continue;
                if (strcmp(players[i].status_msg, "Press START to begin!") != 0) {
                    strcpy(players[i].status_msg, "Press START to begin!");
                    http_server_push_state(&players[i]);
                }
                if (players[i].start_requested)
                {
                    players[i].start_requested = false;
                    for (int j = 0; j < MAX_PLAYERS; j++)
                    {
                        if (!players[j].active) continue;
                        players[j].start_requested = false;
                        strcpy(players[j].game_state, "betting");
                        http_server_push_state(&players[j]);
                    }
                    state = betting;
                }
            }
            break;

        case betting:{
            if (!shuffled) {
                deck.build_deck();
                deck.shuffle_deck();
                shuffled = true;
            }
            lv_label_set_text(this->status_label, "Place your bets!");
            for (int i = 0; i < MAX_PLAYERS; i++) 
            {
                if (!players[i].active) continue;
                if (strcmp(players[i].game_state, "betting") == 0)
                {
                    if (strcmp(players[i].status_msg, "Place your bet!") != 0) {
                        strcpy(players[i].status_msg, "Place your bet!");
                        http_server_push_state(&players[i]);
                    }
                }
                
                // read bet increment buttons
                if (players[i].bet_100_requested) 
                {
                    players[i].bet_100_requested = false;
                    if(players[i].chips >= players[i].bet_amount + 100)
                    {
                        players[i].bet_amount += 100;
                        http_server_push_state(&players[i]);
                    }
                } 
                else if (players[i].bet_10_requested) 
                {
                    players[i].bet_10_requested = false;
                    if(players[i].chips >= players[i].bet_amount + 10)
                    {
                        players[i].bet_amount += 10;
                        http_server_push_state(&players[i]);
                    }
                } 
                else if (players[i].bet_1_requested) 
                {
                    players[i].bet_1_requested = false;
                    if(players[i].chips >= players[i].bet_amount + 1)
                    {
                        players[i].bet_amount += 1;
                        http_server_push_state(&players[i]);
                    }
                }
                else if (players[i].bet_5_requested) 
                {
                    players[i].bet_5_requested = false;
                    if(players[i].chips >= players[i].bet_amount + 5)
                    {
                        players[i].bet_amount += 5;
                        http_server_push_state(&players[i]);
                    }
                }

                //read other bet buttons
                if (players[i].place_bet_requested) 
                {
                    players[i].place_bet_requested = false;
                    strcpy(players[i].game_state, "dealing");
                    strcpy(players[i].status_msg, "Waiting for other players...");
                    players[i].place_bet(players[i].bet_amount);
                    http_server_push_state(&players[i]);
                }
                else if (players[i].reset_bet_requested) 
                {
                    players[i].bet_amount = 15;
                    players[i].reset_bet_requested = false;
                    http_server_push_state(&players[i]);
                }
            }

            //check if every player is done betting
            int still_betting = 0;
            for (int i = 0; i < MAX_PLAYERS; i++) 
            {
                if (players[i].active && strcmp(players[i].game_state, "betting") == 0)
                    still_betting++;
            }
            if (still_betting == 0) 
            {
                state = dealing1;
            }
            break;}

        case dealing1:
            if (!d1_started) {
                dealer.draw_card(deck.deal_card());
                animate_deal(371, 130, 150, 130, 1000, [](lv_anim_t *a){
                    BlackjackApp::instance->updateDealerCards(false);  
                    state = dealing2;
                });
                d1_started = true;
            }
            break;
        
        case dealing2:
            if (!d2_started) {
                dealer.draw_card(deck.deal_card());
                animate_deal(371, 130, 165, 130, 1000, [](lv_anim_t *b){ 
                    BlackjackApp::instance->updateDealerCards(false);  
                    state = dealing3;
                });
                d2_started = true;
            }
            break;

        case dealing3:
            updateDealerCards(false);
            for (int i = 0; i < MAX_PLAYERS; i++) 
            {
                if (!players[i].active) continue;
                players[i].draw_card(deck.deal_card());
                players[i].draw_card(deck.deal_card());
                strcpy(players[i].game_state, "standby1");
                strcpy(players[i].status_msg, "Waiting for your turn...");
                http_server_push_state(&players[i]);
            }
            lv_obj_invalidate(lv_screen_active());
            //lv_label_set_text(this->status_label, "Player 1's turn");
            state = playing;
            break;

        case playing:{
            static int current_player = 0;
            
            // find next active player starting from current_player
            while (current_player < MAX_PLAYERS && !players[current_player].active)
                current_player++;
            
            if (current_player < MAX_PLAYERS)
            {
                lv_label_set_text(this->status_label, ("Player " + std::to_string(current_player+1) + "'s turn").c_str());
                if (strcmp(players[current_player].game_state, "playing") != 0) {
                    strcpy(players[current_player].game_state, "playing");
                    strcpy(players[current_player].status_msg, "Your turn");
                    http_server_push_state(&players[current_player]);
                }

                if (players[current_player].hit_requested)
                {
                    players[current_player].draw_card(deck.deal_card());
                    players[current_player].hit_requested = false;
                    http_server_push_state(&players[current_player]);
                }

                if (players[current_player].is_bust() || players[current_player].stand_requested)
                {
                    players[current_player].stand_requested = false;
                    strcpy(players[current_player].game_state, "standby2");
                    strcpy(players[current_player].status_msg, players[current_player].is_bust() ? "You Busted!" : "You Stand");
                    http_server_push_state(&players[current_player]);
                    current_player++; // move to next player next tick
                }
            }
            else
            {
                // all players done, reset for next round
                current_player = 0;
                lv_label_set_text(this->status_label, "Dealer's Turn");
                lv_label_set_text(this->dealer_value_label, ("Value: " + std::to_string(dealer.cards_value)).c_str());
                updateDealerCards(true);
                state = showing1;
            }
            break;}
        case showing1:{
            sleep_ms(2000);
                if (dealer.cards_value < 17)
                {
                    s1_started = false;
                    dealer.draw_card(deck.deal_card());
                    state = showing2;
                }
                else
                {
                    state = done;
                }
            break;}

        case showing2:
            if (!s1_started) {
                showing_card_num++;
                animate_deal(371, 130, 120 + ((showing_card_num) * 25), 130, 1000, [](lv_anim_t *c){
                    state = showing1;
                    BlackjackApp::instance->updateDealerCards(true);
                    lv_label_set_text(BlackjackApp::instance->dealer_value_label, ("Value: " + std::to_string(dealer.cards_value)).c_str());
                    lv_obj_invalidate(lv_screen_active());
                });
                s1_started = true;
            }
            break;

        case done:
            lv_label_set_text(this->status_label, "Done");
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (!players[i].active) continue;
                if (players[i].is_bust())
                {
                    strcpy(players[i].status_msg, "You Lose!");
                }
                else if (dealer.is_bust() || players[i].cards_value > dealer.cards_value)
                {
                    strcpy(players[i].status_msg, "You Win!");
                    players[i].chips += 2 * players[i].bet_amount;
                }
                else if (players[i].cards_value == dealer.cards_value)
                {
                    strcpy(players[i].status_msg, "Push!");
                    players[i].chips += players[i].bet_amount;
                }
                else
                {
                    strcpy(players[i].status_msg, "You Lose!");
                }
                http_server_push_state(&players[i]);
                
            }
            sleep_ms(5000);
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (!players[i].active) continue;
                players[i].reset_hand();
                players[i].bet_amount = 15;
                strcpy(players[i].game_state, "waiting");
                http_server_push_state(&players[i]);
            }
            dealer.reset_hand();
            clearDealerCards();
            lv_label_set_text(this->dealer_value_label, "");
            shuffled = false;
            this->d1_started = false;
            this->d2_started = false;
            this->s1_started = false;
            this->showing_card_num = 2;
            lv_obj_invalidate(lv_screen_active());
            state = waiting;
            break;
        default:
            break;
    }
}
