#ifndef FIND_PATH_H_
#define FIND_PATH_H_

#include<bits/stdc++.h>

/**global**/
struct Search_edge{
    int to;
    std::vector<int> timestamps;
};
std::ofstream ofs("C:\\vscode\\output\\origin.txt");

int num[100]={0};

template<class T>
class FindPath{

    private:
        T threshold;
        /**control*/
        bool ofs_on=true;
        bool withbundle=1;
        int quadLimit=1000;
        int SearchLimit=2;
        std::map<int, std::map<int, T> >us;
        std::map<int, T> ct;
        std::map<int, T> res;
        int maxlen=0;
        std::map<int, std::map<int, std::vector<T> > > subgraph;
        std::map<int, std::map<T, std::set<int> > > Adj_list;//for paths without bundles
        //

    public:
        //set some required values
        bool set_threshold(T w);
        bool cycle_search_single(bool);
        void UnBlock(int node, T tv);
        bool UnBlock_single(int node, T tv);
        void show_u(int node);
        void show_cx(int node);
        void Extend(int node, int w, T t);
        void compute_query(std::map<T, int> * qs);

        bool show_quadruple(int node);
        bool show_cycle_input(int node);
        void find_cycle(char *filename);
        bool cycle_search(bool);
        void DFS_path(int s, std::vector<Search_edge> *stk, T pre,
                      int pos,std::vector<std::pair<int,T> > *cur_path);
        void print_without_bundle(int s, std::vector<Search_edge> *stk);
        int ALLPATH_single(int s, int vcur, T endt, T timestamp, 
                            std::vector<std::pair<int, T> > stk, std::set<int> * candi);
        T ALLPATH(int s, int vcur, std::vector<T> *timestamps, std::vector<Search_edge> stk);
        std::vector<Search_edge> Expand(int t,std::vector<T> &ts, std::vector<Search_edge> stk);
        void print(int s, Search_edge eg, std::vector<Search_edge> *stk);
        void print_single(int s, std::vector<std::pair<int, T> > *stk);
        virtual ~FindPath() {
        }

};


template<class T>
void FindPath<T>::find_cycle(char *filename){
    //read in query
    std::ifstream query_ifs("C:\\vscode\\output\\query.txt");
    T window;
    int hop,querynum, maxhop=0; 
    query_ifs>>window>>hop;
    window*=3600;
    threshold=window;
    SearchLimit=hop;
    query_ifs.close();
    //read in graph
    int a, b;
    T t;
    clock_t startTime,endTime;
    startTime = clock();
    std::ifstream ifs(filename);
    printf("start!\n");
        while(ifs>>a>>b>>t){
        if(a==b) continue;
        Adj_list[a][t].insert(b);
    }
    ifs.close();
    //  
    printf("search limit=%d\n", SearchLimit);
    printf("finding cycle\n");
    if(withbundle)
        cycle_search(1);
    else 
        cycle_search_single(1);
    endTime = clock();
    std::cout << "finding paths time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    //for(int i=0; i<=30; i++)
    //    printf("num[%d]=%d\n", i, num[i]);
}


template<class T>
bool FindPath<T>::cycle_search(bool isquery){
    std::ifstream quad_ifs("quad.txt");
    typename std::map<int, std::vector<T> >::iterator sub_it;

    int nd, node;
    T ts, te, tn;
    std::set<int> candidates;
    int improvecnt=0;
    int quad_cnt=0;
    while(quad_ifs>>node){
        quad_ifs>>ts>>te>>tn;
        quad_cnt++;
        if(quad_cnt%quadLimit==0){
            printf("quad=%d\n",quad_cnt);
        }
        candidates.clear();
        while(quad_ifs>>nd){
            if(nd==-1) break;
            candidates.insert(nd);
        }

        if(candidates.size()<=1){
            continue;
        }

        candidates.insert(node);
        subgraph.clear();
        //ofs<<"ts="<<ts<<"te="<<te<<"\n";
        std::set<int>::iterator set_it, to_it;
        typename std::map<T, std::set<int> >::iterator s_it, e_it;
        for(set_it=candidates.begin(); set_it!=candidates.end(); set_it++){
            int ffrom=*set_it;
            s_it=Adj_list[ffrom].lower_bound(ts);
            e_it=Adj_list[ffrom].lower_bound(te+1);
            for(;s_it!=e_it; s_it++){
                T time_now=(*s_it).first;
                for(to_it=(*s_it).second.begin(); to_it!=(*s_it).second.end(); to_it++){
                    int to_=*to_it;
                    if(candidates.find(to_)==candidates.end()) continue;
                    subgraph[ffrom][to_].push_back(time_now);
                }
            }
        }
        
        for(set_it=candidates.begin(); set_it!=candidates.end(); set_it++){
            if(typeid(T)==typeid(long long int))
                ct[*set_it]=LLONG_MAX;
            else
                ct[*set_it]=INT_MAX;
            us[*set_it].clear();// empty set ->structs
        }


        std::vector<Search_edge> stk;
        for(sub_it=subgraph[node].begin(); sub_it!=subgraph[node].end(); sub_it++){
            Search_edge e;
            e.to=sub_it->first;
            e.timestamps=sub_it->second;

            std::vector<T> ts;
            for(int j=0; j<e.timestamps.size(); j++){
                if(e.timestamps[j]<tn){
                    ts.push_back(e.timestamps[j]);
                }
            }
            Search_edge newe;
            newe.to=e.to;
            newe.timestamps=ts;
            stk.clear();
            stk.push_back(newe);

            if(!ts.empty()){
                ALLPATH(node, e.to, &ts, stk);
            }
        }
        //
    };
    printf("improvecnt=%d\n", improvecnt);
}


