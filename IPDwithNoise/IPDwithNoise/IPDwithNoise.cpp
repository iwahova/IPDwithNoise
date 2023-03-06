#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <graphics.h>
#include <conio.h>
#include <algorithm> 
#include <utility>
#include <math.h>
#include <map>
#include <fstream>
#include <vector>

using namespace std;
/*number representitives of two actions*/
#define COOPERATE	-1
#define DEFECT		-2
#define lineNum     200
#define rowNum		200

//number of different kinds of payoffs
#define Reward		3.0
#define Temptation  5.0
#define Sucker		0.0
#define Punishment  1.0

#define ROUNDS	50

/* Strategies in IPD tournament */
#define AIID	0 //black
#define AIIC	1 //white
#define TFT		2 //brown
#define GTFT	3 //generous tft //yellow
#define CTFT	4 //contrite tft //MAGENTA			
#define STFT	5
#define PAVLOV	6
#define RAND	7
#define NEG		8
#define TTFT	9
#define FBF		10
#define GRADUAL		11
#define ADAPTIVE	12
#define C_TFT		13
#define G_TFT		14
#define PROBER		15
#define MASTER		16
#define SLAVE		17
#define CSMSM       18

#define Contrite 0
#define Content  1
#define Provoked 2

string output_file1 = "tft_output.txt";
string output_file2 = "gtft_output.txt";
string output_file3 = "ctft_output.txt";

int noiseSamples;

//paths for storing images
map<int, vector<const wchar_t*>> paths_0percent_noise;
map<int, vector<const wchar_t*>> paths_1percent_noise;
map<int, vector<const wchar_t*>> paths_3percent_noise;
map<int, vector<const wchar_t*>> paths_5percent_noise;
map<int, vector<const wchar_t*>> paths_10percent_noise;
map<double, map<int, vector<const wchar_t*>>> paths;

//paths for storing offline payoffs under different noise
map<int, string> outputPaths_0percent_noise;
map<int, string> outputPaths_1percent_noise;
map<int, string> outputPaths_3percent_noise;
map<int, string> outputPaths_5percent_noise;
map<int, string> outputPaths_10percent_noise;
map<double, map<int, string>> outputFile_paths;




namespace classes{
	class strategy {
	public:
		int type = -2;
		double payoff = 0;
		strategy() {}
		int response() {}

		int getType() {
			return type;
		}

		double getPayoff() {
			return payoff;
		}

		void resetPayoff() {
			payoff = 0;
		}

		void addPayoff(double payoffToBeAdded) {
			if (type >= 0) {
				payoff += payoffToBeAdded;
			}
		}
		void getAveragePayoff(int numOfNeighbors) {
			payoff = payoff / numOfNeighbors;
		}
		void changeToSlave() {
			type = SLAVE;
		}
		void changeToMaster() {
			type = MASTER;
		}

		int addNoise(int choice) {
			int n = rand() % 100;
			//cout << noiseSamples << endl;
			if (n < noiseSamples) {
				if (choice == COOPERATE) {
					return DEFECT;
				}
				else {
					return COOPERATE;
				}
			}
			else {
				return choice;
			}
		}
	};

	class aiic : public strategy {
	public:
		aiic() : strategy() {
			type = AIIC;
		}
	};

	class aiid : public strategy {
	public:
		aiid() : strategy() {
			type = AIID;
		}
	};

	class tft : public strategy {
	public:
		bool firstRound = true;
		int lastMoveOfOpponent;
		tft() : strategy() {
			type = TFT;
		}

		void moveOfOpponent(int move) {
			lastMoveOfOpponent = move;
		}

		int getChoice() {
			int choice;
			if (firstRound) {
				choice = COOPERATE;
				firstRound = false;
			}
			else {
				choice = lastMoveOfOpponent;
			}
			return addNoise(choice);
		}
	};

	class gtft : public tft {
	public:
		gtft() : tft() {
			type = GTFT;
		}
		void moveOfOpponent(int move) {
			lastMoveOfOpponent = move;
		}

		int getChoice() {
			int choice;
			if (firstRound) {
				choice = COOPERATE;
				firstRound = false;
			}
			else {
				if (lastMoveOfOpponent == DEFECT) {
					int random = rand() % 100;
					if (random < 10) {
						choice = COOPERATE;
						//cout << "yuan liang" << endl << endl;
					}
					else {
						choice = DEFECT;
					}
				}
				else {
					choice = lastMoveOfOpponent;
				}
			}
			return addNoise(choice);
		}
	};

	class ctft : public tft {
	public:
		int currentMove = COOPERATE;
		int state = Content;
		bool wrong = false;
		ctft() : tft() {
			type = CTFT;
		}
		void moveOfOpponent(int move) {
			lastMoveOfOpponent = move;
			if (wrong) {
				state = Contrite;
			}
			else if (move == COOPERATE) {
				if (state == Provoked || state == Contrite) {
					state = Content;
				}
			}
			else if (move == DEFECT && state == Content) {
				state = Provoked;
			}
		}

		int getChoice() {
			int choice = 0;
			switch (state) {//make choice according to state
			case Content: {choice = COOPERATE;
				break; }
			case Contrite: {choice = COOPERATE;
				break; }
			case Provoked: {choice = DEFECT;
				break; }
			}
			currentMove = addNoise(choice);
			if (currentMove == DEFECT && choice == COOPERATE) {
				wrong = true;
			}
			return currentMove;
		}
	};

	class csmsm : public strategy {
	public:
		vector<int> sequence = { DEFECT, COOPERATE, COOPERATE, DEFECT, COOPERATE };
		int lastMove;
		csmsm() : strategy() {
			type = CSMSM;
		}

		int getChoice() {}

	};

	class master : public csmsm {
	public:
		bool isCS;
		bool trigerred;
		int currentMove;
		master() : csmsm() {
			type = MASTER;
			lastMove = COOPERATE;
			trigerred = false;
			isCS = true;
		}

		void moveOfOpponent(int move) {
			if (sequence.size() != 0) {
				if (currentMove != move) {
					isCS = false;
				}
				sequence.pop_back();
			}
			else {
				if (move == DEFECT) {
					trigerred = true;
				}
			}
		}

		int getChoice() {
			int choice;
			if (!isCS) {
				choice = DEFECT;
			}
			else if (sequence.size() != 0) {
				choice = sequence.back();
			}
			else {
				if (trigerred) {
					choice = DEFECT;
				}
				else {
					choice = COOPERATE;
				}
			}
			currentMove = addNoise(choice);
			return currentMove;
		}
	};

	class slave : public csmsm {
	public:
		bool isCS;
		int currentMove;
		int nextMove;
		int lastMoveOfOpponent;
		slave() : csmsm() {
			type = SLAVE;
			lastMove = COOPERATE;
			isCS = true;
		}

		void moveOfOpponent(int move) {
			lastMoveOfOpponent = move;
			if (sequence.size() != 0) {
				if (currentMove != move) {
					isCS = false;
				}
				sequence.pop_back();
			}
		}

		int getChoice() {
			int choice = 0;
			if (!isCS) {
				choice = DEFECT;
			}
			else if (sequence.size() != 0) {
				choice = sequence.back();
			}
			else {//reverse TFT
				if (lastMoveOfOpponent == COOPERATE) {
					choice = DEFECT;
				}
				else {
					choice = COOPERATE;
				}
			}
			currentMove = addNoise(choice);
			return currentMove;
		}
	};
}


using namespace classes;
class IPDcontroller {
public:
	vector<vector<strategy>> entities;
	map<int, COLORREF> colors;
	map<pair<int, int>, double> typeToPayoffs;
	map<pair<int, int>, double> typeToPayoffsWithNoise;
	map<int, string> outputFiles;
	int numOfGeneration;
	int strategy1, strategy2;
	int numOfTFT = 0, numOfAIIC = 0, numOfAIID = 0, numOfCS = 0, numOfGTFT = 0, numOfCTFT = 0;
	int numOfS1 = 20000, numOfS2 = 20000;
	double noiseRate = 0;

