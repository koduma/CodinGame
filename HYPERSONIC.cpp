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
#include <iomanip>

using namespace std;

#define player_num 4

chrono::system_clock::time_point start;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

struct kiki{
    int x;
    int y;
    int t;
};

struct entity{

    int entityType;
    int owner;
    int x;
    int y;
    int param1;
    int param2;

};

struct bomb{
    int x;
    int y;
    int owner;
    int remain_time;
    int range;
    int delete_flag;
};
struct item{
    int x;
    int y;
    int item_id;
    int delete_flag;
};
struct player{
    int x;
    int y;
    int id;
    int remain_bombs;
    int range;
    int is_dead;
};
struct explode_bomb{
    int x;
    int y;
    int dir;
    int owner;
    int range;
};

struct GameState{
    int width;
    int height;
    int myId;
    int delete_bomb[15][15];//boardから消えるbomb
    int delete_item[15][15];//boardから消えるitem
    int bomb_owner[15][15];//bombの存在する位置
    int item_id[15][15];//itemの存在する位置
    int explode_area[15][15];
    int bomb_range[15][15];
    vector<bomb>bombs;
    vector<item>items;
    string board[15];
    player p[4];
    int boxes_destroyed[4];
    int max_bombs[4];
    int score;
    string command;
    bool operator >(const GameState &n) const {
    return score < n.score;

  }
};
struct Action {
	int score;
    string command;
};

GameState explode(GameState state,int x,int y,int range,int owner){

    int visit[15][15]={0};

    memcpy(visit,state.explode_area,sizeof(visit));

    queue<explode_bomb>que;
            
    int dy[4]={-1,0,0,1};
    int dx[4]={0,-1,1,0};

    for(int i=0;i<4;i++){
    explode_bomb eb;
    eb.y=y;
    eb.x=x;
    eb.dir=i;
    eb.owner=owner;
    eb.range=range-1;
    que.push(eb);
    }

    state.delete_bomb[y][x]=114514;

    visit[y][x]=1;

    //cerr<<"range="<<range<<endl;

    string update_board[15];

    for(int i=0;i<state.height;i++){
        update_board[i]=state.board[i];
    }

    while (!que.empty()) {
        explode_bomb v = que.front(); que.pop();
        y=v.y;
        x=v.x;
        int ny=y+dy[v.dir];
        int nx=x+dx[v.dir];
        if(0<=ny&&ny<state.height && 0<=nx&&nx<state.width&&v.range>=1){
            if(state.board[ny][nx]=='.'){
                if(state.bomb_owner[ny][nx]>=0){
                    if(state.delete_bomb[ny][nx]!=114514){
                        for(int i=0;i<4;i++){
                            explode_bomb eb;
                            eb.y=ny;
                            eb.x=nx;
                            eb.dir=i;
                            eb.owner=state.bomb_owner[ny][nx];
                            //if(i==v.dir){eb.range=max(v.range-1,state.bomb_range[ny][nx]-1);}
                            //else{eb.range=state.bomb_range[ny][nx]-1;}
                            if(v.dir!=i){
                            eb.range=state.bomb_range[ny][nx]-1;
                            }
                            else{
                                if(v.range>state.bomb_range[ny][nx]){
                                    eb.range=min(v.range-1,state.bomb_range[ny][nx]-1);
                                }
                                else{
                                    eb.range=max(v.range-1,state.bomb_range[ny][nx]-1);
                                }
                            }
                            que.push(eb);
                        }
                        }
                        else{
                        explode_bomb eb;
                        eb.y=ny;
                        eb.x=nx;
                        eb.dir=v.dir;
                        eb.owner=v.owner;
                        eb.range=v.range-1;
                        que.push(eb);
                        }
                        state.delete_bomb[ny][nx]=114514;
                    }
                    else if(state.item_id[ny][nx]>=0){
                        state.delete_item[ny][nx]=114514;
                    }
                    else{
                        explode_bomb eb;
                        eb.y=ny;
                        eb.x=nx;
                        eb.dir=v.dir;
                        eb.owner=v.owner;
                        eb.range=v.range-1;
                        que.push(eb);
                    }
                    visit[ny][nx]=1;
                    //cerr<<"(nx,ny)->("<<nx<<","<<ny<<")"<<endl;
                }
                else if(state.board[ny][nx]=='0'){
                    state.boxes_destroyed[v.owner]++;
                    update_board[ny][nx]='.';
                    visit[ny][nx]=1;
                    //cerr<<"(nx,ny)->("<<nx<<","<<ny<<")"<<endl;
                }
                else if(state.board[ny][nx]=='1'){
                    state.boxes_destroyed[v.owner]++;
                    update_board[ny][nx]='.';
                    state.item_id[ny][nx]=1;
                    state.delete_item[ny][nx]=-1;
                    item it;
                    it.delete_flag=0;
                    it.item_id=1;
                    it.x=nx;
                    it.y=ny;
                    state.items.push_back(it);
                    visit[ny][nx]=1;
                    //cerr<<"(nx,ny)->("<<nx<<","<<ny<<")"<<endl;
                }
                else if(state.board[ny][nx]=='2'){
                    state.boxes_destroyed[v.owner]++;
                    update_board[ny][nx]='.';
                    state.item_id[ny][nx]=2;
                    state.delete_item[ny][nx]=-1;
                    item it;
                    it.delete_flag=0;
                    it.item_id=2;
                    it.x=nx;
                    it.y=ny;
                    state.items.push_back(it);
                    visit[ny][nx]=1;
                    //cerr<<"(nx,ny)->("<<nx<<","<<ny<<")"<<endl;
                }
            }
        }

        for(int i=0;i<state.height;i++){
        state.board[i]=update_board[i];
        }


    memcpy(state.explode_area,visit,sizeof(visit));

    return state;

}

