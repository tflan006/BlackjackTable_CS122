#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lwip/tcp.h"
#include "http_server.h"

// WEB CONTENT
static const char INDEX_HTML[] = R"HTML(<!DOCTYPE html>
<html>
<head>
    <title>Blackjack Table</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { background-color: #005500; color: #ffffff; font-family: 'Segoe UI', Arial, sans-serif; text-align: center; margin: 0; padding: 20px; }
        .table-container { border: 4px solid #8B4513; border-radius: 15px; padding: 20px; max-width: 500px; margin: 20px auto; background-color: #005500; box-shadow: 0px 10px 20px rgba(0,0,0,0.3); }
        h1 { color: #ffcc00; margin-bottom: 5px; }
        .data-box { background: rgba(0,0,0,0.3); padding: 10px; border-radius: 8px; margin: 15px 0; font-size: 1.2em; }
        .btn { background-color: #ffcc00; color: #000; border: none; padding: 12px 25px; font-size: 1.1em; font-weight: bold; border-radius: 5px; cursor: pointer; margin: 5px; min-width: 100px; }
        .btn:active { background-color: #cca300; }
        .btn-stand { background-color: #d9534f; color: white; }
        .btn-stand:active { background-color: #c9302c; }
    </style>
</head>
<body>
    <div class="table-container">
    <h1>Blackjack Table</h1>
    <div id="msg" class="data-box">Connecting to table...</div>
    <div class="data-box">
        <div>Chips: $<span id="chips">0</span></div>
        <div>Bet: $<span id="bet">0</span></div>
        <div>Your Cards: <div id="p-cards"></div></div>
        <div>Hand Value: <span id="hand-value">0</span></div>
    </div>
    <div id="start-button">
        <button class="btn" onclick="sendMove('Start')">START</button>
    </div>
    <div id="betting-buttons">
        <button class="btn" onclick="sendMove('1')">$1</button>
        <button class="btn" onclick="sendMove('5')">$5</button>
        <button class="btn" onclick="sendMove('10')">$10</button>
        <button class="btn" onclick="sendMove('100')">$100</button>
        <button class="btn" onclick="sendMove('reset_bet')">RESET BET</button>
        <button class="btn" onclick="sendMove('place_bet')">PLACE BET</button>
    </div>
    <div id="action-buttons">
        <button class="btn" onclick="sendMove('hit')">HIT</button>
        <button class="btn btn-stand" onclick="sendMove('stand')">STAND</button>
    </div>
</div>
<script>
    function sendMove(type) {
        fetch('/action.cgi?move=' + type)
        .catch(err => console.error("Action error:", err));
    }
    function makeCard(value, suit) {
        const suitInt = parseInt(suit);
        const suitSymbols = {0:'&hearts;', 1:'&diams;', 2:'&clubs;', 3:'&spades;'};
        const suitChar = suitSymbols[suitInt];
        const suitColor = (suitInt === 0 || suitInt === 1) ? '#cc0000' : '#000000';
        const faces = {1:'A', 11:'J', 12:'Q', 13:'K'};
        const label = faces[value] || value;
        return `<div style="
            display: inline-block;
            width: 60px;
            height: 90px;
            background: white;
            border-radius: 8px;
            border: 1px solid #ccc;
            color: ${suitColor};
            font-size: 1.4em;
            font-weight: bold;
            margin: 4px;
            padding: 4px;
            box-shadow: 2px 2px 5px rgba(0,0,0,0.3);
            position: relative;
            vertical-align: top;
        ">
            <div style="position:absolute; top:4px; left:6px;">${label}${suitChar}</div>
            <div style="position:absolute; bottom:4px; right:6px; transform:rotate(180deg);">${label}${suitChar}</div>
            <div style="position:absolute; top:50%; left:50%; transform:translate(-50%,-50%); font-size:1.4em;">${suitChar}</div>
        </div>`;
    }
    function connectSSE() {
        const evtSource = new EventSource('/events');
        evtSource.onmessage = (e) => {
            const data = JSON.parse(e.data);
            document.getElementById('chips').innerText = data.chips;
            document.getElementById('bet').innerText = data.bet;
            document.getElementById('msg').innerText = data.message;
            document.getElementById('hand-value').innerText = data.hand_value;

            let cardsHTML = data.player_cards.map(c => makeCard(c.v, c.s)).join('');
            document.getElementById('p-cards').innerHTML = cardsHTML;

            document.getElementById('start-button').style.display = data.game_state === 'waiting' ? 'block' : 'none';
            document.getElementById('betting-buttons').style.display = data.game_state === 'betting' ? 'block' : 'none';
            document.getElementById('action-buttons').style.display = data.game_state === 'playing' ? 'block' : 'none';
        };
    }
    connectSSE();
</script>
</body>
</html>)HTML";

static const int INDEX_HTML_LEN = sizeof(INDEX_HTML) - 1;

// HTTP SERVER
typedef struct {
    char request[512];
    int  bytes_received;
    bool response_sent;
} http_conn_t;

Player* find_player(ip4_addr_t ip) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players[i].active && ip4_addr_eq(&players[i].ip, &ip)) {
            return &players[i];
        }
    }
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!players[i].active) {
            players[i].active = true;
            players[i].ip = ip;
            players[i].chips = 1000;
            strcpy(players[i].status_msg, "Place your bet!");
            strcpy(players[i].game_state, "waiting");
            return &players[i];
        }
    }
    printf("Server full — connection rejected\n");
    return NULL;
}

void http_server_push_state(Player *player) {
    if (!player || !player->sse_pcb) return;

    char cards_buf[128] = "[";
    for (int i = 0; i < player->card_count; i++) {
        char tmp[32];
        snprintf(tmp, sizeof(tmp), "{\"v\":%d,\"s\":%d}",
            player->cards[i].value, player->cards[i].suit);
        strcat(cards_buf, tmp);
        if (i < player->card_count - 1) strcat(cards_buf, ",");
    }
    strcat(cards_buf, "]");

    char json[256];
    snprintf(json, sizeof(json),
        "{\"chips\":%d,\"bet\":%d,\"message\":\"%s\","
        "\"player_cards\":%s,\"hand_value\":%d,\"game_state\":\"%s\"}",
        player->chips, player->bet_amount, player->status_msg,
        cards_buf, player->cards_value, player->game_state);

    char event[320];
    int len = snprintf(event, sizeof(event), "data: %s\n\n", json);

    if (tcp_sndbuf(player->sse_pcb) < (u16_t)len) return;

    err_t err = tcp_write(player->sse_pcb, event, len, TCP_WRITE_FLAG_COPY);
    if (err == ERR_OK) tcp_output(player->sse_pcb);
}

static void process_action(const char *request, Player *player) {
    if (strstr(request, "move=hit")) {
        printf("WEB REQ: Client chose HIT\n");
        player->hit_requested = true;
    } else if (strstr(request, "move=stand")) {
        printf("WEB REQ: Client chose STAND\n");
        player->stand_requested = true;
    } else if (strstr(request, "move=Start")) {
        printf("WEB REQ: Client chose START\n");
        player->start_requested = true;
    } else if (strstr(request, "move=place_bet")) {
        printf("WEB REQ: Client chose PLACE BET\n");
        player->place_bet_requested = true;
    } else if (strstr(request, "move=100")) {
        printf("WEB REQ: Client bets $100\n");
        player->bet_100_requested = true;
    } else if (strstr(request, "move=10")) {
        printf("WEB REQ: Client bets $10\n");
        player->bet_10_requested = true;
    } else if (strstr(request, "move=1")) {
        printf("WEB REQ: Client bets $1\n");
        player->bet_1_requested = true;
    } else if (strstr(request, "move=5")) {
        player->bet_5_requested = true;
        printf("WEB REQ: Client bets $5\n");
    } else if (strstr(request, "reset_bet")) {
        player->reset_bet_requested = true;
        printf("WEB REQ: Client chose RESET BET\n");
    }
}

static void close_connection(struct tcp_pcb *pcb, http_conn_t *conn) {
    tcp_arg(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_err(pcb, NULL);
    if (conn) free(conn);
    tcp_close(pcb);
}

static err_t http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    http_conn_t *conn = (http_conn_t *)arg;

    if (!p) {
        if (conn) {
            close_connection(pcb, conn);
        } else {
            tcp_close(pcb);
        }
        return ERR_OK;
    }

    if (err != ERR_OK || !conn) {
        pbuf_free(p);
        return ERR_VAL;
    }

    int space = sizeof(conn->request) - conn->bytes_received - 1;
    int copy_len = (int)p->tot_len < space ? (int)p->tot_len : space;
    pbuf_copy_partial(p, conn->request + conn->bytes_received, copy_len, 0);
    conn->bytes_received += copy_len;
    conn->request[conn->bytes_received] = '\0';
    tcp_recved(pcb, p->tot_len);
    pbuf_free(p);

    if (!conn->response_sent && strstr(conn->request, "\r\n\r\n")) {
        conn->response_sent = true;
        char header[256];

        ip4_addr_t client_ip = *ip_2_ip4(&pcb->remote_ip);
        Player *player = find_player(client_ip);

        if (strstr(conn->request, "GET /events")) {
            if (player) {
                player->sse_pcb = pcb;
                tcp_arg(pcb, player);
                tcp_err(pcb, [](void *arg, err_t) {
                    Player *p = (Player *)arg;
                    if (p) p->sse_pcb = NULL;
                });
                tcp_recv(pcb, [](void *arg, struct tcp_pcb *tpcb, struct pbuf *buf, err_t) -> err_t {
                    if (buf) { pbuf_free(buf); return ERR_OK; }
                    Player *p = (Player *)arg;
                    if (p) 
                    {
                        p->sse_pcb = NULL;
                        p->active = false;       
                        p->reset_hand();            
                        p->bet_amount = 15;         
                        strcpy(p->game_state, "waiting");  
                    }
                    tcp_close(tpcb);
                    return ERR_OK;
                });
            }
            const char *sse_header =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/event-stream\r\n"
                "Cache-Control: no-cache\r\n"
                "Connection: keep-alive\r\n\r\n";
            tcp_write(pcb, sse_header, strlen(sse_header), TCP_WRITE_FLAG_COPY);
            tcp_output(pcb);
            free(conn);
            if (player) http_server_push_state(player);
            // tcp_arg already set to player above, don't null it

        } else if (strstr(conn->request, "GET /action.cgi")) {
            if (player) process_action(conn->request, player);
            const char *redirect = "HTTP/1.1 302 Found\r\nLocation: /\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
            tcp_write(pcb, redirect, strlen(redirect), TCP_WRITE_FLAG_COPY);
            tcp_output(pcb);
            close_connection(pcb, conn);

        } else {
            snprintf(header, sizeof(header),
                "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n"
                "Cache-Control: no-store, no-cache, must-revalidate\r\nPragma: no-cache\r\n"
                "Expires: 0\r\nConnection: close\r\n\r\n",
                INDEX_HTML_LEN);
            tcp_write(pcb, header,     strlen(header),     TCP_WRITE_FLAG_COPY);
            tcp_write(pcb, INDEX_HTML, INDEX_HTML_LEN,     0);
            tcp_output(pcb);
            close_connection(pcb, conn);
        }
    }

    return ERR_OK;
}

static void http_err(void *arg, err_t err) {
    http_conn_t *conn = (http_conn_t *)arg;
    if (conn) free(conn);
}

static err_t http_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    if (err != ERR_OK || !newpcb) return ERR_VAL;

    tcp_setprio(newpcb, TCP_PRIO_MIN);

    http_conn_t *conn = (http_conn_t *)calloc(1, sizeof(http_conn_t));
    if (!conn) return ERR_MEM;

    tcp_arg(newpcb, conn);
    tcp_recv(newpcb, http_recv);
    tcp_err(newpcb, http_err);

    return ERR_OK;
}

bool http_server_init() {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Error: could not create TCP PCB\n");
        return false;
    }
    if (tcp_bind(pcb, IP4_ADDR_ANY, 80) != ERR_OK) {
        printf("Error: could not bind port 80\n");
        return false;
    }
    struct tcp_pcb *listen_pcb = tcp_listen(pcb);
    if (!listen_pcb) {
        printf("Error: tcp_listen failed\n");
        tcp_close(pcb);
        return false;
    }
    tcp_accept(listen_pcb, http_accept);
    return true;
}