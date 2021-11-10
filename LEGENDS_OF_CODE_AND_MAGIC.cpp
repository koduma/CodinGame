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

typedef unsigned long long ull;
#define ADD_CARD 10

chrono::system_clock::time_point start;

struct card{
     int cardNumber;
     int instanceId;
     int location;
     int cardType;
     int cost;
     int attack;
     int defense;
     string abilities;
     int myHealthChange;
     int opponentHealthChange;
     int cardDraw;
     int is_summon;
     int is_attack;
 };

 struct GameState{
     int status;
     int first;
     int value;
     int playerHealth[2];
     int playerMana[2];
     int playerDeck[2];
     int playerRune[2];
     int playerDraw[2];
     int playerHandsize[2];
     int playerMaxMana[2];
     int playerBoardsize[2];
     string action;
     vector<card> mycreature[2];//hand
     vector<card> myitem[2];//hand
     vector<card> myboard[2];//board
 };

ull xor128() {
	static unsigned long long rx = 123456789, ry = 362436069, rz = 521288629, rw = 88675123;
	ull rt = (rx ^ (rx << 11));
	rx = ry; ry = rz; rz = rw;
	return (rw = (rw ^ (rw >> 19)) ^ (rt ^ (rt >> 8)));
}

unsigned int xorshift(){ 
    static unsigned int xx = 123456789;
    static unsigned int yy = 362436069;
    static unsigned int zz = 521288629;
    static unsigned int ww = 88675123; 
    unsigned int tt;
    tt = xx ^ (xx << 11);
    xx = yy; yy = zz; zz = ww;
    return ww = (ww ^ (ww >> 19)) ^ (tt ^ (tt >> 8)); 
}
int evalScore(GameState state,int player){

    if(state.playerHealth[player]<=0){return -INT_MAX;}
    if(state.playerHealth[1-player]<=0){return INT_MAX;}

    int score=0;
    score+=(state.playerHealth[player]-state.playerHealth[1-player])*2;
    //score+=state.playerHandsize[player]-state.playerHandsize[1-player];
    //score+=state.playerDeck[player]-state.playerDeck[1-player];
    for(int i=0;i<(int)state.myboard[player].size();i++){
        int attack=state.myboard[player][i].attack;
        int defense=state.myboard[player][i].defense;
        int abilities=0;
        if(state.myboard[player][i].abilities[0]=='B'){abilities+=1;}
        if(state.myboard[player][i].abilities[1]=='C'){abilities+=2;}
        if(state.myboard[player][i].abilities[2]=='D'){abilities+=3;}
        if(state.myboard[player][i].abilities[3]=='G'){abilities+=3;}
        if(state.myboard[player][i].abilities[4]=='L'){abilities+=4;}
        if(state.myboard[player][i].abilities[5]=='W'){abilities+=attack;}
        if(defense>0){
        score+=(attack+defense+abilities)*5;
        }
    }
    for(int i=0;i<(int)state.myboard[1-player].size();i++){
        int attack=state.myboard[1-player][i].attack;
        int defense=state.myboard[1-player][i].defense;
        int abilities=0;
        if(state.myboard[1-player][i].abilities[0]=='B'){abilities+=1;}
        if(state.myboard[1-player][i].abilities[1]=='C'){abilities+=2;}
        if(state.myboard[1-player][i].abilities[2]=='D'){abilities+=3;}
        if(state.myboard[1-player][i].abilities[3]=='G'){abilities+=3;}
        if(state.myboard[1-player][i].abilities[4]=='L'){abilities+=4;}
        if(state.myboard[1-player][i].abilities[5]=='W'){abilities+=attack;}
        if(defense>0){
        score-=(attack+defense+abilities)*5;
        }
    }

    return score;
}

bool can_attack(GameState state,int player,int Attacker_num,int Target_num){
    if(state.myboard[player][Attacker_num].defense<=0){return false;}

    bool foundGuard=false;

    for(int i=0;i<(int)state.myboard[1-player].size();i++){
        char G=state.myboard[1-player][i].abilities[3];
        if(G=='G'&&state.myboard[1-player][i].defense>0){
            foundGuard=true;
            break;
        }
    }
    if(!foundGuard){
        if(Target_num==-1){
        return true;
        }
        if(state.myboard[1-player][Target_num].defense>0){
            return true;
        }
        return false;
    }
    else{
        if(Target_num==-1){
            return false;
        }
       char G=state.myboard[1-player][Target_num].abilities[3];
       if(G=='G'&&state.myboard[1-player][Target_num].defense>0){return true;}
       else{return false;}
    }
}

