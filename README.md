# Blackjack Table

A multiplayer blackjack game system built around a Raspberry Pi Pico 2 W and an Icesugar-Pro FPGA. Players connect their phones, tablets, or laptops to the Pico's Wi-Fi access point and play through a hosted webpage, while a shared 480x272 RGB LCD, driven by the FPGA as a dedicated display controller, acts as the table, showing the dealer's hand, game state, and card animations in real time.

> Built for CS 122A as a custom laboratory project. Full lab report: [`tflan006_custom_lab_report.pdf`](./tflan006_custom_lab_report.pdf)

## How it works

The Pico runs the game logic and renders the UI (cards, panels, status messages, animations) with LVGL into a frame buffer, then sends that frame data to the FPGA over SPI. The FPGA reads the SPI stream, stores the frame in its onboard SDRAM, and drives the RGB LCD's timing and parallel interface to put the image on screen, acting as a dedicated display controller so the Pico doesn't have to bit-bang the display itself.

At the same time, the Pico hosts its own Wi-Fi access point and a custom HTTP/SSE server (built directly on raw lwIP, after `pico_lwip_http` proved too restrictive for persistent multiplayer connections). Each connected device gets a Server-Sent Events connection, so the webpage updates instantly whenever the game state changes. The whole system is event-driven both ways: player actions trigger game-state changes, and game-state changes push live updates back out to every connected device.

## Getting started

1. Power on the Pico 2 W and the FPGA board.
2. Connect your phone (or other device) to the Wi-Fi access point named **"Blackjack Table"**.
3. Open a browser and go to **http://192.168.4.1** to load the game webpage.
4. One player presses **Start** to begin a round.
5. Play out a normal round of blackjack: place your bet, then hit or stand on your turn, using the on-screen prompts and buttons.
6. When you're done, close the browser and disconnect from the network.

## Hardware

- Raspberry Pi Pico 2 W
- Icesugar-Pro FPGA
- PMOD RGBLCD Expansion Board
- 480x272 RGB LCD display
- Personal devices for players (phone, tablet, computer)

<img width="1172" height="707" alt="Screenshot 2026-06-08 120904" src="https://github.com/user-attachments/assets/87bd480b-fbff-4be0-a8c7-047848057791" />

<img width="600" height="700" alt="Screenshot 2026-06-07 161026" src="https://github.com/user-attachments/assets/d373c5ed-5806-4769-b96e-3baa5be6f082" />


## System design

<img width="1235" height="690" alt="image" src="https://github.com/user-attachments/assets/8c4c3086-3679-4ee7-8ab4-22a0a22d3764" />


- **CS122_Project / main:** Initializes hardware, Wi-Fi, and SPI, and starts the app.
- **dhcpserver:** Assigns each connecting device a unique IP address.
- **httpserver:** Stores the webpage and handles event-driven data transfer (button events, game/player state) between the Pico and connected devices via SSE.
- **spi_display:** Sends the LVGL-rendered partial frame buffer to the FPGA over SPI.
- **BlackjackApp:** Builds the LVGL UI, runs the game's state machine, and drives card animations.
- **FPGA (SPI Peripheral → Display Controller → SDRAM Framebuffer → Async FIFO → LCD Timing):** Decodes incoming SPI data, buffers frames in SDRAM, handles the clock-domain crossing, and generates the LCD's sync/timing signals to drive the display.

## Software

| Library | Purpose | 
|---|---|
| `pico_stdlib` | Core Pico SDK functionality |
| `hardware_spi` | Low-level SPI driver that sends pixel data to the FPGA |
| `pico_cyw43_arch_lwip_threadsafe_background` | WiFi chip init + lwIP TCP/IP stack running safely via background callbacks |
| `dhcpserver` | Assigns IP addresses to connecting player devices |
| `LVGL` | Renders all on-screen UI cards, chips, panels, status text, and animations into the frame buffer sent to the FPGA |
| `pico_rand` | Hardware-backed random number generation for deck shuffling |

## Protocols

- **SPI**: carries rendered frame/pixel data from the Pico to the FPGA display controller
- **WiFi**: broadcasts the access point and exchanges data with player devices
- **TCP / IP**: reliable, addressed delivery of data between the Pico and player devices
- **DHCP**: automatic IP assignment for devices joining the access point
- **HTTP**: serves the game webpage, handles player actions, and pushes live game-state updates

## Notable design elements

- **Real-time multiplayer web interface** via Server-Sent Events - every connected device gets instant game-state pushes (cards dealt, turn changes, bets placed) with no polling.
- **Custom HTTP/SSE server on raw lwIP** - handles HTTP parsing, routing, persistent per-player SSE connections, connection teardown, and panic-free buffer management.
- **Graceful connect/disconnect handling** - each player is tied to a `Player` struct; if they disconnect, they're marked inactive, their hand/bet resets, and the game continues smoothly for everyone else.
- **FPGA as a dedicated display controller** - the Icesugar-Pro uses its onboard SDRAM as a framebuffer, decoding the SPI stream from the Pico and driving the LCD directly.
- **LVGL image rendering & animations** - card and chip graphics are pre-converted to C pixel arrays and animated to their destination over a set duration.
- **Fully event-driven execution** - networking is entirely callback-driven, and game-state pushes fire in response to state changes rather than polling loops.

## AI usage

- Used Claude to help design and write the custom `http_server` implementation after `pico_lwip_http` proved too restrictive for persistent multiplayer SSE connections.
- Used AI assistance to write the webpage's HTML/CSS/JS (first time writing HTML).
- Used AI to help understand instructor-provided driver code and integrate it with custom game code.
- General research, Q&A, and planning support throughout the project.

## Acknowledgements

- [Raspberry Pi Pico C SDK documentation](https://www.raspberrypi.com/documentation/pico-sdk/)
- [raspberrypi/pico-examples](https://github.com/raspberrypi/pico-examples)
- [LVGL: Light and Versatile Embedded UI Ecosystem](https://lvgl.io/)
- Allan Knight: author of the SPI display driver 
- Claude / Gemini
