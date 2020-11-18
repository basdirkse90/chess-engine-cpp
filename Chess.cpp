#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include "EnumList.h"
#include "Chess.h"
#include <map>

using namespace std;

// Constructors
Chess::Chess(const std::string& fen) {
	istringstream(fen) >> pos;
	init_pos = pos;

	// Initialize piece_count
	for(const EPieceCode piece : pos.square_list) {
		piece_count[(int)piece]++;
	}

	// Initialize pseudolegal_moves
	pseudolegal_moves.reserve(100);
	generate_pseudolegal_moves(pseudolegal_moves);
}


// Method to generate pseudolegal moves
void Chess::generate_pseudolegal_moves(vector<Move>& res) {
	for (int i = 0; i < 64; i++) {
		if (get_clr(pos.square_list[i]) != pos.side_to_move) continue;

		int r = i / 8;
		int f = i % 8;

		switch (get_ept(pos.square_list[i])) {
		case EPieceType::ept_queen:
			gen_rooklike(res, i, r, f);
			gen_bishoplike(res, i, r, f);
			break;
		case EPieceType::ept_rook:
			gen_rooklike(res, i, r, f);
			break;
		case EPieceType::ept_bishop:
			gen_bishoplike(res, i, r, f);
			break;
		case EPieceType::ept_king:
			gen_king(res, i, r, f);
			break;
		case EPieceType::ept_knight:
			gen_knight(res, i, r, f);
			break;
		case EPieceType::ept_wpawn:
			gen_wpawn(res, i, r, f);
			break;
		case EPieceType::ept_bpawn:
			gen_bpawn(res, i, r, f);
			break;
		default:
			break;
		}
	}
}

void Chess::add_move(vector<Move>& move_list, int from, int to, bool capture, EPieceCode prom, bool is_ep) {
	EPieceCode moving_piece = pos.square_list[from];

	Move mv{ from, to };
	mv.promotion = prom;
	mv.en_passant = is_ep;
	mv.old_en_passant_square = pos.en_passant_square;
	mv.old_halfmove_count = pos.half_move_count;

	EPieceCode capt;
	if (is_ep) {
		if (to > from)
			capt = EPieceCode::epc_bpawn;
		else
			capt = EPieceCode::epc_wpawn;
	}
	else {
		capt = pos.square_list[to];
	}

	mv.capture = capt;

	// Lose castling rights after king or rook move
	if (moving_piece == EPieceCode::epc_wking)
		mv.lost_castle_rights = pos.castling_rights & CastlingRights::cr_white_both;
	else if (moving_piece == EPieceCode::epc_bking)
		mv.lost_castle_rights = pos.castling_rights & CastlingRights::cr_black_both;
	else if (moving_piece == EPieceCode::epc_wrook) {
		if (from == 0)
			mv.lost_castle_rights = pos.castling_rights & CastlingRights::cr_white_long;
		else if (from == 7)
			mv.lost_castle_rights = pos.castling_rights & CastlingRights::cr_white_short;
	}
	else if (moving_piece == EPieceCode::epc_brook) {
		if (from == 56)
			mv.lost_castle_rights = pos.castling_rights & CastlingRights::cr_black_long;
		else if (from == 63)
			mv.lost_castle_rights = pos.castling_rights & CastlingRights::cr_black_short;
	}
	else
		mv.lost_castle_rights = CastlingRights::cr_none;

	// Additionally, lose castling rights after rook is captured
	if (capture) {
		if (to == 63 && capt == EPieceCode::epc_brook) {
			mv.lost_castle_rights = mv.lost_castle_rights ^ (pos.castling_rights & CastlingRights::cr_black_short);
		}
		else if (to == 56 && capt == EPieceCode::epc_brook) {
			mv.lost_castle_rights = mv.lost_castle_rights ^ (pos.castling_rights & CastlingRights::cr_black_long);
		}
		else if (to == 7 && capt == EPieceCode::epc_wrook) {
			mv.lost_castle_rights = mv.lost_castle_rights ^ (pos.castling_rights & CastlingRights::cr_white_short);
		}
		else if (to == 0 && capt == EPieceCode::epc_wrook) {
			mv.lost_castle_rights = mv.lost_castle_rights ^ (pos.castling_rights & CastlingRights::cr_white_long);
		}

	}

	move_list.push_back(mv);
}