//GameState explode(GameState state,int x,int y,int range,int owner)

GameState countdown_and_explode(GameState state){
    memset(state.explode_area,0,sizeof(state.explode_area));
    for(int i=0;i<(int)state.bombs.size();i++){
        state.bombs[i].remain_time--;
        if(state.bombs[i].remain_time==0){
            state=explode(state, state.bombs[i].x, state.bombs[i].y, state.bombs[i].range, state.bombs[i].owner);
            for(int j=0;j<player_num;j++){
                if(state.p[j].is_dead==0){
                if(state.explode_area[state.p[j].y][state.p[j].x]==1){
                    state.p[j].is_dead=1;
                }
                }
            }
        }
    }

/*
    int width;
    int height;
    int myId;

    int delete_bomb[15][15];//(y,x)の消去爆弾
    int bomb_owner[15][15];//(y,x)の爆弾のオーナー
    int bomb_range[15][15];//(y,x)の爆弾レンジ
    vector<bomb>bombs;

    int delete_item[15][15];//(y,x)の消去アイテム
    int item_id[15][15];//(y,x)のアイテムの種類
    vector<item>items;

    int explode_area[15][15];

    string board[15];
    player p[player_num];
    int boxes_destroyed[player_num];
    int max_bombs[player_num];
    int score;
    string command;
    */

    for(int i=(int)state.bombs.size()-1;i>=0;i--){
        if(state.delete_bomb[state.bombs[i].y][state.bombs[i].x]==114514){
            state.delete_bomb[state.bombs[i].y][state.bombs[i].x]=-1;
            state.bomb_owner[state.bombs[i].y][state.bombs[i].x]=-1;
            state.bomb_range[state.bombs[i].y][state.bombs[i].x]=-1;
            state.bombs.erase(state.bombs.begin()+i);
        }
    }
    for(int i=(int)state.items.size()-1;i>=0;i--){
        if(state.delete_item[state.items[i].y][state.items[i].x]==114514){
            state.delete_item[state.items[i].y][state.items[i].x]=-1;
            state.item_id[state.items[i].y][state.items[i].x]=-1;
            state.items.erase(state.items.begin()+i);
        }
    }
    return state;
}

