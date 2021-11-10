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

typedef long long ll;
typedef char F_T;

#define FIELD_WIDTH 6
#define FIELD_HEIGHT 12
#define WIDTH 9
#define HEIGHT 15
#define DROP 6
#define MAX_TURN 300
#define BEAM_WIDTH 100
#define SEARCH_DEPTH 15
#define ROT 4

chrono::system_clock::time_point start;

struct Pack {
	int p[2];

	Pack() {
		memset(p, -1, sizeof(p));
	}
};
struct Action {
	int command;
	int value;
	int fireTurn;
    int score;
    int r;

	Action(int command = -1, int value = 0, int fireTurn = MAX_TURN,int score=0,int r=-1) {
		this->command = command;
		this->value = value;
		this->fireTurn = fireTurn;
        this->score = score;
        this->r=r;
	}
};
struct Node {
	int value;
	int score;
	bool chain;
	int command;
    int r;
	F_T field[HEIGHT][WIDTH];

	Node(int value=0,int score=0,bool chain=false,int command = -1,int r=-1) {
		this->value = value;
        this->score=score;
        this->chain=chain;
        this->command=command;
        this->r=r;
	}

	bool operator >(const Node& n) const {
		return value < n.value;
	}
}fff[BEAM_WIDTH * FIELD_WIDTH * ROT];

unsigned long long xor128() {
	static unsigned long long rx = 123456789, ry = 362436069, rz = 521288629, rw = 88675123;
	unsigned long long rt = (rx ^ (rx << 11));
	rx = ry; ry = rz; rz = rw;
	return (rw = (rw ^ (rw >> 19)) ^ (rt ^ (rt >> 8)));
}

ll hashCode(F_T board[HEIGHT][WIDTH], ll g_zoblishField[HEIGHT][WIDTH][DROP+1]) {
	ll hash = 0;
	for (int y = 1; y <= FIELD_HEIGHT; ++y) {
		for (int x = 1; x <= FIELD_WIDTH; ++x) {
			int num = (int)board[y][x];
			if (num == 0) { continue; }
			hash ^= g_zoblishField[y][x][num];
		}
	}

	return hash;
}

