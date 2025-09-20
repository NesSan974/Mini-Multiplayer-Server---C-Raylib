#ifndef __PLAYER_H__
#define __PLAYER_H__

/**
 * @brief Broadcasts a "Go Play" signal to all players.
 * 
 * Sends a 'G' message to all players in the `da_players` array, indicating
 * they should start playing. It also sets each player's state to `PLAYER_ACTIVE`.
 * 
 * This function prints debug information for each player it sends the message to.
 */
void broadcast_all_player_goplay();

/**
 * @brief Broadcasts the positions of all active players to each active player.
 * 
 * For each player with state `PLAYER_ACTIVE`, sends a 'U' message type meaning update.
 * data sent is the array `da_players`
 */
void broadcast_all_player_positions();

/**
 * @brief Finds the index of a player in the `da_players` by their socket descriptor.
 * 
 * Search in the array `da_players`, the player with `da_players.socket_fd` == to the param `socket`
 * 
 * @param socket The socket descriptor to search for.
 * @return int Returns:
 * - The index of the player in `da_players`
 * - `-1` if not found.
 */
int getPlayerIndex(int socket);

#endif //__PLAYER_H__