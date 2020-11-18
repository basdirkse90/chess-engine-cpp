#include <iostream>
#include <string>
#include <sstream>
#include <future>
#include "UCIReader.h"
#include "Chess.h"

using namespace std;

const std::string UCIReader::ENGINENAME = "BasEngine v1";
const std::string UCIReader::ENGINEAUTHOR = "Bas Dirkse";


void UCIReader::uciCommunication() {
	while (true) {
		std::string inputLine;
		std::getline(std::cin, inputLine);

		std::string firstWord;
		std::string remainder;

		size_t sep = inputLine.find(' ');
		
		if (sep == std::string::npos) {
			firstWord = inputLine;
			remainder = "";
		}
		else {
			firstWord = inputLine.substr(0, sep);
			remainder = inputLine.substr(sep+1);
		}

		if (firstWord == "uci") {
			std::cout << "id name " << ENGINENAME << std::endl;
			std::cout << "id author " << ENGINEAUTHOR << std::endl;
			// send supporting options (nothing now)
			std::cout << "uciok" << std::endl;
		}
		else if (firstWord == "debug") {
			if (remainder == "on") {

			}
			else if (remainder == "off") {

			}

		}
		else if (firstWord == "isready") {
			// set up engine
			std::cout << "readyok" << std::endl;
		}
		else if (firstWord == "setoption") {

		}
		else if (firstWord == "register") {

		}
		else if (firstWord == "ucinewgame") {

		}
		else if (firstWord == "position") {

		}
		else if (firstWord == "go") {

		}
		else if (firstWord == "stop") {

		}
		else if (firstWord == "ponderhit") {

		}
		else if (firstWord == "quit") {
			return;
		}
		// Non UCI commands next
		else if (firstWord == "print" || firstWord == "d") {

		}
		else if (firstWord == "myperft") {
			if (remainder == "auto deep")
				myPerft(true, true);
			else if (remainder == "auto")
				myPerft(true);
			else 
				myPerft();
		}
		else {
			std::cout << "Unknown command: " << inputLine << std::endl;
		}



	}
}


void UCIReader::myPerft(bool runall, bool deep) {
	const int fen_len = 22;
	
	string fen_list[fen_len];
	fen_list[0] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";  					// Default initial position
	fen_list[1] = "rnbqkb1r/ppp2pp1/5n1p/2Ppp3/4P3/7P/PP1P1PP1/RNBQKBNR w KQkq d6 0 5";
	fen_list[2] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";  		// Position2.json
	fen_list[3] = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";  								// Position3.json
	fen_list[4] = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"; 			// Position4.json
	fen_list[5] = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"; 					// Position5.json
	fen_list[6] = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 "; 	// Position6.json

	fen_list[7] = "1k6/1b6/8/8/7R/8/8/4K2R b K - 0 1";
	fen_list[8] = "3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1";				//--Illegal ep move #1
	fen_list[9] = "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1";				//--Illegal ep move #2
	fen_list[10] = "8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1";			//--EP Capture Checks Opponent
	fen_list[11] = "5k2/8/8/8/8/8/8/4K2R w K - 0 1";				//--Short Castling Gives Check
	fen_list[12] = "3k4/8/8/8/8/8/8/R3K3 w Q - 0 1";				//--Long Castling Gives Check
	fen_list[13] = "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1";		//--Castle Rights
	fen_list[14] = "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1";		//--Castling Prevented
	fen_list[15] = "2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1";				//--Promote out of Check
	fen_list[16] = "8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1";			//--Discovered Check
	fen_list[17] = "4k3/1P6/8/8/8/8/K7/8 w - - 0 1";				//--Promote to give check
	fen_list[18] = "8/P1k5/K7/8/8/8/8/8 w - - 0 1";					//--Under Promote to give check
	fen_list[19] = "K1k5/8/P7/8/8/8/8/8 w - - 0 1";					//--Self Stalemate
	fen_list[20] = "8/k1P5/8/1K6/8/8/8/8 w - - 0 1";				//--Stalemate & Checkmate
	fen_list[21] = "8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1";				//--Stalemate & Checkmate

	if (runall) {
		vector<int> depth;
		vector<int> perft;

		if (deep) {
			depth = { 5, 4, 4, 5, 4, 4, 4, 5, 6, 6, 6, 6, 6, 4, 4, 6, 5, 6, 6, 6, 7, 4 };
			perft = { 4865609, 1073768, 4085603, 674624, 422333, 2103487, 3894594, 1063513, 1134888, 1015133, 1440467, 661072,
					  803711, 1274206, 1720476, 3821001, 1004658, 217342, 92683, 2217, 567584, 23527};
		}
		else {
			depth = { 4, 3, 3, 4, 3, 3, 3, 4, 5, 5, 5, 5, 5, 3, 3, 5, 4, 5, 6, 6, 7, 4 };
			perft = { 197281, 32636, 97862, 43238, 9467, 62379, 89890, 85765, 185429, 135655, 206379, 120330,
					  141077, 27826, 50509, 266199, 31961, 38983, 92683, 2217, 567584, 23527 };
		}

		std::future<int> res[fen_len];
		for (int i = 0; i < fen_len; i++) {
			res[i] = std::async(std::launch::async, [&fen_list, &depth, i]() { return Chess(fen_list[i]).perft(depth[i], false, false); });
			cout << "Computing perft(" << depth[i] << ") from position " << i << ": " << fen_list[i] << endl;
		}
		cout << endl;


		int correct = 0;
		for (int i = 0; i < fen_len; i++) {
			try {
				int result = res[i].get();
				cout << "Result of perft(" << depth[i] << ") from position " << i << ": " << result;
				if (result == perft[i]) {
					cout << " as expected." << endl;
					correct++;
				}
				else {
					cout << ", but expected " << perft[i] << "!!!" << endl;
				}
			}
			catch (const std::exception& e) {
				std::cerr << e.what() << endl;
			}
		}

		cout << "Perfts finished with " << correct << "/" << fen_len << " positions correct!" << endl;



	}
	else {
		while (true) {
			for (int i = 0; i < fen_len; i++) {
				cout << i << ": " << fen_list[i] << endl;
			}
			cout << endl;

			unsigned int p = 0, n = 0;

			cout << "Enter a fen identifier: ";
			while (!(cin >> p)) {
				cout << "Error: value must be unsigned int." << endl;
				cout << "Enter a fen identifier: ";
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
			}
			cin.ignore(numeric_limits<streamsize>::max(), '\n');

			cout << "Enter a perft depth: ";
			while (!(cin >> n)) {
				cout << "Error: value must be unsigned int." << endl;
				cout << "Enter a perft depth: ";
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
			}
			cin.ignore(numeric_limits<streamsize>::max(), '\n');



			cout << endl << endl << "Position " << p << ": " << fen_list[p] << endl;

			Chess c1 = Chess(fen_list[p]);
			cout << "perft(" << n << "): " << endl;
			int res = c1.perft(n, true, true);
			cout << endl << "Nodes searched: " << res << endl << endl;

			cout << "Do you want to continue? (y/n): ";
			string sentinal = "";
			cin >> sentinal;

			if (sentinal != "y") {
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				return;
			}
				
		}
	}
}

