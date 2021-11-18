#ifndef BLOOM_FILTER_SEED_H
#define BLOOM_FILTER_SEED_H
#include "bloom_filter.hpp"
typedef std::pair<int, int> pii;

template<class T>
class GenerateSeedsBloom{
    private:
        int cleanupLimit=100000;
        T threshold=3600*10;
        short searchLimit=5; 
        struct Triad{
            int from;
            T time;
            bool operator<(const struct Triad & right) const{
                if(this->time!=right.time)
                    return this->time>right.time;
                return this->from<right.from;
            }
            Triad(int from, T t){
                this->from=from;
                this->time=t;
            }
        };
        struct Quintuple{
            T starttime, endtime;
            std::set<int> candidates;//candidates are points
            short hop;
            Quintuple(T starttime, T endtime, std::set<int> &candidates, short hop){
                this->starttime=starttime;
                this->endtime=endtime;
                this->hop=hop;
                this->candidates=candidates;
            }
            bool operator<(const struct Quintuple & right) const{
                if(this->starttime!=right.starttime)
                    return this->starttime<right.starttime;
                return this->endtime>right.endtime;
            }
            Quintuple(){}
        };

    public:
        void GetSeeds(char* filename);
        void GenerateSeeds_descend(char* filename);
        void GenerateSeeds_ascend(char* filename);
        void combine_tuples();
        void query(char *file);
        void compute_query(std::map<T, short> * qs);
};


template<class T>
void GenerateSeedsBloom<T>::GetSeeds(char* filename){
    GenerateSeeds_descend(filename);                                                      
    GenerateSeeds_ascend(filename);
}


template<class T>
void GenerateSeedsBloom<T>::GenerateSeeds_descend(char* filename){
    
    std::ifstream ifs(filename);
    
    std::vector<std::pair<pii, T> > input_graph;//temporally store graph
    std::ofstream seed_ofs("seed.txt");
    //last active time of nodes
    std::map<int, T> Lastactive;
    typename std::map<int, T>::iterator Lit;
    //set bloom filter
    std::map<int, bloom_filter> S;
    typename std::map<int, bloom_filter>::iterator from_it, to_it;
    bloom_parameters parameters;
    parameters.projected_element_count = 1000;
    parameters.false_positive_probability = 0.0001; // 1 in 10000
    parameters.random_seed = 0xA5A5A5A5;
    if (!parameters){
       std::cout << "Error - Invalid set of bloom filter parameters!" << std::endl;
       return;
    }    
    parameters.compute_optimal_parameters();
    
    std::cout<<"computing cycles"<<std::endl;
    std::cout<<"start!\n";
    std::string str;
    int from, to;
    T timestamp;
    while(ifs>>from>>to>>timestamp){
        input_graph.push_back(std::make_pair(std::make_pair(to,from), timestamp));
    }
    ifs.close();

    int count=0, selfloop=0;
    for(int i=input_graph.size()-1; i>=0; i--){
        from=input_graph[i].first.first;
        to=input_graph[i].first.second;
        timestamp=input_graph[i].second;
        if(from==to){
            selfloop++;continue;
        }
        count++;
        
        if(S.count(to)==0){ 
            bloom_filter filter(parameters);
            filter.insert(from);
            S[to]=filter;
        }
        else{
            if(Lastactive[to]-timestamp>threshold)
                S[to].clear();
            S[to].insert(from);
        }
        
        Lastactive[to]=timestamp;
        
        from_it=S.find(from);
        to_it=S.find(to);
        bloom_filter &t=to_it->second;
        if(from_it!=S.end()){
            if(Lastactive[from]-timestamp<=threshold){
                if(from_it->second.contains(to)){//find cycle
                    if(from_it->second.element_count()>1)//neglect<searchLimit
                        seed_ofs<<to<<" "<<timestamp<<" "<<from<<"\n";
                }
                t |= from_it->second; //union candidates
                t.update_element_count(from_it->second.element_count());
            }
            else    
                from_it->second.clear();//out of window
        }
        
        if(count%cleanupLimit==0){//clean up
            printf("count=%d\n", count);
            for(Lit=Lastactive.begin(); Lit!=Lastactive.end(); Lit++){
                if(Lit->second-timestamp>threshold){
                    S.erase(Lit->first);
                }
            }
        }
    }
    S.clear();
    Lastactive.clear();
}