GameState move(GameState state,int x,int y,int player){
    for(int i=0;i<player_num;i++){
        if(state.p[i].id==player&&state.board[y][x]=='.'&&state.bomb_owner[y][x]<0){
            if(state.item_id[y][x]==1){
                state.p[i].range++;
                state.delete_item[y][x]=114514;
            }
            else if(state.item_id[y][x]==2){
                state.p[i].remain_bombs++;
                state.max_bombs[i]++;
                state.delete_item[y][x]=114514;
            }
            state.p[i].x=x;
            state.p[i].y=y;
            break;
        }
    }
    for(int i=(int)state.items.size()-1;i>=0;i--){
        if(state.delete_item[state.items[i].y][state.items[i].x]==114514){
            state.delete_item[state.items[i].y][state.items[i].x]=-1;
            state.item_id[state.items[i].y][state.items[i].x]=-1;
            state.items.erase(state.items.begin()+i);
        }
    }
    return state;
}
GameState set_bombs(GameState state,int x,int y,int player){
    /*
     int delete_bomb[15][15];//(y,x)の消去爆弾
    int bomb_owner[15][15];//(y,x)の爆弾のオーナー
    int bomb_range[15][15];//(y,x)の爆弾レンジ
    vector<bomb>bombs;
    */
    if(state.p[player].remain_bombs>0){
    state.p[player].remain_bombs--;
    bomb b;
    b.owner=player;
    b.range=state.p[player].range;
    b.remain_time=8;
    b.x=state.p[player].x;
    b.y=state.p[player].y;
    b.delete_flag=0;
    state.bomb_owner[state.p[player].y][state.p[player].x]=player;
    state.bomb_range[state.p[player].y][state.p[player].x]=state.p[player].range;
    state.bombs.push_back(b);
    }

    state=move(state,x,y,player);

    return state;

}
int evalScore(GameState state,int player){

    int score=0;

    for(int i=0;i<player_num;i++){
        if(state.p[i].id==player&&state.p[i].is_dead==1){
            return -1000000;
        }
        else if(state.p[i].is_dead==1){
            score+=1000;
        }
    }

    for(int i=0;i<player_num;i++){
        if(state.p[i].id==player){
            score+=state.max_bombs[i]*100;
            score+=state.p[i].range*100;
        }
        else if(state.p[i].is_dead==0){
            score-=state.max_bombs[i]*100;
            score-=state.p[i].range*100;
            score-=state.boxes_destroyed[i]*1000;
        }
    }

    /*

    //typeA

    int canmove[player_num][11][15][15]={0};

    for(int i=0;i<player_num;i++){
    if(state.p[i].is_dead==0){
    canmove[i][0][state.p[i].y][state.p[i].x]=1;
    }
    }

    for(int i=0;i<player_num;i++){
    if(state.p[i].id!=state.myId&&state.p[i].is_dead==0){
    state=set_bombs(state,state.p[i].x,state.p[i].y,state.p[i].id);
    }
    }


    for(int t=0;t<10;t++){
        state=countdown_and_explode(state);
        for(int i=0;i<state.height;i++){
            for(int j=0;j<state.width;j++){
                for(int k=0;k<player_num;k++){                    
                if(canmove[k][t][i][j]==1){
                    if(state.board[i][j]=='.'&&state.bomb_owner[i][j]<0){
                        canmove[k][t+1][i][j]=1;
                    }
                    if(state.explode_area[i][j]==1){
                        canmove[k][t+1][i][j]=0;
                    }
                    if(i+1<state.height && state.board[i+1][j]=='.' &&state.bomb_owner[i+1][j]<0){
                        canmove[k][t+1][i+1][j]=1;
                    }
                    if(i+1<state.height &&state.explode_area[i+1][j]==1){
                        canmove[k][t+1][i+1][j]=0;
                    }
                    if(i-1>=0 && state.board[i-1][j]=='.'&&state.bomb_owner[i-1][j]<0){
                        canmove[k][t+1][i-1][j]=1;
                    }
                    if(i-1>=0 &&state.explode_area[i-1][j]==1){
                        canmove[k][t+1][i-1][j]=0;
                    }
                    if(j+1<state.width && state.board[i][j+1]=='.'&&state.bomb_owner[i][j+1]<0){
                        canmove[k][t+1][i][j+1]=1;
                    }
                    if(j+1<state.width &&state.explode_area[i][j+1]==1){
                        canmove[k][t+1][i][j+1]=0;
                    }
                    if(j-1>=0 && state.board[i][j-1]=='.'&&state.bomb_owner[i][j-1]<0){
                        canmove[k][t+1][i][j-1]=1;
                    } 
                    if(j-1>=0&&state.explode_area[i][j-1]==1){
                        canmove[k][t+1][i][j-1]=0;
                    }
                }
                }
            }
        }
    }

    */

    /*

    //typeB

    int canmove[15][15]={0};

    canmove[state.p[player].y][state.p[player].x]=1;

    int max_turn=10;

    for(int t=1;t<=max_turn;t++){
        state=countdown_and_explode(state);
        for(int i=0;i<state.height;i++){
            for(int j=0;j<state.width;j++){               
                if(state.explode_area[i][j]==1){
                    canmove[i][j]=0;
                }
                if(canmove[i][j]==1){
                    if(i+1<state.height && state.board[i+1][j]=='.' &&state.bomb_owner[i+1][j]<0){
                        canmove[i+1][j]=1;
                    }
                    if(i+1<state.height &&state.explode_area[i+1][j]==1){
                        canmove[i+1][j]=0;
                    }
                    if(i-1>=0 && state.board[i-1][j]=='.'&&state.bomb_owner[i-1][j]<0){
                        canmove[i-1][j]=1;
                    }
                    if(i-1>=0 &&state.explode_area[i-1][j]==1){
                        canmove[i-1][j]=0;
                    }
                    if(j+1<state.width && state.board[i][j+1]=='.'&&state.bomb_owner[i][j+1]<0){
                        canmove[i][j+1]=1;
                    }
                    if(j+1<state.width &&state.explode_area[i][j+1]==1){
                        canmove[i][j+1]=0;
                    }
                    if(j-1>=0 && state.board[i][j-1]=='.'&&state.bomb_owner[i][j-1]<0){
                        canmove[i][j-1]=1;
                    } 
                    if(j-1>=0&&state.explode_area[i][j-1]==1){
                        canmove[i][j-1]=0;
                    }
                }
            }
        }
    }

    */

    /*

    //typeC

    int canmove[11][15][15]={0};

    canmove[0][state.p[player].y][state.p[player].x]=1;

    for(int i=0;i<player_num;i++){
    if(state.p[i].id!=state.myId&&state.p[i].is_dead==0){
    state=set_bombs(state,state.p[i].x,state.p[i].y,state.p[i].id);
    }
    }


    for(int t=0;t<10;t++){
        state=countdown_and_explode(state);
        for(int i=0;i<state.height;i++){
            for(int j=0;j<state.width;j++){                 
                if(canmove[t][i][j]==1){
                    if(state.board[i][j]=='.'&&state.bomb_owner[i][j]<0){
                        canmove[t+1][i][j]=1;
                    }
                    if(state.explode_area[i][j]==1){
                        canmove[t+1][i][j]=0;
                    }
                    if(i+1<state.height && state.board[i+1][j]=='.' &&state.bomb_owner[i+1][j]<0){
                        canmove[t+1][i+1][j]=1;
                    }
                    if(i+1<state.height &&state.explode_area[i+1][j]==1){
                        canmove[t+1][i+1][j]=0;
                    }
                    if(i-1>=0 && state.board[i-1][j]=='.'&&state.bomb_owner[i-1][j]<0){
                        canmove[t+1][i-1][j]=1;
                    }
                    if(i-1>=0 &&state.explode_area[i-1][j]==1){
                        canmove[t+1][i-1][j]=0;
                    }
                    if(j+1<state.width && state.board[i][j+1]=='.'&&state.bomb_owner[i][j+1]<0){
                        canmove[t+1][i][j+1]=1;
                    }
                    if(j+1<state.width &&state.explode_area[i][j+1]==1){
                        canmove[t+1][i][j+1]=0;
                    }
                    if(j-1>=0 && state.board[i][j-1]=='.'&&state.bomb_owner[i][j-1]<0){
                        canmove[t+1][i][j-1]=1;
                    } 
                    if(j-1>=0&&state.explode_area[i][j-1]==1){
                        canmove[t+1][i][j-1]=0;
                    }
                }
            }
        }
    }

    */

    //latest

    /*

    for(int i=0;i<player_num;i++){
    if(state.p[i].id!=state.myId&&state.p[i].is_dead==0){
    state=set_bombs(state,state.p[i].x,state.p[i].y,state.p[i].id);
    }
    }
    */

    char canmove[15][15]={0};
    
    canmove[state.p[player].y][state.p[player].x]=1;

    int max_turn=10;

    for(int t=1;t<=max_turn;t++){
        state=countdown_and_explode(state);
        for(int i=0;i<state.height;i++){
            for(int j=0;j<state.width;j++){               
                if(state.explode_area[i][j]==1){
                    canmove[i][j]=0;
                }
                if(canmove[i][j]==1){
                    if(i+1<state.height && state.board[i+1][j]=='.' &&state.bomb_owner[i+1][j]<0){
                        canmove[i+1][j]=1;
                    }
                    if(i+1<state.height &&state.explode_area[i+1][j]==1){
                        canmove[i+1][j]=0;
                    }
                    if(i-1>=0 && state.board[i-1][j]=='.'&&state.bomb_owner[i-1][j]<0){
                        canmove[i-1][j]=1;
                    }
                    if(i-1>=0 &&state.explode_area[i-1][j]==1){
                        canmove[i-1][j]=0;
                    }
                    if(j+1<state.width && state.board[i][j+1]=='.'&&state.bomb_owner[i][j+1]<0){
                        canmove[i][j+1]=1;
                    }
                    if(j+1<state.width &&state.explode_area[i][j+1]==1){
                        canmove[i][j+1]=0;
                    }
                    if(j-1>=0 && state.board[i][j-1]=='.'&&state.bomb_owner[i][j-1]<0){
                        canmove[i][j-1]=1;
                    } 
                    if(j-1>=0&&state.explode_area[i][j-1]==1){
                        canmove[i][j-1]=0;
                    }
                }
            }
        }
    }

    /*

    //botu

    int maxt=0;

    for(int i=0;i<player_num;i++){
    if(state.p[i].id!=state.myId&&state.p[i].is_dead==0){
    state=set_bombs(state,state.p[i].x,state.p[i].y,state.p[i].id);
    }
    }

    int canmove[11][15][15]={0};
    
    canmove[0][state.p[player].y][state.p[player].x]=1;

    queue<kiki>que;
        
    int dy[5]={-1,0,0,1,0};  
    int dx[5]={0,-1,1,0,0};

    kiki root;
    root.x=state.p[player].x;
    root.y=state.p[player].y;
    root.t=0;

    que.push(root);

    GameState state_layer[11];

    state_layer[0]=state;

    for(int i=0;i<10;i++){
    state_layer[i+1]=countdown_and_explode(state_layer[i]);
    }

    while (!que.empty()) {
        kiki v = que.front(); que.pop();
        int Y=v.y;
        int X=v.x;
        int T=v.t;
        if(canmove[T][Y][X]==1&&state_layer[T].explode_area[Y][X]==0){
        for (int dir=0;dir<5;dir++) {
        	int ny=v.y+dy[dir];
        	int nx=v.x+dx[dir];
            int nt=v.t+1;
        	if(0<=ny&&ny<state.height && 0<=nx&&nx<state.width&&nt<11){
            if (canmove[nt][ny][nx] == 0&& state_layer[nt].explode_area[ny][nx]==0&&
            state_layer[nt].board[ny][nx]=='.'&&state_layer[nt].bomb_owner[ny][nx]<0) {
            canmove[nt][ny][nx]=1;
            maxt=max(maxt,nt);
            kiki next_kiki;
            next_kiki.y=ny;
            next_kiki.x=nx;
            next_kiki.t=nt;
            que.push(next_kiki);
            }
            }
        }
    }
    }
    */

    int add=0;

    int box_distance=100;

    /*

    for(int i=0;i<state.height;i++){
        for(int j=0;j<state.width;j++){
            //if(state.bomb_owner[i][j]==player){score+=50;}
            if(state.board[i][j]=='0' ||state.board[i][j]=='1'||state.board[i][j]=='2'){
                box_distance=min(box_distance,max(i-state.p[player].y,state.p[player].y-i)+max(j-state.p[player].x,state.p[player].x-j));
            }
            for(int k=0;k<player_num;k++){
            if(state.p[k].id==player){
            if(canmove[state.p[k].id][10][i][j]==1){add+=100;}
            }
            else{
            if(canmove[state.p[k].id][10][i][j]==1){score--;}
            }
            }  
        }
    }
    */

    
    for(int i=0;i<state.height;i++){
        for(int j=0;j<state.width;j++){
            if(state.board[i][j]=='0' ||state.board[i][j]=='1'||state.board[i][j]=='2'){
                box_distance=min(box_distance,max(i-state.p[player].y,state.p[player].y-i)+max(j-state.p[player].x,state.p[player].x-j));
            }
            if(canmove[i][j]==1){
                if(i-1>=0&&(state.board[i-1][j]=='0' ||state.board[i-1][j]=='1'||state.board[i-1][j]=='2')){
                    score++;
                }
                if(i+1<state.height&&(state.board[i+1][j]=='0' ||state.board[i+1][j]=='1'||state.board[i+1][j]=='2')){
                    score++;
                }
                if(j-1>=0&&(state.board[i][j-1]=='0' ||state.board[i][j-1]=='1'||state.board[i][j-1]=='2')){
                    score++;
                }
                if(j+1<state.width&&(state.board[i][j+1]=='0' ||state.board[i][j+1]=='1'||state.board[i][j+1]=='2')){
                    score++;
                }
                add+=100;
            }
        }
    }



    score-=box_distance;

    score+=state.boxes_destroyed[state.myId]*1000;

    if(add==0){
        score=-1000000;
    }
    else{
        score+=add;
    }

    return score;
}
unsigned int rnd(int mini, int maxi) {
	static mt19937 mt((int)time(0));
	uniform_int_distribution<int> dice(mini, maxi);
	return dice(mt);
}

