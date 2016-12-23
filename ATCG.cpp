#include <fcntl.h> 
#include <unistd.h>

#include <ctime>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "fmap.h"
#include "fm_index.h"

using namespace std;

inline unsigned char ATCG(char c) {
	switch (c) {
	case 'A':
		return 1;
	case 'T':
		return 2;
	case 'C':
		return 3;
	case 'G':
		return 4;
	default:
		return 0;
	}
}

int main(int argc, char *argv[]) {
	if(argc <= 1) {
		cerr << "wrong argument" << endl;
		return 0;
	}
	time_t stamp;
	string filename(argv[1]);
	if(access((filename + ".idx").c_str(), R_OK)) {
		if(access((filename + ".cln").c_str(), R_OK)) {
			cerr << "cleaning data ..." << endl;
			stamp = time(NULL);
			ifstream in(filename, ios::binary);
			ofstream out(filename + ".cln", ios::binary);
			char ch;
			while(in.get(ch)) {
				if(ATCG(ch)) {
					out.put(ATCG(ch));
				}
			}
			out.put(0);
			cerr << "\tusing " << time(NULL) - stamp << " seconds" << endl;
		}
		cerr << "creating FM_index ..." << endl;
		stamp = time(NULL);
		ifmap cln(filename + ".cln");
		fm_index<unsigned char, unsigned int, 5>::create(cln.begin(), cln.end(), filename + ".idx");
		cerr << "\tusing " << time(NULL) - stamp << " seconds" << endl;
	}
	cerr << "querying ..." << endl;
	fm_index<char, unsigned int, 5> idx(filename + ".idx");
	istream *search;
	if(argc == 3) {
		search = new ifstream(argv[2]);
	} else {
		search = &cin;
	}
	string str;
	size_t times = 0;
	clock_t total = 0;
	while(*search >> str) {
		++times;
		clock_t stamp = clock();
		bool flag = true;
		for(auto ptr = str.begin(); ptr != str.end(); ++ptr) {
			if(ATCG(*ptr)) {
				*ptr = ATCG(*ptr);
			} else {
				cerr << "illegal input in query" << times << endl;
				flag = false;
				break;
			}
		}
		if(flag) {
			for(auto index : idx.query(str)) {
				cout << index << " ";
			}
		}
		cout << endl;
		total += clock() - stamp;
	}
	cerr << "\tusing " << total * 1000.0 / CLOCKS_PER_SEC  << " ms in " << times << " queries" << endl;
	cerr << "\ttaking " << total * 1000.0 / CLOCKS_PER_SEC / times << " ms in each query on average" << endl;
	return 0;
}