GameState do_attack(GameState state,int player,int Attacker_num,int Target_num){
    if(state.myboard[player][Attacker_num].defense<=0){return state;}
    if(Target_num==-1){//face_attack
        state.playerHealth[1-player]-=state.myboard[player][Attacker_num].attack;
        if(state.myboard[player][Attacker_num].abilities[2]=='D'){state.playerHealth[player]+=state.myboard[player][Attacker_num].attack;}
        if(state.playerHealth[1-player]<=25&&state.playerRune[1-player]>4){state.playerDraw[1-player]++;state.playerRune[1-player]--;}
        if(state.playerHealth[1-player]<=20&&state.playerRune[1-player]>3){state.playerDraw[1-player]++;state.playerRune[1-player]--;}
        if(state.playerHealth[1-player]<=15&&state.playerRune[1-player]>2){state.playerDraw[1-player]++;state.playerRune[1-player]--;}
        if(state.playerHealth[1-player]<=10&&state.playerRune[1-player]>1){state.playerDraw[1-player]++;state.playerRune[1-player]--;}
        if(state.playerHealth[1-player]<=5&&state.playerRune[1-player]>0){state.playerDraw[1-player]++;state.playerRune[1-player]--;}
    }
    else{
        if(state.myboard[1-player][Target_num].defense<=0){return state;}
        if(state.myboard[1-player][Target_num].abilities[5]=='W'){//enemy->Ward
            state.myboard[1-player][Target_num].abilities[5]='-';
            if(state.myboard[player][Attacker_num].abilities[5]=='W'){//me->Ward
                state.myboard[player][Attacker_num].abilities[5]='-';
                return state;
            }
            else{
                 if(state.myboard[1-player][Target_num].abilities[4]=='L'){//enemy->lethal,ward,me->not_ward
                     state.myboard[player][Attacker_num].defense=0;
                     state.playerBoardsize[player]--;
                     return state;
                 }
                 else{//enemy->Ward,not_lethal,me->none
                 state.myboard[player][Attacker_num].defense-=state.myboard[1-player][Target_num].attack;
                 if(state.myboard[player][Attacker_num].defense<=0){
                 state.myboard[player][Attacker_num].defense=0;
                 state.playerBoardsize[player]--;
                 }
                 return state;
                 }
            }
        }
        else{//enemy->not_ward
        if(state.myboard[player][Attacker_num].abilities[5]=='W'){//me->Ward
            state.myboard[player][Attacker_num].abilities[5]='-';
            state.myboard[1-player][Target_num].defense-=state.myboard[player][Attacker_num].attack;
            if(state.myboard[player][Attacker_num].abilities[0]=='B'){
            if(state.myboard[1-player][Target_num].defense<0){state.playerHealth[1-player]+=state.myboard[1-player][Target_num].defense;}
            }
            if(state.myboard[player][Attacker_num].abilities[2]=='D'){
                state.playerHealth[player]+=state.myboard[player][Attacker_num].attack;
            }
            if(state.myboard[player][Attacker_num].abilities[4]=='L' || state.myboard[1-player][Target_num].defense<=0){
                state.myboard[1-player][Target_num].defense=0;
                state.playerBoardsize[1-player]--;
            }
            return state;
            }
            else{//enemy->not_ward,me->not_ward

            state.myboard[player][Attacker_num].defense-=state.myboard[1-player][Target_num].attack;
            state.myboard[1-player][Target_num].defense-=state.myboard[player][Attacker_num].attack;
            if(state.myboard[player][Attacker_num].abilities[0]=='B'){
            if(state.myboard[1-player][Target_num].defense<0){state.playerHealth[1-player]+=state.myboard[1-player][Target_num].defense;}
            }
            if(state.myboard[player][Attacker_num].abilities[2]=='D'){
                state.playerHealth[player]+=state.myboard[player][Attacker_num].attack;
            }
            int delete_creature[2]={0};
            if(state.myboard[player][Attacker_num].abilities[4]=='L'||state.myboard[1-player][Target_num].defense<=0){
                delete_creature[1-player]=1;
                state.playerBoardsize[1-player]--;
            }
            if(state.myboard[1-player][Target_num].abilities[4]=='L'||state.myboard[player][Attacker_num].defense<=0){
                delete_creature[player]=1;
                state.playerBoardsize[player]--;
            }
            if(delete_creature[player]==1){state.myboard[player][Attacker_num].defense=0;}
            if(delete_creature[1-player]==1){state.myboard[1-player][Target_num].defense=0;}
            return state;
            }
        }
    }
    return state;
}
bool can_summon(GameState state,int player,int summon_num){
    if(state.mycreature[player][summon_num].defense<=0){return false;}
    if(state.playerBoardsize[player]<6&&state.mycreature[player][summon_num].cost<=state.playerMana[player]){
     return true;   
    }
    return false;
}
GameState do_summon(GameState state,int player,int summon_num){
    state.playerMana[player]-=state.mycreature[player][summon_num].cost;
    state.playerDraw[player]+=state.mycreature[player][summon_num].cardDraw;
    state.playerHealth[player]+=state.mycreature[player][summon_num].myHealthChange;
    state.playerHealth[1-player]+=state.mycreature[player][summon_num].opponentHealthChange;
    state.myboard[player].push_back(state.mycreature[player][summon_num]);
    state.mycreature[player][summon_num].defense=0;
    state.playerHandsize[player]--;
    state.playerBoardsize[player]++;
    return state;
}
bool can_use(GameState state,int player,int item_num,int use_num,int Target_num){
    if(state.myitem[player][use_num].cost>state.playerMana[player]){return false;}
    if(state.myitem[player][use_num].cardType<=0 || state.myitem[player][use_num].cardType>3){return false;}
    if(Target_num==-1){
        if(item_num==3){return true;}
        else{return false;}
    }
    if(item_num<=0 || item_num>3){return false;}
    else if(item_num==1){
    if(state.myboard[player][Target_num].defense<=0){return false;}
    else{return true;}
    }
    else if(item_num==2){
    if(state.myboard[1-player][Target_num].defense<=0){return false;}
    else{return true;}
    }
    else{
        if(state.myitem[player][use_num].defense<0 && state.myboard[1-player][Target_num].defense>0){return true;}
        else{return false;}
    }
}