Action MC(GameState state,double timelimit){

    int SEARCH_DEPTH=6;//10->rank_60,9->,8->,7->,6->rank_46,5->

    Action bestAction;

    int dx[5]={-1,0,0,0,1};
    int dy[5]={0,-1,0,1,0};

    int ev=-INT_MAX;

    GameState tmp=state;

    unordered_map<int, bool> checkNodeList;

    int pow10[10]={0};

    pow10[0]=1;

    for(int i=1;i<=8;i++){
        pow10[i]=10*pow10[i-1];
    }

    while(1){

    double elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;

    if(elapsed>=timelimit){return bestAction;}

    string action[SEARCH_DEPTH];

    for(int m=0;m<SEARCH_DEPTH;m++){
        action[m]="";
    }

    state=tmp;

    int hash=0;

    for(int depth=0;depth<SEARCH_DEPTH;depth++){

        state=countdown_and_explode(state);

        int set;
        int dir;

        map<pair<int,int>, int>mp;

        int cnt=0;

        int nx;
        int ny;

        while(1){
            set=rnd(0,1);
            dir=rnd(0,4);
            if(set==0){
                nx=state.p[state.myId].x+dx[dir];
                ny=state.p[state.myId].y+dy[dir];
                if(0<=ny&&ny<state.height&&0<=nx&&nx<state.width&&state.board[ny][nx]=='.'&&state.bomb_owner[ny][nx]<0){break;}
            }
            else if(state.p[state.myId].remain_bombs>0){
                nx=state.p[state.myId].x+dx[dir];
                ny=state.p[state.myId].y+dy[dir];
                if(0<=ny&&ny<state.height&&0<=nx&&nx<state.width&&state.board[ny][nx]=='.'&&state.bomb_owner[ny][nx]<0){break;}
            }
            mp[make_pair(set,dir)]=1;
            cnt=0;
            for(int i=0;i<2;i++){
                for(int j=0;j<5;j++){
                    cnt+=mp[make_pair(i,j)];
                }
            }
            if(cnt==10){break;}
        }

        if(cnt==10){
            break;
        }

        hash+=((set*5)+dir)*pow10[depth];

        if(set==0){
        state=move(state,nx,ny,state.myId);
        action[depth]="MOVE "+to_string(nx)+" "+to_string(ny);
        }
        else{
        state=set_bombs(state,nx,ny,state.myId);
        action[depth]="BOMB "+to_string(nx)+" "+to_string(ny);
        }
        }
        if(!checkNodeList[hash]){

        int e=evalScore(state, state.myId);
        if(e>ev){
         ev=e;
         bestAction.command=action[0];
         bestAction.score=ev;   
        }
        checkNodeList[hash]=true;
        }

    }
    return bestAction;
}

