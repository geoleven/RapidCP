#include "rapcopy.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <fcntl.h>
#include <iomanip>

#define bufs 1024


using namespace std;

rapcopy::rapcopy(string corig, string cdest, int cver, int clin, string pep, int rt)
{
	originaldir = corig;
	destination = cdest;
	verbose = cver;
	linked = clin;
	totalcountf = 0;
	copiedcountf = 0;
	totalcountd = 0;
	copiedcountd = 0;
	filesdeleted = 0;
	direcdeleted = 0;
	root = rt;
	kb = 0;
	program_execution_path = pep;

	/*Diagrafis teleytaias / apo to monopati an uparxei*/
	if (originaldir[originaldir.length()-1] == '/')
	{
		originaldir.erase((originaldir.length() - 1), string::npos);
	}
	if (destination[destination.length()-1] == '/')
	{
		destination.erase((destination.length() - 1), string::npos);
	}

	/*Settarisma tou execution path sto trexwn oste na mporei na ginei metatropi apo ta sxetika se apoluta*/
	if (root)
		program_execution_path = parentstr(program_execution_path);
	originaldir = relative_to_absolute(originaldir);
	destination = relative_to_absolute(destination);

	master_path = originaldir.length();
}

/*Geniki sunartisi antigrafis*/
int rapcopy::cplot()
{

	/*Elegxos gia to an monopati proorismou kai antigrafis einai ta idia i empeiriexetai to ena mesa sto allo*/
	if ( ((originaldir.find(destination)) != (string::npos)) || ((destination.find(originaldir)) != (string::npos)))
	{
		cout << "To monopati antigrafis kai proorismou den mporoun na exoun koino meros!" << endl;
		return 2;
	}

	int file_nonexistance_flag = 0;

	if (lstat(originaldir.c_str(), &inbuf) == -1)
	{
		perror(" Failed to get original file status ");
		return 1;
	}

	if(lstat(destination.c_str(), &otbuf) == -1)
	{
		parent_of_dest = parentstr(destination);
		struct stat nth;
		if(lstat(parent_of_dest.c_str(), &nth) == -1)
		{
			if (verbose)
			{
				cout << "To monopati proorismou den uparxei! Exiting now." << endl;
			}
			return 2;
		}
		else
		{
			file_nonexistance_flag = 1;
		}
	}


	int status = 0;
	if (file_nonexistance_flag)
		status = cpnew();
	else
		status = cpexs();

	if (status == 1)
		cout << "Look the status!" << endl;

	return 0;
}

/*Sunartisi antigrafis stoixeiwn se periptwsi pou den prouparxoun*/
int rapcopy::cpnew()
{

	if(verbose)
		cout << "." << originaldir.substr(master_path, string::npos) << endl;

	kb = kb + (inbuf.st_size/1024);
	if (S_ISREG(inbuf.st_mode))
	{
		totalcountf++;
		copiedcountf++;
		int orfd;
		int dsfd;

		if ((orfd = open(originaldir.c_str(), O_RDONLY|O_NONBLOCK)) < 0)
		{
			perror("Gia kapoio logo to arxeio pros antigrafi den mporese na diabastei!");
		}
		else
		{
			if ((dsfd = open(destination.c_str(), O_CREAT|O_EXCL|O_NONBLOCK|O_WRONLY)) < 0)
			{
				perror("Gia kapoio logo to neo arxeio den mporese na dimiourgithei!");
			}
			else
			{
				cpfile(orfd, dsfd);
			}
			close(dsfd);
		}
		close(orfd);
		/*Copy the permissions of original file to the destination file*/
		if(chmod(destination.c_str(), (inbuf.st_mode & 07777)))
		{
			perror("Could not set new file's attributes!");
		}
	}
	else if(S_ISDIR(inbuf.st_mode))
	{
		totalcountd++;
		copiedcountd++;
		if(!mkdir(destination.c_str(), 0777))
		{
			cpdir(0);
		}
		else
		{
			perror("Den mporousa na dimiourgiso katalogo me tin mkdir!");
		}

		/*Copy the permissions of original dir to the destination file*/
		if(chmod(destination.c_str(), (inbuf.st_mode & 07777)))
		{
			perror("Could not set new file's attributes!");
		}
	}
	else
	{
		return 1;
	}


	return 0;
}

