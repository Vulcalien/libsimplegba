/* Copyright 2026 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "libsimplegba/serial/linkcable.h"

#include "libsimplegba/hw/interrupt.h"
#include "libsimplegba/hw/system.h"
#include "libsimplegba/hw/timer.h"

#define SERIAL_CONTROL *((u16 *) 0x04000128)
#define SERIAL_MODE    *((u16 *) 0x04000134)

#define SERIAL_DATA ((struct Packet *) 0x04000120)
#define SERIAL_SEND ((struct Packet *) 0x0400012a)

#define SLAVE_BIT BIT(2)
#define READY_BIT BIT(3)
#define START_BIT BIT(7)
#define IRQ_BIT   BIT(14)

#define DEFAULT_BITRATE 512

// TODO:
// - handle communication errors and disconnections

// ================================================================== //
//                               Packet                               //
// ================================================================== //

struct Packet {
    u8 invalid   : 1;
    u8 checksum  : 4;

    u8 dest      : 2;
    u8 broadcast : 1;

    u8 payload;
};

static_assert(sizeof(struct Packet) == 2, "sizeof(struct Packet) != 2");

static u8 table[256] = {
0x00, 0x07, 0x0e, 0x09, 0x05, 0x02, 0x0b, 0x0c,
0x0a, 0x0d, 0x04, 0x03, 0x0f, 0x08, 0x01, 0x06,
0x0d, 0x0a, 0x03, 0x04, 0x08, 0x0f, 0x06, 0x01,
0x07, 0x00, 0x09, 0x0e, 0x02, 0x05, 0x0c, 0x0b,
0x03, 0x04, 0x0d, 0x0a, 0x06, 0x01, 0x08, 0x0f,
0x09, 0x0e, 0x07, 0x00, 0x0c, 0x0b, 0x02, 0x05,
0x0e, 0x09, 0x00, 0x07, 0x0b, 0x0c, 0x05, 0x02,
0x04, 0x03, 0x0a, 0x0d, 0x01, 0x06, 0x0f, 0x08,
0x06, 0x01, 0x08, 0x0f, 0x03, 0x04, 0x0d, 0x0a,
0x0c, 0x0b, 0x02, 0x05, 0x09, 0x0e, 0x07, 0x00,
0x0b, 0x0c, 0x05, 0x02, 0x0e, 0x09, 0x00, 0x07,
0x01, 0x06, 0x0f, 0x08, 0x04, 0x03, 0x0a, 0x0d,
0x05, 0x02, 0x0b, 0x0c, 0x00, 0x07, 0x0e, 0x09,
0x0f, 0x08, 0x01, 0x06, 0x0a, 0x0d, 0x04, 0x03,
0x08, 0x0f, 0x06, 0x01, 0x0d, 0x0a, 0x03, 0x04,
0x02, 0x05, 0x0c, 0x0b, 0x07, 0x00, 0x09, 0x0e,
0x0c, 0x0b, 0x02, 0x05, 0x09, 0x0e, 0x07, 0x00,
0x06, 0x01, 0x08, 0x0f, 0x03, 0x04, 0x0d, 0x0a,
0x01, 0x06, 0x0f, 0x08, 0x04, 0x03, 0x0a, 0x0d,
0x0b, 0x0c, 0x05, 0x02, 0x0e, 0x09, 0x00, 0x07,
0x0f, 0x08, 0x01, 0x06, 0x0a, 0x0d, 0x04, 0x03,
0x05, 0x02, 0x0b, 0x0c, 0x00, 0x07, 0x0e, 0x09,
0x02, 0x05, 0x0c, 0x0b, 0x07, 0x00, 0x09, 0x0e,
0x08, 0x0f, 0x06, 0x01, 0x0d, 0x0a, 0x03, 0x04,
0x0a, 0x0d, 0x04, 0x03, 0x0f, 0x08, 0x01, 0x06,
0x00, 0x07, 0x0e, 0x09, 0x05, 0x02, 0x0b, 0x0c,
0x07, 0x00, 0x09, 0x0e, 0x02, 0x05, 0x0c, 0x0b,
0x0d, 0x0a, 0x03, 0x04, 0x08, 0x0f, 0x06, 0x01,
0x09, 0x0e, 0x07, 0x00, 0x0c, 0x0b, 0x02, 0x05,
0x03, 0x04, 0x0d, 0x0a, 0x06, 0x01, 0x08, 0x0f,
0x04, 0x03, 0x0a, 0x0d, 0x01, 0x06, 0x0f, 0x08,
0x0e, 0x09, 0x00, 0x07, 0x0b, 0x0c, 0x05, 0x02,
};

THUMB
NO_INLINE i32 wow(const u8 *buffer, i32 n) {
    u32 crc = 0xffffffff;
    while(n--) {
        u32 index = crc ^ *buffer++;
        crc = (crc >> 8) ^ table[index & 0xff];
    }
    return (crc ^ 0xffffffff) & 0x0f;

    /*u32 crc = 0;*/
    /*for(i32 i = 0; i < 4; i++) {*/
        /*u32 index = (crc ^ (data >> (i * 8))) & 0xff;*/
        /*crc = (crc >> 8) ^ table[index];*/
    /*}*/
    /*return crc;*/
}

THUMB
static NO_INLINE i32 crc(u32 data) {
    const u32 polynomial = 0xc;

    u32 crc = 0xffffffff & 15;
    for(i32 i = 0; i < 16; i++) {
        u32 feedback = (crc ^ (data >> i)) & 1;
        crc = (crc >> 1) ^ (feedback * polynomial);
    }
    return (~crc) & 15;
}