	int getAverageNoise() {
		//calculate master-master competition with noise
		double P1 = 0, P2 = 0;
		int numberOfSamples = 100;
		for (int i = 0; i < numberOfSamples; i++) {
			vector<int> s1, s2;
			double p1 = 0, p2 = 0;
			master m1 = master();
			master m2 = master();
			for (int j = 0; j < 100; j++) {
				int c1 = m1.getChoice();
				int c2 = m2.getChoice();
				s1.push_back(c1);
				s2.push_back(c2);

				//judge the payoff
				if (c1 == COOPERATE && c2 == COOPERATE) {
					p1 += Reward;
					p2 += Reward;
				}else if (c1 == COOPERATE && c2 == DEFECT) {
					p1 += Sucker;
					p2 += Temptation;
				}else if (c1 == DEFECT && c2 == COOPERATE) {
					p1 += Temptation;
					p2 += Sucker;
				}else {
					p1 += Punishment;
					p2 += Punishment;
				}

				//feedback the choice of the opponent
				m1.moveOfOpponent(c2);
				m2.moveOfOpponent(c1);
			}
			/*cout << "sequence of master1: ";
			for (auto& item : s1) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}
				
			}
			cout << "payoff of master1 : " << p1 << endl; 
			cout << "sequence of master2: ";
			for (auto& item : s2) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}
			}
			cout << "payoff of master1 : " << p2 << endl << endl;*/
			P1 += p1;
			P2 += p2;
		}
		cout << "average payoff of master1: " << P1 / numberOfSamples << "\taverage payoff of master2: " << P2 / numberOfSamples << endl;
		auto relation = pair<int, int>(MASTER, MASTER);
		typeToPayoffsWithNoise[relation] = P1 / numberOfSamples;
		

		//calculate slave-slave competition with noise
		P1 = 0, P2 = 0;
		for (int i = 0; i < numberOfSamples; i++) {
			vector<int> s1, s2;
			double p1 = 0, p2 = 0;
			slave slave1 = slave();
			slave slave2 = slave();
			for (int j = 0; j < 100; j++) {
				int c1 = slave1.getChoice();
				int c2 = slave2.getChoice();
				s1.push_back(c1);
				s2.push_back(c2);

				//judge the payoff
				if (c1 == COOPERATE && c2 == COOPERATE) {
					p1 += Reward;
					p2 += Reward;
				}
				else if (c1 == COOPERATE && c2 == DEFECT) {
					p1 += Sucker;
					p2 += Temptation;
				}
				else if (c1 == DEFECT && c2 == COOPERATE) {
					p1 += Temptation;
					p2 += Sucker;
				}
				else {
					p1 += Punishment;
					p2 += Punishment;
				}

				//feedback the choice of the opponent
				slave1.moveOfOpponent(c2);
				slave2.moveOfOpponent(c1);
			}
			/*cout << "sequence of slave1: ";
			for (auto& item : s1) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}

			}
			cout << "payoff of slave1: " << p1 << endl;
			cout << "sequence of slave2: ";
			for (auto& item : s2) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}
			}
			cout << "payoff of slave2: " << p2 << endl << endl;*/
			P1 += p1;
			P2 += p2;
		}
		cout << "average payoff of slave1: " << P1 / numberOfSamples << "\taverage payoff of slave2: " << P2 / numberOfSamples << endl;
		relation = pair<int, int>(SLAVE, SLAVE);
		typeToPayoffsWithNoise[relation] = P1 / numberOfSamples;

		//calculate slave-master (master-slave) competition with noise
		P1 = 0, P2 = 0;
		for (int i = 0; i < numberOfSamples; i++) {
			vector<int> s1, s2;
			double p1 = 0, p2 = 0;
			slave s = slave();
			master m = master();
			for (int j = 0; j < 100; j++) {
				int c1 = s.getChoice();
				int c2 = m.getChoice();
				s1.push_back(c1);
				s2.push_back(c2);

				//judge the payoff
				if (c1 == COOPERATE && c2 == COOPERATE) {
					p1 += Reward;
					p2 += Reward;
				}
				else if (c1 == COOPERATE && c2 == DEFECT) {
					p1 += Sucker;
					p2 += Temptation;
				}
				else if (c1 == DEFECT && c2 == COOPERATE) {
					p1 += Temptation;
					p2 += Sucker;
				}
				else {
					p1 += Punishment;
					p2 += Punishment;
				}

				//feedback the choice of the opponent
				s.moveOfOpponent(c2);
				m.moveOfOpponent(c1);
			}
			/*cout << "sequence of slave: ";
			for (auto& item : s1) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}

			}
			cout << "payoff of slave : " << p1 << endl;
			cout << "sequence of master: ";
			for (auto& item : s2) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}
			}
			cout << "payoff of master : " << p2 << endl << endl;*/
			P1 += p1;
			P2 += p2;
		}
		cout << "average payoff of slave: " << P1 / numberOfSamples << "\taverage payoff of master: " << P2 / numberOfSamples << endl;
		relation = pair<int, int>(SLAVE, MASTER);
		typeToPayoffsWithNoise[relation] = P1 / numberOfSamples;
		relation = pair<int, int>(MASTER, SLAVE);
		typeToPayoffsWithNoise[relation] = P2 / numberOfSamples;


		//calculate slave-TFT (master-slave) competition with noise
		P1 = 0, P2 = 0;
		for (int i = 0; i < numberOfSamples; i++) {
			vector<int> s1, s2;
			double p1 = 0, p2 = 0;
			slave s = slave();
			tft t = tft();
			for (int j = 0; j < 100; j++) {
				int c1 = s.getChoice();
				int c2 = t.getChoice();
				s1.push_back(c1);
				s2.push_back(c2);

				//judge the payoff
				if (c1 == COOPERATE && c2 == COOPERATE) {
					p1 += Reward;
					p2 += Reward;
				}
				else if (c1 == COOPERATE && c2 == DEFECT) {
					p1 += Sucker;
					p2 += Temptation;
				}
				else if (c1 == DEFECT && c2 == COOPERATE) {
					p1 += Temptation;
					p2 += Sucker;
				}
				else {
					p1 += Punishment;
					p2 += Punishment;
				}

				//feedback the choice of the opponent
				s.moveOfOpponent(c2);
				t.moveOfOpponent(c1);
			}
			/*cout << "sequence of slave: ";
			for (auto& item : s1) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}

			}
			cout << "payoff of slave : " << p1 << endl;
			cout << "sequence of tft: ";
			for (auto& item : s2) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}
			}
			cout << "payoff of tft : " << p2 << endl << endl;*/
			P1 += p1;
			P2 += p2;
		}
		cout << "average payoff of slave: " << P1 / numberOfSamples << "\taverage payoff of tft: " << P2 / numberOfSamples << endl;
		relation = pair<int, int>(SLAVE, TFT);
		typeToPayoffsWithNoise[relation] = P1 / numberOfSamples;
		relation = pair<int, int>(TFT, SLAVE);
		typeToPayoffsWithNoise[relation] = P2 / numberOfSamples;


		//calculate master-TFT (master-slave) competition with noise
		P1 = 0, P2 = 0;
		for (int i = 0; i < numberOfSamples; i++) {
			vector<int> s1, s2;
			double p1 = 0, p2 = 0;
			master m = master();
			tft t = tft();
			for (int j = 0; j < 100; j++) {
				int c1 = m.getChoice();
				int c2 = t.getChoice();
				s1.push_back(c1);
				s2.push_back(c2);

				//judge the payoff
				if (c1 == COOPERATE && c2 == COOPERATE) {
					p1 += Reward;
					p2 += Reward;
				}
				else if (c1 == COOPERATE && c2 == DEFECT) {
					p1 += Sucker;
					p2 += Temptation;
				}
				else if (c1 == DEFECT && c2 == COOPERATE) {
					p1 += Temptation;
					p2 += Sucker;
				}
				else {
					p1 += Punishment;
					p2 += Punishment; 
				}

				//feedback the choice of the opponent
				m.moveOfOpponent(c2);
				t.moveOfOpponent(c1);
			}
			/*cout << "sequence of master: ";
			for (auto& item : s1) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}

			}
			cout << "payoff of master : " << p1 << endl;
			cout << "sequence of tft: ";
			for (auto& item : s2) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}
			}
			cout << "payoff of tft : " << p2 << endl << endl;*/
			P1 += p1;
			P2 += p2;
		}
		cout << "average payoff of master: " << P1 / numberOfSamples << "\taverage payoff of tft: " << P2 / numberOfSamples << endl;
		relation = pair<int, int>(MASTER, TFT);
		typeToPayoffsWithNoise[relation] = P1 / numberOfSamples;
		relation = pair<int, int>(TFT, MASTER);
		typeToPayoffsWithNoise[relation] = P2 / numberOfSamples;

		//calculate TFT-TFT competition with noise
		P1 = 0, P2 = 0;
		for (int i = 0; i < numberOfSamples; i++) {
			vector<int> s1, s2;
			double p1 = 0, p2 = 0;
			tft t1 = tft();
			tft t2 = tft();
			for (int j = 0; j < 100; j++) {
				int c1 = t1.getChoice();
				int c2 = t2.getChoice();
				s1.push_back(c1);
				s2.push_back(c2);

				//judge the payoff
				if (c1 == COOPERATE && c2 == COOPERATE) {
					p1 += Reward;
					p2 += Reward;
				}
				else if (c1 == COOPERATE && c2 == DEFECT) {
					p1 += Sucker;
					p2 += Temptation;
				}
				else if (c1 == DEFECT && c2 == COOPERATE) {
					p1 += Temptation;
					p2 += Sucker;
				}
				else {
					p1 += Punishment;
					p2 += Punishment;
				}

				//feedback the choice of the opponent
				t1.moveOfOpponent(c2);
				t2.moveOfOpponent(c1);
			}
			/*cout << "sequence of master: ";
			for (auto& item : s1) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}

			}
			cout << "payoff of master : " << p1 << endl;
			cout << "sequence of tft: ";
			for (auto& item : s2) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}
			}
			cout << "payoff of tft : " << p2 << endl << endl;*/
			P1 += p1;
			P2 += p2;
		}
		cout << "average payoff of tft1: " << P1 / numberOfSamples << "\taverage payoff of tft2: " << P2 / numberOfSamples << endl;
		relation = pair<int, int>(TFT, TFT);
		typeToPayoffsWithNoise[relation] = P1 / numberOfSamples;


		//calculate slave-GTFT competition with noise
		P1 = 0, P2 = 0;
		for (int i = 0; i < numberOfSamples; i++) {
			vector<int> s1, s2;
			double p1 = 0, p2 = 0;
			slave s = slave();
			gtft t = gtft();
			for (int j = 0; j < 100; j++) {
				int c1 = s.getChoice();
				int c2 = t.getChoice();
				s1.push_back(c1);
				s2.push_back(c2);

				//judge the payoff
				if (c1 == COOPERATE && c2 == COOPERATE) {
					p1 += Reward;
					p2 += Reward;
				}
				else if (c1 == COOPERATE && c2 == DEFECT) {
					p1 += Sucker;
					p2 += Temptation;
				}
				else if (c1 == DEFECT && c2 == COOPERATE) {
					p1 += Temptation;
					p2 += Sucker;
				}
				else {
					p1 += Punishment;
					p2 += Punishment;
				}

				//feedback the choice of the opponent
				s.moveOfOpponent(c2);
				t.moveOfOpponent(c1);
			}
			/*cout << "sequence of slave: ";
			for (auto& item : s1) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}

			}
			cout << "payoff of slave : " << p1 << endl;
			cout << "sequence of tft: ";
			for (auto& item : s2) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}
			}
			cout << "payoff of tft : " << p2 << endl << endl;*/
			P1 += p1;
			P2 += p2;
		}
		cout << "average payoff of slave: " << P1 / numberOfSamples << "\taverage payoff of gtft: " << P2 / numberOfSamples << endl;
		relation = pair<int, int>(SLAVE, GTFT);
		typeToPayoffsWithNoise[relation] = P1 / numberOfSamples;
		relation = pair<int, int>(GTFT, SLAVE);
		typeToPayoffsWithNoise[relation] = P2 / numberOfSamples;


		//calculate master-GTFT competition with noise
		P1 = 0, P2 = 0;
		for (int i = 0; i < numberOfSamples; i++) {
			vector<int> s1, s2;
			double p1 = 0, p2 = 0;
			master m = master();
			gtft t = gtft();
			for (int j = 0; j < 100; j++) {
				int c1 = m.getChoice();
				int c2 = t.getChoice();
				s1.push_back(c1);
				s2.push_back(c2);

				//judge the payoff
				if (c1 == COOPERATE && c2 == COOPERATE) {
					p1 += Reward;
					p2 += Reward;
				}
				else if (c1 == COOPERATE && c2 == DEFECT) {
					p1 += Sucker;
					p2 += Temptation;
				}
				else if (c1 == DEFECT && c2 == COOPERATE) {
					p1 += Temptation;
					p2 += Sucker;
				}
				else {
					p1 += Punishment;
					p2 += Punishment;
				}

				//feedback the choice of the opponent
				m.moveOfOpponent(c2);
				t.moveOfOpponent(c1);
			}
			/*cout << "sequence of master: ";
			for (auto& item : s1) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}

			}
			cout << "payoff of master : " << p1 << endl;
			cout << "sequence of tft: ";
			for (auto& item : s2) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}
			}
			cout << "payoff of tft : " << p2 << endl << endl;*/
			P1 += p1;
			P2 += p2;
		}
		cout << "average payoff of master: " << P1 / numberOfSamples << "\taverage payoff of gtft: " << P2 / numberOfSamples << endl;
		relation = pair<int, int>(MASTER, GTFT);
		typeToPayoffsWithNoise[relation] = P1 / numberOfSamples;
		relation = pair<int, int>(GTFT, MASTER);
		typeToPayoffsWithNoise[relation] = P2 / numberOfSamples;

		//calculate GTFT-GTFT competition with noise
		P1 = 0, P2 = 0;
		for (int i = 0; i < numberOfSamples; i++) {
			vector<int> s1, s2;
			double p1 = 0, p2 = 0;
			gtft t1 = gtft();
			gtft t2 = gtft();
			for (int j = 0; j < 100; j++) {
				int c1 = t1.getChoice();
				int c2 = t2.getChoice();
				s1.push_back(c1);
				s2.push_back(c2);

				//judge the payoff
				if (c1 == COOPERATE && c2 == COOPERATE) {
					p1 += Reward;
					p2 += Reward;
				}
				else if (c1 == COOPERATE && c2 == DEFECT) {
					p1 += Sucker;
					p2 += Temptation;
				}
				else if (c1 == DEFECT && c2 == COOPERATE) {
					p1 += Temptation;
					p2 += Sucker;
				}
				else {
					p1 += Punishment;
					p2 += Punishment;
				}

				//feedback the choice of the opponent
				t1.moveOfOpponent(c2);
				t2.moveOfOpponent(c1);
			}
			cout << "sequence of gtft1: ";
			for (auto& item : s1) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}

			}
			cout << "payoff of gtft1: " << p1 << endl;
			cout << "sequence of gtft2: ";
			for (auto& item : s2) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}
			}
			cout << "payoff of gtft2: " << p2 << endl << endl;
			P1 += p1;
			P2 += p2;
		}
		cout << "average payoff of gtft1: " << P1 / numberOfSamples << "\taverage payoff of gtft2: " << P2 / numberOfSamples << endl;
		relation = pair<int, int>(GTFT, GTFT);
		typeToPayoffsWithNoise[relation] = P1 / numberOfSamples;

		//calculate slave-CTFT competition with noise
		P1 = 0, P2 = 0;
		for (int i = 0; i < numberOfSamples; i++) {
			vector<int> s1, s2;
			double p1 = 0, p2 = 0;
			slave s = slave();
			ctft t = ctft();
			for (int j = 0; j < 100; j++) {
				int c1 = s.getChoice();
				int c2 = t.getChoice();
				s1.push_back(c1);
				s2.push_back(c2);

				//judge the payoff
				if (c1 == COOPERATE && c2 == COOPERATE) {
					p1 += Reward;
					p2 += Reward;
				}
				else if (c1 == COOPERATE && c2 == DEFECT) {
					p1 += Sucker;
					p2 += Temptation;
				}
				else if (c1 == DEFECT && c2 == COOPERATE) {
					p1 += Temptation;
					p2 += Sucker;
				}
				else {
					p1 += Punishment;
					p2 += Punishment;
				}

				//feedback the choice of the opponent
				s.moveOfOpponent(c2);
				t.moveOfOpponent(c1);
			}
			/*cout << "sequence of slave: ";
			for (auto& item : s1) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}

			}
			cout << "payoff of slave : " << p1 << endl;
			cout << "sequence of tft: ";
			for (auto& item : s2) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}
			}
			cout << "payoff of tft : " << p2 << endl << endl;*/
			P1 += p1;
			P2 += p2;
		}
		cout << "average payoff of slave: " << P1 / numberOfSamples << "\taverage payoff of ctft: " << P2 / numberOfSamples << endl;
		relation = pair<int, int>(SLAVE, CTFT);
		typeToPayoffsWithNoise[relation] = P1 / numberOfSamples;
		relation = pair<int, int>(CTFT, SLAVE);
		typeToPayoffsWithNoise[relation] = P2 / numberOfSamples;


		//calculate master-CTFT competition with noise
		P1 = 0, P2 = 0;
		for (int i = 0; i < numberOfSamples; i++) {
			vector<int> s1, s2;
			double p1 = 0, p2 = 0;
			master m = master();
			ctft t = ctft();
			for (int j = 0; j < 100; j++) {
				int c1 = m.getChoice();
				int c2 = t.getChoice();
				s1.push_back(c1);
				s2.push_back(c2);

				//judge the payoff
				if (c1 == COOPERATE && c2 == COOPERATE) {
					p1 += Reward;
					p2 += Reward;
				}
				else if (c1 == COOPERATE && c2 == DEFECT) {
					p1 += Sucker;
					p2 += Temptation;
				}
				else if (c1 == DEFECT && c2 == COOPERATE) {
					p1 += Temptation;
					p2 += Sucker;
				}
				else {
					p1 += Punishment;
					p2 += Punishment;
				}

				//feedback the choice of the opponent
				m.moveOfOpponent(c2);
				t.moveOfOpponent(c1);
			}
			/*cout << "sequence of master: ";
			for (auto& item : s1) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}

			}
			cout << "payoff of master : " << p1 << endl;
			cout << "sequence of tft: ";
			for (auto& item : s2) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}
			}
			cout << "payoff of tft : " << p2 << endl << endl;*/
			P1 += p1;
			P2 += p2;
		}
		cout << "average payoff of master: " << P1 / numberOfSamples << "\taverage payoff of gtft: " << P2 / numberOfSamples << endl;
		relation = pair<int, int>(MASTER, CTFT);
		typeToPayoffsWithNoise[relation] = P1 / numberOfSamples;
		relation = pair<int, int>(CTFT, MASTER);
		typeToPayoffsWithNoise[relation] = P2 / numberOfSamples;

		//calculate CTFT-CTFT competition with noise
		P1 = 0, P2 = 0;
		for (int i = 0; i < numberOfSamples; i++) {
			vector<int> s1, s2;
			double p1 = 0, p2 = 0;
			ctft t1 = ctft();
			ctft t2 = ctft();
			for (int j = 0; j < 100; j++) {
				int c1 = t1.getChoice();
				int c2 = t2.getChoice();
				s1.push_back(c1);
				s2.push_back(c2);

				//judge the payoff
				if (c1 == COOPERATE && c2 == COOPERATE) {
					p1 += Reward;
					p2 += Reward;
				}
				else if (c1 == COOPERATE && c2 == DEFECT) {
					p1 += Sucker;
					p2 += Temptation;
				}
				else if (c1 == DEFECT && c2 == COOPERATE) {
					p1 += Temptation;
					p2 += Sucker;
				}
				else {
					p1 += Punishment;
					p2 += Punishment;
				}

				//feedback the choice of the opponent
				t1.moveOfOpponent(c2);
				t2.moveOfOpponent(c1);
			}
			cout << "sequence of ctft1: ";
			for (auto& item : s1) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}

			}
			cout << "payoff of ctft1: " << p1 << endl;
			cout << "sequence of ctft2: ";
			for (auto& item : s2) {
				if (item == COOPERATE) {
					cout << "C" << " ";
				}
				else {
					cout << "D" << " ";
				}
			}
			cout << "payoff of ctft2: " << p2 << endl << endl;
			P1 += p1;
			P2 += p2;
		}
		cout << "average payoff of ctft1: " << P1 / numberOfSamples << "\taverage payoff of ctft2: " << P2 / numberOfSamples << endl;
		relation = pair<int, int>(CTFT, CTFT);
		typeToPayoffsWithNoise[relation] = P1 / numberOfSamples;
		 
		return 0;
	}


	IPDcontroller(int strategy1, int strategy2, int Generations, double noiseRate) {
		
		this->noiseRate = noiseRate;
		noiseSamples = noiseRate * 100;

		//set the corresponding file as the output stream
	
		streambuf* originalBuf = cout.rdbuf(); //store original output stream
		ofstream of(outputFile_paths.at(noiseRate).at(strategy2), ios::out);
		streambuf* fileBuf = of.rdbuf();
		
		cout.rdbuf(fileBuf);

		
		this->numOfGeneration = Generations;
		this->strategy1 = strategy1;
		this->strategy2 = strategy2;
		cout << "number of generation: " << this->numOfGeneration << endl;
		COLORREF rgblist[20]{ BLACK, WHITE, LIGHTBLUE,LIGHTGREEN,LIGHTGRAY,RGB(0,255,0),RGB(106,90,205),RGB(0,255,255),
			RGB(255,246,112),RGB(0,100,0),RGB(255,255,0),RGB(255,193,37),RGB(205,92,92),RGB(255,215,0),RGB(255,0,0),RGB(155,48,255),
			RED,RED,RED,RGB(255,215,0) };
		
		//build vectors for entity storing
		for (int i = 0; i < lineNum; i++) {
			vector<strategy> row(200);
			entities.push_back(row);
		}

		//build a map for colors
		for (int i = 0; i < 20; i++) {
			colors.insert(pair<int, COLORREF>(i, rgblist[i]));
		}

		pair<int, int> relations[20] = { pair<int, int>(AIIC, AIID), pair<int, int>(AIID, AIIC), pair<int, int>(AIIC, AIIC), pair<int, int>(AIID, AIID),
						pair<int, int>(TFT, AIID), pair<int, int>(AIID, TFT), pair<int, int>(TFT, TFT), pair<int, int>(AIIC, TFT),
						pair<int, int>(TFT, AIIC), pair<int, int>(TFT, MASTER), pair<int, int>(TFT, SLAVE), pair<int, int>(MASTER, TFT),
						pair<int, int>(SLAVE, TFT), pair<int, int>(MASTER, SLAVE), pair<int, int>(SLAVE, MASTER), 
						pair<int, int>(MASTER, MASTER), pair<int, int>(SLAVE, SLAVE)};
		double payoffSequence[20] = { Sucker * 100, Temptation * 100, Reward * 100, Punishment * 100, 
						99 * Punishment + Sucker, Temptation + 99 * Punishment, Reward * 100, Reward * 100, Reward * 100, 
						2 * Reward + 2 * Sucker + Temptation + 95 * Punishment, 2 * Reward + 2 * Sucker + Temptation + 95 * Punishment, 
						2 * Reward + 2 * Temptation + Sucker + 95 * Punishment, 2 * Reward + 2 * Temptation + Sucker + 95 * Punishment, 
						3 * Reward + 3 * Punishment + Sucker + 93 * Temptation, 3 * Reward + 3 * Punishment + Temptation + 93 * Sucker,
						98 * Reward + 2 * Punishment, 50 * Reward + 50 * Punishment};
	

		//build a map for payoff
		for (int i = 0; i < 20; i++) {
			typeToPayoffs[relations[i]] = payoffSequence[i];
			//typeToPayoffs.insert(pair<pair<int, int>, double>(relations[i], payoffSequence[i]));
		}
		
		initializeAgents(strategy1, strategy2);
		
		//cout << "value of i is: " << i << endl;
		cout << "number of s1: " << numOfS1 << "\tnumber of s2: " << numOfS2 << endl;

		//calculate the avarage payoff with noise
		getAverageNoise();
		auto relation = pair<int, int>(MASTER, MASTER);
		cout << "average payoff of master-master with noise: " << typeToPayoffsWithNoise.at(relation) << endl; 
		cout << "average payoff of master-master: " << typeToPayoffs.at(relation) << endl << endl;

		relation = pair<int, int>(SLAVE, SLAVE);
		cout << "average payoff of slave-slave with noise:  " << typeToPayoffsWithNoise.at(relation) << endl;
		cout << "average payoff of slave-slave: " << typeToPayoffs.at(relation) << endl << endl;

		relation = pair<int, int>(MASTER, SLAVE);
		cout << "average payoff of master-slave with noise, master: " << typeToPayoffsWithNoise.at(relation) << endl;
		cout << "average payoff of master-slave, master: " << typeToPayoffs.at(relation) << endl << endl;

		relation = pair<int, int>(SLAVE, MASTER);
		cout << "average payoff of master-slave with noise, slave: " << typeToPayoffsWithNoise.at(relation) << endl;
		cout << "average payoff of master-slave, slave: " << typeToPayoffs.at(relation) << endl << endl;

		relation = pair<int, int>(MASTER, TFT);
		cout << "average payoff of master-tft with noise, master: " << typeToPayoffsWithNoise.at(relation) << endl;
		cout << "average payoff of master-tft, master: " << typeToPayoffs.at(relation) << endl << endl;

		relation = pair<int, int>(TFT, MASTER);
		cout << "average payoff of master-tft with noise, tft: " << typeToPayoffsWithNoise.at(relation) << endl;
		cout << "average payoff of master-tft, tft: " << typeToPayoffs.at(relation) << endl << endl;

		relation = pair<int, int>(SLAVE, TFT);
		cout << "average payoff of slave-tft with noise, slave: " << typeToPayoffsWithNoise.at(relation) << endl;
		cout << "average payoff of slave-tft, slave: " << typeToPayoffs.at(relation) << endl << endl;

		relation = pair<int, int>(TFT, SLAVE);
		cout << "average payoff of slave-tft with noise, tft: " << typeToPayoffsWithNoise.at(relation) << endl;
		cout << "average payoff of slave-tft, tft: " << typeToPayoffs.at(relation) << endl << endl;

		relation = pair<int, int>(TFT, TFT);
		cout << "average payoff of tft-tft with noise: " << typeToPayoffsWithNoise.at(relation) << endl;
		cout << "average payoff of tft-tft: " << typeToPayoffs.at(relation) << endl << endl;

		relation = pair<int, int>(GTFT, GTFT);
		cout << "average payoff of gtft-gtft with noise: " << typeToPayoffsWithNoise.at(relation) << endl << endl;

		relation = pair<int, int>(GTFT, MASTER);
		cout << "average payoff of gtft-master with noise: " << typeToPayoffsWithNoise.at(relation) << endl << endl;

		relation = pair<int, int>(MASTER, GTFT);
		cout << "average payoff of master-gtft with noise: " << typeToPayoffsWithNoise.at(relation) << endl << endl;

		relation = pair<int, int>(GTFT, SLAVE);
		cout << "average payoff of gtft-slave with noise: " << typeToPayoffsWithNoise.at(relation) << endl << endl;

		relation = pair<int, int>(SLAVE, GTFT);
		cout << "average payoff of slave-gtft with noise: " << typeToPayoffsWithNoise.at(relation) << endl << endl;

		relation = pair<int, int>(CTFT, CTFT);
		cout << "average payoff of ctft-ctft with noise: " << typeToPayoffsWithNoise.at(relation) << endl << endl;

		relation = pair<int, int>(CTFT, MASTER);
		cout << "average payoff of ctft-master with noise: " << typeToPayoffsWithNoise.at(relation) << endl << endl;

		relation = pair<int, int>(MASTER, CTFT);
		cout << "average payoff of master-ctft with noise: " << typeToPayoffsWithNoise.at(relation) << endl << endl;

		relation = pair<int, int>(CTFT, SLAVE);
		cout << "average payoff of ctft-slave with noise: " << typeToPayoffsWithNoise.at(relation) << endl << endl;

		relation = pair<int, int>(SLAVE, CTFT);
		cout << "average payoff of slave-ctft with noise: " << typeToPayoffsWithNoise.at(relation) << endl << endl;

		SpatialIPDWithNoise();

		//flush the buffer
		of.flush();
		of.close();

		//reset the stream
		cout.rdbuf(originalBuf);
	}
	
	void initializeAgents(int strategy1, int strategy2) {
		srand((unsigned)time(NULL));
		int rowIndex, lineIndex, i = 0;
		int restOfS1 = 20000, restOfS2 = 20000;
		int restLineNum = 200, restRowNum = 200;
		vector<pair<int, vector<int>>> restRowIndexes(200);
		for (int i = 0; i < 200; i++) {
			vector<int> restLineIndexes(200);
			for (int j = 0; j < 200; j++) {
				restLineIndexes[j] = j;
			}
			auto relation = pair<int, vector<int>>(i, restLineIndexes);
			restRowIndexes[i] = relation;
		}

		switch(strategy1){
			case AIIC: {
				for (int i = 0; i < 20000; i++) {
					rowIndex = rand() % restRowIndexes.size();
					aiic s1 = aiic();
					//auto relation = restRowIndexes[rowIndex];
					int actualRowNum = restRowIndexes[rowIndex].first;
					lineIndex = rand() % restRowIndexes[rowIndex].second.size();
					entities[actualRowNum][restRowIndexes[rowIndex].second[lineIndex]] = s1;
					numOfAIIC += 1;
					auto iter = restRowIndexes[rowIndex].second.erase(restRowIndexes[rowIndex].second.begin() + lineIndex);
					if (restRowIndexes[rowIndex].second.size() == 0) {
						auto iter = restRowIndexes.erase(restRowIndexes.begin() + rowIndex);
					}
				}
				break;
			}
			case AIID: {
				for (int i = 0; i < 20000; i++) {
					rowIndex = rand() % restRowIndexes.size();
					aiid s1 = aiid();
					//auto relation = restRowIndexes[rowIndex];
					int actualRowNum = restRowIndexes[rowIndex].first;
					lineIndex = rand() % restRowIndexes[rowIndex].second.size();
					entities[actualRowNum][restRowIndexes[rowIndex].second[lineIndex]] = s1;
					numOfAIID += 1;
					auto iter = restRowIndexes[rowIndex].second.erase(restRowIndexes[rowIndex].second.begin() + lineIndex);
					if (restRowIndexes[rowIndex].second.size() == 0) {
						auto iter = restRowIndexes.erase(restRowIndexes.begin() + rowIndex);
					}
				}
				break;
			}
			case TFT:  {
				for (int i = 0; i < 20000; i++) {
					rowIndex = rand() % restRowIndexes.size();
					tft s1 = tft();
					//auto relation = restRowIndexes[rowIndex];
					int actualRowNum = restRowIndexes[rowIndex].first;
					lineIndex = rand() % restRowIndexes[rowIndex].second.size();
					entities[actualRowNum][restRowIndexes[rowIndex].second[lineIndex]] = s1;
					numOfTFT += 1;
					auto iter = restRowIndexes[rowIndex].second.erase(restRowIndexes[rowIndex].second.begin() + lineIndex);
					if (restRowIndexes[rowIndex].second.size() == 0) {
						auto iter = restRowIndexes.erase(restRowIndexes.begin() + rowIndex);
					}
				}
				break;
			}
			case GTFT: {
				for (int i = 0; i < 20000; i++) {
					rowIndex = rand() % restRowIndexes.size();
					gtft s1 = gtft();
					//auto relation = restRowIndexes[rowIndex];
					int actualRowNum = restRowIndexes[rowIndex].first;
					lineIndex = rand() % restRowIndexes[rowIndex].second.size();
					entities[actualRowNum][restRowIndexes[rowIndex].second[lineIndex]] = s1;
					numOfGTFT += 1;
					auto iter = restRowIndexes[rowIndex].second.erase(restRowIndexes[rowIndex].second.begin() + lineIndex);
					if (restRowIndexes[rowIndex].second.size() == 0) {
						auto iter = restRowIndexes.erase(restRowIndexes.begin() + rowIndex);
					}
				}
				break;
			}
			case CTFT: {
				for (int i = 0; i < 20000; i++) {
					rowIndex = rand() % restRowIndexes.size();
					ctft s1 = ctft();
					//auto relation = restRowIndexes[rowIndex];
					int actualRowNum = restRowIndexes[rowIndex].first;
					lineIndex = rand() % restRowIndexes[rowIndex].second.size();
					entities[actualRowNum][restRowIndexes[rowIndex].second[lineIndex]] = s1;
					numOfCTFT += 1;
					auto iter = restRowIndexes[rowIndex].second.erase(restRowIndexes[rowIndex].second.begin() + lineIndex);
					if (restRowIndexes[rowIndex].second.size() == 0) {
						auto iter = restRowIndexes.erase(restRowIndexes.begin() + rowIndex);
					}
				}
				break;
			}
			case CSMSM:{
				for (int i = 0; i < 20000; i++) {
					rowIndex = rand() % restRowIndexes.size();
					int sign = rand() % 7;
					csmsm s1;
					if (sign == 0) {
						s1 = master();
					}
					else {
						s1 = slave();
					}
					
					//auto relation = restRowIndexes[rowIndex];
					int actualRowNum = restRowIndexes[rowIndex].first;
					lineIndex = rand() % restRowIndexes[rowIndex].second.size();
					entities[actualRowNum][restRowIndexes[rowIndex].second[lineIndex]] = s1;
					numOfCS += 1;
					auto iter = restRowIndexes[rowIndex].second.erase(restRowIndexes[rowIndex].second.begin() + lineIndex);
					if (restRowIndexes[rowIndex].second.size() == 0) {
						auto iter = restRowIndexes.erase(restRowIndexes.begin() + rowIndex);
					}
				}
				break;
			}
		}

		//distribute strategy2
		switch (strategy2) {
			case AIIC: {
				for (int i = 0; i < 20000; i++) {
					rowIndex = rand() % restRowIndexes.size();
					aiic s2 = aiic();
					//auto relation = restRowIndexes[rowIndex];
					int actualRowNum = restRowIndexes[rowIndex].first;
					lineIndex = rand() % restRowIndexes[rowIndex].second.size();
					entities[actualRowNum][restRowIndexes[rowIndex].second[lineIndex]] = s2;
					numOfAIIC += 1;
					auto iter = restRowIndexes[rowIndex].second.erase(restRowIndexes[rowIndex].second.begin() + lineIndex);
					if (restRowIndexes[rowIndex].second.size() == 0) {
						auto iter = restRowIndexes.erase(restRowIndexes.begin() + rowIndex);
					}
				}
				break;
			}
			case AIID: {
				for (int i = 0; i < 20000; i++) {
					rowIndex = rand() % restRowIndexes.size();
					aiid s2 = aiid();
					//auto relation = restRowIndexes[rowIndex];
					int actualRowNum = restRowIndexes[rowIndex].first;
					lineIndex = rand() % restRowIndexes[rowIndex].second.size();
					entities[actualRowNum][restRowIndexes[rowIndex].second[lineIndex]] = s2;
					numOfAIID += 1;
					auto iter = restRowIndexes[rowIndex].second.erase(restRowIndexes[rowIndex].second.begin() + lineIndex);
					if (restRowIndexes[rowIndex].second.size() == 0) {
						auto iter = restRowIndexes.erase(restRowIndexes.begin() + rowIndex);
					}
				}
				break;
			}
			case TFT: {
				for (int i = 0; i < 20000; i++) {
					rowIndex = rand() % restRowIndexes.size();
					tft s2 = tft();
					//auto relation = restRowIndexes[rowIndex];
					int actualRowNum = restRowIndexes[rowIndex].first;
					lineIndex = rand() % restRowIndexes[rowIndex].second.size();
					entities[actualRowNum][restRowIndexes[rowIndex].second[lineIndex]] = s2;
					numOfTFT += 1;
					auto iter = restRowIndexes[rowIndex].second.erase(restRowIndexes[rowIndex].second.begin() + lineIndex);
					if (restRowIndexes[rowIndex].second.size() == 0) {
						auto iter = restRowIndexes.erase(restRowIndexes.begin() + rowIndex);
					}
				}
				break;
			}
			case GTFT: {
				for (int i = 0; i < 20000; i++) {
					rowIndex = rand() % restRowIndexes.size();
					gtft s2 = gtft();
					//auto relation = restRowIndexes[rowIndex];
					int actualRowNum = restRowIndexes[rowIndex].first;
					lineIndex = rand() % restRowIndexes[rowIndex].second.size();
					entities[actualRowNum][restRowIndexes[rowIndex].second[lineIndex]] = s2;
					numOfGTFT += 1;
					auto iter = restRowIndexes[rowIndex].second.erase(restRowIndexes[rowIndex].second.begin() + lineIndex);
					if (restRowIndexes[rowIndex].second.size() == 0) {
						auto iter = restRowIndexes.erase(restRowIndexes.begin() + rowIndex);
					}
				}
				break;
			}
			case CTFT: {
				for (int i = 0; i < 20000; i++) {
					rowIndex = rand() % restRowIndexes.size();
					ctft s2 = ctft();
					//auto relation = restRowIndexes[rowIndex];
					int actualRowNum = restRowIndexes[rowIndex].first;
					lineIndex = rand() % restRowIndexes[rowIndex].second.size();
					entities[actualRowNum][restRowIndexes[rowIndex].second[lineIndex]] = s2;
					numOfCTFT += 1;
					auto iter = restRowIndexes[rowIndex].second.erase(restRowIndexes[rowIndex].second.begin() + lineIndex);
					if (restRowIndexes[rowIndex].second.size() == 0) {
						auto iter = restRowIndexes.erase(restRowIndexes.begin() + rowIndex);
					}
				}
				break;
			}
			case CSMSM: {
				for (int i = 0; i < 20000; i++) {
					rowIndex = rand() % restRowIndexes.size();
					int sign = rand() % 7;
					csmsm s2;
					if (sign <= 1) {
						s2 = master();
					}
					else {
						s2 = slave();
					}

					//auto relation = restRowIndexes[rowIndex];
					int actualRowNum = restRowIndexes[rowIndex].first;
					lineIndex = rand() % restRowIndexes[rowIndex].second.size();
					entities[actualRowNum][restRowIndexes[rowIndex].second[lineIndex]] = s2;
					numOfCS += 1;
					auto iter = restRowIndexes[rowIndex].second.erase(restRowIndexes[rowIndex].second.begin() + lineIndex);
					if (restRowIndexes[rowIndex].second.size() == 0) {
						auto iter = restRowIndexes.erase(restRowIndexes.begin() + rowIndex);
					}
				}
				break;
			}
		}
		

		/*for (int i = 0; i < 20000; i++) {
			rowIndex = rand() % restRowIndexes.size();
			aiid s2 = aiid();
			//auto relation = restRowIndexes[rowIndex];
			int actualRowNum = restRowIndexes[rowIndex].first;
			lineIndex = rand() % restRowIndexes[rowIndex].second.size();
			entities[actualRowNum][restRowIndexes[rowIndex].second[lineIndex]] = s2;
			auto iter = restRowIndexes[rowIndex].second.erase(restRowIndexes[rowIndex].second.begin() + lineIndex);
			if (restRowIndexes[rowIndex].second.size() == 0) {
				auto iter = restRowIndexes.erase(restRowIndexes.begin() + rowIndex);
			}
		}*/
	}

	void competition(strategy &entity, vector<strategy> &neighbors, int lineIndex, int rowIndex) {
		for (auto& neighbor : neighbors) {
			auto relation = pair<int, int>(entity.getType(), neighbor.getType());
			auto it = typeToPayoffs.find(relation);
			if (it == typeToPayoffs.end()) {
				cout << "relation not found! that is: <" << entity.getType() << "," << neighbor.getType() << ">" << endl;
			}
			auto pff = typeToPayoffs.at(pair<int, int>(entity.getType(), neighbor.getType()));
			entity.addPayoff(pff);
		}
		entity.getAveragePayoff(neighbors.size());
		
	}

	void SpatialIPD() {
		visualize(0);
		//for each generation
		for (int i = 0; i < numOfGeneration; i++) {
			//clear initial payoff, convert about 70% of csmsm agents to master
			for (auto& row : entities) {
				for (auto& entity : row) {
					entity.resetPayoff();
					if (strategy1 == CSMSM || strategy2 == CSMSM) {
						if (entity.getType() == MASTER || entity.getType() == SLAVE) {
							entity.changeToMaster();
							int ifChange = rand() % 10;
							if (ifChange < 7) {
								entity.changeToSlave();
							}
						}
					}
				}
			}
			

			//consider each entity, let them compete 
			int rowIndex = 0, lineIndex = 0, numOfCorner = 0, numOfBorder = 0, numInMiddle = 0;
			for (auto& row : entities) {
				for (auto& entity : row) {
					//case that the entity is on the corner (have 3 neighbors)
					if (rowIndex == 0 && lineIndex == 0 ){
						//entity on the top left corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//compete
						competition(entity, neighbors, lineIndex, rowIndex);
					
					}
					else if (rowIndex == rowNum - 1 && lineIndex == lineNum - 1) {
						//entity on the bottom right corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));

						//compete
						competition(entity, neighbors, lineIndex, rowIndex);
					}
					else if (rowIndex == 0 && lineIndex == lineNum - 1) {
						//entity on the top right corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));

						//compete
						competition(entity, neighbors, lineIndex, rowIndex);
					}
					else if (rowIndex == rowNum - 1 && lineIndex == 0) {
						//entity on the bottom left corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));

						//compete
						competition(entity, neighbors, lineIndex, rowIndex);
					}
					else if (rowIndex == 0){//case that the entity is on the border (have 5 neighbors), case on the top border
						
						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//compete
						competition(entity, neighbors, lineIndex, rowIndex);
					}
					else if (lineIndex == 0) {//case on the left border
						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//compete
						competition(entity, neighbors, lineIndex, rowIndex);
					}
					else if (rowIndex == rowNum - 1) {//case on the bottom border
						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));

						//compete
						competition(entity, neighbors, lineIndex, rowIndex);
					}
					else if (lineIndex == lineNum - 1) {//case on the right border
						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));

						//compete
						competition(entity, neighbors, lineIndex, rowIndex);
					}
					else {//case that the entity is in the middle (have 8 neighbors)
						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));
						
						//compete
						competition(entity, neighbors, lineIndex, rowIndex);
						numInMiddle++;
					}
					lineIndex++;
				}
				rowIndex++;
				lineIndex = 0;
			}

			//examine each agent to for possible, implement evolution
			rowIndex = 0, lineIndex = 0;
			for (auto& row : entities) {
				for (auto& entity : row) {
					//case that the entity is on the corner (have 3 neighbors)
					if (rowIndex == 0 && lineIndex == 0) {
				
						//entity on the top left corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (rowIndex == rowNum - 1 && lineIndex == lineNum - 1) {
						//entity on the bottom right corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (rowIndex == 0 && lineIndex == lineNum - 1) {
						//entity on the top right corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (rowIndex == rowNum - 1 && lineIndex == 0) {
						//entity on the bottom left corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (rowIndex == 0) {//case that the entity is on the border (have 5 neighbors), case on the top border

						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (lineIndex == 0) {//case on the left border
						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (rowIndex == rowNum - 1) {//case on the bottom border
						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (lineIndex == lineNum - 1) {//case on the right border
						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else {//case that the entity is in the middle (have 8 neighbors)
						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
						numInMiddle++;
					}
					lineIndex++;
				}
				rowIndex++;
				lineIndex = 0;
			}


			if (strategy1 == AIIC || strategy2 == AIIC) {
				cout << "number of AIIC agents: " << numOfAIIC << "\t";
			}
			if (strategy1 == AIID || strategy2 == AIID) {
				cout << "number of AIID agents: " << numOfAIID << "\t";
			}
			if (strategy1 == TFT || strategy2 == TFT) {
				cout << "number of TFT agents: " << numOfTFT << "\t";
			}
			if (strategy1 == CSMSM || strategy2 == CSMSM){
				cout << "number of CSMSM agents: " << numOfCS << "\t";
			}
			cout << endl;
			
			
			visualize(i+1);
			
		}
		
	}

	void competitionWithNoise(strategy& entity, vector<strategy>& neighbors, int lineIndex, int rowIndex) {
		for (auto& neighbor : neighbors) {
			auto relation = pair<int, int>(entity.getType(), neighbor.getType());
			auto it = typeToPayoffsWithNoise.find(relation);
			if (it == typeToPayoffsWithNoise.end()) {
				cout << "relation not found! that is: <" << entity.getType() << "," << neighbor.getType() << ">" << endl;
			}
			auto pff = typeToPayoffsWithNoise.at(pair<int, int>(entity.getType(), neighbor.getType()));
			entity.addPayoff(pff);
		}
		entity.getAveragePayoff(neighbors.size());
	}

	void SpatialIPDWithNoise() {
		cout << "system noise rate: " << noiseRate << endl;

		visualizeWithNoise(0, strategy2);
		//for each generation
		for (int i = 0; i < numOfGeneration; i++) {
			
			//clear initial payoff, convert about 70% of csmsm agents to master
			for (auto& row : entities) {
				for (auto& entity : row) {
					entity.resetPayoff();
					if (strategy1 == CSMSM || strategy2 == CSMSM) {
						if (entity.getType() == MASTER || entity.getType() == SLAVE) {
							entity.changeToMaster();
							int ifChange = rand() % 10;
							if (ifChange < 7) {
								entity.changeToSlave();
							}
						}
					}
				}
			}


			//consider each entity, let them compete 
			int rowIndex = 0, lineIndex = 0, numOfCorner = 0, numOfBorder = 0, numInMiddle = 0;
			for (auto& row : entities) {
				for (auto& entity : row) {
					//case that the entity is on the corner (have 3 neighbors)
					if (rowIndex == 0 && lineIndex == 0) {
						//entity on the top left corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//compete
						competitionWithNoise(entity, neighbors, lineIndex, rowIndex);

					}
					else if (rowIndex == rowNum - 1 && lineIndex == lineNum - 1) {
						//entity on the bottom right corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));

						//compete
						competitionWithNoise(entity, neighbors, lineIndex, rowIndex);
					}
					else if (rowIndex == 0 && lineIndex == lineNum - 1) {
						//entity on the top right corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));

						//compete
						competitionWithNoise(entity, neighbors, lineIndex, rowIndex);
					}
					else if (rowIndex == rowNum - 1 && lineIndex == 0) {
						//entity on the bottom left corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));

						//compete
						competitionWithNoise(entity, neighbors, lineIndex, rowIndex);
					}
					else if (rowIndex == 0) {//case that the entity is on the border (have 5 neighbors), case on the top border

						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//compete
						competitionWithNoise(entity, neighbors, lineIndex, rowIndex);
					}
					else if (lineIndex == 0) {//case on the left border
						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//compete
						competitionWithNoise(entity, neighbors, lineIndex, rowIndex);
					}
					else if (rowIndex == rowNum - 1) {//case on the bottom border
						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));

						//compete
						competitionWithNoise(entity, neighbors, lineIndex, rowIndex);
					}
					else if (lineIndex == lineNum - 1) {//case on the right border
						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));

						//compete
						competitionWithNoise(entity, neighbors, lineIndex, rowIndex);
					}
					else {//case that the entity is in the middle (have 8 neighbors)
						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//compete
						competitionWithNoise(entity, neighbors, lineIndex, rowIndex);
						numInMiddle++;
					}
					lineIndex++;
				}
				rowIndex++;
				lineIndex = 0;
			}

			//examine each agent to for possible, implement evolution
			rowIndex = 0, lineIndex = 0;
			for (auto& row : entities) {
				for (auto& entity : row) {
					//case that the entity is on the corner (have 3 neighbors)
					if (rowIndex == 0 && lineIndex == 0) {

						//entity on the top left corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (rowIndex == rowNum - 1 && lineIndex == lineNum - 1) {
						//entity on the bottom right corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (rowIndex == 0 && lineIndex == lineNum - 1) {
						//entity on the top right corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (rowIndex == rowNum - 1 && lineIndex == 0) {
						//entity on the bottom left corner
						numOfCorner++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (rowIndex == 0) {//case that the entity is on the border (have 5 neighbors), case on the top border

						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (lineIndex == 0) {//case on the left border
						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (rowIndex == rowNum - 1) {//case on the bottom border
						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else if (lineIndex == lineNum - 1) {//case on the right border
						numOfBorder++;

						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
					}
					else {//case that the entity is in the middle (have 8 neighbors)
						//obtain all neighbors
						vector<strategy> neighbors;
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex - 1).at(lineIndex + 1));
						neighbors.push_back(row.at(lineIndex - 1));
						neighbors.push_back(row.at(lineIndex + 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex - 1));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex));
						neighbors.push_back(entities.at(rowIndex + 1).at(lineIndex + 1));

						//evolution
						IPDEvolution(entity, neighbors, rowIndex, lineIndex);
						numInMiddle++;
					}
					lineIndex++;
				}
				rowIndex++;
				lineIndex = 0;
			}


			if (strategy1 == AIIC || strategy2 == AIIC) {
				cout << "number of AIIC agents: " << numOfAIIC << "\t";
			}
			if (strategy1 == AIID || strategy2 == AIID) {
				cout << "number of AIID agents: " << numOfAIID << "\t";
			}
			if (strategy1 == TFT || strategy2 == TFT) {
				cout << "number of TFT agents: " << numOfTFT << "\t";
			}
			if (strategy1 == GTFT || strategy2 == GTFT) {
				cout << "number of GTFT agents: " << numOfGTFT << "\t";
			}
			if (strategy1 == CTFT || strategy2 == CTFT) {
				cout << "number of CTFT agents: " << numOfCTFT << "\t";
			}
			if (strategy1 == CSMSM || strategy2 == CSMSM) {
				cout << "number of CSMSM agents: " << numOfCS << "\t";
			}
			cout << endl;


			visualizeWithNoise(i+1, strategy2);

			//end competition if complete annex
			if (numOfCS == 0 || numOfCS == 40000) {
				break;
			}

		}
	}

	void IPDEvolution(strategy& entity, vector<strategy>& neighbors, int rowIndex, int lineIndex) {
		strategy maxNeighbor;
		int i = 1;
		//find the neighbor with maximum payoff
		for (auto& neighbor : neighbors) {
			if (i == 1) {
				maxNeighbor = neighbor;
				i = 0;
				continue;
			}
			if (neighbor.getPayoff() > maxNeighbor.getPayoff()) {
				maxNeighbor = neighbor;
			}
		}
		//switch if there is a bigger neighbor
		if (entity.getPayoff() < maxNeighbor.getPayoff() && entity.getType() != maxNeighbor.getType()) {
			//count number 
			switch (entity.getType()) {
			case AIIC: {
				numOfAIIC -= 1;
				break;
			}
			case AIID: {
				numOfAIID -= 1;
				break;
			}
			case TFT: {
				numOfTFT -= 1;
				break;
			}
			case GTFT: {
				numOfGTFT -= 1;
				break;
			}
			case CTFT: {
				numOfCTFT -= 1;
				break;
			}
			case SLAVE: {
				numOfCS -= 1;
				break;
			}
			case MASTER: {
				numOfCS -= 1;
				break;
			}
			}


			//create a new instance of max neighbor
			switch (maxNeighbor.getType()) {
			case AIIC: {
				strategy newEntity = aiic();
				//put the new agent at the position
				entities[rowIndex][lineIndex] = newEntity;
				numOfAIIC += 1;
				break;
			}
			case AIID: {
				strategy newEntity = aiid();
				//put the new agent at the position
				entities[rowIndex][lineIndex] = newEntity;
				numOfAIID += 1;
				break;
			}
			case TFT: {
				strategy newEntity = tft();
				//put the new agent at the position
				entities[rowIndex][lineIndex] = newEntity;
				numOfTFT += 1;
				break;
			}
			case GTFT: {
				strategy newEntity = gtft();
				//put the new agent at the position
				entities[rowIndex][lineIndex] = newEntity;
				numOfGTFT += 1;
				break;
			}
			case CTFT: {
				strategy newEntity = ctft();
				//put the new agent at the position
				entities[rowIndex][lineIndex] = newEntity;
				numOfCTFT += 1;
				break;
			}
			case SLAVE: {
				strategy newEntity = master();
				//put the new agent at the position
				entities[rowIndex][lineIndex] = newEntity;
				numOfCS += 1;
				break;
			}
			case MASTER: {
				strategy newEntity = master();
				//put the new agent at the position
				entities[rowIndex][lineIndex] = newEntity;
				numOfCS += 1;
				break;
			}
			}
			//cout << "entity occupied!" << endl;

		}
	}

	void printInfo() const {
		/*cout << problem->problem_id << endl;
		cout << " obj=   " << objective << "   " << abs << endl;
		for (auto& bin : *bins) {
			bin->printBin();
		}*/
		cout << "helloworld" << endl;
		cout << "helloworld" << endl;
		cout << "helloworld3333333fffff" << endl;
	}

	//set the output stream to enable printing more conveniently
	/*void writeIntoFile() {
		streambuf* originalBuf = cout.rdbuf();
		ofstream of(output_file, ios::out);
		streambuf* fileBuf = of.rdbuf();

		//set the file as the output stream
		cout.rdbuf(fileBuf);
		//print necessary information
		printInfo();

		//flush the buffer
		of.flush();
		of.close();

		//reset the stream
		cout.rdbuf(originalBuf);
	}*/

	void visualize(int generationNow) {
		cout << "generationNow is: " << generationNow << endl;
		wchar_t path[100];
		initgraph(200, 200);     /*初始化图形界面*/
		setbkcolor(WHITE);
		int i = 0, j = 0;
		for (auto& row : entities) {
			for (auto& entity : row) {
				//draw a pixel using a color according to the strategy type
				putpixel(i, j++, colors.at(entity.getType()));
			}
			j = 0;
			i++;
		}
		
		if(generationNow == 0){
			saveimage(_T("C:\\Users\\ASUS\\Desktop\\images\\image.png"));
		}
		else {
			swprintf_s(path, L"C:\\Users\\ASUS\\Desktop\\images\\image%d.png", generationNow);
			saveimage(path);
		}
		//saveimage(_T("C:\\Users\\ASUS\\Desktop\\images\\image.png"));
		closegraph();
		
	}

	void visualizeWithNoise(int generationNow, int strategyNum) {
		cout << "generationNow is: " << generationNow << endl;
		wchar_t path[100];
		initgraph(200, 200);     /*initialize graph*/
		setbkcolor(WHITE);
		int i = 0, j = 0;
		for (auto& row : entities) {
			for (auto& entity : row) {
				//draw a pixel using a color according to the strategy type
				putpixel(i, j++, colors.at(entity.getType()));
			}
			j = 0;
			i++;
		}
		if (generationNow == 0) {
			saveimage(paths.at(noiseRate).at(strategyNum).at(0));
		}
		else {
			swprintf_s(path, paths.at(noiseRate).at(strategyNum).at(1), generationNow);
			saveimage(path);
		}
		
		
		/*if (noiseRate == 0) {
			if (generationNow == 0) {
				saveimage(paths_0percent_noise.at(strategyNum).at(0));
			}
			else {
				swprintf_s(path, paths_0percent_noise.at(strategyNum).at(1), generationNow);
				saveimage(path);
			}
		}else if (noiseRate == 0.01) {
			if (generationNow == 0) {
				saveimage(paths_1percent_noise.at(strategyNum).at(0));
			}
			else {
				swprintf_s(path, paths_1percent_noise.at(strategyNum).at(1), generationNow);
				saveimage(path);
			}
		}
		else if (noiseRate == 0.03) {
			if (generationNow == 0) {
				saveimage(paths_3percent_noise.at(strategyNum).at(0));
			}
			else {
				swprintf_s(path, paths_3percent_noise.at(strategyNum).at(1), generationNow);
				saveimage(path);
			}
		}
		else if (noiseRate == 0.05) {
			if (generationNow == 0) {
				saveimage(paths_5percent_noise.at(strategyNum).at(0));
			}
			else {
				swprintf_s(path, paths_5percent_noise.at(strategyNum).at(1), generationNow);
				saveimage(path);
			}
		}
		else if (noiseRate == 0.1) {
			if (generationNow == 0) {
				saveimage(paths_10percent_noise.at(strategyNum).at(0));
			}
			else {
				swprintf_s(path, paths_10percent_noise.at(strategyNum).at(1), generationNow);
				saveimage(path);
			}
		}*/

		
		
		closegraph();

	}

};



