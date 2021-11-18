#include<bits/stdc++.h>
using namespace std;
int main(){
    ifstream ifs1("D:\\output\\higgs_1h_transed.txt");
    ifstream ifs2("D:\\output\\origin.txt");
    string line;
    set<string> lines;
    while(ifs1>>line){
       lines.insert(line); 
    }
    while(ifs2>>line){
        if(lines.find(line)==lines.end()){
            cout<<line<<"\n";
        }
    }
    return 0;
}