template<class T>
void GenerateSeedsBloom<T>::GenerateSeeds_ascend(char* filename){
    std::ifstream ifs(filename);
    std::ifstream ifs_seed("seed.txt");
    std::ofstream ofs("seeds_bloom.txt");

    std::vector<std::pair<pii, T> > pre_seed;
    std::map<int, T> lastActive;//2 meanings
    typename std::map<int, T>::iterator Lit;
    std::map<int, std::map<Triad, short> > S;
    std::pair<typename std::map<Triad, short>::iterator, bool > ret;
    typename std::map<int, std::map<Triad, short> > ::iterator sit;
    typename std::map<Triad, short>::iterator tit;
    std::set<int> stk;
    std::set<int>::iterator stk_it;
    int root, neigh, from, to;
    T starttime, timestamp;

    //seed get before
    while(ifs_seed>>root>>starttime>>neigh){
        pre_seed.push_back(std::make_pair(std::make_pair(root, neigh), starttime));
    }
    ifs_seed.close();
    //
    int current_seed_pos=pre_seed.size()-1, used_seed_pos=-1, root_now, neigh_now;
    T time_now;
    int count=0;
    while(ifs>>from>>to>>timestamp){
        if(from==to) continue;
        count++;
        if(used_seed_pos!=current_seed_pos&&current_seed_pos>=0){
            used_seed_pos=current_seed_pos;
            root_now=pre_seed[used_seed_pos].first.first;
            neigh_now=pre_seed[used_seed_pos].first.second;
            time_now=pre_seed[used_seed_pos].second;
        }
        
        if(from==root_now&&timestamp==time_now&&to==neigh_now){//find
            lastActive[to]=timestamp;
            S[to].insert(std::make_pair(Triad(from,timestamp),1));
            current_seed_pos--;
        }
        else{
            if(lastActive.find(from)!=lastActive.end()){
                if(timestamp-lastActive[from]>threshold){
                    lastActive.erase(from);//can not be used as pre-node
                    S.erase(from);
                }
                else{
                    lastActive[to]=timestamp;
                    S[to].insert(std::make_pair(Triad(from,timestamp),1));
                }
            }
        }
        
        
        sit=S.find(from);
        if(sit!=S.end()){//update nodes and find cycle
            stk.clear();
            for(tit=sit->second.begin();tit!=sit->second.end();){
                Triad now=tit->first;
                short now_hop=tit->second;
                if(timestamp-now.time>threshold){//delete and break
                    S[from].erase(tit++);
                }
                else{
                    short new_hop=now_hop+1;
                    if(now.from==to){
                        if(stk.size()>0&&new_hop<=searchLimit){//output
                            ofs<<to<<" "<<now.time<<" "<<timestamp<<" "<<new_hop<<" "<<from;
                            for(stk_it=stk.begin(); stk_it!=stk.end(); stk_it++)
                                ofs<<" "<<*stk_it;
                            ofs<<" -1\n";
                        }
                    }
                    else{
                        if(new_hop<searchLimit){
                            ret=S[to].insert(std::make_pair(now, new_hop));
                            short hhop=ret.first->second;
                            if(!ret.second&&hhop>new_hop){//already exist
                                ret.first->second=new_hop;
                                hhop=new_hop;
                            }
                            if(hhop<=searchLimit)
                                stk.insert(now.from);
                        }
                    }

                    tit++;
                }
            }
        }
        //clean up
        if(count%cleanupLimit==0){
            std::vector<int> deletelist;
            for(sit=S.begin(); sit!=S.end(); sit++){
                if(sit->second.size()>0){
                    for(tit=sit->second.begin(); tit!=sit->second.end(); tit++){
                        if(timestamp-tit->first.time>threshold){
                            sit->second.erase(tit, sit->second.end());
                            break;
                        }
                    }
                }
                else
                    deletelist.push_back(sit->first);
            }
            for(int i=0; i<deletelist.size(); i++){
                S.erase(deletelist[i]);
            }
            printf("count=%d, erase %d\n", count, deletelist.size());
        }
        
    }
    S.clear();
    lastActive.clear();
    printf("current seed pos=%d\n", current_seed_pos);
}


