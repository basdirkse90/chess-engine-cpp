#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include "EnumList.h"
#include "Chess.h"

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
			gen_rooklike(res, i, r, f, true);
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

void Chess::gen_raymoves(vector<Move>& moves, const function<bool(int)> &cond, int diff, int i, bool is_rook) {
	int offset = diff;
	while (cond(offset / diff) && pos.square_list[i + offset] == EPieceCode::epc_empty) {
		Move mv {i, i + offset, pos.en_passant_square, pos.half_move_count };
		if (is_rook) {
			switch(i) {
			case  0 : mv.lost_castle_rights = (pos.castling_rights & cr_white_long);  break;
			case  7 : mv.lost_castle_rights = (pos.castling_rights & cr_white_short); break;
			case 56 : mv.lost_castle_rights = (pos.castling_rights & cr_black_long);  break;
			case 63 : mv.lost_castle_rights = (pos.castling_rights & cr_black_short); break;
			}
		}
		moves.push_back(mv);
		offset += diff;
	}
	if (cond(offset / diff) && get_clr(pos.square_list[i+offset]) == !pos.side_to_move) {
		Move mv{ i, i + offset, pos.en_passant_square, pos.half_move_count, pos.square_list[i + offset] };
		if (is_rook) {
			switch(i) {
			case  0 : mv.lost_castle_rights = (pos.castling_rights & cr_white_long);  break;
			case  7 : mv.lost_castle_rights = (pos.castling_rights & cr_white_short); break;
			case 56 : mv.lost_castle_rights = (pos.castling_rights & cr_black_long);  break;
			case 63 : mv.lost_castle_rights = (pos.castling_rights & cr_black_short); break;
			}
		}
		moves.push_back(mv);
	}
};

void Chess::gen_rooklike(vector<Move>& moves, int i, int r, int f, bool is_rook) {
	gen_raymoves(moves, [f](int offset)->bool { return f + offset < 8; }, 1, i);    // move right
	gen_raymoves(moves, [f](int offset)->bool { return f - offset >= 0; }, -1, i);	// move left
	gen_raymoves(moves, [r](int offset)->bool { return r + offset < 8; }, 8, i);	// move up
	gen_raymoves(moves, [r](int offset)->bool { return r - offset >= 0; }, -8, i);	// move down

}

void Chess::gen_bishoplike(vector<Move>& moves, int i, int r, int f) {
	gen_raymoves(moves, [f, r](int offset)->bool { return f + offset < 8 && r + offset < 8; }, 9, i);	 // move up right
	gen_raymoves(moves, [f, r](int offset)->bool { return f - offset >= 0 && r + offset < 8; }, 7, i);	 // move up left
	gen_raymoves(moves, [f, r](int offset)->bool { return f + offset < 8 && r - offset >= 0; }, -7, i);	 // move down right
	gen_raymoves(moves, [f, r](int offset)->bool { return f - offset >= 0 && r - offset >= 0; }, -9, i); // move down left

}

void Chess::gen_knight(vector<Move>& moves, int i, int r, int f) {
	for (int dr : {-2, -1, 1, 2}) {
		if (dr == 0) continue;
		for (int df = -3 + abs(dr); df <= 3 - abs(dr); df += 2 * (3 - abs(dr))) {
			if (0 <= r + dr && r + dr < 8 && 0 <= f + df && f + df < 8) {
				int target = (r + dr) * 8 + f + df;
				if (pos.square_list[target] == EPieceCode::epc_empty)
					moves.push_back(Move{ i, target, pos.en_passant_square, pos.half_move_count });
				else if (get_clr(pos.square_list[target]) == !pos.side_to_move)
					moves.push_back(Move{ i, target, pos.en_passant_square, pos.half_move_count, pos.square_list[target] });
			}
		}
	}

}