bool Chess::is_on_board(const int r, const int f, const int dr, const int df) {
	return 0 <= r+dr && r+dr < 8 && 0 <= f+df && f+df < 8;
}

void Chess::gen_raymoves(vector<Move>& moves, int i, int r, int f, int dr, int df) {
	int dist = 1;
	int offset = dr*8 + df;
	while (is_on_board(r, f, dr*dist, df*dist) && pos.square_list[i + offset] == EPieceCode::epc_empty) {
		add_move(moves, i, i + offset);
		dist++;
		offset = dist*(dr*8 + df);
	}
	if (is_on_board(r, f, dr * dist, df * dist) && get_clr(pos.square_list[i+offset]) == !pos.side_to_move)
		add_move(moves, i, i + offset, true);
};

void Chess::gen_rooklike(vector<Move>& moves, int i, int r, int f) {
	gen_raymoves(moves, i, r, f,  0,  1);   // move right
	gen_raymoves(moves, i, r, f,  0, -1);	// move left
	gen_raymoves(moves, i, r, f,  1,  0);	// move up
	gen_raymoves(moves, i, r, f, -1,  0);	// move down
}

void Chess::gen_bishoplike(vector<Move>& moves, int i, int r, int f) {
	gen_raymoves(moves, i, r, f,  1,  1);	 // move up right
	gen_raymoves(moves, i, r, f,  1, -1);	 // move up left
	gen_raymoves(moves, i, r, f, -1,  1);	 // move down right
	gen_raymoves(moves, i, r, f, -1, -1);    // move down left
}

void Chess::gen_knight(vector<Move>& moves, int i, int r, int f) {
	for (int dr : {-2, -1, 1, 2}) {
		for (int df = -3 + abs(dr); df <= 3 - abs(dr); df += 2 * (3 - abs(dr))) {
			if (is_on_board(r, f, dr, df)) {
				int target = (r + dr) * 8 + f + df;
				if (pos.square_list[target] == EPieceCode::epc_empty)
					add_move(moves, i, target);
				else if (get_clr(pos.square_list[target]) == !pos.side_to_move)
					add_move(moves, i, target, true);
			}
		}
	}

}

void Chess::gen_king(vector<Move>& moves, int i, int r, int f) {
	// Normal moves
	for (int dr = -1; dr <= 1; dr++) {
		for (int df = -1; df <= 1; df++) {
			if (dr == 0 && df == 0) continue;
			if (is_on_board(r, f, dr, df)) {
				int target = (r + dr) * 8 + f + df;
				if (pos.square_list[target] == EPieceCode::epc_empty)
					add_move(moves, i, target);
				else if  (get_clr(pos.square_list[target]) == !pos.side_to_move)
					add_move(moves, i, target, true);
			}
		}
	}

	// Castling moves
	if (pos.side_to_move == EPieceColor::clr_white) {
		if ((pos.castling_rights & cr_white_short) && pos.square_list[5] == EPieceCode::epc_empty && pos.square_list[6] == EPieceCode::epc_empty)
			add_move(moves, 4, 6);
		if ((pos.castling_rights & cr_white_long)  && pos.square_list[1] == EPieceCode::epc_empty && pos.square_list[2] == EPieceCode::epc_empty && pos.square_list[3] == EPieceCode::epc_empty)
			add_move(moves, 4, 2);
	}
	else {
		if ((pos.castling_rights & cr_black_short) && pos.square_list[61] == EPieceCode::epc_empty && pos.square_list[62] == EPieceCode::epc_empty)
			add_move(moves, 60, 62);
		if ((pos.castling_rights & cr_black_long) && pos.square_list[57] == EPieceCode::epc_empty && pos.square_list[58] == EPieceCode::epc_empty && pos.square_list[59] == EPieceCode::epc_empty) 
			add_move(moves, 60, 58);
	}

}

