#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "rapcopy.h"
#include <sys/times.h>
//#include <cstdio>
#include <unistd.h>
#include <iomanip>


using namespace std;

int main(int argc, char *argv[])
{
	double t1, t2, cpu_time;
	struct tms tb1, tb2;
	double ticspersec;
	ticspersec = (double) sysconf(_SC_CLK_TCK);
	t1 = (double) times(&tb1);

	int argcount = 1;
	int origindirpresent = 0;
	int destindirpresent = 0;
	int verbosemode = 0;
	int linkedmode = 0;
	int exitflag = 0;

	char v[3] = { '-', 'v', '\0'};
	char l[3] = { '-', 'l', '\0'};
	char origindirstr[100] = { '\0' };
	char destindirstr[100] = { '\0' };

	/*Diadikasia anagrwrisis odigiwn xristi*/
	while(argcount < argc && !(origindirpresent == 1 && destindirpresent == 1))
			{
				if (strcmp(argv[argcount], v)==0)
				{
					verbosemode = 1;
				}
				else if (strcmp(argv[argcount], l)==0)
				{
					linkedmode = 1;
				}
				else
				{
					strcpy(origindirstr, argv[argcount]);
					origindirpresent = 1;
					argcount = argcount + 1;
					if (argcount > argc)
					{
						cout << "Den dwsate monopati proeleusis i proorismou! To programma tha termatisei." << endl;
						exitflag = 1;
					}
					else
					{
						strcpy(destindirstr, argv[argcount]);
						destindirpresent = 1;
					}
				}
				argcount++;
			}

	if (exitflag == 1 || argc < 2)
	{
		cout << "Den swthike swsti entoli!" << endl;
		return 1;
	}

	string originaldir(origindirstr);
	string destination(destindirstr);

	double rythmos= 0;

	/*Kainourgio block oste oi ektypwseis tou distructor tis rapcopy na ginoun se swsti seira*/
	{
		rapcopy mycopy(originaldir, destination, verbosemode, linkedmode, string(argv[0]), 1);
		if (mycopy.cplot() == 1)
			cout << "Something wicked happened!" << endl;
		t2 = (double) times(&tb2);
		if((mycopy.totalcountd + mycopy.totalcountf) > 0)
			rythmos = (double)((t2-t1)/ticspersec)/(double)(mycopy.kb*1024);
		else
			rythmos = 0;
	}

	cpu_time = (double) ((tb2.tms_utime + tb2.tms_stime) - (tb1.tms_utime + tb1.tms_stime));
	cout << setprecision(10) << "Run time was " << (t2 - t1) / ticspersec << " sec (REAL time) although we used the CPU for " << cpu_time / ticspersec << " sec (CPU time).\n" <<  endl;
	cout << "O rythmos antigrafis twn arxeiwn itan: " << setprecision(10) << rythmos <<"b/s." << endl;
	cout << "Copy procedure completed. RapCp will exit now!" << endl;

	return 0;
}
