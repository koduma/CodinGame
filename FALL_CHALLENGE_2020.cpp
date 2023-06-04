#pragma GCC optimize("O3","unroll-loops","omit-frame-pointer","inline")
#pragma GCC option("arch=native","tune=native","no-zero-upper")
#pragma GCC target("avx")
#include <vector>
#include <cfloat>
#include <cstdio>
#include <cstring>
#include <climits>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <string>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <cassert>
#include <random>
#include <queue>
#include <deque>
#include <list>
#include <map>
#include <array>
#include <chrono>
#include <fstream>
#include <functional>
#include <unordered_map>

using namespace std;

typedef unsigned long long ll;

chrono::system_clock::time_point start;

double learn_premium=0.1;
double rest_premium=0.1;
double cast_premium=1.5;
double brew_premium=2.0;
double bonus_premium=1.0;
int ttt;

struct item{
    int actionId;
    int delta0;
    int delta1;
    int delta2;
    int delta3;
    int price;
    bool castable;
    bool repeatable;
    bool learnable;
    int tomeIndex;
    int taxCount;
    string actionType;
};

struct player{
    int inv0;
    int inv1;
    int inv2;
    int inv3;
    int score;
};
struct Action {
	double score;
    string command;
};

struct GameState{
vector<item>brew_list;
vector<item>cast_list;
vector<item>learn_list;
player player_list[2];
double score;
int inventory[2];
string ans;
double value;
int brewed;
int my_score;
bool operator >(const GameState &n) const {
    return value < n.value;
  }
};

int okbrew=0;

unsigned int rnd(int mini, int maxi) {
	static mt19937 mt((int)time(0));
	uniform_int_distribution<int> dice(mini, maxi);
	return dice(mt);
}

ll xor128() {
	static unsigned long long rx = 123456789, ry = 362436069, rz = 521288629, rw = 88675123;
	ll rt = (rx ^ (rx << 11));
	rx = ry; ry = rz; rz = rw;
	return (rw = (rw ^ (rw >> 19)) ^ (rt ^ (rt >> 8)));
}