template<class T>
std::vector<Search_edge> FindPath<T>::Expand( int t, std::vector<T> &ts, std::vector<Search_edge> stk){

    std::vector<Search_edge> new_stk(stk);//copy
    //
    if(new_stk.empty()) return stk;//replace unsuccessfully
    Search_edge tmp=new_stk[new_stk.size()-1];
    Search_edge newitem;
    std::vector<T> tmp_t=tmp.timestamps;
    if(tmp_t.empty()) return stk;
    T min_timestp=tmp_t[0];
    typename std::vector<T>::iterator it=std::upper_bound(ts.begin(), ts.end(), min_timestp);
    int pos=it-ts.begin();
    //printf("pos=%d\n", pos);

    if(pos==ts.size()){//clear
        for(int i=0; i<new_stk.size(); i++){
            new_stk[i].timestamps.clear();
        }
        newitem.timestamps.clear();
    }
    else{
        ts.assign(it, ts.end());
        T maxx=ts[ts.size()-1];
        for(int i=new_stk.size()-1; i>=0; i--){
            it=std::lower_bound(new_stk[i].timestamps.begin(), new_stk[i].timestamps.end(), maxx);
            new_stk[i].timestamps.erase(it, new_stk[i].timestamps.end());
            maxx=new_stk[i].timestamps[new_stk[i].timestamps.size()-1];
        }
        newitem.timestamps=ts;
    }
    newitem.to=t;
    new_stk.push_back(newitem);
    return new_stk;
}


template<class T>
void FindPath<T>::DFS_path(int s, std::vector<Search_edge> *stk, T pre,
                             int pos, std::vector<std::pair<int,T> > *cur_path){
    for(int i=0; i<(*stk)[pos].timestamps.size(); i++){
        if((*stk)[pos].timestamps[i]<=pre) continue;
        //print
        (*cur_path)[pos]=std::make_pair((*stk)[pos].to, (*stk)[pos].timestamps[i]);
        if(pos!=(*stk).size()-1) DFS_path(s, stk, (*stk)[pos].timestamps[i], pos+1, cur_path);
        else{
            if(ofs_on) ofs<<s;
            else std::cout<<s;
            for(int j=0; j<(*stk).size(); j++){
                if(ofs_on) ofs<<"--"<<(*cur_path)[j].second<<"-->"<<(*cur_path)[j].first;
            }
            if(ofs_on) ofs<<"\n";
            else std::cout<<"\n";
            //num[(*stk).size()]++;
        }
    }
}

template<class T>
void FindPath<T>::print_without_bundle(int s, std::vector<Search_edge> *stk){
    if((*stk).size()>SearchLimit||(*stk).size()<=2) return;
    std::vector<std::pair<int, T> > cur_path; 
    cur_path.resize((*stk).size());
    DFS_path(s, stk, -1, 0, &cur_path);
}

extern int ccnt;
template<class T>
void FindPath<T>::print(int s, Search_edge eg, std::vector<Search_edge> *stk){
    if((*stk).size()>SearchLimit||(*stk).size()<=2) return;
    if(ofs_on)
        ofs<<s;
    else
        printf("%d", s);

    for(int i=0; i<(*stk).size(); i++){
        Search_edge e=(*stk)[i];
        
        if(ofs_on)
            ofs<<"--";
        else
            printf("--");
        for(int j=0; j<e.timestamps.size(); j++){
            if(ofs_on) ofs<<"|"<<e.timestamps[j];
            else std::cout<<"|"<<e.timestamps[j];
        }
        if(ofs_on)
            ofs<<"-->"<<e.to;
        else
            printf("-->%d ", e.to);
    }
    if(ofs_on)
        ofs<<" "<<std::endl;
    else
        printf("\n");
}



