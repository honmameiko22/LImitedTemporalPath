#ifndef INIT_GRAPH_H_
#define INIT_GRAPH_H_
using namespace std;

struct Edge{
    int from, to, timestamp;
    Edge(int f, int t, int ts){
        this->from=f;
        this->to=t;
        this->timestamp=ts;
    }
};
vector<Edge> edges;

void rerange(char *infilename, char *outfilename){
    std::ifstream ifs(infilename);
    if(!ifs) printf("ifs error!\n");
    int from, to, timestamp;
    int maxid=0;
    int V=0;
    set<int> st;
    for(;ifs>>from>>to>>timestamp;){
        Edge e=Edge(from, to, timestamp);
        if(e.timestamp<edges[edges.size()-1].timestamp) printf("error!\n");
        edges.push_back(e);
        maxid=max(maxid, from);
        V=std::max(V, from);
        V=std::max(V, to);
        st.insert(from);
        st.insert(to);
    }
    printf("maxv=%d\n", V);
    printf("total size=%d\n", st.size());

    vector<int> Reverse_table(V+1);
    set<int>::iterator it;
    int pointer=1;
    for(it=st.begin(); it!=st.end(); it++){
      //  printf("%d\n", *it);
        Reverse_table[*it]=pointer++;
    }

    int newfrom, newto;
    ofstream ofs(outfilename);
    if(!ofs) printf("ofs error!\n");
    for(int i=0; i<edges.size(); i++){
        Edge e=edges[i];
        newfrom=Reverse_table[e.from];
        newto=Reverse_table[e.to];
       // printf("newfrom=%d,newto=%d\n", newfrom, newto);
        ofs<<newfrom<<" "<<newto<<" "<<e.timestamp<<" ";
    }

}

#endif // INIT_GRAPH_H_