void Chess::gen_wpawn(vector<Move>& moves, int i, int r, int f) {
	// forward moves
	if (pos.square_list[i + 8] == EPieceCode::epc_empty) {
		if (r == 6) {  // Move is to promotion square
			for (EPieceCode prom : {EPieceCode::epc_wknight, EPieceCode::epc_wbishop, EPieceCode::epc_wrook, EPieceCode::epc_wqueen}) {
				add_move(moves, i, i + 8, false, prom);
			}
		}
		else {
			add_move(moves, i, i + 8);
			if (r == 1 && pos.square_list[i + 16] == EPieceCode::epc_empty)
				add_move(moves, i, i + 16);
		}
	}

	// captures
	for (int offset : {7, 9}) {
		if ((f==0 && offset==7) || (f==7 && offset==9)) continue;
		if (get_clr(pos.square_list[i+offset]) == !pos.side_to_move) {
			if (r == 6) {
				for (EPieceCode prom : {EPieceCode::epc_wknight, EPieceCode::epc_wbishop, EPieceCode::epc_wrook, EPieceCode::epc_wqueen}) {
					add_move(moves, i, i+offset, true, prom);
				}
			}
			else {
				add_move(moves, i, i+offset, true);
			}
		}
		if (i + offset == pos.en_passant_square)
			add_move(moves, i, i + offset, true, EPieceCode::epc_empty, true);
	}
}

void Chess::gen_bpawn(vector<Move>& moves, int i, int r, int f) {
	// forward moves
	if (pos.square_list[i - 8] == EPieceCode::epc_empty) {
		if (r == 1) {
			for (EPieceCode prom : {EPieceCode::epc_bknight, EPieceCode::epc_bbishop, EPieceCode::epc_brook, EPieceCode::epc_bqueen}) {
				add_move(moves, i, i - 8, false, prom);
			}
		}
		else {
			add_move(moves, i, i - 8);
			if (r == 6 && pos.square_list[i - 16] == EPieceCode::epc_empty)
				add_move(moves, i, i - 16);
		}
	}

	// captures
	for (int offset : {-7, -9}) {
		if ((f==0 && offset==-9) || (f==7 && offset==-7)) continue;
		if (get_clr(pos.square_list[i+offset]) == !pos.side_to_move) {
			if (r == 1) {
				for (EPieceCode prom : {EPieceCode::epc_bknight, EPieceCode::epc_bbishop, EPieceCode::epc_brook, EPieceCode::epc_bqueen}) {
					add_move(moves, i, i + offset, true, prom);
				}
			}
			else {
				add_move(moves, i, i + offset, true);
			}
		}
		if (i + offset == pos.en_passant_square)
			add_move(moves, i, i + offset, true, EPieceCode::epc_empty, true);
	}
}

void Chess::print_pseudolegal_moves() {
	cout << "Pseudolegal Moves:" << endl << endl;
	cout << "#\tMove\told_ep\told_hm\tcapt\tprom\tis_ep\tlost_castle" << endl;
	cout << "-----------------------------------------------------------" << endl;
	int i = 0;
	for (auto const& mv : pseudolegal_moves) {
		cout << ++i << '\t' << mv << endl;
	}
	cout << endl;
}

void Chess::print_board() {
	cout << "FEN: " << pos << std::endl << std::endl;

	for (int r = 7; r >= 0; r--) {
		cout << "+---+---+---+---+---+---+---+---+" << std::endl;
		for (int f = 0; f < 8; f++) {
			cout << "| " << pos.square_list[r * 8 + f] << " ";
		}
		cout << "| " << r + 1 << std::endl;
	}
	cout << "+---+---+---+---+---+---+---+---+" << std::endl;
	cout << "  a   b   c   d   e   f   g   h  " << std::endl;
	cout << std::endl;
}