void Chess::gen_king(vector<Move>& moves, int i, int r, int f) {
	// Normal moves
	for (int dr = -1; dr <= 1; dr++) {
		for (int df = -1; df <= 1; df++) {
			if (dr == 0 && df == 0) continue;
			if (0 <= r + dr && r + dr < 8 && 0 <= f + df && f + df < 8) {
				int target = (r + dr) * 8 + f + df;
				Move mv;
				if (pos.square_list[target] == EPieceCode::epc_empty)
					mv = { i, target, pos.en_passant_square, pos.half_move_count };
				else if  (get_clr(pos.square_list[target]) == !pos.side_to_move)
					mv = { i, target, pos.en_passant_square, pos.half_move_count, pos.square_list[target]};
				else
					continue;

				if (pos.side_to_move == EPieceColor::clr_white)
					mv.lost_castle_rights = pos.castling_rights & cr_white_both;
				else
					mv.lost_castle_rights = pos.castling_rights & cr_black_both;
				moves.push_back(mv);
			}
		}
	}

	// Castling moves
	if (pos.side_to_move == EPieceColor::clr_white) {
		if ((pos.castling_rights & cr_white_short) &&
			(pos.square_list[5] == EPieceCode::epc_empty) &&
			(pos.square_list[6] == EPieceCode::epc_empty))
		{
			moves.push_back(Move {4, 6, pos.en_passant_square, pos.half_move_count, EPieceCode::epc_empty, EPieceCode::epc_empty, false, pos.castling_rights & cr_white_both} ); // @suppress("Symbol is not resolved")
		}
		if ((pos.castling_rights & cr_white_long) &&
			(pos.square_list[1] == EPieceCode::epc_empty) &&
			(pos.square_list[2] == EPieceCode::epc_empty) &&
			(pos.square_list[3] == EPieceCode::epc_empty))
		{
			moves.push_back(Move {4, 2, pos.en_passant_square, pos.half_move_count, EPieceCode::epc_empty, EPieceCode::epc_empty, false, pos.castling_rights & cr_white_both }); // @suppress("Symbol is not resolved")
		}
	}
	else {
		if ((pos.castling_rights & cr_black_short) &&
			(pos.square_list[61] == EPieceCode::epc_empty) &&
			(pos.square_list[62] == EPieceCode::epc_empty))
		{
			moves.push_back(Move {60, 62, pos.en_passant_square, pos.half_move_count, EPieceCode::epc_empty, EPieceCode::epc_empty, false, pos.castling_rights & cr_black_both }); // @suppress("Symbol is not resolved")
		}
		if ((pos.castling_rights & cr_black_long) &&
			(pos.square_list[57] == EPieceCode::epc_empty) &&
			(pos.square_list[58] == EPieceCode::epc_empty) &&
			(pos.square_list[59] == EPieceCode::epc_empty))
		{
			moves.push_back(Move {60, 58, pos.en_passant_square, pos.half_move_count, EPieceCode::epc_empty, EPieceCode::epc_empty, false, pos.castling_rights & cr_black_both });
		}
	}

}

void Chess::gen_wpawn(vector<Move>& moves, int i, int r, int f) {
	// forward moves
	if (pos.square_list[i + 8] == EPieceCode::epc_empty) {
		if (r == 6) {  // Move is to promotion square
			for (EPieceCode prom : {EPieceCode::epc_wknight, EPieceCode::epc_wbishop, EPieceCode::epc_wrook, EPieceCode::epc_wqueen}) {
				moves.push_back(Move{ i, i + 8, pos.en_passant_square, pos.half_move_count, EPieceCode::epc_empty, prom });
			}
		}
		else {
			moves.push_back(Move{i, i + 8, pos.en_passant_square, pos.half_move_count });
			if (r == 1 && pos.square_list[i + 16] == EPieceCode::epc_empty)
				moves.push_back(Move{ i, i + 16, pos.en_passant_square, pos.half_move_count });
		}
	}

	// captures
	for (int offset : {7, 9}) {
		if ((f==0 && offset==7) || (f==7 && offset==9)) continue;
		if (get_clr(pos.square_list[i+offset]) == !pos.side_to_move) {
			if (r == 6) {
				for (EPieceCode prom : {EPieceCode::epc_wknight, EPieceCode::epc_wbishop, EPieceCode::epc_wrook, EPieceCode::epc_wqueen}) {
					moves.push_back(Move{ i, i + offset, pos.en_passant_square, pos.half_move_count, pos.square_list[i + offset], prom });
				}
			}
			else {
				moves.push_back(Move{ i, i + offset, pos.en_passant_square, pos.half_move_count, pos.square_list[i + offset] });
			}
		}
		if (i + offset == pos.en_passant_square)
			moves.push_back(Move{ i, i + offset, pos.en_passant_square, pos.half_move_count, EPieceCode::epc_bpawn, EPieceCode::epc_empty, true });
	}
}

