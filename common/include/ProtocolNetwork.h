/**
 * @file ProtocolNetwork.h
 * @brief Defines the network protocol structure and related message types.
 *
 * ## Network Message Format
 *
 * All messages exchanged between the client and server follow a compact binary protocol:
 *
 * ```
 * [ Msg_Type : 1 byte ] [ Data_Length : 2 bytes ] [ Data : N bytes ]
 * ```
 *
 * - **Msg_Type** (`Net_msgType_t`, typedef of `char`)  
 *   This byte indicates the type of message. It should be set using the `Msg_Type` enum
 *   (e.g., `GOPLAY`, `UPDATE_ALL_PLAYERS`, etc.), and, preferably, casted to `Net_msgType_t` when sending.
 *   It will be cast in the function anyway
 *
 *   ```c
 *   sendMessage(sockfd, (Net_msgType_t)GOPLAY, NULL, 0);
 *   // or 
 *   sendMessage(sockfd, GOPLAY, NULL, 0);
 *   ```
 *  
 * - **Data_Length** (`Net_length_t`, typedef of `uint16_t`)  
 *   Length of the data payload in bytes.
 *
 * - **Data** (raw bytes)  
 *   The actual content of the message, such as a structure, array, or simple value.
 *
 * ## Enum: Msg_Type
 *
 * The enum `Msg_Type` defines all supported message types, each corresponding to
 * a single character used as the first byte of a network packet.
 *
 * Example values:
 * ```c
 * enum Msg_Type {
 *     HELLO   = 'H',
 *     WELCOME = 'W',
 *     READY   = 'R',
 *     GOPLAY  = 'G',
 *     PLAYER_MOVE = 'M',
 *     UPDATE_ALL_PLAYERS = 'U',
 *     ERROR   = 'E',
 *     BYE     = 'B'
 * };
 * ```
 *
 * These enums improve readability and maintainability. Internally, they are sent as raw byte (`Net_msgType_t`).
 *
 * ## Sending and Receiving Messages
 *
 * You can use the following utility functions to send and receive protocol-compliant messages:
 *
 * - `int sendMessage(int sockfd, Net_msgType_t type, const void *buffer, size_t size);`  
 *   Sends a message with the given type and payload. Internally transmit length in network byte order (big-endian)
 *   It's your responsability to serilize/convert the data in network byte order
 *
 * - `int recvMessage(int sockfd, Net_msgType_t *type, void *buf, size_t *size);`  
 *   Attempts to read a message if one is available on the socket.
 *   Internally convert length in host byte order (big-endian).
 *   It's your responsability to de-serilize/convert the data to host byte order
 *
 * Example usage:
 * ```c
 * PlayerNetwork p = ...;
 * sendMessage(sockfd, PLAYER_MOVE, &p, sizeof(p));
 * ```
 *
 * ## Notes
 *
 * - The receiver should always read the first 3 bytes to get the message type and payload size,
 *   then read the payload accordingly to the size received.
 * - This protocol is designed to be compact, easy to use and more like "template" for real-time multiplayer games.
*/

#ifndef __NET_PROTOCOL_H__
#define __NET_PROTOCOL_H__

#define PORT 22222
#define TICK_RATE_MS 50 // 20 updates/sec

/**
 * @enum Msg_Type
 * @brief Represents the type of message exchanged between client and server.
 *
 * Each value corresponds to the first byte in the protocol. 
 * 
 * @see Net_msgType_t
 */
enum Msg_Type
{
    HELLO = 'H',
    WELCOME = 'W',
    
    READY = 'R',
    GOPLAY = 'G',
    
    PLAYER_MOVE = 'M',
    
    UPDATE_ALL_PLAYERS = 'U',
    
    ERROR = 'E',
    
    BYE = 'B',
};


/// @brief network size of the `enum Msg_Type`
typedef char Net_msgType_t;
/// @brief network size of the length in protocol
typedef uint16_t Net_length_t;


/**
 * @brief Gets the current system time in milliseconds.
 *
 * This function retrieves the current time using `gettimeofday()` and converts it
 * to a 64-bit integer representing the number of milliseconds since the Unix epoch.
 * This function is use to keep track of the tick rate
 *
 * @return The current time in milliseconds as a `uint64_t`.
 */
uint64_t current_time_ms();

/**
 * @brief Sends a protocol-compliant message over a socket.
 *
 * This function sends a message following the custom protocol:
 * ```
 * [ Msg_Type (1 byte) ][ Data_Length (2 bytes) ][ Data (N bytes) ]
 * ```
 *
 * - The message type is sent as a single byte (from `Msg_Type`, casted to `Net_msgType_t`).
 * - The length is sent as a 2-byte unsigned integer (`Net_length_t`).
 * - The payload is sent as raw bytes.
 *
 * @param sockfd The socket file descriptor to send the message on.
 * @param type The message type (usually from the `Msg_Type` enum).
 * @param buffer Pointer to the payload data to send (can be NULL if size is 0).
 * @param size Size of the payload data in bytes.
 *
 * @return The total number of bytes sent, or -1 if an error occurred.
 *
 * @see recvMessage()
 */
int sendMessage(int sockfd, Net_msgType_t type, const void *buffer, size_t size);

/**
 * @brief Receive a message from a socket using the custom protocol format.
 *
 * This function reads a message from the socket, using non-blocking and blocking
 * reads in combination:
 *
 * - The message type is read in blocking or non-blocking (`MSG_DONTWAIT`), depanding of shouldWait.
 * - The length and payload are read using blocking mode (`MSG_WAITALL`).
 *
 * Message format:
 * ```
 * [ Msg_Type (1 byte) ][ Data_Length (2 bytes) ][ Data (N bytes) ]
 * ```
 *
 * @param sockfd The socket file descriptor to read from.
 * @param type Pointer to store the received message type (`Net_msgType_t`).
 * @param data Buffer to store the received payload (must be large enough).
 * @param payload_size_received Pointer to store the size of the received payload.
 * @param shouldWait Indicate if the Msg_Type should be read blocking or non-blocking.
 *
 * @return 
 *      - The total number of bytes reador a negative value if an error occurred.
 *      - If no data is available, returns 0.
 *
 * @warning This function does not perform any validation or type casting of the payload.
 *          The caller is responsible for interpreting `data` correctly based on `type`.
 *
 * @see sendMessage()
 */
int recvMessage(int sockfd, Net_msgType_t *type, void *buf, Net_length_t *size, bool shouldWait);

#endif // __NET_PROTOCOL_H__