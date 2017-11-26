#include <iostream>
#include <mutex>
#include <chrono>
#include <string>
#include "BusyThreadFactorizator.h"

using namespace std;

int main(int argc, char** argv) {
	if (argc != 3) {
		cerr << "Incorrect format of the input data. The number of parameters: " << argc - 1 << std::endl;
		cerr << "Usage - <main_file_name> <Source_file_name> <Output_file_name>" << std::endl;
		exit(1); // Don't have any objects yet.
	}	 

	ConcurrentFactorizator factorizator(argc, argv, 10);
	if (factorizator.isCorrect()) {
		cout << "The factorizator is successfully initialized" << endl;
	} else {
		cout << "Failed to initialize the factorizator" << endl;
		return 0;
	}
	factorizator.start();

	string cmd;
	while (!factorizator.isDone()) {
		cin >> cmd;
		if (cmd.compare("abort") == 0 || cmd.compare("quit") == 0 || cmd.compare("exit") == 0) {
			factorizator.abort();
			break;
		}
		else if (cmd.compare("pause") == 0 || cmd.compare("suspend") == 0) {
			factorizator.suspend();
		}
		else if (cmd.compare("resume") == 0 || cmd.compare("continue") == 0) {
			factorizator.resume();
		}
	}
	factorizator.join();
	cout << "The program is ended" << endl;


	system("pause");
	return 0;
}