void setPutPackLine(int x, F_T board[HEIGHT][WIDTH], F_T g_putPackLine[WIDTH]) {
	int y = 1;

	while (board[y][x] != 0 && y < FIELD_HEIGHT+1) {
		++y;
    }

	g_putPackLine[x] = y;
}
void updatePutPackLine(F_T board[HEIGHT][WIDTH], F_T g_putPackLine[WIDTH]) {
	for (int x = 1; x <= FIELD_WIDTH; ++x) {
		setPutPackLine(x, board, g_putPackLine);
	}
}
void readPackInfo(int t,Pack g_packs[MAX_TURN]) {

	for (int i = 0; i < 8; i++) {
        Pack pack;
		int colorA; // color of the first block
        int colorB; // color of the attached block
        cin >> colorA >> colorB; cin.ignore();

		pack.p[0] = colorA; pack.p[1] = colorB;

		g_packs[t+i] = pack;
	}
}
bool rot0_putLinePack(int x, int t0, int t1, F_T g_putPackLine[WIDTH], F_T board[HEIGHT][WIDTH],
	F_T g_chainCheck[HEIGHT][WIDTH]) {

	int y = g_putPackLine[x];

	if (t0 != 0) {
		if (y < 1 || y > FIELD_HEIGHT) { return false; }
		board[y][x] = t0;
		g_chainCheck[y][x] = 1;
		++x;
	}
    g_putPackLine[x-1] = y+1;

    y=g_putPackLine[x];

	if (t1 != 0) {
		if (y < 1 || y > FIELD_HEIGHT) { return false; }
		board[y][x] = t1;
		g_chainCheck[y][x] = 1;
		++x;
	}
	g_putPackLine[x-1] = y+1;
	return true;
}
bool rot1_putLinePack(int x, int t0, int t1, F_T g_putPackLine[WIDTH], F_T board[HEIGHT][WIDTH],
	F_T g_chainCheck[HEIGHT][WIDTH]) {

	int y = g_putPackLine[x];

	if (t0 != 0) {
		if (x < 1 || x > FIELD_WIDTH) { return false; }
		board[y][x] = t0;
		g_chainCheck[y][x] = 1;
		++y;
	}
	if (t1 != 0) {
		if (x < 1 || x > FIELD_WIDTH) { return false; }
		board[y][x] = t1;
		g_chainCheck[y][x] = 1;
		++y;
	}
	g_putPackLine[x] = y;
	return true;
}
bool rot2_putLinePack(int x, int t0, int t1, F_T g_putPackLine[WIDTH], F_T board[HEIGHT][WIDTH],
	F_T g_chainCheck[HEIGHT][WIDTH]) {

	int y = g_putPackLine[x];

	if (t1 != 0) {
		if (y < 1 || y > FIELD_HEIGHT) { return false; }
		board[y][x] = t1;
		g_chainCheck[y][x] = 1;
		--x;
	}
	g_putPackLine[x+1] = y+1;

    y=g_putPackLine[x];

    if(t0!=0){
        if(y<1 || y > FIELD_HEIGHT){return false;}
        board[y][x]=t0;
        g_chainCheck[y][x]=1;
        --x;
    }
    g_putPackLine[x+1]=y+1;

	return true;
}
bool putPack(int x,int rot,const Pack& pack, F_T g_putPackLine[WIDTH], F_T board[HEIGHT][WIDTH]
	, F_T g_chainCheck[HEIGHT][WIDTH]) {
	
    bool success = true;

    if(rot==0){

    success &= rot0_putLinePack(x + 1, pack.p[0], pack.p[1], g_putPackLine, board, g_chainCheck);

    }
    else if(rot==1){
    success &= rot1_putLinePack(x + 1, pack.p[0], pack.p[1], g_putPackLine, board, g_chainCheck);
    }
    else if(rot==2){

    success &= rot2_putLinePack(x + 1, pack.p[1], pack.p[0], g_putPackLine, board, g_chainCheck);

    }
    else{
    success &= rot1_putLinePack(x + 1, pack.p[1], pack.p[0], g_putPackLine, board, g_chainCheck);
    }

	return success;
}
void updateMaxHeight(int* g_maxHeight, F_T g_putPackLine[WIDTH]) {
	*g_maxHeight = 0;

	for (int x = 1; x <= FIELD_WIDTH; ++x) {
		*g_maxHeight = max(*g_maxHeight, g_putPackLine[x] - 1);
	}
}
int chain(int nrw, int ncl, F_T d, F_T board[HEIGHT][WIDTH],
	F_T chkflag[HEIGHT][WIDTH]) {
	int count = 0;
#define CHK_CF(Y,X) (board[Y][X] == d && chkflag[Y][X]==0 && 1<=d&&d<DROP)
	if (CHK_CF(nrw, ncl)) {
		++count;
		chkflag[nrw][ncl]=1;
		if (1 < nrw && CHK_CF(nrw - 1, ncl)) {
			count += chain(nrw - 1, ncl, d, board, chkflag);
		}
		if (nrw < FIELD_HEIGHT && CHK_CF(nrw + 1, ncl)) {
			count += chain(nrw + 1, ncl, d, board, chkflag);
		}
		if (1 < ncl && CHK_CF(nrw, ncl - 1)) {
			count += chain(nrw, ncl - 1, d, board, chkflag);
		}
		if (ncl < FIELD_WIDTH && CHK_CF(nrw, ncl + 1)) {
			count += chain(nrw, ncl + 1, d, board, chkflag);
		}
	}
	return count;
}
//g_chaincheckは消えるか怪しいマスで設置or落下のみ、g_packdeletecheckerは消えるマス
int calc_score(int g_maxHeight, F_T board[HEIGHT][WIDTH], 
F_T g_packDeleteChecker[HEIGHT][WIDTH], F_T g_chainCheck[HEIGHT][WIDTH],int* CB,int* GB) {
    
    int score=0;
    F_T chkflag[HEIGHT][WIDTH]={0};
    F_T unchkflag[HEIGHT][WIDTH]={0};

    int dy[4]={-1,0,0,1};
    int dx[4]={0,-1,1,0};

    int erase_color[DROP]={0};

    int cb=0;

	for (int y = 1; y <= g_maxHeight; y++) {
		for (int x = 1; x <= FIELD_WIDTH; x++) {
			if (g_chainCheck[y][x] == 1) {
                if(chkflag[y][x]==0){
                int sc=chain(y,x,board[y][x],board,chkflag);
                if(sc>=4){
                    score+=sc*10;
                    if(sc==5){cb+=1;}
                    else if(sc==6){cb+=2;}
                    else if(sc==7){cb+=3;}
                    else if(sc==8){cb+=4;}
                    else if(sc==9){cb+=5;}
                    else if(sc==10){cb+=6;}
                    else if(sc>=11){cb+=8;}
                }
                else{chain(y,x,board[y][x],board,unchkflag);}
                }
				g_chainCheck[y][x] = 0;
            }
        }
    }
    for(int y=1;y<=g_maxHeight;y++){
        for(int x=1;x<=FIELD_WIDTH;x++){
            if(chkflag[y][x]==1&&unchkflag[y][x]==0){
                g_packDeleteChecker[y][x]=1;               
                for(int k=0;k<4;k++){
                    if(1<=y+dy[k]&&y+dy[k]<=FIELD_HEIGHT&&1<=x+dx[k]&&x+dx[k]<=FIELD_WIDTH){
                        if(board[y+dy[k]][x+dx[k]]==DROP){
                            g_packDeleteChecker[y+dy[k]][x+dx[k]]=1;
                        }
                    }
                }
                if(0<board[y][x]&&board[y][x]<DROP){
                    erase_color[board[y][x]]=1;
                }
            }
        }
    }

    int color=0;

    for(int i=1;i<DROP;i++){
        color+=erase_color[i];
    }

    if(color==2){*CB=2;}
    else if(color==3){*CB=4;}
    else if(color==4){*CB=8;}
    else if(color==5){*CB=16;}

    *GB=cb;

    return score;
}
void fallPack(F_T board[HEIGHT][WIDTH], F_T g_putPackLine[WIDTH], F_T g_packDeleteChecker[HEIGHT][WIDTH],
	F_T g_chainCheck[HEIGHT][WIDTH]) {

	for (int x = 1; x <= FIELD_WIDTH; ++x) {
		int fallCnt = 0;
		int limitY = g_putPackLine[x];

		for (int y = 1; y < limitY; ++y) {
			if (g_packDeleteChecker[y][x] == 1) {
				board[y][x] = 0;
				fallCnt++;
				g_putPackLine[x]--;
			}
			else if (fallCnt > 0) {
				int t = y - fallCnt;
				board[t][x] = board[y][x];
				board[y][x] = 0;
                g_chainCheck[t][x]=1;
			}
		}
	}
}
int simulate(int* g_maxHeight, F_T g_putPackLine[WIDTH], F_T board[HEIGHT][WIDTH], F_T g_packDeleteChecker[HEIGHT][WIDTH],
	F_T g_chainCheck[HEIGHT][WIDTH]) {

	int score = 0;
	int g_deleteCount = 0;
    int CP=0;

	F_T tmp[HEIGHT][WIDTH];

	for (int t=0;;t++) {

        if(t==0){CP=0;}
        else if(t==1){CP=8;}
        else{CP*=2;}

		updateMaxHeight(g_maxHeight, g_putPackLine);

        int CB=0;
        int GB=0;

		int B=calc_score(*g_maxHeight, board, g_packDeleteChecker, g_chainCheck,&CB,&GB);

		fallPack(board, g_putPackLine, g_packDeleteChecker, g_chainCheck);

		memset(g_packDeleteChecker, 0, sizeof(tmp));
        
        score+=B*max(CP+CB+GB,1);

		if (B== 0) { break; }

	}

	return score;
}
int evaluate(int* g_maxHeight, F_T board[HEIGHT][WIDTH], F_T g_putPackLine[WIDTH],
	F_T g_packDeleteChecker[HEIGHT][WIDTH], F_T g_chainCheck[HEIGHT][WIDTH]){
        
    int maxValue = 0;
	F_T g_tempField[HEIGHT][WIDTH];
	F_T g_tempPutPackLine[WIDTH];
	memcpy(g_tempField, board, sizeof(g_tempField));
	memcpy(g_tempPutPackLine, g_putPackLine, sizeof(g_tempPutPackLine));

	int dy[4] = {-1,0,0,1};
	int dx[4] = {0,-1,1,0};

	for (int x = 1; x <= FIELD_WIDTH; x++) {

		for (int num = 1; num < DROP; num++) {

			memset(g_packDeleteChecker, 0, sizeof(g_tempField));
			memset(g_chainCheck, 0, sizeof(g_tempField));

			int y = g_putPackLine[x];

			int ok=0;

            F_T chkflag[HEIGHT][WIDTH]={0};

            board[y][x]=(F_T)num;

            int sc=chain(y,x,(F_T)num,board,chkflag);            

            if(sc>=4){ok=1;}

            board[y][x]=0;            

			if(ok==1){

			board[y][x] = (F_T)num;

            g_chainCheck[y][x]=1;

			g_putPackLine[x] = y + 1;

			int score = simulate(g_maxHeight, g_putPackLine, board, g_packDeleteChecker, g_chainCheck);

			maxValue = max(maxValue, score);

			memcpy(board, g_tempField, sizeof(g_tempField));
			memcpy(g_putPackLine, g_tempPutPackLine, sizeof(g_tempPutPackLine));
			}

            }
		}
        
        int friend_x_pos[FIELD_WIDTH+1][DROP] = { 0 };
        
        for (int x = 1; x <= FIELD_WIDTH; x++) {
		
		int limit_y=(int)g_tempPutPackLine[x];
		
		for(int y=1;y<limit_y;y++){	
			if (0 < board[y][x] && board[y][x] < DROP) {
				friend_x_pos[x][board[y][x]]++;
                }
            }
        }
        
        int ev=maxValue;

        int add=0;
        
        for (int i = 1; i < DROP; i++) {
            for (int x = 1; x <= FIELD_WIDTH; x++) {
			if (2 <= x) {
				add += min(friend_x_pos[x][i], friend_x_pos[x - 1][i]);
			}
			if (x < FIELD_WIDTH) {
				add += min(friend_x_pos[x][i], friend_x_pos[x + 1][i]);
			}
			add += friend_x_pos[x][i];
		}
	}

    ev+=add;

    return ev;
}

