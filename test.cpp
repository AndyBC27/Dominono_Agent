#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <unordered_map>
#include <chrono>
 
using namespace std;
using namespace std::chrono;

int INF = 1e9;
int glebia = 8;

struct Node{
    vector<int> state;
    unordered_map<int, int> occupied; //jak 0 - nie zajete, 1 - my, 2 - przeciwnik
};

//trzymamy vector zajetych
// zakladam indeksowanie od zera

struct Solution {
 
    bool start; //0 - second, 1 - first
    int n, m;
    Node curr;
    
    int evaluation(Node x)
    {
        vector < bool > seen_us(n*m,0);
        vector < bool > seen_them(n*m,0);
        int res_us = 0;
        int res_them = 0;
        
        bool lose_us = 0;
        bool lose_them = 0;
        
        for(auto &it: x.state)
        {
            if(it%m != m-1 && x.occupied[it+1] == 0)
            {
                if(!seen_us[it+1] && x.occupied[it] == 1)
                {
                    res_us++;
                    seen_us[it+1] = 1;
                }
                else if(!seen_them[it+1] && x.occupied[it] == 2)
                {
                    res_them++;
                    seen_them[it+1] = 1;
                }
            }
            else if(it%m != m-1 && x.occupied[it+1] == 1)
            {
                if(x.occupied[it] == 1)
                {
                    lose_us = true;
                    break;
                }
            }
            else if(it%m != (m-1) && x.occupied[it+1] == 2)
            {
                if(x.occupied[it] == 2)
                {
                    lose_them = true;
                    break;
                }
            }
            
            if(it%m != 0 && x.occupied[it-1] == 0)
            {
                if(!seen_us[it-1] && x.occupied[it] == 1)
                {
                    res_us++;
                    seen_us[it-1] = 1;
                }
                else if(!seen_them[it-1] && x.occupied[it] == 2)
                {
                    res_them++;
                    seen_them[it-1] = 1;
                }
            }
            else if(it%m != 0 && x.occupied[it-1] == 1)
            {
                if(x.occupied[it] == 1)
                {
                    lose_us = true;
                    break;
                }
            }
            else if(it%m != 0 && x.occupied[it-1] == 2)
            {
                if(x.occupied[it] == 2)
                {
                    lose_them = true;
                    break;
                }
            }
            
            if(it-m > 0  && x.occupied[it-m] == 0)
            {
                if(!seen_us[it-m] && x.occupied[it] == 1)
                {
                    res_us++;
                    seen_us[it-m] = 1;
                }
                else if(!seen_them[it-m] && x.occupied[it] == 2)
                {
                    res_them++;
                    seen_them[it-m] = 1;
                }
            }
            else if(it-m > 0 && x.occupied[it-m] == 1)
            {
                if(x.occupied[it] == 1)
                {
                    lose_us = true;
                    break;
                }
            }
            else if(it-m > 0 && x.occupied[it-m] == 2)
            {
                if(x.occupied[it] == 2)
                {
                    lose_them = true;
                    break;
                }
            }
            
            if(it < m*n-m && x.occupied[it+m] == 0)
            {
                if(!seen_us[it+m] && x.occupied[it] == 1)
                {
                    res_us++;
                    seen_us[it+m] = 1;
                }
                else if(!seen_them[it+m] && x.occupied[it] == 2)
                {
                    res_them++;
                    seen_them[it+m] = 1;
                }
            }
            else if(it < m*n-m  && x.occupied[it+m] == 1)
            {
                if(x.occupied[it] == 1)
                {
                    lose_us = true;
                    break;
                }
            }
            else if(it < m*n-m  && x.occupied[it+m] == 2)
            {
                if(x.occupied[it] == 2)
                {
                    lose_them = true;
                    break;
                }
            }
        }
        
        if(lose_us) return -INF;
        if(lose_them) return INF;
        
        if(x.state.size() == m * n) return -INF;
        
        return res_them - res_us;
        
    }
    
    vector<vector<pair<int, Node>>> get_tree(){
        vector<vector<pair<int, Node>>> tree; //trzeba trzymac kogo to dziecko
        tree.resize(glebia + 1);
        tree[0].push_back({0, curr});
        int depth = 1; //na razie niech max depth to 5
        auto start = high_resolution_clock::now();
        glebia = m*n-1;
        for(; depth <= glebia; ++depth){
            int i = 0;
            for(auto curr_node : tree[depth - 1]){
                for(int j = 0; j < n * m; ++j){
                    if(curr_node.second.occupied[j] == 0){
                        tree[depth].push_back(curr_node);
                        tree[depth][tree[depth].size() - 1].first = i;
                        if(evaluation(curr_node.second) != INF && evaluation(curr_node.second) != -INF){
                            tree[depth][tree[depth].size() - 1].second.state.push_back(j);
                            if(depth % 2 == 1)
                                tree[depth][tree[depth].size() - 1].second.occupied[j] = 1;
                            else
                                tree[depth][tree[depth].size() - 1].second.occupied[j] = 2;
                        }
                    }
                }
                auto stop = high_resolution_clock::now();
                auto duration = duration_cast<microseconds>(stop - start);
                //cout << duration.count() << endl;
                if(duration.count() >= 9100000)
                {
                    glebia = --depth;
                    //cout << glebia << endl;
                    return tree;
                }
                i++;
            }
        }
        glebia = --depth;
        //cout << glebia << endl;
        return tree;
    }
    
