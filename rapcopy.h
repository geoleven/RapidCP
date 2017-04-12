#ifndef RAPCOPY_H_
#define RAPCOPY_H_

#include <string>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

class rapcopy
{
	private:
		string originaldir;
		string destination;
		string parent_of_dest;
		int verbose;
		int linked;
		struct stat inbuf;
		struct stat otbuf;
		int cpnew();
		int cpexs();
		int cpfile(int, int);
		int cpdir(int);
		int delete_recursive(string);
		string relative_to_absolute(string);
		int root;
	public:
		int totalcountf;
		int copiedcountf;
		int totalcountd;
		int copiedcountd;
		int filesdeleted;
		int direcdeleted;
		int master_path;
		string program_execution_path;
		string parentstr(string);
		int cplot();
		double kb;
		rapcopy(string, string, int, int, string, int);
		~rapcopy();
};


#endif /* RAPCOPY_H_ */