class IPDWithRate {
public:
	IPDWithRate(int numOfGenerations) {
		//three competitions under 0 percent noise
		/*IPDcontroller p0_tft = IPDcontroller(CSMSM, TFT, numOfGenerations, 0.0);
		IPDcontroller p0_gtft = IPDcontroller(CSMSM, GTFT, numOfGenerations, 0.0);
		IPDcontroller p0_ctft = IPDcontroller(CSMSM, CTFT, numOfGenerations, 0.0);

		//three competitions under 1 percent noise
		IPDcontroller p1_tft = IPDcontroller(CSMSM, TFT, numOfGenerations, 0.01);
		IPDcontroller p1_gtft = IPDcontroller(CSMSM, GTFT, numOfGenerations, 0.01);
		IPDcontroller p1_ctft = IPDcontroller(CSMSM, CTFT, numOfGenerations, 0.01);

		//three competitions under 3 percent noise
		IPDcontroller p3_tft = IPDcontroller(CSMSM, TFT, numOfGenerations, 0.03);
		IPDcontroller p3_gtft = IPDcontroller(CSMSM, GTFT, numOfGenerations, 0.03);
		IPDcontroller p3_ctft = IPDcontroller(CSMSM, CTFT, numOfGenerations, 0.03);*/

		//three competitions under 5 percent noise
		IPDcontroller p5_tft = IPDcontroller(CSMSM, TFT, numOfGenerations, 0.05);
		IPDcontroller p5_gtft = IPDcontroller(CSMSM, GTFT, numOfGenerations, 0.05);
		IPDcontroller p5_ctft = IPDcontroller(CSMSM, CTFT, numOfGenerations, 0.05);

		//three competitions under 10 percent noise
		IPDcontroller p10_tft = IPDcontroller(CSMSM, TFT, numOfGenerations, 0.1);
		IPDcontroller p10_gtft = IPDcontroller(CSMSM, GTFT, numOfGenerations, 0.1);
		IPDcontroller p10_ctft = IPDcontroller(CSMSM, CTFT, numOfGenerations, 0.1);
	}
};

