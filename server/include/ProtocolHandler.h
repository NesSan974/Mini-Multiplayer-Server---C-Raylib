#ifndef __PROTOCOLHANDLER_H__
#define __PROTOCOLHANDLER_H__

/**
 * @brief Handles a message received from a client socket.
 *
 * This function receives and interprets a custom protocol message
 * from a client. The protocol follows the format:
 * [type (1 byte)][data length (2 bytes)][data].
 *
 * The `type` field corresponds to a value of the `Msg_Type` enum.
 * Based on this type, the function handles the appropriate logic:
 *
 * - HELLO: Registers the player and sends a WELCOME type response.
 * - READY: Marks the player as ready; if >= players registered are ready, broadcasts GO type.
 * - PLAYER_MOVE: Updates the player's position in the array "players".
 * - BYE: Cleans up and closes the player's socket.
 * - Unknown type: Logs an unreachable state.
 *
 * @param socket The file descriptor of the client socket.
 *
 * @note The function assumes a non-blocking or reliable connection.
 *       If `recv()` returns 0 (client disconnected), the socket is closed.
 *
 * @return void
 *
 * @see enum Msg_type
 */
void HandleMessage(int socket);

/**
 * @brief Handles messages from all currently connected players.
 *
 * This function iterates over all connected players sockets,
 * adds them to a `fd_set` for reading, and uses `select()` to check
 * which ones have incoming data. For each client ready to communicate,
 * it delegates message handling to the `HandleMessage()` function.
 *
 * It does **not** handle new connections — only players that have already been accepted
 * and are stored in the global `da_players` dynamic array.
 *
 * @note This function assumes that the global `PlayerNetwork *da_players` array and `fd_set readfds` exist.
 *
 * @return int Returns:
 *  - `0` on success,
 *  - `-1` if there are no connected clients (`max_fd < 0`) or `select()` fails.
 *
 * @see HandleMessage
 */
int HandleConnectedPlayers();


#endif // __PROTOCOLHANDLER_H__