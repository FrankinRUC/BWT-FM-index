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

class ATCG : public fm_index<char, unsigned int, 5> {
private:
	using super = fm_index<char, unsigned int, 5>;
	static inline char clean(char c) {
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
public:
	static void create(const string &src, const string &dest) {
		ifstream in(src, ios::binary);
		in.seekg(0, in.end);
		auto str = new unsigned char[in.tellg()], ptr = str;
		char ch;
		in.seekg(0, in.beg);
		while(in.get(ch)) {
			if(clean(ch)) {
				*ptr++ = clean(ch);
			}
		}
		*ptr++ = 0;
		super::create(str, ptr, dest);
		delete[] str;
		return;
	}
	inline ATCG(const std::string &filename) : super(filename) {
	}
	template<typename BidirIt>
	inline vector<unsigned int> query(BidirIt first, BidirIt last, size_t times) {
		for(auto iter = first; iter != last; ++iter) {
			if(clean(*iter)) {
				*iter = clean(*iter);
			} else {
				cerr << "illegal input in query" << times << endl;
				return vector<unsigned int>();
			}
		}
		return super::query(first, last);
	}
};

int main(int argc, char *argv[]) {
	if(argc <= 1) {
		cerr << "wrong argument" << endl;
		return 0;
	}
	string filename(argv[1]);
	if(access((filename + ".idx").c_str(), R_OK)) {
		cerr << "creating FM_index ..." << endl;
		time_t stamp = time(NULL);
		ATCG::create(filename, filename + ".idx");
		cerr << "\tusing " << time(NULL) - stamp << " seconds" << endl;
	}
	cerr << "querying ..." << endl;
	clock_t total = clock();
	istream *search;
	if(argc == 3) {
		search = new ifstream(argv[2]);
		cerr << argv[2] << endl;
	} else {
		search = &cin;
	}
	ATCG idx(filename + ".idx");
	string str;
	size_t times = 0;
	total = clock() - total;
	while(*search >> str) {
		clock_t stamp = clock();
		for(auto index : idx.query(str.begin(), str.end(), ++times)) {
			cout << index << " ";
		}
		cout << endl;
		total += clock() - stamp;
	}
	cerr << "\tusing " << total * 1000.0 / CLOCKS_PER_SEC  << " ms in " << times << " queries" << endl;
	cerr << "\ttaking " << total * 1000.0 / CLOCKS_PER_SEC / times << " ms in each query on average" << endl;
	return 0;
}