#include <bits/stdc++.h>
using namespace std;

struct Cage {
    vector<pair<int,int>> cells;
    int target;
};

int bit_count(int x){ return __builtin_popcount((unsigned)x); }
int lowbit_digit(int mask) { 
    if (!mask) return -1;
    int d = __builtin_ctz(mask) + 1;
    return d;
}

using Board = array<array<int,9>,9>;
using MaskGrid = array<array<int,9>,9>;

struct Solver {
    Board start;
    string variation;
    vector<Cage> cages;
    MaskGrid cand;
    vector<Board> solutions;

    Solver(const Board& b, const string& var, const vector<Cage>& cs):
        start(b), variation(var), cages(cs)
    {
        int fullMask = (1<<9) - 1;
        for(int r=0;r<9;r++) for(int c=0;c<9;c++) cand[r][c] = fullMask;
    }

    inline int boxIndex(int r, int c){ return (r/3)*3 + (c/3); }

    bool initialize() {
        int full = (1<<9)-1;
        for(int r=0;r<9;r++) for(int c=0;c<9;c++) cand[r][c] = full;

        for(int r=0;r<9;r++){
            for(int c=0;c<9;c++){
                int v = start[r][c];
                if (v!=0){
                    int mask = 1<<(v-1);
                    cand[r][c] = mask;
                }
            }
        }

        bool changed = true;
        while (changed) {
            changed = false;
            for(int r=0;r<9;r++){
                for(int c=0;c<9;c++){
                    int curMask = cand[r][c];
                    if (curMask == 0) return false;
                    if (bit_count(curMask) == 1){
                        int d = __builtin_ctz(curMask) + 1;
                        for(int cc=0; cc<9; cc++){
                            if (cc==c) continue;
                            int before = cand[r][cc];
                            int after = before & ~(1<<(d-1));
                            if (after != before){
                                cand[r][cc] = after; changed = true;
                                if (after==0) return false;
                            }
                        }
                        for(int rr=0; rr<9; rr++){
                            if (rr==r) continue;
                            int before = cand[rr][c];
                            int after = before & ~(1<<(d-1));
                            if (after != before){
                                cand[rr][c] = after; changed = true;
                                if (after==0) return false;
                            }
                        }
                        int br = (r/3)*3, bc = (c/3)*3;
                        for(int rr=br; rr<br+3; rr++){
                            for(int cc=bc; cc<bc+3; cc++){
                                if (rr==r && cc==c) continue;
                                int before = cand[rr][cc];
                                int after = before & ~(1<<(d-1));
                                if (after != before){
                                    cand[rr][cc] = after; changed = true;
                                    if (after==0) return false;
                                }
                            }
                        }
                        if (variation=="diagonal"){
                            if (r==c){
                                for(int i=0;i<9;i++){
                                    if (i==r) continue;
                                    int before = cand[i][i];
                                    int after = before & ~(1<<(d-1));
                                    if (after!=before){
                                        cand[i][i]=after; changed=true;
                                        if (after==0) return false;
                                    }
                                }
                            }
                            if (r+c==8){
                                for(int i=0;i<9;i++){
                                    int rr = i, cc = 8-i;
                                    if (rr==r && cc==c) continue;
                                    int before = cand[rr][cc];
                                    int after = before & ~(1<<(d-1));
                                    if (after!=before){
                                        cand[rr][cc]=after; changed=true;
                                        if (after==0) return false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        for(int r=0;r<9;r++){
            int seen=0;
            for(int c=0;c<9;c++){
                int v = start[r][c];
                if (!v) continue;
                int bit = 1<<(v-1);
                if (seen & bit) return false;
                seen |= bit;
            }
        }
        for(int c=0;c<9;c++){
            int seen=0;
            for(int r=0;r<9;r++){
                int v = start[r][c];
                if (!v) continue;
                int bit = 1<<(v-1);
                if (seen & bit) return false;
                seen |= bit;
            }
        }
        for(int br=0;br<3;br++) for(int bc=0;bc<3;bc++){
            int seen=0;
            for(int r=3*br;r<3*br+3;r++) for(int c=3*bc;c<3*bc+3;c++){
                int v = start[r][c];
                if (!v) continue;
                int bit = 1<<(v-1);
                if (seen & bit) return false;
                seen |= bit;
            }
        }
        if (variation=="diagonal"){
            int seen=0;
            for(int i=0;i<9;i++){
                int v = start[i][i];
                if (!v) continue;
                int bit=1<<(v-1);
                if (seen & bit) return false;
                seen |= bit;
            }
            seen=0;
            for(int i=0;i<9;i++){
                int v = start[i][8-i];
                if (!v) continue;
                int bit=1<<(v-1);
                if (seen & bit) return false;
                seen |= bit;
            }
        }
        if (variation=="killer"){
            if (!check_all_cages_partial(start, cand)) return false;
        }

        return true;
    }

    bool check_all_cages_partial(const Board &board, const MaskGrid &masks) {
        for(const auto &cage : cages){
            int assignedSum = 0;
            vector<int> assignedDigits;
            vector<pair<int,int>> empties;
            for(auto [r,c] : cage.cells){
                int v = board[r][c];
                if (v!=0) assignedDigits.push_back(v), assignedSum += v;
                else empties.emplace_back(r,c);
            }
            int seen=0;
            for(int d: assignedDigits){
                int bit = 1<<(d-1);
                if (seen & bit) return false;
                seen |= bit;
            }
            int remCells = empties.size();
            int remTarget = cage.target - assignedSum;
            if (remCells==0){
                if (remTarget != 0) return false;
                continue;
            }
            vector<int> mins, maxs;
            for(auto [r,c] : empties){
                int mask = masks[r][c];
                mask &= ~seen;
                if (!mask) return false;
                int minD = 10, maxD = 0;
                for(int d=1; d<=9; d++){
                    if (mask & (1<<(d-1))){
                        minD = min(minD, d);
                        maxD = max(maxD, d);
                    }
                }
                if (minD==10) return false;
                mins.push_back(minD);
                maxs.push_back(maxD);
            }
            int sumMin = accumulate(mins.begin(), mins.end(), 0);
            int sumMax = accumulate(maxs.begin(), maxs.end(), 0);
            if (remTarget < sumMin || remTarget > sumMax) return false;

            vector<int> allowedMasks;
            for(auto [r,c] : empties){
                int m = masks[r][c] & ~seen;
                allowedMasks.push_back(m);
            }
            vector<int> availCount(10,0);
            for(int am : allowedMasks){
                for(int d=1; d<=9; d++){
                    if (am & (1<<(d-1))) availCount[d]++;
                }
            }
            int needed = remCells;
            int greedyMinSum = 0;
            vector<bool> usedDigit(10,false);
            for(int d=1; d<=9 && needed>0; d++){
                if (availCount[d] > 0 && !usedDigit[d]){
                    greedyMinSum += d;
                    usedDigit[d] = true;
                    needed--;
                }
            }
            if (needed>0) return false;
            if (remTarget < greedyMinSum) return false;

            needed = remCells;
            int greedyMaxSum = 0;
            fill(usedDigit.begin(), usedDigit.end(), false);
            for(int d=9; d>=1 && needed>0; d--){
                if (availCount[d] > 0 && !usedDigit[d]){
                    greedyMaxSum += d;
                    usedDigit[d] = true;
                    needed--;
                }
            }
            if (needed>0) return false;
            if (remTarget > greedyMaxSum) return false;
        }
        return true;
    }

    bool pick_MRV_cell(const Board &board, int &out_r, int &out_c) {
        int bestCount = 100;
        int br=-1, bc=-1;
        for(int r=0;r<9;r++){
            for(int c=0;c<9;c++){
                if (board[r][c] == 0) {
                    int pc = bit_count(cand[r][c]);
                    if (pc == 0) return false;
                    if (pc < bestCount){
                        bestCount = pc;
                        br = r; bc = c;
                        if (bestCount == 1) { out_r=br; out_c=bc; return true; }
                    }
                }
            }
        }
        if (br==-1) return false;
        out_r = br; out_c = bc;
        return true;
    }

    struct Change {
        int r,c;
        int beforeMask;
    };

    bool assign_with_forward(Board &board, int r, int c, int d, vector<Change> &history) {
        int maskDigit = 1<<(d-1);
        if (!(cand[r][c] & maskDigit)) return false;
        board[r][c] = d;
        history.push_back({r,c,cand[r][c]});
        cand[r][c] = maskDigit;

        for(int cc=0; cc<9; cc++){
            if (cc==c) continue;
            if (cand[r][cc] & maskDigit){
                history.push_back({r,cc,cand[r][cc]});
                cand[r][cc] &= ~maskDigit;
                if (cand[r][cc] == 0) return false;
            }
        }
        for(int rr=0; rr<9; rr++){
            if (rr==r) continue;
            if (cand[rr][c] & maskDigit){
                history.push_back({rr,c,cand[rr][c]});
                cand[rr][c] &= ~maskDigit;
                if (cand[rr][c] == 0) return false;
            }
        }
        int br=(r/3)*3, bc=(c/3)*3;
        for(int rr=br; rr<br+3; rr++) for(int cc=bc; cc<bc+3; cc++){
            if (rr==r && cc==c) continue;
            if (cand[rr][cc] & maskDigit){
                history.push_back({rr,cc,cand[rr][cc]});
                cand[rr][cc] &= ~maskDigit;
                if (cand[rr][cc] == 0) return false;
            }
        }
        if (variation=="diagonal"){
            if (r==c){
                for(int i=0;i<9;i++){
                    if (i==r) continue;
                    if (cand[i][i] & maskDigit){
                        history.push_back({i,i,cand[i][i]});
                        cand[i][i] &= ~maskDigit;
                        if (cand[i][i] == 0) return false;
                    }
                }
            }
            if (r+c==8){
                for(int i=0;i<9;i++){
                    int rr=i, cc=8-i;
                    if (rr==r && cc==c) continue;
                    if (cand[rr][cc] & maskDigit){
                        history.push_back({rr,cc,cand[rr][cc]});
                        cand[rr][cc] &= ~maskDigit;
                        if (cand[rr][cc] == 0) return false;
                    }
                }
            }
        }
        if (variation=="killer"){
            if (!check_all_cages_partial(board, cand)) return false;
        }

        return true;
    }

    void undo_changes(Board &board, vector<Change> &history) {
        for(int i=(int)history.size()-1; i>=0; --i){
            auto &ch = history[i];
            cand[ch.r][ch.c] = ch.beforeMask;
        }
    }

    void backtrack(Board &board) {
        int r,c;
        bool anyEmpty=false;
        for(int rr=0; rr<9; rr++) for(int cc=0; cc<9; cc++) if (board[rr][cc]==0) anyEmpty=true;
        if (!anyEmpty){
            solutions.push_back(board);
            return;
        }

        int bestCount = 100;
        int br=-1, bc=-1;
        for(int rr=0; rr<9; rr++){
            for(int cc=0; cc<9; cc++){
                if (board[rr][cc]==0){
                    int pc = bit_count(cand[rr][cc]);
                    if (pc == 0) return;
                    if (pc < bestCount){
                        bestCount = pc; br = rr; bc = cc;
                        if (bestCount == 1) break;
                    }
                }
            }
            if (bestCount == 1) break;
        }
        r = br; c = bc;

        int mask = cand[r][c];
        vector<int> digits;
        for(int d=1; d<=9; d++) if (mask & (1<<(d-1))) digits.push_back(d);

        for(int d : digits){
            vector<Change> history;
            Board prevBoard = board;
            if (!assign_with_forward(board, r, c, d, history)){
                undo_changes(board, history);
                board = prevBoard;
                continue;
            }
            backtrack(board);
            board[r][c] = 0;
            undo_changes(board, history);
        }
    }

    vector<Board> solve() {
        solutions.clear();
        if (!initialize()) return solutions;
        Board b = start;
        backtrack(b);
        return solutions;
    }
};

void print_board(const Board &b){
    for(int r=0;r<9;r++){
        for(int c=0;c<9;c++){
            cout << b[r][c] << (c==8? "" : " ");
        }
        cout << "\n";
    }
    cout << "\n";
}

int main(){
    Board classic = {{
        {5,3,0,0,7,0,0,0,0},
        {6,0,0,1,9,5,0,0,0},
        {0,9,8,0,0,0,0,6,0},
        {8,0,0,0,6,0,0,0,3},
        {4,0,0,8,0,3,0,0,1},
        {7,0,0,0,2,0,0,0,6},
        {0,6,0,0,0,0,2,8,0},
        {0,0,0,4,1,9,0,0,5},
        {0,0,0,0,8,0,0,7,9}
    }};

    Solver solverClassic(classic, "classic", {});
    auto solsClassic = solverClassic.solve();
    cout << "Classic solutions found: " << solsClassic.size() << "\n";
    if (!solsClassic.empty()) {
        cout << "First solution:\n";
        print_board(solsClassic[0]);
    }

    Solver solverDiag(classic, "diagonal", {});
    auto solsDiag = solverDiag.solve();
    cout << "Diagonal variation solutions found (on same board): " << solsDiag.size() << "\n";

    vector<Cage> cages;
    Cage cage1;
    cage1.cells = {{0,0},{0,1},{1,0},{1,1}}; cage1.target = 15;
    Cage cage2; cage2.cells = {{0,4},{0,5},{1,4},{1,5}}; cage2.target = 20;
    cages.push_back(cage1);
    cages.push_back(cage2);

    Solver solverKiller(classic, "killer", cages);
    auto solsKiller = solverKiller.solve();
    cout << "Killer (illustrative cages) solutions found: " << solsKiller.size() << "\n";

    return 0;
}