template<class T>
T FindPath<T>::ALLPATH(int s, int vcur, std::vector<T> *timestamps, std::vector<Search_edge> stk){//timestamps are arranged in ascending order
    //improvetag
    std::vector<Search_edge> new_stk;
    std::vector<T> ts, tss;

    T tcur=(*timestamps)[0];
    ct[vcur]=tcur;
    T lastp=0;


    typename std::map<int, std::vector<T> >::iterator sub_it;

    for(sub_it=subgraph[vcur].begin(); sub_it!=subgraph[vcur].end(); sub_it++){
        Search_edge e;
        e.to=sub_it->first;
        e.timestamps=sub_it->second;
        ts.clear();
        tss.clear();

        for(int j=0; j<e.timestamps.size(); j++){
            if(e.timestamps[j]<=tcur) continue;
            //if(e.timestamps[j]<=tcur||j&&e.timestamps[j-1]==e.timestamps[j]) continue;
            //if(e.timestamps[j]>=ct[e.to]) break;
            ts.push_back(e.timestamps[j]);
            if(e.timestamps[j]>=ct[e.to]) continue;
            tss.push_back(e.timestamps[j]);
        }
        if(e.to==s){//get cycle
            if(!ts.empty()){
                T maxt=ts[ts.size()-1];
                if(maxt>lastp) lastp=maxt;
            }
            new_stk=Expand(e.to, ts, stk);
            if(!new_stk.empty()&&!new_stk[0].timestamps.empty()){
                print(s, e, &new_stk);
                //print_without_bundle(s, &new_stk);
            }
        }
        else{
            T ret=0;
            if(!tss.empty()){
                new_stk=Expand(e.to, tss, stk);
                if(new_stk.size()<SearchLimit){
                    if(!new_stk.empty()&&!new_stk[0].timestamps.empty()){
                        ret=ALLPATH(s, e.to, &tss, new_stk);
                    }
                }
                else{
                    if(!new_stk.empty()&&!new_stk.back().timestamps.empty())
                       ret=new_stk.back().timestamps.back()+1;//why
                }
            }
            int pos=std::lower_bound(ts.begin(), ts.end(), ret)-ts.begin();//the first edge that can not pass 
            if(pos!=ts.size()){
                Extend(e.to, vcur, ts[pos]);
            }
            if(ret){//pass
                int pos=std::lower_bound(ts.begin(), ts.end(), ret)-ts.begin();
                if(pos>0&&ts[pos-1]>lastp) lastp=ts[pos-1];
            }
            /****/
        }
    }

    if(lastp>0){
        UnBlock(vcur, lastp);
    }
    return lastp;
}


template<class T>
bool FindPath<T>::cycle_search_single(bool isquery){
    int nd, node;
    T ts, te, tn;
    std::set<int> candidates;
    std::ifstream quad_ifs("quad.txt");
    int quad_cnt=0;
    int improvecnt=0;
    while(quad_ifs>>node){
        quad_ifs>>ts>>te>>tn;
        quad_cnt++;
        if(quad_cnt%quadLimit==0){
            printf("quad=%d\n",quad_cnt);
        }

        candidates.clear();
        while(quad_ifs>>nd){
            if(nd==-1) break;
            candidates.insert(nd);
        }
        if(candidates.size()<=1){
            continue;
        }
        candidates.insert(node);
        std::set<int>::iterator set_it;
        for(set_it=candidates.begin(); set_it!=candidates.end(); set_it++){
            if(typeid(T)==typeid(long long int))
                ct[*set_it]=LLONG_MAX;
            else
                ct[*set_it]=INT_MAX;
            us[*set_it].clear();// empty set ->structs
        }
        std::vector<std::pair<int ,T> > stk;
        typename std::map<T, std::set<int> >::iterator s_it, e_it;
        std::set<int>::iterator to_it;
        s_it=Adj_list[node].lower_bound(ts);
        e_it=Adj_list[node].lower_bound(std::min(tn,te+1));
        for(; s_it!=e_it; s_it++){
            T time_now=(*s_it).first;
            for(to_it=(*s_it).second.begin(); to_it!=(*s_it).second.end(); to_it++){
                int to_=*to_it;
                if(candidates.find(to_)==candidates.end()) continue;
                std::pair<int, T> pr=std::make_pair(to_, time_now);
                stk.clear();
                stk.push_back(pr);
                ALLPATH_single(node, to_, te, time_now, stk, &candidates);
            }
        }
    };
    printf("improvecnt=%d\n", improvecnt);
}

