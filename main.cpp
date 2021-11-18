#include <iostream>
#include<set>
#include<cstdio>
#include"Generate_Seeds_Bloom.h"
#include"Find_Path.h"
#include"Bloom_Filter_seed.h"

using namespace std;
int ccnt;   
std::set<std::pair<int, int> > couple;
int main() 
{
    ccnt=0;
    GenerateSeeds<int> seed;    
    seed.query("C:\\vscode\\dataset\\test.txt");
    //FindPath<int> path;
    //path.find_cycle("C:\\vscode\\dataset\\wiki.txt");
    //GenerateSeedsBloom<int> seed;
    //seed.query("C:\\vscode\\dataset\\wiki.txt");
    //FindPath<int> path;
    //path.find_cycle("C:\\vscode\\dataset\\wiki.txt");

    printf("\n");
    printf("ccnt=%d\n", ccnt);//paths longer than 2   
    return 0;
}





//change to long long
//compare with basic 