// Attempt to do move m. Responsible for checking if (pseudo)legal, and if so, update pseudolegal moves and in_check. Returns succes flag.
bool Chess::do_move(const Move& mv) {
	EPieceCode moving_piece = pos.square_list[mv.from];
	update_board(mv);
	vector<Move> new_moves{};
	new_moves.reserve(100);
	generate_pseudolegal_moves(new_moves);

	// Check if move was legal

	// First, check if any move can simply capture the king (i.e. was left or put in check)
	for (Move &new_mv : new_moves) {
		if (get_ept(new_mv.capture) == EPieceType::ept_king) {
			revert_board(mv);
			return false;
		}
	}

	// If move was castle, check if castle was not FROM or THROUGH check
	if (get_ept(moving_piece) == EPieceType::ept_king && abs(mv.to - mv.from)==2) {
		int through_sq = (mv.to + mv.from)/2;

		// Check Non-pawn pieces -- they can pseudolegally move to the FROM or THROUGH square when they attack them.
		for (Move &new_mv : new_moves) {
			if (pos.square_list[new_mv.from] != EPieceCode::epc_wpawn &&
				pos.square_list[new_mv.from] != EPieceCode::epc_bpawn &&
				(new_mv.to == mv.from || new_mv.to == through_sq))
			{
				revert_board(mv);
				return false;
			}
		}

		// Check if white pawn pieces attack blacks FROM or THROUGH squares
		if (through_sq/8 == 7) {
			for(int i : {through_sq - 7, through_sq - 9, mv.from - 7, mv.from - 9}) {
				if (pos.square_list[i] == EPieceCode::epc_wpawn) {
					revert_board(mv);
					return false;
				}
			}
		}
		// Check if black pawn pieces attack whites FROM or THROUGH squares
		else {
			for(int i : {through_sq + 7, through_sq + 9, mv.from + 7, mv.from + 9}) {
				if (pos.square_list[i] == EPieceCode::epc_bpawn) {
					revert_board(mv);
					return false;
				}
			}
		}
	}


	// Move was legal!

	//Update piece_count
	if (mv.capture != EPieceCode::epc_empty) {
		piece_count[(int)mv.capture]--;
		piece_count[0]++;  // empty = 0, increase empty count.
	}

	if (mv.promotion != EPieceCode::epc_empty) {
		if(mv.to == 7) {
			piece_count[(int)mv.promotion]++;
			piece_count[(int)EPieceCode::epc_wpawn]--;
		}
		else {
			piece_count[(int)mv.promotion]++;
			piece_count[(int)EPieceCode::epc_bpawn]--;

		}
	}

	// Update move history
	move_history.push_back(mv);

	// Update pseudolegal moves (already calculated)
	pseudolegal_moves = std::move(new_moves);

	return true;
}

// Undo last n moves in move_history
void Chess::undo_last_moves(const int n, const bool recalc_pseudolegal_moves) {
	for (int i = n; i > 0 && !move_history.empty(); i--) {
		Move mv = move_history.back();
		move_history.pop_back();

		revert_board(mv);

		// revert piece_count
		if (mv.capture != EPieceCode::epc_empty) {
			piece_count[(int)mv.capture]++;
			piece_count[0]--;
		}

		if (mv.promotion != EPieceCode::epc_empty) {
			if(mv.to == 7) {
				piece_count[(int)mv.promotion]--;
				piece_count[(int)EPieceCode::epc_wpawn]++;
			}
			else {
				piece_count[(int)mv.promotion]--;
				piece_count[(int)EPieceCode::epc_bpawn]++;

			}
		}
	}
	if (recalc_pseudolegal_moves)
		generate_pseudolegal_moves(pseudolegal_moves);
}

// No checking nothing, just modify Board struct pos by performing Move mv
void Chess::update_board(const Move& mv) {
	EPieceCode moving_piece = pos.square_list[mv.from];

	// Move piece
	pos.square_list[mv.to] = moving_piece;
	pos.square_list[mv.from] = EPieceCode::epc_empty;

	// Check if move is castling move and move Rook!
	if (get_ept(moving_piece) == EPieceType::ept_king && abs(mv.to - mv.from)==2) {
		if(mv.to > mv.from) {   // Short castle
			pos.square_list[mv.to-1] = pos.square_list[mv.from+3];
			pos.square_list[mv.from+3] = EPieceCode::epc_empty;
		}
		else {   // Long castle
			pos.square_list[mv.to+1] = pos.square_list[mv.from-4];
			pos.square_list[mv.from-4] = EPieceCode::epc_empty;
		}
	}

	// Replace promote
	if (mv.promotion != EPieceCode::epc_empty) {
		pos.square_list[mv.to] = mv.promotion;
	}

	// Remove en-passant capture
	if (mv.en_passant) {
		if(mv.to > mv.from) {
			pos.square_list[mv.to-8] = EPieceCode::epc_empty;
		}
		else {
			pos.square_list[mv.to+8] = EPieceCode::epc_empty;
		}
	}

	pos.side_to_move = !pos.side_to_move;
	pos.castling_rights = pos.castling_rights ^ mv.lost_castle_rights;

	if (moving_piece == EPieceCode::epc_wpawn && mv.to - mv.from == 16)
		pos.en_passant_square = mv.to-8;
	else if (moving_piece == EPieceCode::epc_bpawn && mv.to - mv.from == -16)
		pos.en_passant_square = mv.to+8;
	else
		pos.en_passant_square = -1;

	if (moving_piece == EPieceCode::epc_wpawn || moving_piece == EPieceCode::epc_bpawn || mv.capture != EPieceCode::epc_empty)
		pos.half_move_count = 0;
	else
		pos.half_move_count++;

	if(pos.side_to_move == EPieceColor::clr_white)   // Side to move was already flipped, so it was really Black who moved.
		pos.full_move_count++;
}

