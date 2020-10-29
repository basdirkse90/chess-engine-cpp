#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include<algorithm>
#include "Chess.h"


using namespace std;

int main() {
	string fen_list[7];
	fen_list[0] = "rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 1 1";
	fen_list[1] = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ";
	fen_list[2] = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
	fen_list[3] = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
	fen_list[4] = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
	fen_list[5] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
	fen_list[6] = "rnbqkb1r/ppp2pp1/5n1p/2Ppp3/4P3/7P/PP1P1PP1/RNBQKBNR w KQkq d6 0 5";


	Chess c1 = Chess();
	c1.print_board();
	c1.print_pseudolegal_moves();

	int nmin = 3;
	int nmax = 3;
	bool split = true;

	for (int n = nmin; n <= nmax; n++) {
		if (split) {
			cout << "perft(" << n << "): \n";
			int res = c1.perft(n, true);
			cout << "    TOTAL: " << res << endl << endl;
		}
		else{
			cout << "perft(" << n << "): " << c1.perft(n) << endl;
		}

	}

	c1.print_board();
	c1.print_pseudolegal_moves();


	/*
	for(string fen : fen_list) {
		cout << endl << "===============================================================================================" << endl << endl << endl;
		c1 = Chess(fen);
		c1.print_board();
		c1.print_pseudolegal_moves();
	}
	*/


	return 0;
}