void test(GameState state){

    memset(state.explode_area,0,sizeof(state.explode_area));
    for(int i=0;i<(int)state.bombs.size();i++){
        state.bombs[i].remain_time--;
        if(state.bombs[i].remain_time==0){
            state=explode(state, state.bombs[i].x, state.bombs[i].y, state.bombs[i].range, state.bombs[i].owner);
            for(int j=0;j<player_num;j++){
                if(state.p[j].is_dead==0){
                if(state.explode_area[state.p[j].y][state.p[j].x]==1){
                    state.p[j].is_dead=1;
                }
                }
            }
        }
    }

    for(int i=0;i<state.height;i++){
        for(int j=0;j<state.width;j++){
            cerr<<state.explode_area[i][j];
        }
        cerr<<endl;
    }
    for(int i=0;i<state.height;i++){
        for(int j=0;j<state.width;j++){
            if(state.bomb_range[i][j]>0){
                cerr<<"range="<<state.bomb_range[i][j]<<",(x,y)->("<<j<<","<<i<<")"<<endl;
            }
        }
    }
}
int main()
{
    int width;
    int height;
    int myId;
    cin >> width >> height >> myId; cin.ignore();

    // game loop
    for (int t=0;;t++) {

        GameState state;

        for(int i=0;i<15;i++){
            for(int j=0;j<15;j++){
                state.bomb_owner[i][j]=-1;
                state.item_id[i][j]=-1;
                state.delete_bomb[i][j]=-1;
                state.delete_item[i][j]=-1;
                state.bomb_range[i][j]=-1;
            }
        }

        state.width=width;
        state.height=height;
        state.myId=myId;
        for(int i=0;i<4;i++){
        state.boxes_destroyed[i]=0;
        state.max_bombs[i]=1;
        state.p[i].is_dead=1;
        state.p[i].id=-1;
        }

        for (int i = 0; i < height; i++) {
            string row;
            cin >> row; cin.ignore();
            state.board[i]=row;
        }
        int entities;
        cin >> entities; cin.ignore();
        for (int i = 0; i < entities; i++) {
            int entityType;
            int owner;
            int x;
            int y;
            int param1;
            int param2;
            cin >> entityType >> owner >> x >> y >> param1 >> param2; cin.ignore();
            if(entityType==0){
                player p;
                p.id=owner;
                p.x=x;
                p.y=y;
                p.remain_bombs=param1;
                p.range=param2;
                p.is_dead=0;
                state.p[owner]=p;
            }
            else if(entityType==1){
                bomb b;
                b.owner=owner;
                b.range=param2;
                b.remain_time=param1;
                b.x=x;
                b.y=y;
                b.delete_flag=0;
                state.bomb_range[y][x]=param2;
                state.bomb_owner[y][x]=owner;
                state.bombs.push_back(b);
            }
            else if(entityType==2){
                item it;
                it.item_id=param1;
                it.x=x;
                it.y=y;
                it.delete_flag=0;
                state.item_id[y][x]=param1;
                state.items.push_back(it);
            }
        }
        start = chrono::system_clock::now();
        Action a;
        if(t==0){
        a=MC(state,0.9);
        }
        else{
        a=MC(state,0.09);
        }
        if(a.command==""){
            cout<<"MOVE "<<state.p[state.myId].x<<" "<<state.p[state.myId].y<<endl;
        }
        else{
        cout<<a.command<<endl;
        }
        cerr<<"score="<<a.score<<endl;
        double elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;
        cerr<<"elapsed="<<elapsed<<endl;
        //test(state);
    }
    return 0;
}