THUMB
static NO_INLINE i32 checksum(struct Packet packet) {
    const u32 polynomial = 0xc;
    const u32 data = packet.dest      << 9 |
                     packet.broadcast << 8 |
                     packet.payload   << 0;

    u32 crc = 0;
    for(i32 i = 0; i < 11; i++) {
        const u32 feedback = (crc ^ (data >> i)) & 1;
        crc = (crc >> 1) ^ (polynomial * feedback);
    }
    return crc;
}

// ================================================================== //
//                               Queues                               //
// ================================================================== //

#define QUEUE_SIZE 64

SBSS_SECTION
struct Queue {
    u32 head;
    u32 tail;
    struct Packet packets[QUEUE_SIZE];
} outgoing, incoming[3];

static INLINE u32 queue_next_index(u32 index) {
    return (index + 1) % QUEUE_SIZE;
}

static INLINE void enqueue(struct Queue *queue, struct Packet packet) {
    queue->packets[queue->head] = packet;
    queue->head = queue_next_index(queue->head);

    // if queue overflowed, discard the oldest item
    if(queue->head == queue->tail)
        queue->tail = queue_next_index(queue->tail);
}

static INLINE struct Packet dequeue(struct Queue *queue) {
    // if queue is empty, return an invalid packet
    if(queue->head == queue->tail)
        return (struct Packet) { .invalid = 1 };

    struct Packet packet = queue->packets[queue->tail];
    queue->tail = queue_next_index(queue->tail);
    return packet;
}

static INLINE bool queue_empty(struct Queue *queue) {
    return queue->head == queue->tail;
}

static INLINE bool queue_full(struct Queue *queue) {
    return queue_next_index(queue->head) == queue->tail;
}

// ================================================================== //

static INLINE i32 get_player_id(void) {
    return (SERIAL_CONTROL >> 4) & 3;
}

IWRAM_SECTION
static void serial_isr(void) {
    *SERIAL_SEND = dequeue(&outgoing);

    const i32 id = get_player_id();

    #pragma GCC unroll 0
    for(i32 i = 0; i < 4; i++) {
        // ignore device's own packets
        if(i == id)
            continue;

        struct Packet packet = SERIAL_DATA[i];
        if(packet.invalid)
            continue;

        // if packet is addressed to this device, enqueue it
        if(packet.dest == id || packet.broadcast) {
            const i32 index = i - (i > id);
            enqueue(&incoming[index], packet);
        }
    }
}

IWRAM_SECTION
static void timer2_isr(void) {
    // if device is master, and all GBAs are ready, start a transfer
    if((SERIAL_CONTROL & (SLAVE_BIT | READY_BIT)) == READY_BIT)
        SERIAL_CONTROL |= START_BIT;
}

#include "libsimplegba/misc/random.h"

THUMB
void linkcable_init(void) {
    // prevent accidentally sending a packet on connection
    *SERIAL_SEND = (struct Packet) { .invalid = 1 };

    // enable serial interrupt
    interrupt_toggle(IRQ_SERIAL, true);
    interrupt_handler(IRQ_SERIAL, serial_isr);

    // enable Timer 2 interrupt
    interrupt_toggle(IRQ_TIMER2, true);
    interrupt_handler(IRQ_TIMER2, timer2_isr);

    // configure serial port
    SERIAL_MODE = 0;
    SERIAL_CONTROL = (SERIAL_CONTROL & IRQ_BIT) | (
        2 << 0  | // baud rate 57600 bps
        1 << 13   // must be 1
    );

    for(i32 i = 0; i < 256; i++) {
        u32 a = random(65000);
        a = 0xfffe;
        /*if(crc(a) != math_crc4(&a, 4))*/
        mgba_printf("i=%x CRC=%x math_crc4=%x", a, crc(a), math_crc4(&a, 2));
    }

    linkcable_speed(DEFAULT_BITRATE);
}

THUMB
i32 linkcable_send(i32 dest, const void *buffer, u32 n) {
    interrupt_mask(IRQ_SERIAL, true); // enter critical section

    i32 count = 0;
    while(count < n && !queue_full(&outgoing)) {
        struct Packet packet = {
            .dest      = dest,
            .broadcast = (dest < 0),
            .payload   = ((u8 *) buffer)[count++]
        };
        packet.checksum = checksum(packet);
        enqueue(&outgoing, packet);
    }

    interrupt_mask(IRQ_SERIAL, false); // exit critical section
    return count;
}

THUMB
static NO_INLINE i32 receive(struct Queue *queue, void *buffer, u32 n) {
    interrupt_mask(IRQ_SERIAL, true); // enter critical section

    i32 count = 0;
    while(count < n && !queue_empty(queue)) {
        struct Packet packet = dequeue(queue);
        if(packet.checksum == checksum(packet))
            ((u8 *) buffer)[count++] = packet.payload;
    }

    interrupt_mask(IRQ_SERIAL, false); // exit critical section
    return count;
}

THUMB
i32 linkcable_receive(i32 *src, void *buffer, u32 n) {
    const i32 id = get_player_id();

    // try to find data in any queue
    for(i32 i = 0; i < 3; i++) {
        i32 count = receive(&incoming[i], buffer, n);
        if(count > 0) {
            if(src)
                *src = i + (i >= id);
            return count;
        }
    }
    return 0;
}