Action getBestAction(int turn, F_T board[HEIGHT][WIDTH], Pack g_packs[MAX_TURN], int g_scoreLimit,ll g_zoblishField[HEIGHT][WIDTH][DROP+1]) {

	F_T tmp1[HEIGHT][WIDTH];

	Node root;
	memcpy(root.field, board, sizeof(tmp1));
	Action bestAction;
	int maxValue = -9999;

	deque<Node> que;
	que.push_back(root);

	unordered_map<ll, bool> checkNodeList;

	for (int depth = 0; depth < SEARCH_DEPTH; depth++) {
		int update = 0;
		int ks = (int)que.size();
        for(int k=0;k<BEAM_WIDTH * FIELD_WIDTH * ROT;k++){
            fff[k].command=-1;
        }
		for (int k = 0; k < ks; k++) {

            double elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;

            if(elapsed>0.09){return bestAction;}

			Pack pack = g_packs[turn + depth];

            if(depth>=8){
                pack.p[0]=(int)(xor128()%5)+1;
                pack.p[1]=(int)(xor128()%5)+1;
            }

			Node node = que[k];

			F_T g_tempfield[HEIGHT][WIDTH];
			F_T g_putPackLine[WIDTH];
			F_T g_tempPutPackLine[WIDTH];
			F_T g_packDeleteChecker[HEIGHT][WIDTH];
			F_T g_chainCheck[HEIGHT][WIDTH];

			memcpy(g_tempfield, node.field, sizeof(g_tempfield));
			updatePutPackLine(g_tempfield, g_putPackLine);
			memcpy(g_tempPutPackLine, g_putPackLine, sizeof(g_putPackLine));

			for (int x = 0; x < FIELD_WIDTH; x++) {

                for(int rot=1;rot<=3;rot+=2){

					memset(g_packDeleteChecker, 0, sizeof(g_packDeleteChecker));
					memset(g_chainCheck, 0, sizeof(g_chainCheck));

					if (putPack(x, rot,pack, g_putPackLine, g_tempfield,
						g_chainCheck)) {
						Node cand;
						int g_maxHeight;
						cand.score = simulate(&g_maxHeight, g_putPackLine, g_tempfield, g_packDeleteChecker, g_chainCheck);
						if (g_maxHeight <= FIELD_HEIGHT) {
							cand.value = cand.score;
							memcpy(cand.field, g_tempfield, sizeof(g_tempfield));
							cand.chain = (cand.score > 0);
							if (!cand.chain) {
								cand.value += evaluate(&g_maxHeight, g_tempfield, g_putPackLine, g_packDeleteChecker,
									g_chainCheck);
								update++;
							}
                            if (cand.score >= g_scoreLimit) { cand.value += 100 * cand.score; }
							cand.command = (depth == 0) ? x : node.command;
                            cand.r = (depth == 0) ? rot : node.r;
							fff[(FIELD_WIDTH*ROT*k)+(ROT*x)+rot] = cand;
						}
						else {
							cand.value = -1;
							cand.score = -1;
                            cand.command=-1;
							fff[(FIELD_WIDTH*ROT*k)+(ROT*x)+rot] = cand;
						}
					}
					else {
						Node cand2;
						cand2.value = -1;
						cand2.score = -1;
                        cand2.command=-1;
						fff[(FIELD_WIDTH*ROT*k)+(ROT*x)+rot] = cand2;
					}
					memcpy(g_tempfield, node.field, sizeof(node.field));
					memcpy(g_putPackLine, g_tempPutPackLine, sizeof(g_tempPutPackLine));
				}
			}
            for (int x = 0; x < FIELD_WIDTH-1; x++) {

                for(int rot=0;rot<=0;rot++){

					memset(g_packDeleteChecker, 0, sizeof(g_packDeleteChecker));
					memset(g_chainCheck, 0, sizeof(g_chainCheck));

					if (putPack(x, rot,pack, g_putPackLine, g_tempfield,
						g_chainCheck)) {
						Node cand;
						int g_maxHeight;
						cand.score = simulate(&g_maxHeight, g_putPackLine, g_tempfield, g_packDeleteChecker, g_chainCheck);
						if (g_maxHeight <= FIELD_HEIGHT) {
							cand.value = cand.score;
							memcpy(cand.field, g_tempfield, sizeof(g_tempfield));
							cand.chain = (cand.score > 0);
							if (!cand.chain) {
								cand.value += evaluate(&g_maxHeight, g_tempfield, g_putPackLine, g_packDeleteChecker,
									g_chainCheck);
								update++;
							}
                            if (cand.score >= g_scoreLimit) { cand.value += 100 * cand.score; }
							cand.command = (depth == 0) ? x : node.command;
                            cand.r = (depth == 0) ? rot : node.r;
							fff[(FIELD_WIDTH*ROT*k)+(ROT*x)+rot] = cand;
						}
						else {
							cand.value = -1;
							cand.score = -1;
							cand.command=-1;
							fff[(FIELD_WIDTH*ROT*k)+(ROT*x)+rot] = cand;
						}
					}
					else {
						Node cand2;
						cand2.value = -1;
						cand2.score = -1;
						cand2.command=-1;
						fff[(FIELD_WIDTH*ROT*k)+(ROT*x)+rot] = cand2;
					}
					memcpy(g_tempfield, node.field, sizeof(node.field));
					memcpy(g_putPackLine, g_tempPutPackLine, sizeof(g_tempPutPackLine));
				}
			}
            for (int x = 1; x < FIELD_WIDTH; x++) {

                for(int rot=2;rot<=2;rot++){

					memset(g_packDeleteChecker, 0, sizeof(g_packDeleteChecker));
					memset(g_chainCheck, 0, sizeof(g_chainCheck));

					if (putPack(x, rot,pack, g_putPackLine, g_tempfield,
						g_chainCheck)) {
						Node cand;
						int g_maxHeight;
						cand.score = simulate(&g_maxHeight, g_putPackLine, g_tempfield, g_packDeleteChecker, g_chainCheck);
						if (g_maxHeight <= FIELD_HEIGHT) {
							cand.value = cand.score;
							memcpy(cand.field, g_tempfield, sizeof(g_tempfield));
							cand.chain = (cand.score > 0);
							if (!cand.chain) {
								cand.value += evaluate(&g_maxHeight, g_tempfield, g_putPackLine, g_packDeleteChecker,
									g_chainCheck);
								update++;
							}
                            if (cand.score >= g_scoreLimit) { cand.value += 100 * cand.score; }
							cand.command = (depth == 0) ? x : node.command;
                            cand.r = (depth == 0) ? rot : node.r;
							fff[(FIELD_WIDTH*ROT*k)+(ROT*x)+rot] = cand;
						}
						else {
							cand.value = -1;
							cand.score = -1;
							cand.command=-1;
							fff[(FIELD_WIDTH*ROT*k)+(ROT*x)+rot] = cand;
						}
					}
					else {
						Node cand2;
						cand2.value = -1;
						cand2.score = -1;
						cand2.command=-1;
						fff[(FIELD_WIDTH*ROT*k)+(ROT*x)+rot] = cand2;
					}
					memcpy(g_tempfield, node.field, sizeof(node.field));
					memcpy(g_putPackLine, g_tempPutPackLine, sizeof(g_tempPutPackLine));
				}
			}
        }
		que.clear();
		vector<pair<int, int> >vec;
		int ks2 = 0;
		for (int j = 0; j < FIELD_WIDTH*ROT*ks; j++) {
			if (fff[j].command != -1) {
				vec.push_back(make_pair(fff[j].value, j));
				ks2++;
			}
		}
		sort(vec.begin(), vec.end());
		int push_node=0;
		for (int j = 0; push_node < BEAM_WIDTH && j < ks2; j++) {
            double elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;

            if(elapsed>0.09){return bestAction;}

			Node node = fff[vec[ks2-1-j].second];

			if (node.score >= g_scoreLimit) {
				return Action(node.command, node.value, turn + depth,node.score,node.r);
			}

			if (maxValue < node.value) {
				maxValue = node.value;
				bestAction = Action(node.command, node.value, turn + depth,node.score,node.r);
			}

			if (depth < SEARCH_DEPTH - 1) {
				ll hash = hashCode(node.field, g_zoblishField);

				if (!checkNodeList[hash] && (!node.chain || (update == 0 && node.score <= 40))) {
					checkNodeList[hash] = true;
					que.push_back(node);
					push_node++;
				}
			}
		}
	}

	return bestAction;
}

