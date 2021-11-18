#include<bits/stdc++.h>
using namespace std;
struct edge{
    int a, b, t;
};
bool cmp(edge x, edge y){
    return x.t<y.t;
}
int main(){
    ifstream ifs("C:\\vscode\\dataset\\wiki-talk-temporal.txt");
    ofstream ofs("C:\\vscode\\output\\wiki.txt");

    int a, b, c;
    vector<edge>es;
    while(ifs>>a>>b>>c){
        edge now;
        now.a=a;now.b=b;now.t=c;
        es.push_back(now);

    }
    sort(es.begin(), es.end(), cmp);
    for(int i=0; i<es.size(); i++)
        ofs<<es[i].a<<" "<<es[i].b<<" "<<es[i].t<<"\n";
    return 0;
}