void buildPathMaps() {
	//set path maps for output files
	//set output paths for simulation with no noise
	string p0_tft_path = "zero_percent_noise\\imagesWithNoise_TFT\\tft_output.txt";
	string p0_gtft_path = "zero_percent_noise\\imagesWithNoise_GTFT\\gtft_output.txt";
	string p0_ctft_path = "zero_percent_noise\\imagesWithNoise_CTFT\\ctft_output.txt";
	
	outputPaths_0percent_noise[TFT] = p0_tft_path;
	outputPaths_0percent_noise[GTFT] = p0_gtft_path;
	outputPaths_0percent_noise[CTFT] = p0_ctft_path;
	outputFile_paths[0.00] = outputPaths_0percent_noise;

	//set output paths for simulation with 1% noise
	string p1_tft_path = "one_percent_noise\\imagesWithNoise_TFT\\tft_output.txt";
	string p1_gtft_path = "one_percent_noise\\imagesWithNoise_GTFT\\gtft_output.txt";
	string p1_ctft_path = "one_percent_noise\\imagesWithNoise_CTFT\\ctft_output.txt";

	outputPaths_1percent_noise[TFT] = p1_tft_path;
	outputPaths_1percent_noise[GTFT] = p1_gtft_path;
	outputPaths_1percent_noise[CTFT] = p1_ctft_path;
	outputFile_paths[0.01] = outputPaths_1percent_noise;

	//set output paths for simulation with 3% noise
	string p3_tft_path = "three_percent_noise\\imagesWithNoise_TFT\\tft_output.txt";
	string p3_gtft_path = "three_percent_noise\\imagesWithNoise_GTFT\\gtft_output.txt";
	string p3_ctft_path = "three_percent_noise\\imagesWithNoise_CTFT\\ctft_output.txt";

	outputPaths_3percent_noise[TFT] = p3_tft_path;
	outputPaths_3percent_noise[GTFT] = p3_gtft_path;
	outputPaths_3percent_noise[CTFT] = p3_ctft_path;
	outputFile_paths[0.03] = outputPaths_3percent_noise;

	//set output paths for simulation with 5% noise
	string p5_tft_path = "five_percent_noise\\imagesWithNoise_TFT\\tft_output.txt";
	string p5_gtft_path = "five_percent_noise\\imagesWithNoise_GTFT\\gtft_output.txt";
	string p5_ctft_path = "five_percent_noise\\imagesWithNoise_CTFT\\ctft_output.txt";

	outputPaths_5percent_noise[TFT] = p5_tft_path;
	outputPaths_5percent_noise[GTFT] = p5_gtft_path;
	outputPaths_5percent_noise[CTFT] = p5_ctft_path;
	outputFile_paths[0.05] = outputPaths_5percent_noise;

	//set output paths for simulation with 10% noise
	string p10_tft_path = "ten_percent_noise\\imagesWithNoise_TFT\\tft_output.txt";
	string p10_gtft_path = "ten_percent_noise\\imagesWithNoise_GTFT\\gtft_output.txt";
	string p10_ctft_path = "ten_percent_noise\\imagesWithNoise_CTFT\\ctft_output.txt";

	outputPaths_10percent_noise[TFT] = p10_tft_path;
	outputPaths_10percent_noise[GTFT] = p10_gtft_path;
	outputPaths_10percent_noise[CTFT] = p10_ctft_path;
	outputFile_paths[0.10] = outputPaths_10percent_noise;



	//build path maps for 0 percent noise simulation
	//paths for TFT
	vector<const wchar_t*> p0_tft_paths;
	auto p0_tft_path1 = _T("zero_percent_noise\\imagesWithNoise_TFT\\image.png");
	auto p0_tft_path2 = L"zero_percent_noise\\imagesWithNoise_TFT\\image%d.png";
	p0_tft_paths.push_back(p0_tft_path1);
	p0_tft_paths.push_back(p0_tft_path2);
	paths_0percent_noise[TFT] = p0_tft_paths;

	//paths for GTFT
	vector<const wchar_t*> p0_gtft_paths;
	auto p0_gtft_path1 = _T("zero_percent_noise\\imagesWithNoise_GTFT\\image.png");
	auto p0_gtft_path2 = L"zero_percent_noise\\imagesWithNoise_GTFT\\image%d.png";
	p0_gtft_paths.push_back(p0_gtft_path1);
	p0_gtft_paths.push_back(p0_gtft_path2);
	paths_0percent_noise[GTFT] = p0_gtft_paths;

	//paths for CTFT
	vector<const wchar_t*> p0_ctft_paths;
	auto p0_ctft_path1 = _T("zero_percent_noise\\imagesWithNoise_CTFT\\image.png");
	auto p0_ctft_path2 = L"zero_percent_noise\\imagesWithNoise_CTFT\\image%d.png";
	p0_ctft_paths.push_back(p0_ctft_path1);
	p0_ctft_paths.push_back(p0_ctft_path2);
	paths_0percent_noise[CTFT] = p0_ctft_paths;
	paths[0.0] = paths_0percent_noise;

	//build path maps for 1 percent noise simulation
	//paths for TFT
	vector<const wchar_t*> p1_tft_paths;
	auto p1_tft_path1 = _T("one_percent_noise\\imagesWithNoise_TFT\\image.png");
	auto p1_tft_path2 = L"one_percent_noise\\imagesWithNoise_TFT\\image%d.png";
	p1_tft_paths.push_back(p1_tft_path1);
	p1_tft_paths.push_back(p1_tft_path2);
	paths_1percent_noise[TFT] = p1_tft_paths;

	//paths for GTFT
	vector<const wchar_t*> p1_gtft_paths;
	auto p1_gtft_path1 = _T("one_percent_noise\\imagesWithNoise_GTFT\\image.png");
	auto p1_gtft_path2 = L"one_percent_noise\\imagesWithNoise_GTFT\\image%d.png";
	p1_gtft_paths.push_back(p1_gtft_path1);
	p1_gtft_paths.push_back(p1_gtft_path2);
	paths_1percent_noise[GTFT] = p1_gtft_paths;

	//paths for CTFT
	vector<const wchar_t*> p1_ctft_paths;
	auto p1_ctft_path1 = _T("one_percent_noise\\imagesWithNoise_CTFT\\image.png");
	auto p1_ctft_path2 = L"one_percent_noise\\imagesWithNoise_CTFT\\image%d.png";
	p1_ctft_paths.push_back(p1_ctft_path1);
	p1_ctft_paths.push_back(p1_ctft_path2);
	paths_1percent_noise[CTFT] = p1_ctft_paths;
	paths[0.01] = paths_1percent_noise;


	//build path maps for 3 percent noise simulation
	//paths for TFT
	vector<const wchar_t*> p3_tft_paths;
	auto p3_tft_path1 = _T("three_percent_noise\\imagesWithNoise_TFT\\image.png");
	auto p3_tft_path2 = L"three_percent_noise\\imagesWithNoise_TFT\\image%d.png";
	p3_tft_paths.push_back(p3_tft_path1);
	p3_tft_paths.push_back(p3_tft_path2);
	paths_3percent_noise[TFT] = p3_tft_paths;

	//paths for GTFT
	vector<const wchar_t*> p3_gtft_paths;
	auto p3_gtft_path1 = _T("three_percent_noise\\imagesWithNoise_GTFT\\image.png");
	auto p3_gtft_path2 = L"three_percent_noise\\imagesWithNoise_GTFT\\image%d.png";
	p3_gtft_paths.push_back(p3_gtft_path1);
	p3_gtft_paths.push_back(p3_gtft_path2);
	paths_3percent_noise[GTFT] = p3_gtft_paths;

	//paths for CTFT
	vector<const wchar_t*> p3_ctft_paths;
	auto p3_ctft_path1 = _T("three_percent_noise\\imagesWithNoise_CTFT\\image.png");
	auto p3_ctft_path2 = L"three_percent_noise\\imagesWithNoise_CTFT\\image%d.png";
	p3_ctft_paths.push_back(p3_ctft_path1);
	p3_ctft_paths.push_back(p3_ctft_path2);
	paths_3percent_noise[CTFT] = p3_ctft_paths;
	paths[0.03] = paths_3percent_noise;


	//build path maps for 5 percent noise simulation
	//paths for TFT
	vector<const wchar_t*> p5_tft_paths;
	auto p5_tft_path1 = _T("five_percent_noise\\imagesWithNoise_TFT\\image.png");
	auto p5_tft_path2 = L"five_percent_noise\\imagesWithNoise_TFT\\image%d.png";
	p5_tft_paths.push_back(p5_tft_path1);
	p5_tft_paths.push_back(p5_tft_path2);
	paths_5percent_noise[TFT] = p5_tft_paths;

	//paths for GTFT
	vector<const wchar_t*> p5_gtft_paths;
	auto p5_gtft_path1 = _T("five_percent_noise\\imagesWithNoise_GTFT\\image.png");
	auto p5_gtft_path2 = L"five_percent_noise\\imagesWithNoise_GTFT\\image%d.png";
	p5_gtft_paths.push_back(p5_gtft_path1);
	p5_gtft_paths.push_back(p5_gtft_path2);
	paths_5percent_noise[GTFT] = p5_gtft_paths;

	//paths for CTFT
	vector<const wchar_t*> p5_ctft_paths;
	auto p5_ctft_path1 = _T("five_percent_noise\\imagesWithNoise_CTFT\\image.png");
	auto p5_ctft_path2 = L"five_percent_noise\\imagesWithNoise_CTFT\\image%d.png";
	p5_ctft_paths.push_back(p5_ctft_path1);
	p5_ctft_paths.push_back(p5_ctft_path2);
	paths_5percent_noise[CTFT] = p5_ctft_paths;
	paths[0.05] = paths_5percent_noise;


	//build path maps for 10 percent noise simulation
	//paths for TFT
	vector<const wchar_t*> p10_tft_paths;
	auto p10_tft_path1 = _T("ten_percent_noise\\imagesWithNoise_TFT\\image.png");
	auto p10_tft_path2 = L"ten_percent_noise\\imagesWithNoise_TFT\\image%d.png";
	p10_tft_paths.push_back(p10_tft_path1);
	p10_tft_paths.push_back(p10_tft_path2);
	paths_10percent_noise[TFT] = p10_tft_paths;

	//paths for GTFT
	vector<const wchar_t*> p10_gtft_paths;
	auto p10_gtft_path1 = _T("ten_percent_noise\\imagesWithNoise_GTFT\\image.png");
	auto p10_gtft_path2 = L"ten_percent_noise\\imagesWithNoise_GTFT\\image%d.png";
	p10_gtft_paths.push_back(p10_gtft_path1);
	p10_gtft_paths.push_back(p10_gtft_path2);
	paths_10percent_noise[GTFT] = p10_gtft_paths;

	//paths for CTFT
	vector<const wchar_t*> p10_ctft_paths;
	auto p10_ctft_path1 = _T("ten_percent_noise\\imagesWithNoise_CTFT\\image.png");
	auto p10_ctft_path2 = L"ten_percent_noise\\imagesWithNoise_CTFT\\image%d.png";
	p10_ctft_paths.push_back(p10_ctft_path1);
	p10_ctft_paths.push_back(p10_ctft_path2);
	paths_10percent_noise[CTFT] = p10_ctft_paths;
	paths[0.10] = paths_10percent_noise;

	


}


int main()
{
	/*IPDcontroller con1 = IPDcontroller(CSMSM, TFT, 10, 0);
	IPDcontroller con2 = IPDcontroller(CSMSM, GTFT, 10, 0); 
	IPDcontroller con3 = IPDcontroller(CSMSM, CTFT, 10, 0.1);
	//IPDWithRate ipd1 = IPDWithRate(10, 0.1);*/
	
	buildPathMaps();

	IPDWithRate(300);
	
	
}

