#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cmath>
#include <queue>

using namespace std;


struct page
{
	int relP;
	int absP;
	int timeStamp;
	int prog;
	bool used;
	bool inMain;
};

struct program
{
	int ID;
	int size;
	int pagesNeeded;
	vector<page> pTable;
};


void initMMem();
void readPList(string);
void pagingTable();
void pagingSystem(string);
void clockA(int, int, int, page*);
void lru(int, int, int);
void fifo(int, int, int);
int getMain(int, int);
int getLoc(page);


int pageSize=1;
int numPages=0;
int numFrames=0;
int pageCount=0;
int numFaults=0;
int currTime=0;
int paging;
string algorithm;
vector<program> programs;
vector<page> mainMem;
queue<page> fifoQ;
bool full;


int main(int argc, char *args[])
{
	if(argc == 6)
	{
		if((pageSize==1 || pageSize%2 == 0) && pageSize>0)	
			pageSize = atoi(args[3]);
		else
			cout << "ERROR: arg3 must be a positive power of 2.";
		algorithm = args[4];
		
		if(atoi(args[5]) == 1 || atoi(args[5]) == 0)
 	   	{
			paging = atoi(args[5]);
 	   	}
 		else
 		{
 			cout<<"ERROR: arg5 must be 1 or 0."<<endl;
 			return 0;
 		}
		
		cout << "/**************************************" << endl
			 << "Page Size: " << pageSize << endl
			 << "Paging Policy: " << algorithm << endl;
		
		numFrames = 512/pageSize;
		
		readPList(args[1]);
		
		initMMem();
	
		pagingSystem(args[2]);
	}
	else
	{
		cout << "ERROR: Invalid parameters. \n\n";
		return 0;
	}
	return 0;
}

void readPList(string fileName)
{
	ifstream fin;
	fin.open(fileName.c_str());
	while(!fin.eof())
	{
		program p;
		fin >> p.ID;
		fin >> p.size;
		       
  	    p.pagesNeeded = p.size/pageSize;
		
		if(p.size%pageSize != 0)
			p.pagesNeeded++;

		for(int c=0; c<p.pagesNeeded; c++)
		{
			page pg;
			pg.prog = p.ID;
			pg.relP=c; 
			pg.absP = pageCount;
			pageCount++;
			pg.inMain = false;
			
			p.pTable.push_back(pg);
		}

		programs.push_back(p);
	}	
	fin.close();
}

void pagingSystem(string fileName)
{
	ifstream fin;
	fin.open(fileName.c_str());
	
	page *cState = &mainMem[mainMem.size()-1];
	
	while(!fin.eof())
	{
		int pID;
		int word;
		
		fin >> pID;
		fin >> word;
		
		int pageNum = word/pageSize;
		
		page p = programs[pID].pTable[pageNum];
		
		if(p.inMain)
		{
			if(algorithm=="lru")
			{
				p.timeStamp = currTime;
				currTime++;
			}
			else if(algorithm == "clock")
				p.used=true;
		}
		else
		{
			numFaults++;
			if(full)
			{
				if(algorithm=="clock")
				{
					clockA(pID, pageNum, paging, cState);
				}
				else if(algorithm=="lru")
				{
					lru(pID, pageNum, paging);
				}
				else if(algorithm=="fifo")
				{
					fifo(pID, pageNum, paging);
				}
			}				
		}

	}
	fin.close();
	
	cout << "Total Page Faults: " << numFaults << endl
		 << "/**************************************" << endl;
}

void initMMem()
{
	int initialPages = 512/(pageSize*programs.size());
	
	for(int x = 0; x < programs.size(); x++)
	{
		
		for(int y = 0; y < initialPages; y++)
		{
			page p = programs[x].pTable[y];
			
			p.inMain = true;
			
			if(algorithm=="lru")
			{
				p.timeStamp = currTime;
				currTime++;
			}
			else if(algorithm=="clock")
			{
				p.used = false;
			}
			else if(algorithm=="fifo")
			{
				fifoQ.push(p);
			}
			
			mainMem.push_back(p);
		}
	}
	
	full=true;
	for(int c=(programs.size()*initialPages); c<(512/pageSize); c++)
	{
		full=false;
		page pg;
		pg.timeStamp=0;
		pg.relP=0;
		pg.absP=0;
		pg.inMain=true;
		pg.used=false;
		mainMem.push_back(pg);
	}
}

int getLoc(page p)
{
	for(int c=0; c<mainMem.size(); c++)
		if(p.absP == mainMem[c].absP)
			return c;
	return -1;
}

void clockA(int ID, int rel, int pg, page * state)
{
	page p;
	
	for(int c=0; c<programs[ID].pTable.size(); c++)
		if(programs[ID].pTable[c].relP == rel)
			p = programs[ID].pTable[c];
	
	int loc = getLoc(*state);
	
	while(state->used)
	{
		state->used = false;
		loc++;
		if(loc >= mainMem.size())
			loc = 0;
		state = &mainMem[loc];
	}
	
	mainMem[pageCount] = p;
	
	if(pg==1)
	{
		int loc = getLoc(*state);
		while(state->used)
		{
			state->used = false;
			loc++;
			
			if(loc >= mainMem.size())
				loc = 0;
			state = &mainMem[loc];
		}
		mainMem[loc] = p;
	}
}
void fifo(int ID, int rel, int pg)
{
	page p;
	for(int c=0; c<programs[ID].pTable.size(); c++)
		if(programs[ID].pTable[c].relP == rel)
			p = programs[ID].pTable[c];
	
	if(pg==1)
	{
		int loc=pageCount+1;
		if(loc >= mainMem.size())
			loc = 0;
		rel++;
		if(rel >= programs[ID].pTable.size())
			rel = 0;
		return;
	}
	mainMem[pageCount] = p;	
}

void lru(int ID, int rel, int pg)
{
	int temp, val=mainMem[0].timeStamp;
	
	page p;
	for(int c=0; c<programs[ID].pTable.size(); c++)
		if(programs[ID].pTable[c].relP == rel)
			p = programs[ID].pTable[c];
	
	p.timeStamp = currTime;
	
	if(pg==1)
	{
		rel++;
		if(rel >= programs[ID].pTable.size())
			rel = 0;
		for(int c=0; c<mainMem.size(); c++)
		{
			if(mainMem[c].timeStamp < val)
			{
				val = mainMem[c].timeStamp;
				temp = c;
			}
		}
		mainMem[temp] = p;
		return;
	}
	
	for(int c=0; c<mainMem.size(); c++)
	{
		if(mainMem[c].timeStamp < val)
		{
			val = mainMem[c].timeStamp;
			temp = c;
		}
	}
	mainMem[temp] = p;
}