void Chess::gen_bpawn(vector<Move>& moves, int i, int r, int f) {
	// forward moves
	if (pos.square_list[i - 8] == EPieceCode::epc_empty) {
		if (r == 1) {
			for (EPieceCode prom : {EPieceCode::epc_bknight, EPieceCode::epc_bbishop, EPieceCode::epc_brook, EPieceCode::epc_bqueen}) {
				moves.push_back(Move{ i, i - 8, pos.en_passant_square, pos.half_move_count, EPieceCode::epc_empty, prom });
			}
		}
		else {
			moves.push_back(Move{ i, i - 8, pos.en_passant_square, pos.half_move_count });
			if (r == 6 && pos.square_list[i - 16] == EPieceCode::epc_empty)
				moves.push_back(Move{ i, i - 16, pos.en_passant_square, pos.half_move_count });
		}
	}

	// captures
	for (int offset : {-7, -9}) {
		if ((f==0 && offset==-9) || (f==7 && offset==-7)) continue;
		if (get_clr(pos.square_list[i+offset]) == !pos.side_to_move) {
			if (r == 1) {
				for (EPieceCode prom : {EPieceCode::epc_bknight, EPieceCode::epc_bbishop, EPieceCode::epc_brook, EPieceCode::epc_bqueen}) {
					moves.push_back(Move{ i, i + offset, pos.en_passant_square, pos.half_move_count, pos.square_list[i + offset], prom });
				}
			}
			else {
				moves.push_back(Move{ i, i + offset, pos.en_passant_square, pos.half_move_count, pos.square_list[i + offset] });
			}
		}
		if (i + offset == pos.en_passant_square)
			moves.push_back(Move{ i, i + offset, pos.en_passant_square, pos.half_move_count, EPieceCode::epc_wpawn, EPieceCode::epc_empty, true }); // @suppress("Symbol is not resolved")
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
	generate_pseudolegal_moves(new_moves);

	// Check if move was legal

	// First, check if any move can simply capture the king (i.e. was left or put in check)
	for (Move new_mv : new_moves) {
		if (get_ept(new_mv.capture) == EPieceType::ept_king) {
			revert_board(mv);
			return false;
		}
	}

	// If move was castle, check if castle was not FROM or THROUGH check
	if (get_ept(moving_piece) == EPieceType::ept_king && abs(mv.to - mv.from)==2) {
		int through_sq = (mv.to + mv.from)/2;

		// Check Non-pawn pieces -- they can pseudolegally move to the FROM or THROUGH square when they attack them.
		for (Move new_mv : new_moves) {
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
	pos.castling_rights = pos.castling_rights & mv.lost_castle_rights;

	if (moving_piece == EPieceCode::epc_wpawn && mv.to - mv.from == 16)
		pos.en_passant_square = mv.to-8;
	else if (moving_piece == EPieceCode::epc_bpawn && mv.to - mv.from == -16)
		pos.en_passant_square = mv.to+8;

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


int Chess::perft(const unsigned int n, const bool split) {
	if (n == 0) {
		return 1;
	}

	int nodes = 0;


	// Make copy since moving changes pseudolegal_moves
	vector<Move> current_pseudo = pseudolegal_moves;

	for (Move mv : current_pseudo) {
		if (do_move(mv)){
			int add = perft(n-1);
			nodes += add;
			undo_last_moves(1, false);
			if (split)
				cout << "\t" << square_name(mv.from) << '-' << square_name(mv.to) << mv.promotion << ": " << add << endl;
		}
	}

	pseudolegal_moves = std::move(current_pseudo);

	return nodes;

}
