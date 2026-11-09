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

#define SERIAL_CONTROL *((u16 *) 0x04000128)
#define SERIAL_MODE    *((u16 *) 0x04000134)

#define SERIAL_DATA ((struct Packet *) 0x04000120)
#define SERIAL_SEND ((struct Packet *) 0x0400012a)

#define SLAVE_BIT BIT(2)
#define READY_BIT BIT(3)
#define START_BIT BIT(7)
#define IRQ_BIT   BIT(14)

// TODO adjust
#define QUEUE_SIZE 128

struct Packet {
    u8 dest      : 2;
    u8 broadcast : 1;
    u8 invalid   : 1;
    u8 padding   : 4;

    u8 payload;
};

static_assert(
    sizeof(struct Packet) == 2, "size fo struct Packet is incorrect"
);

// TODO SBSS_SECTION ?
struct Queue {
    u32 head;
    u32 tail;
    struct Packet packets[QUEUE_SIZE];
} outgoing, incoming[3];

static bool keep_transfering;

static INLINE void enqueue(struct Queue *queue, struct Packet packet) {
    queue->packets[queue->head] = packet;
    queue->head = (queue->head + 1) % QUEUE_SIZE;

    // check if the queue overflowed: if so, discard the oldest item
    if(queue->head == queue->tail)
        queue->tail = (queue->tail + 1) % QUEUE_SIZE;
}

static INLINE struct Packet dequeue(struct Queue *queue) {
    // if queue is empty, return an invalid packet
    if(queue->head == queue->tail)
        return (struct Packet) { .invalid = 1 };

    struct Packet packet = queue->packets[queue->tail];
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    return packet;
}

static INLINE bool queue_empty(struct Queue *queue) {
    return queue->head == queue->tail;
}

static INLINE bool queue_full(struct Queue *queue) {
    return (queue->head + 1) % QUEUE_SIZE == queue->tail;
}

static INLINE i32 get_player_id(void) {
    return (SERIAL_CONTROL >> 4) & 3;
}

IWRAM_SECTION
static void serial_isr(void) {
    *SERIAL_SEND = dequeue(&outgoing);

    const i32 id = get_player_id();
    keep_transfering = false;

    #pragma GCC unroll 0
    for(i32 i = 0; i < 4; i++) {
        struct Packet packet = SERIAL_DATA[i];
        if(packet.invalid)
            continue;

        // one of the devices sent a valid packet: if master, keep
        // transfering data, more valid packets may follow
        keep_transfering = true;

        // ignore device's own packets
        if(i == id)
            continue;

        // if packet is addressed to this device, enqueue it
        if(packet.dest == id || packet.broadcast) {
            const i32 index = i - (i > id);
            enqueue(&incoming[index], packet);
        }
    }
}

THUMB
void linkcable_init(void) {
    interrupt_toggle(IRQ_SERIAL, true);
    interrupt_isr(IRQ_SERIAL, serial_isr);

    // prevent accidentally sending a packet on connection
    *SERIAL_SEND = (struct Packet) { .invalid = 1 };

    SERIAL_MODE = 0;
    SERIAL_CONTROL = (SERIAL_CONTROL & IRQ_BIT) | BIT(13);
}

THUMB
static NO_INLINE void delay(void) {
    #pragma GCC unroll 0
    for(vi32 i = 0; i < 256; i++);
}

THUMB
void linkcable_update(void) {
    // if device is a slave, or if other GBAs aren't ready, do nothing
    if((SERIAL_CONTROL & (SLAVE_BIT | READY_BIT)) != READY_BIT)
        return;

    // keep transmitting until all devices have no more data to send
    while(true) {
        SERIAL_CONTROL |= START_BIT;
        interrupt_wait(IRQ_SERIAL);

        // check if the ISR requests more data transfers
        if(!keep_transfering)
            break;

        // wait some time so that slaves can finish processing the data
        delay();
    }
}

THUMB
i32 linkcable_send(i32 dest, const void *buffer, u32 n) {
    const i32 id = get_player_id();

    for(i32 i = 0; i < n; i++) {
        // queue is full: stop adding packets
        if(queue_full(&outgoing))
            return i;

        struct Packet packet = {
            .dest      = dest,
            .broadcast = (dest < 0),
            .invalid   = 0,

            .payload = ((u8 *) buffer)[i]
        };
        enqueue(&outgoing, packet);
    }
    return n;
}

THUMB
i32 linkcable_receive(i32 *src, void *buffer, u32 n) {
    const i32 id = get_player_id();

    for(i32 q = 0; q < 3; q++) {
        struct Queue *queue = &incoming[q];
        if(queue_empty(queue))
            continue;

        // found some data: read at most n bytes
        for(i32 i = 0; i < n; i++) {
            // queue is empty: stop taking packets
            if(queue_empty(queue))
                return i;

            struct Packet packet = dequeue(queue);
            ((u8 *) buffer)[i] = packet.payload;

            if(src)
                *src = q + (q >= id);
        }
        return n;
    }
    return 0;
}