/*Sunartisi antigrafis stoixeiou pou idi upirxe ston proorismo*/
int rapcopy::cpexs()
{

	if (S_ISREG(inbuf.st_mode))
	{
		totalcountf++;
		int orfd;
		int dsfd;


		/*Elegxos gia to an xreiazetai na ginei i antigrafi*/
		if(inbuf.st_size == otbuf.st_size)
			if(inbuf.st_mtime <= otbuf.st_mtime)
				return 2;

		kb = kb + (inbuf.st_size/1024);
		copiedcountf++;
		if ((orfd = open(originaldir.c_str(), O_RDONLY|O_NONBLOCK)) < 0)
		{
			perror("Gia kapoio logo to arxeio pros antigrafi den mporese na diabastei!");
			return 1;
		}
		else
		{
			if ((dsfd = open(destination.c_str(), O_CREAT|O_NONBLOCK|O_WRONLY|O_TRUNC)) < 0)
			{
				close(dsfd);
				/*Elegxos periptwsis pou to arxeio pou paei na antigrafei uparxei ston proorismo alla einai fakelos*/
				if (S_ISDIR(otbuf.st_mode))
				{
					delete_recursive(destination);
					if ((dsfd = open(destination.c_str(), O_CREAT|O_NONBLOCK|O_WRONLY|O_TRUNC)) < 0)
					{
							perror("Gia kapoio logo to neo arxeio den mporese na dimiourgithei!");
							return 1;
					}
					if(verbose)
						cout << "." << originaldir.substr(master_path, string::npos) << endl;
					cpfile(orfd, dsfd);
				}
				else
				{
					perror("Gia kapoio logo to neo arxeio den mporese na dimiourgithei!");
					return 1;
				}
			}
			else
			{
				if(verbose)
					cout << "." << originaldir.substr(master_path, string::npos) << endl;
				cpfile(orfd, dsfd);
			}
			close(dsfd);
		}
		close(orfd);

		/*Copy the permissions of original file to the destination file*/
		if(chmod(destination.c_str(), (inbuf.st_mode & 07777)))
		{
			perror("Could not set new file's attributes!");
		}
	}
	else if(S_ISDIR(inbuf.st_mode))
	{
		totalcountd++;

		cpdir(1);

		/*Copy the permissions of original file to the destination file*/
		if(chmod(destination.c_str(), (inbuf.st_mode & 07777)))
		{
			perror("Could not set new file's attributes!");
		}
	}
	else
	{
		return 1;
	}


	return 0;
}

/*Sunartisi antigrafis arxeiwn*/
int rapcopy::cpfile(int orfd, int dsfd)
{
	char buf[bufs];
	int length = 0;
	while((length = read(orfd, buf, bufs)))
	{
		if (write(dsfd, buf, length) == -1)
		{
			perror("Gia kapoio logo den graftikan dedomena sto neo arxeio!");
			return 1;
		}
	}
	return 0;
}

/*Sunartisi antigrafis fakelwn*/
int rapcopy::cpdir(int flag)
{
	DIR* dirp;
	struct dirent* dp;
	if ((dirp = (opendir(originaldir.c_str()))) == NULL)
	{
		perror("Den mporesa na anoikso kapoio directory");
		return 1;
	}


	/*_________________________________Ean kanoume copy dir pou prooupirxe______________________________________________*/

	if (flag)
	{
		DIR* dirpd;
		struct dirent* dpd;
		if ((dirpd = (opendir(destination.c_str()))) == NULL)
		{
			closedir(dirpd);
			/*Elegxos periptwsis pou o fakelos pou paei na antigrafei uparxei ston proorismo alla einai arxeio*/
			if (S_ISREG(otbuf.st_mode))
			{
				delete_recursive(destination);
				if(!mkdir(destination.c_str(), 0777))
				{
					cpdir(0);
				}
				else
				{
					perror("Den mporousa na dimiourgiso katalogo me tin mkdir!");
				}

				/*Copy the permissions of original file to the destination file*/
				if(chmod(destination.c_str(), (inbuf.st_mode & 07777)))
				{
					perror("Could not set new file's attributes!");
				}
				if ((dirpd = (opendir(destination.c_str()))) == NULL)
				{
					perror("Den mporesa na anoikso kapoio directory(destination)");
					return 1;
				}
			}
			else
			{
				perror("Den mporesa na anoikso kapoio directory(destination)");
				cout << "Elegkste ean uparxei idi allo arxeio i fakelos me to idio onoma!" << endl;
				return 1;
			}
		}
		/*Diagrafis oswn periexomenwn tou fakelou den vrikame ston original*/
		int to_be_deleted;
		while( (dpd = (readdir(dirpd))) != NULL )
		{
			to_be_deleted = 1;
			while( (dp = (readdir(dirp))) != NULL )
			{
				//cout << dpd << "   " << dp << endl;
				if (strcmp(dp->d_name, dpd->d_name) == 0)
				{
					to_be_deleted = 0;
					rewinddir(dirp);
					break;
				}
			}

			if(to_be_deleted)
			{
				string tempd = destination;
				tempd.append("/");
				tempd.append(dpd->d_name);
				delete_recursive(tempd);
			}
			rewinddir(dirp);
		}
		closedir(dirpd);
		rewinddir(dirp);
	}

	/*________________________________Antigrafi periexomenwn fakelou________________________________________________________*/


		while( (dp = (readdir(dirp))) != NULL )
		{
			if((strcmp(dp->d_name,".") != 0) && (strcmp(dp->d_name,"..") !=0))
			{
				string tempds = destination;
				string tempor = originaldir;
				tempds.append("/");
				tempor.append("/");
				tempds.append(dp->d_name);
				tempor.append(dp->d_name);
				rapcopy mycopy(tempor, tempds, verbose, linked, program_execution_path, 0);
				mycopy.master_path = master_path;
				if (mycopy.cplot() == 1)
				{
					cout << "Something wicked happened!" << endl;
				}
				totalcountf += mycopy.totalcountf;
				copiedcountf += mycopy.copiedcountf;
				totalcountd += mycopy.totalcountd;
				copiedcountd += mycopy.copiedcountd;
				filesdeleted += mycopy.filesdeleted;
				direcdeleted += mycopy.direcdeleted;
				kb += mycopy.kb;
			}
		}
	closedir(dirp);

	return 0;
}

