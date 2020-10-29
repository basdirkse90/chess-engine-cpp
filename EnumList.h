/*
 * EnumList.h
 *
 *  Created on: 24 Oct 2020
 *      Author: Bas
 */

#pragma once

#include<iostream>

enum class EPieceType {
	ept_pnil = 0,

	ept_wpawn = 1,
	ept_bpawn = 2,
	ept_knight = 3,
	ept_bishop = 4,
	ept_rook = 5,
	ept_queen = 6,
	ept_king = 7,
};


enum class EPieceCode {
	epc_empty = 0,

	epc_wpawn = (int)EPieceType::ept_wpawn,
	epc_wknight = (int)EPieceType::ept_knight,
	epc_wbishop = (int)EPieceType::ept_bishop,
	epc_wrook = (int)EPieceType::ept_rook,
	epc_wqueen = (int)EPieceType::ept_queen,
	epc_wking = (int)EPieceType::ept_king,

	epc_bpawn = (int)EPieceType::ept_bpawn + 8,
	epc_bknight = (int)EPieceType::ept_knight + 8,
	epc_bbishop = (int)EPieceType::ept_bishop + 8,
	epc_brook = (int)EPieceType::ept_rook + 8,
	epc_bqueen = (int)EPieceType::ept_queen + 8,
	epc_bking = (int)EPieceType::ept_king + 8,
};

enum class EPieceColor {
	clr_none = 0,
	clr_white = 1,
	clr_black = 2,
};


enum CastlingRights {
	cr_none = 0,
	cr_white_short = 1,
	cr_white_long = 2,
	cr_black_short = 4,
	cr_black_long = 8,

	cr_white_both = cr_white_short + cr_white_long,
	cr_black_both = cr_black_short + cr_black_long,
	cr_all = cr_white_both + cr_black_both
};


struct Move {
	int from;								// From square (0-63)
	int to;									// To square (0-63)
	int old_en_passant_square;				// En passant square (-1 to 63) before move (to undo)
	unsigned int old_halfmove_count;		// Halfmove count before move (to undo)
	EPieceCode capture;						// Set if move is capture (to undo)
	EPieceCode promotion;					// Set if move is promotion
	bool en_passant;						// Set true if capture was en passant
	CastlingRights lost_castle_rights;		// Set if move loses castling rights (to undo)
};


struct Board {
	/* Represents a chess board state. Does no checking or keeping track at all. That is delegated to the Chess class.
	Minimal info to uniquely define a board:

	square_list -- Array of length 64, keeping track of what piece is at what square (epc_empty for no piece)
	side_to_move -- Stores which side is to play next
	castling_rights -- Stores what castlings are still allowed -- int (cast from CastlingRights; add together multiple)
	en_passant_square -- Target square for en passant capture (0-63 or -1 for none)
	half_move_count -- Counts half moves since last capture or pawn push
	full_move_count -- Counts full moves after black moves
	*/
	EPieceCode square_list[64]{};
	EPieceColor side_to_move{};
	CastlingRights castling_rights{};
	int en_passant_square{};
	unsigned int half_move_count{};
	unsigned int full_move_count{};

};

std::ostream& operator<<(std::ostream& res, Board& b);
std::istream& operator>>(std::istream& in,  Board& b);
std::ostream& operator<<(std::ostream& out, const Move& m);
std::ostream& operator<<(std::ostream& res, const EPieceCode epc);
std::ostream& operator<<(std::ostream& res, const EPieceColor clr);
std::ostream& operator<<(std::ostream& res, const CastlingRights cr);
std::string square_name(int i);



CastlingRights operator&(CastlingRights lhs, CastlingRights rhs);
CastlingRights operator^(CastlingRights lhs, CastlingRights rhs);
EPieceColor operator!(EPieceColor stm);

EPieceCode ept2epc(EPieceType ept, EPieceColor clr);
EPieceType get_ept(EPieceCode epc);
EPieceColor get_clr(EPieceCode epc);