void Chess::revert_board(const Move& mv) {
	EPieceCode moved_piece = pos.square_list[mv.to];

	// Unmove piece
	pos.square_list[mv.from] = moved_piece;
	if (mv.en_passant) {
		pos.square_list[mv.to] = EPieceCode::epc_empty;
		if(mv.to > mv.from) {  // White is capturing en passant
			pos.square_list[mv.to-8] = EPieceCode::epc_bpawn;
		}
		else {
			pos.square_list[mv.to+8] = EPieceCode::epc_wpawn;
		}
	}
	else
		pos.square_list[mv.to] = mv.capture;

	// Check if move is castling move and unmove Rook!
	if (get_ept(moved_piece) == EPieceType::ept_king && abs(mv.to - mv.from)==2) {
		if(mv.to > mv.from) {   // Short castle
			pos.square_list[mv.from+3] = pos.square_list[mv.to-1];
			pos.square_list[mv.to-1] = EPieceCode::epc_empty;
		}
		else {   // Long castle
			pos.square_list[mv.from-4] = pos.square_list[mv.to+1];
			pos.square_list[mv.to+1] = EPieceCode::epc_empty;
		}
	}

	// Unpromote
	if (mv.promotion != EPieceCode::epc_empty){
		if (mv.to/8==7)
			pos.square_list[mv.from] = EPieceCode::epc_wpawn;
		else
			pos.square_list[mv.from] = EPieceCode::epc_bpawn;
	}

	pos.side_to_move = !pos.side_to_move;
	pos.castling_rights = pos.castling_rights ^ mv.lost_castle_rights;
	pos.en_passant_square = mv.old_en_passant_square;
	pos.half_move_count = mv.old_halfmove_count;
	if (pos.side_to_move == EPieceColor::clr_black)
		pos.full_move_count--;

}


bool Chess::do_move(unsigned int n) {
	return do_move(pseudolegal_moves[n-1]);
}


int Chess::perft(const unsigned int n, const bool split, const bool progress) {
	if (n == 0) {
		return 1;
	}

	int nodes = 0;


	// Make copy since moving changes pseudolegal_moves
	vector<Move> current_pseudo = pseudolegal_moves;

	map<string, int> splits;

	float sz = (float)current_pseudo.size();
	for (size_t i = 0; i != current_pseudo.size(); i++) {
		if (progress) {
			std::cout << "[";
			int pos = 70 * (i/sz);
			for (int i = 0; i < 70; ++i) {
				if (i < pos) std::cout << "=";
				else if (i == pos) std::cout << ">";
				else std::cout << " ";
			}
			std::cout << "] " << int(i/sz * 100.0 + 0.0001) << " %\r";
			std::cout.flush();
		}

		Move mv = current_pseudo[i];
		if (do_move(mv)){
			int add = perft(n-1);
			nodes += add;
			undo_last_moves(1, false);
			if (split) {
				stringstream key;
				key << square_name(mv.from) << '-' << square_name(mv.to) << mv.promotion;
				splits[key.str()] = add;
			}
		}
	}

	if (progress) {
		std::cout << "[" << std::string(70, '=') << "] 100 %" << endl;
	}

	for( auto const& x : splits ) {
		cout << x.first << ": " << x.second << endl;
	}

	pseudolegal_moves = std::move(current_pseudo);

	return nodes;

}