/*Anadromiki sunartisi diagrafis arxeiwn kai fakelwn*/
int rapcopy::delete_recursive(string path)
{
	struct stat temp;
	if (lstat(path.c_str(), &temp) == -1)
	{
		return -1;
	}
	if (S_ISREG(temp.st_mode))
	{
		if(remove(path.c_str()) != 0)
		{
			perror("Den katafera na diagrapso ena arxeio pou upirxe mono ston fakelo proorismou!");
		}
		filesdeleted++;
	}
	else if (S_ISDIR(temp.st_mode))
	{
		DIR* dirp;
		struct dirent* dp;
		string temp;
		if ((dirp = (opendir(path.c_str()))) == NULL)
		{
			perror("Den mporesa na anoikso kapoio directory");
		}
		while( (dp = (readdir(dirp))) != NULL )
		{
			if((strcmp(dp->d_name,".") != 0) && (strcmp(dp->d_name,"..") !=0))
			{
				temp = path;
				temp.append("/");
				temp.append(dp->d_name);
				delete_recursive(temp);

			}
			temp = "";
			temp.clear();
		}
		direcdeleted++;
		closedir(dirp);
		if(remove(path.c_str()) != 0)
		{
			cout << path << endl;
			perror("Den katafera na diagrapso ena directoy pou upirxe mono ston fakelo proorismou!");
		}
	}


	return 0;
}


/*Sunartisi pou epistrefei to path tou parent*/
string rapcopy::parentstr(string child)
{
	if (child.length() <= 2)
		return "/";
	return child.substr(0, child.find_last_of("/", string::npos));
}

/*Sunartisi pou metatrepsei sxetika monopatia se apoluta (px: ./ kai /home/george/..*/
string rapcopy::relative_to_absolute(string path)
{
	if (path[0] != '/')
	{
		if (path[0] == '.' && path[1] == '/')
		{
			path = program_execution_path + path.erase(0,1);
		}
		else if (path[0] == '.' && path[1] == '.' && path[2] == '/')
		{
			path = parentstr(program_execution_path) + path.erase(0,2);
		}
		else
		{
			path = program_execution_path + path;
			path = relative_to_absolute(path);
		}
	}
	else
	{
		if (((path[1] == '.') && (path[2] == '/') )|| ((path[1] == '.') && (path[2] == '.') && path[3] == '/') )
		{
			path.erase(0,1);
			path = path.substr(path.find_first_of('/'), string::npos);
			path = relative_to_absolute(path);
		}
	}

	for(unsigned int i = 0; i < (path.length() - 1); i++)
	{
		if (path[i] == '.' && path[i+1] == '.' && ((i + 2 == path.length()) || (path[i+2] == '/')))
		{
			if(i + 2 == path.length())
			{
				path = parentstr(path);
				i = 0;
			}
			else
			{
				string temp = path;
				temp.erase(0,(i+2));
				path.erase((i-1),string::npos);
				path = parentstr(path);
				path = path + temp;
				i = 0;
			}
		}
	}


	return path;
}


rapcopy::~rapcopy()
{
	if (root && verbose)
	{
		cout << "\nSunolo arxeiou/wn stin ierarxia: " << totalcountf << endl;
		cout << "Sunolo fakelou/wn stin ierarxia: " << totalcountd << endl;
		cout << "Sunolo arxeiou/wn pou antigrafike/an: " << copiedcountf << endl;
		cout << "Sunolo fakelou/wn pou antigrafike/an: " << copiedcountd << endl;
		cout << "Sunolo fakelou/wn kai arxeiou/wn pou antigrafike/an: " << copiedcountf+copiedcountd << endl;
		cout << "Geniko sunolo arkeiou/wn kai fakelou/wn: " << totalcountf+totalcountd << endl;
		cout << "Sunolika antigrafisan " << kb << "KBs apo dedomena!" << endl;
		cout << "Sunolika diagrafike/an " << filesdeleted << " arxeio/a." << endl;
		cout << "Sunolika diagrafike/an " << direcdeleted << " fakelos/oi." << endl;
		cout << "Sunolika diagrafike/an " << filesdeleted+direcdeleted << " arxeio/a kai fakelos/oi." << endl;
	}
}