bool canbrew(GameState *state,int i){
        if(state->brew_list[i].castable){
            if(-state->brew_list[i].delta0<=state->player_list[0].inv0){
                if(-state->brew_list[i].delta1<=state->player_list[0].inv1){
                    if(-state->brew_list[i].delta2<=state->player_list[0].inv2){
                        if(-state->brew_list[i].delta3<=state->player_list[0].inv3){
                            return true;
                        }
                    }
                }
            }
        }
    return false;
}
bool cancast(GameState *state,int i){
        if(state->cast_list[i].castable){
            if(state->player_list[0].inv0+state->cast_list[i].delta0<0){
                return false;
            }
            if(state->player_list[0].inv1+state->cast_list[i].delta1<0){
                return false;
            }
            if(state->player_list[0].inv2+state->cast_list[i].delta2<0){
                return false;
            }
            if(state->player_list[0].inv3+state->cast_list[i].delta3<0){
                return false;
            }
            int m=state->inventory[0]+state->cast_list[i].delta0+state->cast_list[i].delta1+state->cast_list[i].delta2+state->cast_list[i].delta3;
            if(m<0||m>10){
                return false;
            }
            else{
                return true;
            }
        }
    return false;
}
bool canlearn(GameState *state,int i){
        if(state->learn_list[i].learnable&&state->player_list[0].inv0>=state->learn_list[i].tomeIndex){
            return true;
        }
    return false;
}
bool iam_checkmate(GameState *state){
    for(int i=0;i<(int)state->cast_list.size();i++){
        if(state->player_list[0].inv0+state->cast_list[i].delta0<0){
            continue;
        }
        if(state->player_list[0].inv1+state->cast_list[i].delta1<0){
            continue;
        }
        if(state->player_list[0].inv2+state->cast_list[i].delta2<0){
            continue;
        }
        if(state->player_list[0].inv3+state->cast_list[i].delta3<0){
            continue;
        }
        int m=state->inventory[0]+state->cast_list[i].delta0+state->cast_list[i].delta1+state->cast_list[i].delta2+state->cast_list[i].delta3;
        if(m<0||m>10){
            continue;
        }
        return false;
    }
    return true;
}
void do_learn(GameState *state,int j){
    state->player_list[0].inv0-=state->learn_list[j].tomeIndex;
    state->inventory[0]-=state->learn_list[j].tomeIndex;
    state->inventory[0]+=state->learn_list[j].taxCount;
    state->player_list[0].inv0+=state->learn_list[j].taxCount;
    state->score-=learn_premium*((double)state->learn_list[j].tomeIndex);
    state->score+=learn_premium*((double)state->learn_list[j].taxCount);
    state->score+=learn_premium*((double)state->learn_list[j].delta0);
    state->score+=learn_premium*2.0*((double)state->learn_list[j].delta1);
    state->score+=learn_premium*3.0*((double)state->learn_list[j].delta2);
    state->score+=learn_premium*4.0*((double)state->learn_list[j].delta3);
    while(1){
        if(state->inventory[0]<=10){break;}
        state->inventory[0]=(state->inventory[0])-1;
        state->player_list[0].inv0=(state->player_list[0].inv0)-1;
    }
    //tax
    for(int i=0;i<(int)state->learn_list.size();i++){
        if(state->learn_list[i].tomeIndex<state->learn_list[j].tomeIndex){
            state->learn_list[i].taxCount=(state->learn_list[i].taxCount)+1;
        }
    }
    //train
    for(int i=0;i<(int)state->learn_list.size();i++){
        if(state->learn_list[i].tomeIndex>=state->learn_list[j].tomeIndex+1){
            state->learn_list[i].tomeIndex=(state->learn_list[i].tomeIndex)-1;
        }
    }

    item it;

    it.actionId=state->learn_list[j].actionId;
    it.price=state->learn_list[j].price;
    it.delta0=state->learn_list[j].delta0;
    it.delta1=state->learn_list[j].delta1;
    it.delta2=state->learn_list[j].delta2;
    it.delta3=state->learn_list[j].delta3;
    it.castable=true;
    it.repeatable=state->learn_list[j].repeatable;
    it.learnable=false;
    it.taxCount=-1;
    it.tomeIndex=-1;

    state->learn_list[j].learnable=false;

    state->cast_list.push_back(it);
}
void do_brew(GameState *state,int i){
    state->inventory[0]+=state->brew_list[i].delta0;
    state->inventory[0]+=state->brew_list[i].delta1;
    state->inventory[0]+=state->brew_list[i].delta2;
    state->inventory[0]+=state->brew_list[i].delta3;
    state->player_list[0].inv0+=state->brew_list[i].delta0;
    state->player_list[0].inv1+=state->brew_list[i].delta1;
    state->player_list[0].inv2+=state->brew_list[i].delta2;
    state->player_list[0].inv3+=state->brew_list[i].delta3;
    state->brew_list[i].castable=false;

    if(state->brew_list[i].taxCount>0){
        state->score+=bonus_premium*((double)state->brew_list[i].tomeIndex);
        state->brew_list[i].taxCount=0;
    }
}
void do_cast(GameState *state,int i){
    state->inventory[0]+=state->cast_list[i].delta0;
    state->inventory[0]+=state->cast_list[i].delta1;
    state->inventory[0]+=state->cast_list[i].delta2;
    state->inventory[0]+=state->cast_list[i].delta3;
    state->player_list[0].inv0+=state->cast_list[i].delta0;
    state->player_list[0].inv1+=state->cast_list[i].delta1;
    state->player_list[0].inv2+=state->cast_list[i].delta2;
    state->player_list[0].inv3+=state->cast_list[i].delta3;
    state->score+=cast_premium*(double)state->cast_list[i].delta0;
    state->score+=cast_premium*2.0*((double)state->cast_list[i].delta1);
    state->score+=cast_premium*3.0*((double)state->cast_list[i].delta2);
    state->score+=cast_premium*4.0*((double)state->cast_list[i].delta3);
    state->cast_list[i].castable=false;
}
void do_repeat_cast(GameState *state,int i){
    state->inventory[0]+=state->cast_list[i].delta0;
    state->inventory[0]+=state->cast_list[i].delta1;
    state->inventory[0]+=state->cast_list[i].delta2;
    state->inventory[0]+=state->cast_list[i].delta3;
    state->player_list[0].inv0+=state->cast_list[i].delta0;
    state->player_list[0].inv1+=state->cast_list[i].delta1;
    state->player_list[0].inv2+=state->cast_list[i].delta2;
    state->player_list[0].inv3+=state->cast_list[i].delta3;
    state->score+=cast_premium*(double)state->cast_list[i].delta0;
    state->score+=cast_premium*2.0*((double)state->cast_list[i].delta1);
    state->score+=cast_premium*3.0*((double)state->cast_list[i].delta2);
    state->score+=cast_premium*4.0*((double)state->cast_list[i].delta3);
}
void do_rest(GameState *state){
    for(int i=0;i<(int)state->cast_list.size();i++){
    if(!state->cast_list[i].castable){
    state->cast_list[i].castable=true;
    state->score+=rest_premium;
    }
    }
}
Action BEAM_SEARCH(GameState state,double timelimit){

    int BEAM_WIDTH=100;

    /*
    not_ev=>{
    beam_width=20:
    beam_width=50:
    beam_width=100,old:new=26:30
    beam_width=150,old:new=30:25
    beam_width=200:
    beam_width=300:
    beam_width=400:
    beam_width=500:
    }
    ev=>{
    beam_width=100,old:new=

    }

    */

    int SEARCH_DEPTH=100;//100

    double ev=-DBL_MAX;

    Action bestAction;

    bestAction.command="";
    bestAction.score=ev;

    queue<GameState>que;

    state.score=0;
    state.ans="";
    que.push(state);

    for(int depth=0;depth<SEARCH_DEPTH;depth++){

        priority_queue<GameState, vector<GameState>, greater<GameState> > pque;
        
        while (!que.empty()) {
            GameState s=que.front();que.pop();
            double elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;
            if(elapsed>=timelimit){
                cerr<<"depth="<<depth<<endl;
                return bestAction;
            }
                vector<pair<int,int> >legal_brew_action;
                vector<pair<int,int> >legal_cast_action;
                vector<pair<int,int> >legal_learn_action;
                vector<pair<int,int> >legal_repeat_cast_action;
                for(int i=0;i<(int)s.brew_list.size();i++){
                    if(canbrew(&s, i)){
                        legal_brew_action.push_back(make_pair(s.brew_list[i].actionId,i));
                    }
                }
                for(int i=0;i<(int)s.cast_list.size();i++){
                    if(cancast(&s,i)&&(!s.cast_list[i].repeatable)){
                        legal_cast_action.push_back(make_pair(s.cast_list[i].actionId,i));
                    }
                }
                for(int i=0;i<(int)s.learn_list.size();i++){
                    if(canlearn(&s,i)){
                        legal_learn_action.push_back(make_pair(s.learn_list[i].actionId,i));
                    }
                }
                for(int i=0;i<(int)s.cast_list.size();i++){
                    if(cancast(&s, i)&&(s.cast_list[i].repeatable)){
                        legal_repeat_cast_action.push_back(make_pair(s.cast_list[i].actionId,i));
                    }
                }
                GameState tmp=s;
                if((int)legal_brew_action.size()>0){
                    for(int a=0;a<(int)legal_brew_action.size();a++){
                        //brew
                        do_brew(&s, legal_brew_action[a].second);
                        if(depth==0){
                        s.ans="BREW "+to_string(legal_brew_action[a].first);
                        }
                        double p=(double)tmp.brew_list[legal_brew_action[a].second].price;
                        s.score+=((p)*brew_premium)/(double)(depth+1);
                        s.player_list[0].score+=p;
                        s.value=s.score;
                        s.brewed++;
                        if(s.brewed+okbrew>=6&&s.player_list[0].score>=70){s.value+=10000/(double)(depth+1);}
                        pque.push(s);
                        s=tmp;
                    }
                }
                else{
                    int x=(int)legal_cast_action.size();
                    int y=(int)legal_learn_action.size();
                    int z=(int)legal_repeat_cast_action.size();
                    if((x==0)&&(y==0)&&(z==0)){//1,ok,000
                    //rest
                    do_rest(&s);
                    if(depth==0){
                    s.ans="REST";
                    }
                    s.value=s.score;
                    if(iam_checkmate(&s)){
                        s.value=-DBL_MAX;
                    }
                    else{
                    pque.push(s);
                    }
                    }
                    else if((x==0)&&(y==0)&&(z>0)){//2,ok,001
                    //rest
                    do_rest(&s);
                    if(depth==0){
                    s.ans="REST";
                    }
                    s.value=s.score;
                    pque.push(s);
                    //repeat
                    for(int a=0;a<z;a++){
                    s=tmp;
                    for(int i=1;i<=5;i++){
                        if(cancast(&s,legal_repeat_cast_action[a].second)){
                            do_repeat_cast(&s,legal_repeat_cast_action[a].second);
                            if(depth==0){
                            s.ans="CAST "+to_string(legal_repeat_cast_action[a].first)+" "+to_string(i);
                            }
                            s.cast_list[legal_repeat_cast_action[a].second].castable=false;
                            s.value=s.score;
                            pque.push(s);
                            s.cast_list[legal_repeat_cast_action[a].second].castable=true;
                        }
                        else{
                            break;
                        }
                    }
                    }
                    }
                    else if((x==0)&&(y>0)&&(z==0)){//3,ok,010
                    //rest
                    do_rest(&s);
                    if(depth==0){
                    s.ans="REST";
                    }
                    s.value=s.score;
                    pque.push(s);
                    //learn
                    for(int a=0;a<y;a++){
                    s=tmp;
                    do_learn(&s,legal_learn_action[a].second);
                    if(depth==0){
                    s.ans="LEARN "+to_string(legal_learn_action[a].first);
                    }
                    s.value=s.score;
                    pque.push(s);
                    }
                    }
                    else if((x==0)&&(y>0)&&(z>0)){//4,ok,011
                    //rest
                    do_rest(&s);
                    if(depth==0){
                    s.ans="REST";
                    }
                    s.value=s.score;
                    pque.push(s);
                    //learn
                    for(int a=0;a<y;a++){
                    s=tmp;
                    do_learn(&s,legal_learn_action[a].second);
                    if(depth==0){
                    s.ans="LEARN "+to_string(legal_learn_action[a].first);
                    }
                    s.value=s.score;
                    pque.push(s);
                    }
                    //repeat
                    for(int a=0;a<z;a++){
                    s=tmp;
                    for(int i=1;i<=5;i++){
                        if(cancast(&s,legal_repeat_cast_action[a].second)){
                            do_repeat_cast(&s,legal_repeat_cast_action[a].second);
                            if(depth==0){
                            s.ans="CAST "+to_string(legal_repeat_cast_action[a].first)+" "+to_string(i);
                            }
                            s.cast_list[legal_repeat_cast_action[a].second].castable=false;
                            s.value=s.score;
                            pque.push(s);
                            s.cast_list[legal_repeat_cast_action[a].second].castable=true;
                        }
                        else{
                            break;
                        }
                    }
                    }

                    }
                    else if((x>0)&&(y==0)&&(z==0)){//5,ok,100
                    //rest
                    do_rest(&s);
                    if(depth==0){
                    s.ans="REST";
                    }
                    s.value=s.score;
                    pque.push(s);
                    //cast
                    for(int a=0;a<x;a++){
                        s=tmp;
                        do_cast(&s, legal_cast_action[a].second);
                        if(depth==0){
                        s.ans="CAST "+to_string(legal_cast_action[a].first);
                        }
                        s.value=s.score;
                        pque.push(s);
                    }
                    }
                    else if((x>0)&&(y==0)&&(z>0)){//6,ok,101
                    //rest
                    do_rest(&s);
                    if(depth==0){
                    s.ans="REST";
                    }
                    s.value=s.score;
                    pque.push(s);
                    //cast
                    for(int a=0;a<x;a++){
                        s=tmp;
                        do_cast(&s, legal_cast_action[a].second);
                        if(depth==0){
                        s.ans="CAST "+to_string(legal_cast_action[a].first);
                        }
                        s.value=s.score;
                        pque.push(s);
                    }
                    //repeat
                    for(int a=0;a<z;a++){
                    s=tmp;
                    for(int i=1;i<=5;i++){
                        if(cancast(&s,legal_repeat_cast_action[a].second)){
                            do_repeat_cast(&s,legal_repeat_cast_action[a].second);
                            if(depth==0){
                            s.ans="CAST "+to_string(legal_repeat_cast_action[a].first)+" "+to_string(i);
                            }
                            s.cast_list[legal_repeat_cast_action[a].second].castable=false;
                            s.value=s.score;
                            pque.push(s);
                            s.cast_list[legal_repeat_cast_action[a].second].castable=true;
                        }
                        else{
                            break;
                        }
                    }
                    }
                    }
                    else if((x>0)&&(y>0)&&(z==0)){//7,ok,110
                    //rest
                    do_rest(&s);
                    if(depth==0){
                    s.ans="REST";
                    }
                    s.value=s.score;
                    pque.push(s);
                    //cast
                    for(int a=0;a<x;a++){
                        s=tmp;
                        do_cast(&s, legal_cast_action[a].second);
                        if(depth==0){
                        s.ans="CAST "+to_string(legal_cast_action[a].first);
                        }
                        s.value=s.score;
                        pque.push(s);
                    }
                    //learn
                    for(int a=0;a<y;a++){
                    s=tmp;
                    do_learn(&s,legal_learn_action[a].second);
                    if(depth==0){
                    s.ans="LEARN "+to_string(legal_learn_action[a].first);
                    }
                    s.value=s.score;
                    pque.push(s);
                    }
                    }
                    else{//8,ok,111
                    //rest
                    do_rest(&s);
                    if(depth==0){
                    s.ans="REST";
                    }
                    s.value=s.score;
                    pque.push(s);
                    //cast
                    for(int a=0;a<x;a++){
                        s=tmp;
                        do_cast(&s, legal_cast_action[a].second);
                        if(depth==0){
                        s.ans="CAST "+to_string(legal_cast_action[a].first);
                        }
                        s.value=s.score;
                        pque.push(s);
                    }
                    //learn
                    for(int a=0;a<y;a++){
                    s=tmp;
                    do_learn(&s,legal_learn_action[a].second);
                    if(depth==0){
                    s.ans="LEARN "+to_string(legal_learn_action[a].first);
                    }
                    s.value=s.score;
                    pque.push(s);
                    }
                    //repeat
                    for(int a=0;a<z;a++){
                    s=tmp;
                    for(int i=1;i<=5;i++){
                        if(cancast(&s,legal_repeat_cast_action[a].second)){
                            do_repeat_cast(&s,legal_repeat_cast_action[a].second);
                            if(depth==0){
                            s.ans="CAST "+to_string(legal_repeat_cast_action[a].first)+" "+to_string(i);
                            }
                            s.cast_list[legal_repeat_cast_action[a].second].castable=false;
                            s.value=s.score;
                            pque.push(s);
                            s.cast_list[legal_repeat_cast_action[a].second].castable=true;
                        }
                        else{
                            break;
                        }
                    }
                }
            }//8,ok,111
        }//not_brew
        }//while

        int node_cnt=0;
        for (int j = 0; node_cnt < BEAM_WIDTH && !pque.empty(); j++) {
            double elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;
            if(elapsed>=timelimit){
                cerr<<"depth="<<depth<<endl;
                return bestAction;
            }
            GameState node=pque.top(); pque.pop();

            if(ttt>=7){
            if(node.value>ev){
                ev=node.value;
                bestAction.command=node.ans;
                bestAction.score=ev;
            }
            }
            else if(node.ans[0]=='L'){
            if(node.value>ev){
                ev=node.value;
                bestAction.command=node.ans;
                bestAction.score=ev;
            }    
            }
            if (depth < SEARCH_DEPTH-1) {
                que.push(node);
                node_cnt++;
            }
        }
    }//depth

    return bestAction;
}

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main()
{

    // game loop
    for (ttt=0;;ttt++) {
        GameState state;
        state.brewed=0;
        int actionCount; // the number of spells and recipes in play
        cin >> actionCount; cin.ignore();
        for (int i = 0; i < actionCount; i++) {
            int actionId; // the unique ID of this spell or recipe
            string actionType; // in the first league: BREW; later: CAST, OPPONENT_CAST, LEARN, BREW
            int delta0; // tier-0 ingredient change
            int delta1; // tier-1 ingredient change
            int delta2; // tier-2 ingredient change
            int delta3; // tier-3 ingredient change
            int price; // the price in rupees if this is a potion
            int tomeIndex; // in the first two leagues: always 0; later: the index in the tome if this is a tome spell, equal to the read-ahead tax
            int taxCount; // in the first two leagues: always 0; later: the amount of taxed tier-0 ingredients you gain from learning this spell
            bool castable; // in the first league: always 0; later: 1 if this is a castable player spell
            bool repeatable; // for the first two leagues: always 0; later: 1 if this is a repeatable player spell
            cin >> actionId >> actionType >> delta0 >> delta1 >> delta2 >> delta3 >> price >> tomeIndex >> taxCount >> castable >> repeatable; cin.ignore();
            item it;
            if(actionType!="OPPONENT_CAST"){
            //cerr<<"actiontype="<<actionType<<",actionId="<<actionId<<",tomeIndex="<<tomeIndex<<",taxCount="<<taxCount<<endl;
            it.actionId=actionId;
            it.delta0=delta0;
            it.delta1=delta1;
            it.delta2=delta2;
            it.delta3=delta3;
            it.price=price;
            it.repeatable=repeatable;
            it.taxCount=taxCount;
            it.tomeIndex=tomeIndex;
            if(actionType=="BREW"){
            it.castable=true;
            it.learnable=false;
            }
            else if(actionType=="LEARN"){
            it.castable=false;
            it.learnable=true;
            }
            else{//cast
            it.castable=castable;
            it.learnable=false;
            }
            it.actionType=actionType;
            if(actionType=="BREW"){
            state.brew_list.push_back(it);
            }
            else if(actionType=="CAST"){
            state.cast_list.push_back(it);
            }
            else{
            state.learn_list.push_back(it);
            }
            }
        }

        for (int i = 0; i < 2; i++) {
            int inv0; // tier-0 ingredients in inventory
            int inv1;
            int inv2;
            int inv3;
            int score; // amount of rupees
            cin >> inv0 >> inv1 >> inv2 >> inv3 >> score; cin.ignore();
            player ps;
            ps.inv0=inv0;
            ps.inv1=inv1;
            ps.inv2=inv2;
            ps.inv3=inv3;
            ps.score=score;
            state.inventory[i]=inv0+inv1+inv2+inv3;
            state.player_list[i]=ps;
        }

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        start = chrono::system_clock::now();

        Action ret;

        if(ttt==0){
            ret=BEAM_SEARCH(state, 0.9);
        }
        else{
            ret=BEAM_SEARCH(state, 0.04);
        }
        
        if(ret.command==""){
        cout<<"WAIT"<<endl;
        }
        else{
        if(ret.command[0]=='B'){okbrew++;}     
        cout<<ret.command<<endl;
        }

        cerr<<"score="<<ret.score<<endl;

        // in the first league: BREW <id> | WAIT; later: BREW <id> | CAST <id> [<times>] | LEARN <id> | REST | WAIT

        double elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;

        cerr<<"elapsed="<<elapsed<<endl;
    }
    return 0;
}
