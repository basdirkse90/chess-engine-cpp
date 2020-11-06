#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <stack>
#include "EnumList.h"

class Chess {
	/* Main Chess class.
	Keeps the state of board and is responsible for:
		1) generates moves (finding legal ones only)
		2) applying moves, searching positions, unding moves (incl peft tests)
		3) Evaluating positions
		4) Recording history
	*/

public:
	// Constructors
	Chess(const std::string& fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

	// Public Methods
	void print_pseudolegal_moves();
	void print_board();

	int perft(const unsigned int n, const bool split=false);

	// Attempt to do move m. Responsible for checking if (pseudo)legal, and if so, update pseudolegal moves and in_check. Returns succes flag.
	bool do_move(const Move& m);

	bool do_move(unsigned int n);

	// Undo last n moves in move_history
	void undo_last_moves(const int n=1, const bool recalc_pseudolegal_moves=true);

private:
	/* Members ---------------------------------------------
	pos -- Current Board representation of the board
	piece_count -- Simple count of pieces in existence
	in_check -- True if King is in check in current position (for side to move)
	pseudolegal_moves -- Current list of pseudolegal moves [Pseudolegal means legal, except if leaves/puts king in check or castles through check]
	TODO: attack_map
	TODO: defence_map

	init_pos -- Save initial position
	move_history -- Sequence of played moves (Move objects stored)
	move_notation -- Sequence of played moves (String, written in algebraic chess notation)
	 */

	// Current state tracking members
	Board pos;
	int piece_count[16]{};
	std::vector<Move> pseudolegal_moves;
	// TODO: add attack map?
	// TODO: add defense map?

	// History tracking members
	Board init_pos;
	std::vector<Move> move_history{};
	std::vector<std::string> move_notation{};  // TODO: Implement


	// Methods -----------------------------------------------
	void generate_pseudolegal_moves(std::vector<Move>& output);
	void update_board(const Move& mv);	// No checking nothing, just modify Board struct pos by performing Move mv
	void revert_board(const Move& mv);   	// No checking nothing, just modify Board struct pos by undoing Move mv

	void add_move(std::vector<Move>& move_list, int from, int to, bool capture = false, EPieceCode prom = EPieceCode::epc_empty, bool is_ep = false);

	void gen_rooklike(std::vector<Move>& moves, const int i, const int r, const int f);
	void gen_bishoplike(std::vector<Move>& moves, const int i, const int r, const int f);
	void gen_raymoves(std::vector<Move>& moves, const std::function<bool(int)>& cond, const int diff, const int i);
	void gen_knight(std::vector<Move>& moves, const int i, const int r, const int f);
	void gen_king(std::vector<Move>& moves, const int i, const int r, const int f);
	void gen_wpawn(std::vector<Move>& moves, const int i, const int r, const int f);
	void gen_bpawn(std::vector<Move>& moves, const int i, const int r, const int f);

};