    int result(){
        vector<vector<pair<int, Node>>> tree = get_tree();
        //robimy minimax na tym drzewie
        vector<vector<pair<int, int>>> minimax; //parent, value
        while(tree[glebia].empty()){
            glebia--;
        }
        minimax.resize(glebia + 1);
        //najpierw ustalay wszystkie wartosci lisci
        for(auto x : tree[glebia]){
            minimax[glebia].push_back({x.first, evaluation(x.second)});
        }
        
        Node wyn = tree[1][0].second; // on nam na koncu powie z kad poszliszmy do naszego koncowego, sus
        for(int i = glebia; i > 0; --i){
            int tmp = i;
            //if(start == 0)
            //   tmp++;
            int ile_ma_dzieci = n * m - curr.state.size() - i + 1;
            if(tmp % 2 == 1){
                int maxx = -INF;
                int curr_par = -1;
                int j = 0;
                int alpha = INF;
                pair<int, int> x;
                for(int j = 0; j < minimax[i].size(); ++j){
                    x = minimax[i][j];
                    if(curr_par == -1){
                        curr_par = x.first;
                    }
                    if(x.first != curr_par || j == tree[i].size() - 1){
                        minimax[i - 1].push_back({tree[i - 1][curr_par].first, maxx});
                        curr_par = x.first;
                        maxx = -INF;
                        if(x.second > maxx){
                            maxx = x.second;
                            wyn = tree[i][j].second;
                            alpha = min(maxx, alpha);
                        }
                    }
                    else{
                        if(x.second > maxx){
                            maxx = x.second;
                            wyn = tree[i][j].second; //tylko w maksie wystarczy
                            alpha = min(maxx, alpha);
                        }
                    }
                    if(maxx > alpha){
                        if(curr_par != minimax[i][0].first){
                            bool czy = 0;
                            while(j + 1 < minimax[i].size() && minimax[i][j + 1].first == x.first){
                                j++;
                                czy = 1;
                            }
                            if(czy && j < minimax[i].size()){
                                j--;
                                x = minimax[i][j + 1];
                                curr_par = x.first;
                            }
                        }
                    }
                }
                if(minimax[i].size() >= 2 && minimax[i][minimax[i].size() - 2].first != minimax[i][minimax[i].size() - 1].first)
                    minimax[i - 1].push_back({tree[i - 1][curr_par].first, maxx});
            }
            else{
                int minn = INF;
                int curr_par = -1;
                int alpha = -INF;
                pair<int, int> x;
                for(int j = 0; j < minimax[i].size(); ++j){
                    x = minimax[i][j];
                    if(curr_par == -1){
                        curr_par = x.first;
                    }
                    if(x.first != curr_par || j == tree[i].size() - 1){
                        minimax[i - 1].push_back({tree[i - 1][curr_par].first, minn});
                        curr_par = x.first;
                        minn = INF;
                        if(x.second < minn){
                            minn = x.second;
                            wyn = tree[i][j].second;
                            alpha = max(minn, alpha);
                        }
                    }
                    else{
                        if(x.second < minn){
                            minn = x.second;
                            wyn = tree[i][j].second;
                            alpha = max(minn, alpha);
                        }
                    }
                    if(minn < alpha){
                        if(curr_par != minimax[i][0].first){
                            bool czy = 0;
                            while(j + 1 < minimax[i].size() && minimax[i][j + 1].first == x.first){
                                j++;
                                czy = 1;
                            }
                            if(czy && j < minimax[i].size()){
                                j--;
                                x = minimax[i][j + 1];
                                curr_par = x.first;
                            }
                        }
                    }
                }
                if(minimax[i].size() >= 2 && minimax[i][minimax[i].size() - 2].first != minimax[i][minimax[i].size() - 1].first)
                    minimax[i - 1].push_back({tree[i - 1][curr_par].first, minn});
            }
        }
        //musze teraz porownac wyn z curr
        //wyn powinien sie roznic tylko o jeden od curr, wiec wypisuje ostatni element w wektorze
        return wyn.state[wyn.state.size() - 1];
    }
    
    void input(){
        char c;
        cin >> c >> n >> m;
        if(c == 'c')
            start = 1;
        else
            start = 0;
    }
 
    void solve() {
        input();
        if((n % 2 == 0 || m % 2 == 0) && start == 0){
            if(m % 2 == 0){
                while(1){
                    int i, j;
                    cin >> i >> j;
                    cout << i << " " << abs(m - j - 1) << "\n";
                }
            }
            else{
                while(1){
                    int i, j;
                    cin >> i >> j;
                    cout << abs(n - i - 1) << " " << j << "\n";
                }
            }
        }
        else{
            if(start == 0){
                int i, j;
                cin >> i >> j;
                curr.state.push_back(i * m + j);
                curr.occupied[i * m + j] = 2;
            }
            if(evaluation(curr) == -INF){
                cout << "Komputer przegral\n";
                return;
            }
            else if(evaluation(curr) == INF){
                cout << "Komputer wygral\n";
                return;
            }
            while(1){
                int wyn = result();
                cout << wyn / m << " " << wyn % m << "\n";
                curr.state.push_back(wyn);
                curr.occupied[wyn] = 1;
                
                if(evaluation(curr) == -INF){
                    cout << "Komputer przegral\n";
                    return;
                }
                else if(evaluation(curr) == INF){
                    cout << "Komputer wygral\n";
                    return;
                }
                
                int i, j;
                cin >> i >> j;
                curr.state.push_back(i * m + j);
                curr.occupied[i * m + j] = 2;
                
                if(evaluation(curr) == -INF){
                    cout << "Komputer przegral\n";
                    return;
                }
                else if(evaluation(curr) == INF){
                    cout << "Komputer wygral\n";
                    return;
                }
            }
        }
    }
};
 
int main() {
    ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0);
 
    int noTestCases = 1;
    //cin >> noTestCases;
 
    while (noTestCases--) {
        Solution test;
        test.solve();
    }
}

/*
 c 3 3
 2 1
 2 0
 1 2
 2 2
 1 0
 1 1
 */
