#include <iostream>
#include <fstream>

using namespace std;

class Logger {
private:
	ofstream fout;
	
public:	
	void init(const string &path) {
		fout.open(path);
	}

	ios::fmtflags setf (ios::fmtflags fmtfl, ios::fmtflags mask){
		ios::fmtflags old;

		old = cout.setf(fmtfl, mask);

		if(fout.is_open())
			old = fout.setf(fmtfl, mask);

		return old;
	}
	
	template<class T> Logger& operator<<(T t){
		cout << t;

		if(fout.is_open())
			fout << t;
		
		return *this;
	}
	
	Logger& operator<<(ostream& (*f)(ostream& o)) {
		cout << f;
		
		if(fout.is_open())
			fout << f;
		
		return *this;
	}
	
	Logger& precision(int prec){
		cout.precision(prec);

		if(fout.is_open())
			fout.precision(prec);
		
		return *this;
	}
};

