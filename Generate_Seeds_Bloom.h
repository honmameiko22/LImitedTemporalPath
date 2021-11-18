#ifndef GENERATE_SEEDS_BLOOM_H_
#define GENERATE_SEEDS_BLOOM_H_

/***
insure the input edges are ordered ascending
all nodes are tagged from 1 to V
***/

#include<bits/stdc++.h>
#include <tr1/unordered_map>



template<class T>
class GenerateSeeds{

    private:
        struct Quintuple{
            T starttime, endtime;
            short hop;
            std::set<int> candidates;//candidates are points
            Quintuple(T starttime, T endtime, std::set<int> &candidates, int from, short hop){
                this->starttime=starttime;
                this->endtime=endtime;
                this->hop=hop;
                this->candidates=candidates;
                if(from>0)
                    this->candidates.insert(from);
            }
            bool operator<(const struct Quintuple & right) const{
                if(this->starttime!=right.starttime)
                    return this->starttime<right.starttime;
                return this->endtime>right.endtime;
            }
            Quintuple(){}
        };
        
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


        T threshold;
        std::map<int, std::set<Quintuple> > Quad_Head;
        /**control*/
        bool ofs_on=true;
        int cleanupLimit=10000;
        int quadLimit=1000;
        short SearchLimit=2;
        //


    public:
        //set some required values
        bool set_threshold(T w);
        bool set_interaction_exact(std::istream &ifs);
        bool combine();
        void GenerateSeedsBloom();
        void query(char *file);
        virtual ~GenerateSeeds() {
        }

};

/*****************/

template<class T>
bool GenerateSeeds<T>::set_threshold(T w){
    threshold=w;
    return true;
}



//store all quadruple<node, start, end, set_of_node>
template<class T>
bool GenerateSeeds<T>::set_interaction_exact(std::istream &ifs){
    std::map<int, std::map<Triad, short> > S;
    std::pair<typename std::map<Triad, short>::iterator, bool > ret;
    std::map<T, std::set<int> > mint_record;//min_time of node , node
    typename std::map<Triad, short>::iterator it, itt;
    std::cout<<"computing cycles"<<std::endl;
    std::set<int> stk;
    std::set<int>::iterator stk_it;
    printf("start!\n");
    clock_t startTime,endTime;
    startTime = clock();
    int count=0, selfloop=0;
    int from, to;
    T timestamp;
    int negle=0;

    while(ifs>>from>>to>>timestamp){ 
        if(from==to){
            selfloop++;continue;
        }

        T time_now=timestamp;
        Triad t(from, time_now);
        S[to].insert(std::make_pair(t, 1));
        int havecycle=0;
        if(S.count(from)>0){//update
            stk.clear(); 
            T minT=0;
            int havecycle=0;
            for(it=S[from].begin(); it!=S[from].end(); ){
                Triad now=it->first;
                short now_hop=it->second;
                if(time_now-now.time>threshold){
                    S[from].erase(it++);
                    if(minT){//the last timestamp that is not erased
                        mint_record[minT].insert(from);
                        minT=0;
                    }
                }
                else{
                    short new_hop=now_hop+1;
                    if(now.from==to){ 
                        if(stk.size()>0&&new_hop<=SearchLimit){
                            Quintuple q(now.time, time_now, stk, from, new_hop);
                            Quad_Head[to].insert(q);
                        }
                    }
                    else{
                        if(new_hop<SearchLimit){
                            ret=S[to].insert(std::make_pair(now, new_hop));
                            short hhop=ret.first->second;
                            if(!ret.second&&hhop>new_hop){//already exist
                                ret.first->second=new_hop;
                                hhop=new_hop;
                            }
                            if(hhop<=SearchLimit)
                                stk.insert(now.from);
                        }
                        else negle++;
                    }
                    it++;
                    minT=now.time;
                }
            }
        }
        count++;
        
        if(count%cleanupLimit==0){//do clean up
            T deletetime=timestamp-threshold;
            typename std::map<T, std::set<int> >::iterator mint_it;
            typename std::set<int>::iterator mint_set;
            typename std::map<Triad, short>::reverse_iterator triad_it;
            int deletecnt=0;
            for(mint_it=mint_record.begin(); mint_it!=mint_record.end(); mint_it++){
                if(mint_it->first>deletetime) break;
                for(mint_set=mint_it->second.begin(); mint_set!=mint_it->second.end(); mint_set++){
                    int node_now=*mint_set;
                    for(triad_it=S[node_now].rbegin(); triad_it!=S[node_now].rend(); triad_it++){
                        if((triad_it->first).time>deletetime) break;
                        else S[node_now].erase((++triad_it).base());deletecnt++;
                    }
                    
                }   
            }
            mint_record.clear();
            printf("count%d , delete%d\n",count , deletecnt);
        }
        
    }
    printf("nwgle=%d\n", negle);
    S.clear();
    int nd;
    endTime = clock();
    std::cout << "finding candidates time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    combine();
    endTime = clock();
    std::cout << "stage1 time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    return true;
}



template<class T>//method 1
void GenerateSeeds<T>::query(char *filename){
    std::ifstream query_ifs("C:\\vscode\\output\\query.txt");
    T window, maxwindow=0;
    short hop;
    query_ifs>>window>>hop;
    window*=3600;
    SearchLimit=hop;
    query_ifs.close();
    threshold=window;
    std::ifstream ifs(filename);
    set_interaction_exact(ifs);
    ifs.close();
}



template<class T>
bool GenerateSeeds<T>::combine(){         

    std::ofstream quad_ofs("quad.txt");
    std::set<int > Call, stk;
    std::set<int>::iterator set_it;
    std::set<int > ::iterator Call_it;
    typename std::set<Quintuple>::iterator it;
    std::string ans;

    std::set<Quintuple>* quadruples;
    std::set<int>::iterator stk_it;
    typename std::map<int, std::set<Quintuple> >::iterator q_it;

    std::cout<<"thresh="<<threshold<<" "<<"hop="<<SearchLimit<<"\n";
    for(q_it=Quad_Head.begin(); q_it!=Quad_Head.end(); q_it++){
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
            if(it->hop>SearchLimit){//within window, out of hop
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
                quad_ofs<<tn;
                maxlen=-1;
                for(stk_it=Call.begin(); stk_it!=Call.end(); stk_it++)
                    quad_ofs<<" "<<*stk_it;
                quad_ofs<<" -1\n";
            }
        }
        tn=ts+threshold+1;
        if(maxlen!=-1){
            quad_ofs<<node<<" "<<ts<<" "<<te<<" "<<tn;
            for(stk_it=Call.begin(); stk_it!=Call.end(); stk_it++)
                quad_ofs<<" "<<*stk_it;
            quad_ofs<<" -1\n";
        }
    }

    quad_ofs.close();
}


#endif // GENERATESEEDSBLOOM_H_