GameState use_item(GameState state,int player,int item_num,int use_num,int Target_num){
    state.playerMana[player]-=state.myitem[player][use_num].cost;
    state.playerHealth[player]+=state.myitem[player][use_num].myHealthChange;
    state.playerHealth[1-player]+=state.myitem[player][use_num].opponentHealthChange;
    state.playerDraw[player]+=state.myitem[player][use_num].cardDraw;
    state.myitem[player][use_num].cardType=4;
    state.playerHandsize[player]--;
    if(Target_num==-1){
        return state;
    }
    if(item_num==1){
        state.myboard[player][Target_num].attack+=state.myitem[player][use_num].attack;
        state.myboard[player][Target_num].defense+=state.myitem[player][use_num].defense;
        if(state.myitem[player][use_num].abilities[0]=='B'){state.myboard[player][Target_num].abilities[0]='B';}
        if(state.myitem[player][use_num].abilities[1]=='C'){state.myboard[player][Target_num].abilities[1]='C';}
        if(state.myitem[player][use_num].abilities[2]=='D'){state.myboard[player][Target_num].abilities[2]='D';}
        if(state.myitem[player][use_num].abilities[3]=='G'){state.myboard[player][Target_num].abilities[3]='G';}
        if(state.myitem[player][use_num].abilities[4]=='L'){state.myboard[player][Target_num].abilities[4]='L';}
        if(state.myitem[player][use_num].abilities[5]=='W'){state.myboard[player][Target_num].abilities[5]='W';}
    }
    else if(item_num==2){
        if(state.myitem[player][use_num].abilities=="BCDGLW" || state.myboard[1-player][Target_num].abilities[5]!='W'){
        state.myboard[1-player][Target_num].attack+=state.myitem[player][use_num].attack;
        state.myboard[1-player][Target_num].defense+=state.myitem[player][use_num].defense;
        if(state.myboard[1-player][Target_num].defense<=0){
        state.playerBoardsize[1-player]--;
        }
        }
        else{
            state.myboard[1-player][Target_num].attack+=state.myitem[player][use_num].attack;
            if(state.myitem[player][use_num].defense<0){
                state.myboard[1-player][Target_num].abilities[5]='-';
            }
        }
        if(state.myitem[player][use_num].abilities[0]=='B'){state.myboard[1-player][Target_num].abilities[0]='-';}
        if(state.myitem[player][use_num].abilities[1]=='C'){state.myboard[1-player][Target_num].abilities[1]='-';}
        if(state.myitem[player][use_num].abilities[2]=='D'){state.myboard[1-player][Target_num].abilities[2]='-';}
        if(state.myitem[player][use_num].abilities[3]=='G'){state.myboard[1-player][Target_num].abilities[3]='-';}
        if(state.myitem[player][use_num].abilities[4]=='L'){state.myboard[1-player][Target_num].abilities[4]='-';}
        if(state.myitem[player][use_num].abilities[5]=='W'){state.myboard[1-player][Target_num].abilities[5]='-';}
    }
    else if(item_num==3){
        if(state.myboard[1-player][Target_num].abilities[5]!='W'){
        state.myboard[1-player][Target_num].attack+=state.myitem[player][use_num].attack;
        state.myboard[1-player][Target_num].defense+=state.myitem[player][use_num].defense;
        if(state.myboard[1-player][Target_num].defense<=0){
        state.playerBoardsize[1-player]--;
        }
        }
        else{
            state.myboard[1-player][Target_num].abilities[5]='-';
        }
    }
    return state;
}
GameState dfs_enemy_attack(int depth,GameState state){
    if(depth<0){
        state.value=evalScore(state, 1);
        return state;
    }
    GameState tmp=state;
    for(int j=-1;j<(int)state.myboard[0].size();j++){
        if(can_attack(state, 1, depth,j)){
        GameState next_state=do_attack(state,1,depth,j);
        GameState ret=dfs_enemy_attack(depth-1,next_state);
        if(ret.value>tmp.value){tmp=ret;}
        }
    }
    GameState ret2=dfs_enemy_attack(depth-1,state);
    if(ret2.value>tmp.value){tmp=ret2;}
    return tmp;
}
GameState dfs_attack(int depth,GameState state){
    if(depth<0){
        state.value=evalScore(state, 0);
        return state;
    }
    GameState tmp=state;
    for(int j=-1;j<(int)state.myboard[1].size();j++){
        if(can_attack(state, 0, depth,j)){
        GameState next_state=do_attack(state,0,depth,j);
        GameState ret=dfs_attack(depth-1,next_state);
        if(ret.value>tmp.value){tmp=ret;}
        }
    }
    GameState ret2=dfs_attack(depth-1,state);
    if(ret2.value>tmp.value){tmp=ret2;}
    return tmp;
}
GameState dfs_summon(int depth,GameState state){
    if(depth<0){
        state.value=evalScore(state, 0);
        return state;
    }
    GameState tmp=state;

    if(can_summon(state,0,depth)){
        GameState next_state=do_summon(state, 0, depth);
        next_state.action+="SUMMON "+to_string(state.mycreature[0][depth].instanceId)+";";
        int id=state.mycreature[0][depth].instanceId;
        int k=(int)next_state.myboard[0].size()-1;
        if(next_state.mycreature[0][depth].abilities[1]=='C' && next_state.myboard[0][k].instanceId==id){
            for(int j=-1;j<(int)next_state.myboard[1].size();j++){
                if(can_attack(next_state, 0, k,j)){
                    GameState next_state2=do_attack(next_state, 0, k, j);
                    if(j==-1){
                        next_state2.action+="ATTACK "+to_string(id)+" "+to_string(-1)+";";
                    }
                    else{
                        next_state2.action+="ATTACK "+to_string(id)+" "+to_string(next_state.myboard[1][j].instanceId)+";";
                    }
                    GameState ret=dfs_summon(depth-1,next_state2);
                    if(ret.value>tmp.value){tmp=ret;}                   
                    }
                }
            }
            GameState ret=dfs_summon(depth-1,next_state);
            if(ret.value>tmp.value){tmp=ret;}
        }
        GameState ret=dfs_summon(depth-1,state);
        if(ret.value>tmp.value){tmp=ret;}

    return tmp;
}
GameState dfs_use_green(int depth,GameState state){
    if(depth<0){
        state.value=evalScore(state, 0);
        return state;
    }

    GameState tmp=state;

    for(int j=0;j<(int)state.myitem[0].size();j++){
        if(state.myitem[0][j].cardType==1){
        if(can_use(state, 0, 1, j, depth)){
            GameState next_state=use_item(state,0,1,j,depth);
            next_state.action+="USE "+to_string(state.myitem[0][j].instanceId)+" "+to_string(state.myboard[0][depth].instanceId)+";";
            GameState ret=dfs_use_green(depth-1,next_state);
            if(ret.value>tmp.value){tmp=ret;}
            }
            GameState ret2=dfs_use_green(depth-1,state);
            if(ret2.value>tmp.value){tmp=ret2;}
        }
    }
    return tmp;
}
GameState dfs_use_red(int depth,GameState state){
    if(depth<0){
        state.value=evalScore(state, 0);
        return state;
    }

    GameState tmp=state;

    for(int j=0;j<(int)state.myitem[0].size();j++){
        if(state.myitem[0][j].cardType==2){
        if(can_use(state, 0, 2, j, depth)){
            GameState next_state=use_item(state,0,2,j,depth);
            next_state.action+="USE "+to_string(state.myitem[0][j].instanceId)+" "+to_string(state.myboard[1][depth].instanceId)+";";
            GameState ret=dfs_use_red(depth-1,next_state);
            if(ret.value>tmp.value){tmp=ret;}
            }
            GameState ret2=dfs_use_red(depth-1,state);
            if(ret2.value>tmp.value){tmp=ret2;}
        }
    }
    return tmp;
}
GameState dfs_use_blue(int depth,GameState state){
    if(depth<-1){
        state.value=evalScore(state, 0);
        return state;
    }

    GameState tmp=state;

    for(int j=0;j<(int)state.myitem[0].size();j++){
        if(state.myitem[0][j].cardType==3){
        if(can_use(state, 0, 3, j, depth)){
            GameState next_state=use_item(state,0,3,j,depth);
            if(depth==-1){next_state.action+="USE "+to_string(state.myitem[0][j].instanceId)+" "+to_string(-1)+";";}
            else{next_state.action+="USE "+to_string(state.myitem[0][j].instanceId)+" "+to_string(state.myboard[1][depth].instanceId)+";";}
            GameState ret=dfs_use_blue(depth-1,next_state);
            if(ret.value>tmp.value){tmp=ret;}
            }
            GameState ret2=dfs_use_blue(depth-1,state);
            if(ret2.value>tmp.value){tmp=ret2;}
        }
    }
    return tmp;
}
int can_action(GameState state,int player){
    int ret=0;
    for(int i=0;i<(int)state.mycreature[player].size();i++){
        if(state.playerMana[player]>=state.mycreature[player][i].cost && state.playerBoardsize[player]<6&&state.mycreature[player][i].defense>0){
            return ret;
        }
    }
    ret|=1;
    for(int i=0;i<(int)state.myitem[player].size();i++){
        if(state.playerMana[player]>=state.myitem[player][i].cost){
            if(state.myitem[player][i].cardType==1){
                if(state.myboard[player].size()>0){return ret;}
            }
            else if(state.myitem[player][i].cardType==2){
                if(state.myboard[1-player].size()>0){return ret;}
            }
            else if(state.myitem[player][i].cardType==3){
                return ret;
            }
        }
    }
    ret|=(1<<1);
    for(int i=0;i<(int)state.myboard[player].size();i++){
        if(state.myboard[player][i].is_summon==0&&state.myboard[player][i].is_attack==0&&state.myboard[player][i].defense>0){return ret;}
    }
    ret|=(1<<2);
    return ret;
}
GameState random_action(GameState state,int player){
    ull type;

    while(1){
        type=xor128()%(3ull);
        if(type==0ull&&state.status==0){break;}
        if(type==1ull&&state.status<=1){break;}
        if(type==2ull&&state.status<=3){break;}
    }

    if(type==0ull){
    int cnt=0;
    int see[10]={0};
    int r;
    //static mt19937 mt((int)time(0));
    if((int)state.mycreature[player].size()==0){return state;}
    //uniform_int_distribution<int> dice(0, (int)state.mycreature[0].size()-1);
    while(1){
    r=xorshift()%(int)state.mycreature[player].size();
    if(can_summon(state, player, r)){break;}
    if(see[r]==0){cnt++;see[r]=1;}
    if(cnt==((int)state.mycreature[player].size())){break;}
    }
    if(!can_summon(state, player, r)){
        return state;
    }
    GameState next_state=do_summon(state, player, r);
    next_state.action+="SUMMON "+to_string(state.mycreature[player][r].instanceId)+";";
    int id=state.mycreature[player][r].instanceId;
    int k=(int)next_state.myboard[player].size()-1;
    next_state.myboard[player][k].is_summon=1;
    if(next_state.mycreature[player][r].abilities[1]=='C' && next_state.myboard[player][k].instanceId==id){
    int cnt2=0;
    int see2[10]={0};
    int r2;
    //uniform_int_distribution<int> dice2(-1, (int)next_state.myboard[1].size()-1);
    while(1){
    r2=xorshift()%((int)next_state.myboard[1-player].size()+1);
    r2--;
    if(can_attack(next_state, player, k,r2)){break;}
    if(see2[r2+1]==0){cnt2++;see2[r2+1]=1;}
    if(cnt2==((int)next_state.myboard[1-player].size()+1)){break;}
    }
    GameState next_state2=do_attack(next_state, player, k, r2);
    if(r2==-1){
    next_state2.action+="ATTACK "+to_string(id)+" "+to_string(-1)+";";
    }
    else{
    next_state2.action+="ATTACK "+to_string(id)+" "+to_string(next_state.myboard[1-player][r2].instanceId)+";";
    }
    next_state2.myboard[player][k].is_attack=1;
    return next_state2;
    }
    return next_state;
    }
    else if(type==1ull){
        if((int)state.myitem[player].size()==0){return state;}
        int have_item[4]={0};
        for(int i=0;i<(int)state.myitem[player].size();i++){
            have_item[state.myitem[player][i].cardType]++;
        }
        //static mt19937 mt((int)time(0));
        //uniform_int_distribution<int> dice(1,3);
        int item_num;
        int Target_num;
        int use_num;
        int dont_use[4]={0};
        for(int i=1;i<=3;i++){
            if(have_item[i]>0){
                if(i==1){
                    if((int)state.myboard[player].size()==0){dont_use[1]=1;}
                }
                else if(i==2){
                    if((int)state.myboard[1-player].size()==0){dont_use[2]=1;}
                }
            }
            else{
                dont_use[i]=1;
            }
        }
        int cnt3=0;
        int see3[5]={0};
        while(1){
            item_num=xorshift()%3;
            item_num++;
            if(dont_use[item_num]==0){break;}
            if(see3[item_num]==0){see3[item_num]=1;cnt3++;}
            if(cnt3==3){break;}
        }
        if(cnt3==3){return state;}
        if(item_num==1){
            if(dont_use[1]==1){return state;}
            //uniform_int_distribution<int> dice2(0,(int)state.myboard[0].size()-1);
            //uniform_int_distribution<int> dice3(0,(int)state.myitem[0].size()-1);
            Target_num=xorshift()%(int)state.myboard[player].size();
            int see[10]={0};
            int cnt=0;
            while(1){
            use_num=xorshift()%(int)state.myitem[player].size();
            if(state.myitem[player][use_num].cardType==1){
            if(can_use(state, player, 1, use_num, Target_num)){
                break;
            }
            }
            if(see[use_num]==0){cnt++;see[use_num]=1;}
            if(cnt==((int)state.myitem[player].size())){break;}
            }
            if(cnt==((int)state.myitem[player].size())){
                return state;
            }
            else{
                GameState next_state=use_item(state, player, 1, use_num, Target_num);
                next_state.action+="USE "+to_string(state.myitem[player][use_num].instanceId)+" "+to_string(state.myboard[player][Target_num].instanceId)+";";
                return next_state;
            }
        }
        else if(item_num==2){
            if(dont_use[2]==1){return state;}
            //uniform_int_distribution<int> dice2(0,(int)state.myboard[1].size()-1);
            //uniform_int_distribution<int> dice3(0,(int)state.myitem[0].size()-1);
            Target_num=xorshift()%(int)state.myboard[1-player].size();
            int see[10]={0};
            int cnt=0;
            while(1){
            use_num=xorshift()%(int)state.myitem[player].size();
            if(state.myitem[player][use_num].cardType==2){
            if(can_use(state, player, 2, use_num, Target_num)){
                break;
            }
            }
            if(see[use_num]==0){cnt++;see[use_num]=1;}
            if(cnt==((int)state.myitem[player].size())){break;}
            }
            if(cnt==((int)state.myitem[player].size())){
                return state;
            }
            else{
                GameState next_state=use_item(state, player, 2, use_num, Target_num);
                next_state.action+="USE "+to_string(state.myitem[player][use_num].instanceId)+" "+to_string(state.myboard[1-player][Target_num].instanceId)+";";
                return next_state;
            }
        }
        else{
            if(dont_use[3]==1){return state;}
            //uniform_int_distribution<int> dice2(-1,(int)state.myboard[1].size()-1);
            //uniform_int_distribution<int> dice3(0,(int)state.myitem[0].size()-1);
            Target_num=xorshift()%((int)state.myboard[1-player].size()+1);
            Target_num--;
            int see[10]={0};
            int cnt=0;
            while(1){
            use_num=xorshift()%(int)state.myitem[player].size();
            if(state.myitem[player][use_num].cardType==3){
            if(can_use(state, player, 3, use_num, Target_num)){
                break;
            }
            }
            if(see[use_num]==0){cnt++;see[use_num]=1;}
            if(cnt==((int)state.myitem[player].size())){break;}
            }
            if(cnt==((int)state.myitem[player].size())){
                return state;
            }
            else{
                GameState next_state=use_item(state, player, 3, use_num, Target_num);
                if(Target_num==-1){
                next_state.action+="USE "+to_string(state.myitem[player][use_num].instanceId)+" "+to_string(-1)+";";
                }
                else{
                next_state.action+="USE "+to_string(state.myitem[player][use_num].instanceId)+" "+to_string(state.myboard[1-player][Target_num].instanceId)+";";
                }
                return next_state;
            }
        }
    }
    else{
        if((int)state.myboard[player].size()==0){return state;}
        //static mt19937 mt((int)time(0));
        //uniform_int_distribution<int> dice(-1,(int)state.myboard[1].size()-1);
        //uniform_int_distribution<int> dice2(0,(int)state.myboard[0].size()-1);
        int i;
        int see2[10]={0};
        int cnt2=0;
        while(1){
        i=xorshift()%(int)state.myboard[player].size();
        if(state.myboard[player][i].is_summon==0 && state.myboard[player][i].is_attack==0 && state.myboard[player][i].defense>0){break;}
        if(see2[i]==0){see2[i]=1;cnt2++;}
        if(cnt2==((int)state.myboard[player].size())){break;}
        }
        if(cnt2==((int)state.myboard[player].size())){return state;}
        int cnt=0;
        int see[10]={0};
        int Target_num;
        while(1){
            Target_num=xorshift()%((int)state.myboard[1-player].size()+1);
            Target_num--;
            if(can_attack(state, player, i, Target_num)){
                break;
            }
            if(see[Target_num+1]==0){see[Target_num+1]=1;cnt++;}
            if(cnt==((int)state.myboard[1-player].size()+1)){break;}
        }
        if(cnt!=((int)state.myboard[1-player].size()+1)){
        state=do_attack(state, player, i, Target_num);
        state.myboard[player][i].is_attack=1;
        if(Target_num==-1){state.action+="ATTACK "+to_string(state.myboard[player][i].instanceId)+" "+to_string(-1)+";";}
        else{
        state.action+="ATTACK "+to_string(state.myboard[player][i].instanceId)+" "+to_string(state.myboard[1-player][Target_num].instanceId)+";";
        }
        }
        return state;
    }
    return state;
}
GameState get_action(GameState state,int player){
    while(1){
        GameState next_state=random_action(state,player);
        state=next_state;
        for(int j=0;j<=1;j++){           
            for(int i=(int)state.mycreature[j].size()-1;i>=0;i--){
                if(state.mycreature[j][i].defense<=0){
                    state.mycreature[j].erase(state.mycreature[j].begin()+i);
                }
            }
            for(int i=(int)state.myboard[j].size()-1;i>=0;i--){
                if(state.myboard[j][i].defense<=0){
                    state.myboard[j].erase(state.myboard[j].begin()+i);
                }
            }
            for(int i=(int)state.myitem[j].size()-1;i>=0;i--){
                if(state.myitem[j][i].cardType==4){
                    state.myitem[j].erase(state.myitem[j].begin()+i);
                }
            }
        }
        int pattern=can_action(state,player);
        state.status=pattern;
        if(pattern==7){break;}
    }
    GameState next_state2;
    if(player==0){
    next_state2=dfs_enemy_attack((int)state.myboard[1].size()-1,state);
    }
    else{
    next_state2=dfs_attack((int)state.myboard[0].size()-1,state);
    }
    state.value=-next_state2.value;
    return state;
}
GameState get_action2(GameState state,int player){
    state.status=0;
    for(int i=0;i<(int)state.myboard[player].size();i++){
        state.myboard[player][i].is_attack=0;
        state.myboard[player][i].is_summon=0;
    }
    for(int i=0;i<(int)state.mycreature[player].size();i++){
        state.mycreature[player][i].is_attack=0;
        state.mycreature[player][i].is_summon=0;
    }
    while(1){
        GameState next_state=random_action(state,player);
        state=next_state;
        for(int j=0;j<=1;j++){           
            for(int i=(int)state.mycreature[j].size()-1;i>=0;i--){
                if(state.mycreature[j][i].defense<=0){
                    state.mycreature[j].erase(state.mycreature[j].begin()+i);
                }
            }
            for(int i=(int)state.myboard[j].size()-1;i>=0;i--){
                if(state.myboard[j][i].defense<=0){
                    state.myboard[j].erase(state.myboard[j].begin()+i);
                }
            }
            for(int i=(int)state.myitem[j].size()-1;i>=0;i--){
                if(state.myitem[j][i].cardType==4){
                    state.myitem[j].erase(state.myitem[j].begin()+i);
                }
            }
        }
        int pattern=can_action(state,player);
        state.status=pattern;
        if(pattern==7){break;}
    }
    state.value=evalScore(state,player);
    return state;
}
int battle(vector<card>deck[2]){

     GameState state;

     if(xor128()%2ull==0ull){state.first=0;}
     else{state.first=1;}

     state.playerHealth[0]=30;
     state.playerHealth[1]=30;
     if(state.first==0){
         state.playerMana[0]=1;
         state.playerMaxMana[0]=1;
         state.playerMana[1]=2;
         state.playerMaxMana[1]=2;
     }
     else{
         state.playerMana[0]=2;
         state.playerMaxMana[0]=2;
         state.playerMana[1]=1;
         state.playerMaxMana[1]=1;
     }
     state.playerDeck[0]=(int)deck[0].size();
     state.playerDeck[1]=(int)deck[1].size();
     state.playerRune[0]=5;
     state.playerRune[1]=5;
     state.playerDraw[0]=1;
     state.playerDraw[1]=1;
     if(state.first==0){
         if(state.playerDeck[0]<4){
             state.playerHandsize[0]=state.playerDeck[0];
         }
         else{
             state.playerHandsize[0]=4;
         }
         if(state.playerDeck[1]<5){
             state.playerHandsize[1]=state.playerDeck[1];
         }
         else{
             state.playerHandsize[1]=5;
         }
     }
     else{
         if(state.playerDeck[0]<5){
             state.playerHandsize[0]=state.playerDeck[0];
         }
         else{
             state.playerHandsize[0]=5;
         }
         if(state.playerDeck[1]<4){
             state.playerHandsize[1]=state.playerDeck[1];
         }
         else{
             state.playerHandsize[1]=4;
         }
     }
    static mt19937 mt((int)time(0));

    for(int i=0;i<2;i++){
	shuffle(deck[i].begin(), deck[i].end(), mt);
    }

    for(int k=0;k<2;k++){

    for(int i=0;i<state.playerHandsize[k];i++){
        if(deck[k][i].cardType==0){
            state.mycreature[k].push_back(deck[k][i]);
        }
        else{
            state.myitem[k].push_back(deck[k][i]);
        }
    }
    deck[k].erase(deck[k].begin(), deck[k].begin() + state.playerHandsize[k]);
    }

    state.playerDeck[0]-=state.playerHandsize[0];
    state.playerDeck[1]-=state.playerHandsize[1];

     int add_mana_flag=0;

     for(int turn=state.first;;turn++){

         if(turn>=49+state.first){
             deck[0].clear();
             deck[1].clear();
         }

         if(state.playerHealth[0]<=0){return 0;}
         if(state.playerHealth[1]<=0){return 1;}

         for(int i=0;i<state.playerDraw[turn%2];i++){

            if((int)deck[turn%2].size()>0){
                if((int)state.mycreature[turn%2].size()+(int)state.myitem[turn%2].size()<8){
                 card c=deck[turn%2][0];
                 deck[turn%2].erase(deck[turn%2].begin());
                 if(c.cardType==0){
                 state.mycreature[turn%2].push_back(c);
                 }
                 else{
                    state.myitem[turn%2].push_back(c);
                 }
                }
            }
            else{
                 if(state.playerRune[turn%2]==0){
                     if(turn%2==0){return 0;}
                     else{return 1;}
                 }
                 else{
                     if(state.playerHealth[turn%2]>=25 && state.playerRune[turn%2]>4){state.playerRune[turn%2]--;state.playerHealth[turn%2]=25;}
                     else if(state.playerHealth[turn%2]>=20 && state.playerRune[turn%2]>3){state.playerRune[turn%2]--;state.playerHealth[turn%2]=20;}
                     else if(state.playerHealth[turn%2]>=15 && state.playerRune[turn%2]>2){state.playerRune[turn%2]--;state.playerHealth[turn%2]=15;}
                     else if(state.playerHealth[turn%2]>=10 && state.playerRune[turn%2]>1){state.playerRune[turn%2]--;state.playerHealth[turn%2]=10;}
                     else if(state.playerHealth[turn%2]>=5 && state.playerRune[turn%2]>0){state.playerRune[turn%2]--;state.playerHealth[turn%2]=5;}
                 }
             }
        }
        state.playerDraw[turn%2]=1;

        state.playerHandsize[0]=(int)state.mycreature[0].size()+(int)state.myitem[0].size();
        state.playerHandsize[1]=(int)state.mycreature[1].size()+(int)state.myitem[1].size();
        state.playerDeck[0]=(int)deck[0].size();
        state.playerDeck[1]=(int)deck[1].size();
        state.playerBoardsize[0]=(int)state.myboard[0].size();
        state.playerBoardsize[1]=(int)state.myboard[1].size();
        state.value=-INT_MAX;

        state=get_action2(state,turn%2);

        state.action="";
        
        for(int j=0;j<=1;j++){
        
        for(int i=(int)state.mycreature[j].size()-1;i>=0;i--){
        if(state.mycreature[j][i].defense<=0){
            state.mycreature[j].erase(state.mycreature[j].begin()+i);
            }
        }
        
        for(int i=(int)state.myboard[j].size()-1;i>=0;i--){
        if(state.myboard[j][i].defense<=0){
            state.myboard[j].erase(state.myboard[j].begin()+i);
            }
        }
        
        for(int i=(int)state.myitem[j].size()-1;i>=0;i--){
        if(state.myitem[j][i].cardType==4){
            state.myitem[j].erase(state.myitem[j].begin()+i);
            }
        }
        }
        
        if(state.playerMana[turn%2]==0&&state.first==(turn+1)%2&&add_mana_flag==0){add_mana_flag=1;state.playerMaxMana[turn%2]--;}
        
        if(state.first==(turn%2)){
        state.playerMaxMana[turn%2]++;
        if(state.playerMaxMana[turn%2]>=13){state.playerMaxMana[turn%2]=12;}
        state.playerMana[turn%2]=state.playerMaxMana[turn%2];
        }
        else{
        state.playerMaxMana[turn%2]++;
        if(state.playerMaxMana[turn%2]>=13){
            if(add_mana_flag==1){
            state.playerMaxMana[turn%2]=12;
            }
            else{
            state.playerMaxMana[turn%2]=13;
            }
        }
        state.playerMana[turn%2]=state.playerMaxMana[turn%2];
        }
    }
}
/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main()
{
    vector<card>deck;
    int iter=0;
    // game loop
    for(int t=0;t<30;t++) {
        for (int i = 0; i < 2; i++) {
            int playerHealth;
            int playerMana;
            int playerDeck;
            int playerRune;
            int playerDraw;
            cin >> playerHealth >> playerMana >> playerDeck >> playerRune >> playerDraw; cin.ignore();
        }
        int opponentHand;
        int opponentActions;
        cin >> opponentHand >> opponentActions; cin.ignore();
        for (int i = 0; i < opponentActions; i++) {
            string cardNumberAndAction;
            getline(cin, cardNumberAndAction);
        }
        card draft_card[3];
        int cardCount;
        cin >> cardCount; cin.ignore();
        for (int i = 0; i < cardCount; i++) {
            int cardNumber;
            int instanceId;
            int location;
            int cardType;
            int cost;
            int attack;
            int defense;
            string abilities;
            int myHealthChange;
            int opponentHealthChange;
            int cardDraw;
            cin >> cardNumber >> instanceId >> location >> cardType >> cost >> attack >> defense >> abilities >> myHealthChange >> opponentHealthChange >> cardDraw; cin.ignore();
            draft_card[i].cardNumber=cardNumber;
            draft_card[i].instanceId=instanceId;
            draft_card[i].location=location;
            draft_card[i].cardType=cardType;
            draft_card[i].cost=cost;
            draft_card[i].attack=attack;
            draft_card[i].defense=defense;
            draft_card[i].abilities=abilities;
            draft_card[i].myHealthChange=myHealthChange;
            draft_card[i].opponentHealthChange=opponentHealthChange;
            draft_card[i].cardDraw=cardDraw;
        }

        string action="";

        int loop=1;

        int priority[3]={0};

        start = chrono::system_clock::now();

        while(1){

        double elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;

        if(t==0){
        if(elapsed>=0.85){break;}
        }
        else{
        if(elapsed>=0.085){break;}
        }

        vector<card>tmp_deck[2];

        tmp_deck[0]=deck;
        tmp_deck[1]=deck;

        tmp_deck[0].push_back(draft_card[0]);
        tmp_deck[1].push_back(draft_card[1]);
        
        //for(int i=0;i<29-t;i++){
        for(int i=0;i<ADD_CARD;i++){
            tmp_deck[0].push_back(draft_card[0]);
            tmp_deck[1].push_back(draft_card[1]);
        }

        for(int i=0;i<(int)tmp_deck[0].size();i++){
            tmp_deck[0][i].instanceId=i+1;
        }

        for(int i=0;i<(int)tmp_deck[1].size();i++){
            tmp_deck[1][i].instanceId=5001+i;
        }

        for(int n=0;n<loop;n++){
            vector<card>tmp2_deck[2];
            tmp2_deck[0]=tmp_deck[0];
            tmp2_deck[1]=tmp_deck[1];
            int result=battle(tmp2_deck);
            if(result==1){
                priority[0]++;
            }
            else{
                priority[1]++;
            }
        }

        elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;

        if(t==0){
        if(elapsed>=0.85){break;}
        }
        else{
        if(elapsed>=0.085){break;}
        }

        tmp_deck[0]=deck;
        tmp_deck[1]=deck;

        tmp_deck[0].push_back(draft_card[0]);
        tmp_deck[1].push_back(draft_card[2]);

        //for(int i=0;i<29-t;i++){
            for(int i=0;i<ADD_CARD;i++){
            tmp_deck[0].push_back(draft_card[0]);
            tmp_deck[1].push_back(draft_card[2]);
        }


        for(int i=0;i<(int)tmp_deck[0].size();i++){
            tmp_deck[0][i].instanceId=i+1;
        }

        for(int i=0;i<(int)tmp_deck[1].size();i++){
            tmp_deck[1][i].instanceId=5001+i;
        }
        for(int n=0;n<loop;n++){
            vector<card>tmp2_deck[2];
            tmp2_deck[0]=tmp_deck[0];
            tmp2_deck[1]=tmp_deck[1];
            int result=battle(tmp2_deck);
            if(result==1){
                priority[0]++;
            }
            else{
                priority[2]++;
            }
        }
        elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;

        if(t==0){
        if(elapsed>=0.85){break;}
        }
        else{
        if(elapsed>=0.085){break;}
        }

        tmp_deck[0]=deck;
        tmp_deck[1]=deck;

        tmp_deck[0].push_back(draft_card[1]);
        tmp_deck[1].push_back(draft_card[2]);

        //for(int i=0;i<29-t;i++){
            for(int i=0;i<ADD_CARD;i++){
            tmp_deck[0].push_back(draft_card[1]);
            tmp_deck[1].push_back(draft_card[2]);
        }


        for(int i=0;i<(int)tmp_deck[0].size();i++){
            tmp_deck[0][i].instanceId=i+1;
        }

        for(int i=0;i<(int)tmp_deck[1].size();i++){
            tmp_deck[1][i].instanceId=5001+i;
        }
        for(int n=0;n<loop;n++){
            vector<card>tmp2_deck[2];
            tmp2_deck[0]=tmp_deck[0];
            tmp2_deck[1]=tmp_deck[1];
            int result=battle(tmp2_deck);
            if(result==1){
                priority[1]++;
            }
            else{
                priority[2]++;
            }
        }

        iter+=3;

        }

        double elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;

        cerr<<"iterate="<<iter<<endl;

        cerr<<"elapsed_time="<<elapsed<<endl;

        for(int i=0;i<3;i++){

        cerr<<"card_number="<<draft_card[i].cardNumber<<endl;
        cerr<<"win_rate="<<(double)priority[i]/(double)(priority[0]+priority[1]+priority[2])<<endl;

        if(draft_card[i].cardNumber==116){
            //priority[i]=114514;
        }

        }

        vector<pair<int,int> >select;

        for(int i=0;i<3;i++){
            select.push_back(make_pair(-priority[i],i));
        }

        sort(select.begin(),select.end());

        action+="PICK "+to_string(select[0].second);

        deck.push_back(draft_card[select[0].second]);

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        cout<<action<<endl;
    }

    for(int t=0;;t++) {
        vector<card> mycreature[2];//hand
        vector<card> myitem[2];//hand
        vector<card> myboard[2];//board
        int playerHealth[2];
        int playerMana[2];
        int playerDeck[2];
        int playerRune[2];
        int playerDraw[2];
        for (int i = 0; i < 2; i++) {
            cin >> playerHealth[i] >> playerMana[i] >> playerDeck[i] >> playerRune[i] >> playerDraw[i]; cin.ignore();
        }
        int opponentHand;
        int opponentActions;
        cin >> opponentHand >> opponentActions; cin.ignore();
        for (int i = 0; i < opponentActions; i++) {
            string cardNumberAndAction;
            getline(cin, cardNumberAndAction);
        }
        int cardCount;
        cin >> cardCount; cin.ignore();
        for (int i = 0; i < cardCount; i++) {
            int cardNumber;
            int instanceId;
            int location;
            int cardType;
            int cost;
            int attack;
            int defense;
            string abilities;
            int myHealthChange;
            int opponentHealthChange;
            int cardDraw;
            cin >> cardNumber >> instanceId >> location >> cardType >> cost >> attack >> defense >> abilities >> myHealthChange >> opponentHealthChange >> cardDraw; cin.ignore();
            card c;
            c.cardNumber=cardNumber;
            c.instanceId=instanceId;
            c.location=location;
            c.cardType=cardType;
            c.cost=cost;
            c.attack=attack;
            c.defense=defense;
            c.abilities=abilities;
            c.myHealthChange=myHealthChange;
            c.opponentHealthChange=opponentHealthChange;
            c.cardDraw=cardDraw;
            c.is_summon=0;
            c.is_attack=0;
            if(location==0){
            if(cardType==0){
                mycreature[0].push_back(c);
            }
            else{
                myitem[0].push_back(c);
            }
            }
            else if(location==1){
                myboard[0].push_back(c);
            }
            else{
                myboard[1].push_back(c);
            }
        }
        start = chrono::system_clock::now();
        GameState state;
        state.action="";
        state.mycreature[0]=mycreature[0];
        state.myitem[0]=myitem[0];
        state.myboard[0]=myboard[0];
        state.myboard[1]=myboard[1];
        state.value=-INT_MAX;
        state.playerHealth[0]=playerHealth[0];
        state.playerHealth[1]=playerHealth[1];
        state.playerMaxMana[0]=playerMana[0];
        state.playerMaxMana[1]=playerMana[1];
        state.playerMana[0]=state.playerMaxMana[0];
        state.playerMana[1]=state.playerMaxMana[1];
        state.playerDeck[0]=playerDeck[0];
        state.playerDeck[1]=playerDeck[1];
        state.playerRune[0]=playerRune[0];
        state.playerRune[1]=playerRune[1];
        state.playerDraw[0]=playerDraw[0];
        state.playerDraw[1]=playerDraw[1];
        state.playerHandsize[0]=(int)mycreature[0].size()+(int)myitem[0].size();
        state.playerHandsize[1]=opponentHand;
        state.playerBoardsize[0]=(int)myboard[0].size();
        state.playerBoardsize[1]=(int)myboard[1].size();
        state.value=-INT_MAX;
        state.status=0;

        //summon_and_attack_and_use

        GameState tmp=state;

        while(1){
            double elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;
            if(elapsed>=0.09){break;}
            GameState next_state=get_action(state,0);
            if(next_state.value>tmp.value){tmp=next_state;}
        }

        state=tmp;

        double elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;
        cerr<<"elapsed="<<elapsed<<endl;
            
        if(state.action==""){cout<<"PASS"<<endl;}
        else{cout<<state.action<<endl;}
    }
    
    
    return 0;
}