template<class T>
int FindPath<T>::ALLPATH_single(int s, int vcur, T endt, T tcur, std::vector<std::pair<int, T> > stk, std::set<int> *candidates){
    ct[vcur]=tcur;
    //printf("ct[%d] become %d\n", vcur, ct[vcur]);
    T lastp=0;
    typename std::map<T, std::set<int> >::iterator s_it, e_it;
    std::set<int>::iterator it;

    s_it=Adj_list[vcur].upper_bound(tcur);//>tcur
    e_it=Adj_list[vcur].upper_bound(endt);//<=endt


    std::map<int,bool> neg;neg.clear();
    for(; s_it!=e_it ;s_it++){
        T now_time=(*s_it).first;
        for(it=(*s_it).second.begin(); it!=(*s_it).second.end(); it++){
            if((*candidates).find(*it)==(*candidates).end()) continue;
            if(neg.find(*it)!=neg.end()) continue;
            if(*it==s){
                if(now_time>lastp) lastp=now_time;
                stk.push_back(std::make_pair(s, now_time));
                print_single(s, &stk);
                stk.pop_back();
            }
            else{
                T tm=now_time;
                int pass=0;
                if(ct[*it]>tm){
                    stk.push_back(std::make_pair(*it, now_time));
                    pass=ALLPATH_single(s, *it, endt, now_time, stk, candidates);
                    stk.pop_back();
                }
                if(!pass){
                    neg[*it]=1;
                    Extend(*it, vcur, tm);
                }
                else{
                    if(tm>lastp) lastp=tm;
                }
            }
        }
    }
    if(lastp>0){
        UnBlock_single(vcur, lastp);
    }
    return (lastp!=0);
}


template<class T>
void FindPath<T>::print_single(int s, std::vector<std::pair<int, T> > *stk){
    if((*stk).size()>SearchLimit||(*stk).size()<=2) return;
    ofs<<(*stk)[0].second<<" "<<(*stk).back().second<<" "<<(*stk).size()<<" ";
    if(ofs_on)
        ofs<<s;
    else printf("%d", s);
    for(int i=0; i<(*stk).size(); i++){
        std::pair<int, T> e=(*stk)[i];
        if(ofs_on) ofs<<"--"<<e.second<<"-->"<<e.first;
        else std::cout<<"--"<<e.second<<"-->"<<e.first;
    }
    if(ofs_on) ofs<<"\n";
    else std::cout<<"\n";
}

template<class T>
bool FindPath<T>::UnBlock_single(int node, T tv){
    typename std::map<int, T>::iterator it;
    typename std::map<T, std::set<int> >::iterator mit;
    
    if(tv>ct[node]){
        ct[node]=tv;
        //printf("ct[%d] become %d\n", node, ct[node]);
        if(us[node].empty()) return false;
        for(it=us[node].begin(); it!=us[node].end(); ){
            if(it->second>=tv){
                it++;
                continue;
            }
            int to=it->first;
            T minT=-1;
            T deletetime=-1;
            
            for(mit=Adj_list[to].begin(); mit!=Adj_list[to].end(); mit++){
                T timet=(*mit).first;
                if((*mit).second.find(node)!=(*mit).second.end()){
                    if(timet>=tv){
                        minT=timet;
                        break;
                    }
                    else deletetime=timet;
                }
            }
            
            if(minT==-1)
                us[node].erase(it++);
            else{
                us[node][to]=minT;it++;
            }
            if(deletetime!=-1)
                UnBlock_single(to, deletetime);
                
        }
    }   
}
/**-------------------------CUT-LINE--------------------------------------**/

template<class T>
void FindPath<T>::UnBlock(int node, T tv){

    /***change a way to rank!!***/
    typename std::map<int, T>::iterator it;
    typename std::map<int, std::vector<T> >::iterator sub_it;
    //less->greater
    if(tv>ct[node]){
        ct[node]=tv;
        if(us[node].empty()) return ;
        for(it=us[node].begin(); it!=us[node].end();){
            if(it->second>=tv){//the first blocked timestamp vs tv
                it++;
                continue;
            }
            int to=it->first;
            //
            for(sub_it=subgraph[to].begin(); sub_it!=subgraph[to].end(); sub_it++){
                Search_edge e;
                e.to=sub_it->first;
                e.timestamps=sub_it->second;
                if(e.to==node){
                    int pos=std::lower_bound(e.timestamps.begin(), e.timestamps.end(), tv)-e.timestamps.begin();
                    T minT;
                    if(pos!=e.timestamps.size()){
                        minT=e.timestamps[pos];
                        us[node][to]=minT;
                        it++;
                    }
                    else
                        us[node].erase(it++);
                    if(pos>0)
                        UnBlock(to, e.timestamps[pos-1]);
                    break;
                }
            }
        }
    }
}


template<class T>
void FindPath<T>::Extend(int node, int w, T t){
    typename std::map<int, T>::iterator it;
    it=us[node].find(w);
    if(it!=us[node].end()){//already have
        if(us[node][w]>t)
            us[node][w]=t;
    }
    else{
        us[node].insert(std::make_pair(w, t));
    }
}

//-------------------------
#endif // FIND_PATH_H_