int main()
{

    F_T board[HEIGHT][WIDTH]={0};
    ll g_zoblishField[HEIGHT][WIDTH][DROP+1];

    for(int i=0;i<HEIGHT;i++){
        for(int j=0;j<WIDTH;j++){
            for(int k=0;k<=DROP;k++){
                g_zoblishField[i][j][k]=xor128();
            }
        }
    }

    Pack g_packs[MAX_TURN];

    // game loop
    for (int t=0;;t++) {
        readPackInfo(t,g_packs);
        int score1;
        cin >> score1; cin.ignore();
        for (int i = 0; i < FIELD_HEIGHT; i++) {
            string row; // One line of the map ('.' = empty, '0' = skull block, '1' to '5' = colored block)
            cin >> row; cin.ignore();
            for (int x = 1; x <= FIELD_WIDTH; ++x) {
                if(row[x-1]=='.'){
                    board[FIELD_HEIGHT-i][x]=0;
                }
                else if(row[x-1]=='0'){
                    board[FIELD_HEIGHT-i][x]=DROP;
                }
                else if(row[x-1]=='1'){
                    board[FIELD_HEIGHT-i][x]=1;
                }
                else if(row[x-1]=='2'){
                    board[FIELD_HEIGHT-i][x]=2;
                }
                else if(row[x-1]=='3'){
                    board[FIELD_HEIGHT-i][x]=3;
                }
                else if(row[x-1]=='4'){
                    board[FIELD_HEIGHT-i][x]=4;
                }
                else if(row[x-1]=='5'){
                    board[FIELD_HEIGHT-i][x]=5;
                }
            }
        }
        int score2;
        cin >> score2; cin.ignore();
        for (int i = 0; i < FIELD_HEIGHT; i++) {
            string row;
            cin >> row; cin.ignore();
        }
         start = chrono::system_clock::now();
         Action a;
         int skull=0;
         for(int i=1;i<=FIELD_HEIGHT;i++){
             for(int j=1;j<=FIELD_WIDTH;j++){
                 if(board[i][j]==DROP){skull++;}
             }
         }
         if(skull<=28){a=getBestAction(t, board, g_packs, 1500,g_zoblishField);}
         else if(skull<=42){a=getBestAction(t, board, g_packs, 1000,g_zoblishField);}
         else{a=getBestAction(t, board, g_packs, 40,g_zoblishField);}
         double elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count() / 1000.;
         if(a.command<0||a.command>5||a.r<0||a.r>3){
			 a.command=1;
			 a.r=0;
			 for (int x = 0; x < FIELD_WIDTH-1; x++) {
                for(int rot=0;rot<=0;rot++){
					if(board[FIELD_HEIGHT][x+1]==0 && board[FIELD_HEIGHT][x+2]==0){
						a.command=x;
						a.r=rot;
					}
				}
			 }
			 for (int x = 0; x < FIELD_WIDTH; x++) {
                for(int rot=1;rot<=1;rot++){
					if(board[FIELD_HEIGHT-1][x]==0&&board[FIELD_HEIGHT][x]==0){
						a.command=x;
						a.r=rot;
					}

				}
			 }
		 }
         cout<<a.command<<" "<<a.r<<endl;
         cerr<<"elapsed_time="<<elapsed<<endl;
         cerr<<"fire_turn="<<a.fireTurn+1<<endl;
         cerr<<"score="<<a.score<<endl;
    }
    return 0;
}
