/*
 * EnumList.cpp
 *
 *  Created on: 24 Oct 2020
 *      Author: Bas
 */

#include "EnumList.h"
#include <sstream>

std::ostream& operator<<(std::ostream& out, const Move& mv) {
	out << square_name(mv.from) << '-' << square_name(mv.to);
	out << '\t' << square_name(mv.old_en_passant_square);
	out << '\t' << mv.old_halfmove_count;

	if (mv.capture == EPieceCode::epc_empty)
		out << "\t-";
	else
		out << '\t' << mv.capture;

	if (mv.promotion == EPieceCode::epc_empty)
		out << "\t-";
	else
		out << '\t' << mv.promotion;

	if (mv.en_passant)
		out << "\tyes";
	else
		out << "\tno";

	out << '\t' << mv.lost_castle_rights;
	return out;
}

std::ostream& operator<<(std::ostream& res, const EPieceCode epc) {
	switch (epc) {
	case EPieceCode::epc_wpawn:		res << "P"; break;
	case EPieceCode::epc_wrook:		res << "R"; break;
	case EPieceCode::epc_wknight:	res << "N"; break;
	case EPieceCode::epc_wbishop:	res << "B"; break;
	case EPieceCode::epc_wking:		res << "K"; break;
	case EPieceCode::epc_wqueen:	res << "Q"; break;

	case EPieceCode::epc_bpawn:		res << "p"; break;
	case EPieceCode::epc_brook:		res << "r"; break;
	case EPieceCode::epc_bknight:	res << "n"; break;
	case EPieceCode::epc_bbishop:	res << "b"; break;
	case EPieceCode::epc_bking:		res << "k"; break;
	case EPieceCode::epc_bqueen:	res << "q"; break;
	default : 						res << " "; break;
	}
	return res;
}

std::ostream& operator<<(std::ostream& res, const EPieceColor clr) {
	switch(clr) {
	case EPieceColor::clr_white : res << "w"; break;
	case EPieceColor::clr_black : res << "b"; break;
	default : res << " ";
	}
	return res;
}

std::ostream& operator<<(std::ostream& res, const CastlingRights cr) {
	if (cr == cr_none) {
		res << "-";
	}
	else {
		if (cr & cr_white_short) res << "K";
		if (cr & cr_white_long)  res << "Q";
		if (cr & cr_black_short) res << "k";
		if (cr & cr_black_long)  res << "q";
	}
	return res;
}

CastlingRights operator&(CastlingRights lhs, CastlingRights rhs) {
	return (CastlingRights) ((int)lhs & (int)rhs);
}

CastlingRights operator^(CastlingRights lhs, CastlingRights rhs) {
	return (CastlingRights) ((int)lhs ^ (int)rhs);
}

EPieceColor operator!(EPieceColor clr) {
	EPieceColor res;
	switch(clr) {
	case(EPieceColor::clr_none)  : res = EPieceColor::clr_none; break;
	case(EPieceColor::clr_white) : res = EPieceColor::clr_black; break;
	case(EPieceColor::clr_black) : res = EPieceColor::clr_white; break;
	}
	return res;
}

EPieceCode ept2epc(EPieceType ept, EPieceColor clr) {
	if (clr==EPieceColor::clr_none || ept==EPieceType::ept_pnil)
		return EPieceCode::epc_empty;
	return (EPieceCode)((int)ept + 8*((int)clr - 1));
}

EPieceType get_ept(EPieceCode epc) {
	return (EPieceType)((int)epc % 8);
}

EPieceColor get_clr(EPieceCode epc) {
	if (epc==EPieceCode::epc_empty)
		return EPieceColor::clr_none;
	return  (EPieceColor)((int)epc / 8 + 1);
}


std::ostream& operator<<(std::ostream& res, Board& b) {
	for (int r = 7; r >= 0; r--) {
		unsigned int empty = 0;

		for (int f = 0; f < 8; f++) {
			int i = r * 8 + f;

			if (b.square_list[i] == EPieceCode::epc_empty)
				empty++;
			else {
				if (empty) {
					res << empty;
					empty = 0;
				}
				res << b.square_list[i];
			}
		}
		if (empty) res << empty;
		if (r) res << "/";
	}

	res << " " << b.side_to_move;
	res << " " << b.castling_rights;
	res << " " << square_name(b.en_passant_square);
	res << " " << b.half_move_count;
	res << " " << b.full_move_count;

	return res;

}


std::istream& operator>>(std::istream& fen,  Board& b){
	std::string tokens[6];
	for (int i = 0; i < 6; i++) {
		fen >> tokens[i];
	}

	// Split first token (board setup) by forward slashes to get the separate ranks setup
	std::stringstream ss(tokens[0]);
	char delim = '/';
	std::string lines[8];
	for (int i = 0; i < 8; i++) {
		std::getline(ss, lines[i], delim);
	}

	// Assign each square the appropriate piece from the lines.
	for (int r = 7; r >= 0; r--) {
		std::string line = lines[7-r];
		int f = 0;
		for (size_t j = 0; j < line.length(); j++) {
			switch (line[j]) {
			case 'p': b.square_list[r * 8 + f] = EPieceCode::epc_bpawn; break;
			case 'r': b.square_list[r * 8 + f] = EPieceCode::epc_brook;  break;
			case 'n': b.square_list[r * 8 + f] = EPieceCode::epc_bknight; break;
			case 'b': b.square_list[r * 8 + f] = EPieceCode::epc_bbishop; break;
			case 'k': b.square_list[r * 8 + f] = EPieceCode::epc_bking; break;
			case 'q': b.square_list[r * 8 + f] = EPieceCode::epc_bqueen; break;

			case 'P': b.square_list[r * 8 + f] = EPieceCode::epc_wpawn; break;
			case 'R': b.square_list[r * 8 + f] = EPieceCode::epc_wrook; break;
			case 'N': b.square_list[r * 8 + f] = EPieceCode::epc_wknight; break;
			case 'B': b.square_list[r * 8 + f] = EPieceCode::epc_wbishop; break;
			case 'K': b.square_list[r * 8 + f] = EPieceCode::epc_wking; break;
			case 'Q': b.square_list[r * 8 + f] = EPieceCode::epc_wqueen; break;
			default:
				f += (int)(line[j] - '1');
			}
			f++;
		}
	}


	if (tokens[1] == "w") {
		b.side_to_move = EPieceColor::clr_white;
	}
	else {
		b.side_to_move = EPieceColor::clr_black;
	}


	for (char c : tokens[2]) {
		switch (c)
		{
		case 'K': b.castling_rights = b.castling_rights ^ cr_white_short; break;
		case 'Q': b.castling_rights = b.castling_rights ^ cr_white_long; break;
		case 'k': b.castling_rights = b.castling_rights ^ cr_black_short; break;
		case 'q': b.castling_rights = b.castling_rights ^ cr_black_long; break;
		}
	}

	if (tokens[3] == "-")
		b.en_passant_square = -1;
	else {
		int f = (int)(tokens[3][0] - 'a');
		int r = (int)(tokens[3][1] - '1');
		b.en_passant_square = r * 8 + f;
	}

	b.half_move_count = stoi(tokens[4]);
	b.full_move_count = stoi(tokens[5]);

	return fen;
}


std::string square_name(int i) {
	if (i < 0 || i>63) {
		return "-";
	}
	std::stringstream res("");
	res << (char)('a' + (i % 8)) << (char)('1' + (i / 8));
	return res.str();
}