template<class T>//method 1
void GenerateSeedsBloom<T>::query(char *filename){
    clock_t startTime,endTime;
    startTime = clock();
    std::ifstream query_ifs("C:\\vscode\\output\\query.txt");
    T window, maxwindow=0;
    short hop;
    query_ifs>>window>>hop;
    window*=3600;
    searchLimit=hop;
    query_ifs.close();
    threshold=window;
    std::cout<<"threshold="<<threshold<<" limit="<<searchLimit<<"\n";
    GetSeeds(filename);
    combine_tuples();
    endTime = clock();
    std::cout << "finding candidates time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
}


template<class T>
void GenerateSeedsBloom<T>::combine_tuples(){
    std::map<int, std::set<Quintuple> > combine_list;
    std::ifstream ifs("seeds_bloom.txt");
    int node, candidate;
    short hop;
    T ts, te;
    std::set<int> stk;
    while(ifs>>node){
        ifs>>ts>>te>>hop;
        stk.clear();
        while(ifs>>candidate){
            if(candidate==-1) break;
            stk.insert(candidate);
        }
        Quintuple q(ts, te, stk, hop);
        combine_list[node].insert(q);
    }
    ifs.close();

    std::ofstream quad_ofs("quad.txt");
    std::set<int > Call;
    std::set<int>::iterator set_it;
    std::set<int > ::iterator Call_it;
    typename std::set<Quintuple>::iterator it;
    std::string ans;

    std::set<Quintuple>* quadruples;
    std::set<int>::iterator stk_it;
    typename std::map<int, std::set<Quintuple> >::iterator q_it;
    typename std::map<short, T>::iterator qs_it;

    printf("thresh=%lld, hop=%d\n", threshold, searchLimit);
    int cnt=0;
    for(q_it=combine_list.begin(); q_it!=combine_list.end(); q_it++){
        int node=q_it->first;
        quadruples=&q_it->second;

        it=quadruples->begin();
        int maxlen=-1, quad_cnt=0;
        T maxlast, ts, te, tn;
        maxlast=0;

        while(it!=quadruples->end()){
            if(it->endtime-it->starttime>threshold){
                it++;continue;
            }
            if(it->hop>searchLimit){//within window, out of hop
                quadruples->erase(it++);
                continue;
            }

            if(maxlen==-1){//first one
                maxlen=0;
                quad_cnt=0;
                maxlast=it->starttime+threshold+1;
                ts=it->starttime;te=it->endtime;
                Call.clear();
            }
            if(it->endtime<maxlast){
                te=std::max(te, it->endtime);
                for(set_it=it->candidates.begin(); set_it!=it->candidates.end(); set_it++)
                    Call.insert(*set_it);
                quadruples->erase(it++);
                quad_cnt++;
            }
            else{
                quad_ofs<<node<<" "<<ts<<" "<<te<<" ";
                tn=it->starttime;
                quad_ofs<<tn;//<<" "<<maxlen;
                maxlen=-1;
                for(stk_it=Call.begin(); stk_it!=Call.end(); stk_it++)
                    quad_ofs<<" "<<*stk_it;
                quad_ofs<<" -1\n";
                cnt++;
            }
        }
        tn=ts+threshold+1;
        if(maxlen!=-1){
            quad_ofs<<node<<" "<<ts<<" "<<te<<" "<<tn;//<<" "<<maxlen;
            for(stk_it=Call.begin(); stk_it!=Call.end(); stk_it++)
                quad_ofs<<" "<<*stk_it;
            quad_ofs<<" -1\n";
            cnt++;
        }
    }
    quad_ofs.close();
    
}

#endif