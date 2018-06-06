#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cstdio>
#include <algorithm>

using namespace std;

#include "common.h"
#include "cout11.h"

typedef long long ll;

enum
{
    E_UNKNOWN = 0,
    E_NO_SOLVED = 1,
    E_SOLVED = 2,
};

vector<int> pyramid, deck;
map<int, int> m_okmask;

int FULL = (1 << 28)-1;

int ok_base[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 1, 2, 3, 4, 5, 6, 7, 8,
    10, 11, 13, 14, 16, 17,
    19, 21, 23 };

// (((mask >> ok_base[i]) & 3) == 3)
int okmask(int mask) {
    if (m_okmask.find(mask) != m_okmask.end()) {
        return m_okmask[mask];
    }

    int ok = 0x3FF;

    if (((mask >> 0) & 3) == 3) ok |= (1 << 10);
    if (((mask >> 1) & 3) == 3) ok |= (1 << 11);
    if (((mask >> 2) & 3) == 3) ok |= (1 << 12);
    if (((mask >> 3) & 3) == 3) ok |= (1 << 13);
    if (((mask >> 4) & 3) == 3) ok |= (1 << 14);
    if (((mask >> 5) & 3) == 3) ok |= (1 << 15);
    if (((mask >> 6) & 3) == 3) ok |= (1 << 16);
    if (((mask >> 7) & 3) == 3) ok |= (1 << 17);
    if (((mask >> 8) & 3) == 3) ok |= (1 << 18);

    if (((mask >> 10) & 3) == 3) ok |= (1 << 19);
    if (((mask >> 11) & 3) == 3) ok |= (1 << 20);
    if (((mask >> 13) & 3) == 3) ok |= (1 << 21);
    if (((mask >> 14) & 3) == 3) ok |= (1 << 22);
    if (((mask >> 16) & 3) == 3) ok |= (1 << 23);
    if (((mask >> 17) & 3) == 3) ok |= (1 << 24);

    if (((mask >> 19) & 3) == 3) ok |= (1 << 25);
    if (((mask >> 21) & 3) == 3) ok |= (1 << 26);
    if (((mask >> 23) & 3) == 3) ok |= (1 << 27);

    m_okmask[mask] = ok;
    return ok;
}

int is_next[14][14];
bool solved = false;

map<ll, int> m_flag_key;

map<ll, ll> m_solve_key;
map<ll, int> m_solve_next_combo;
map<ll, int> m_solve_next_score;

map<ll, ll> m_no_solve_key;
map<ll, int> m_no_solve_next_combo;
map<ll, int> m_no_solve_next_score;

void init() {
    for (int i=0; i<13; ++i) {
        for (int j=0; j<13; ++j) is_next[i][j] = 0;
        is_next[i][(i+1)%13] = 1;
        is_next[i][(i+12)%13] = 1;
    }
}

inline ll _key(int head, int st, int mask) {
    return ((ll)head << 33LL) | ((ll)st << 28LL) | (ll)mask; // 4bit, 5bit + 28bit = 33bit
}

// #include <iostream>
void render_key(ll key, int& head, int& st, int& mask) {
    head = (int)((key >> 33LL) & 15LL);
    st = (int)((key >> 28LL) & 31LL);
    mask = (int)((key & 0xFFFFFFF));
}

char buf[3];
ll T = 0;

string _loc(int loc) {
/* a:    0     1     2
 * b:   0 1   2 3   4 5
 * c:  0 1 2 3 4 5 6 7 8
 * d: 0 1 2 3 4 5 6 7 8 9
 */
  stringstream ss;
  if (loc < 10) {
    ss << 'd' << (1+loc);
  } else if (loc < 19) {
    ss << 'c' << (1+loc-10);
  } else if (loc < 25) {
    ss << 'b' << (1+loc-19);
  } else {
    ss << 'a' << (1+loc-25);
  }
  return ss.str();
}

int bonus(int i, int mask) {
    if (i < 25) return 0;
    int count = ((mask & (1 << 25)) > 0) + ((mask & (1 << 26)) > 0) + ((mask & (1 << 27)) > 0);
    int r = (count == 0) * 5 + (count == 1) * 10 + (count == 2) * 50;
    return r;
}

void trace() {
    if (++T % 1000000 == 0) {
        printf("cycle: %lld, flag_size:%d\r", T, m_flag_key.size());
        fflush(stdout);
    }
}

int solve(int& solve_next_combo, int& solve_next_score, int& no_solve_next_combo, int& no_solve_next_score, int head=deck[0], int st=1, int mask=0) {
    trace();
    ll key = _key(head, st, mask);
    bool is_debug = false;
    //bool is_debug = (key == 0x176de7efff);
    if (m_flag_key.count(key) > 0)
    {
        solve_next_combo = m_solve_next_combo[key];
        solve_next_score = m_solve_next_score[key];
        no_solve_next_combo = m_no_solve_next_combo[key];
        no_solve_next_score = m_no_solve_next_score[key];
        return m_flag_key[key];
    }

    if (mask == FULL) {
        solved = true;
        //trace();
        return E_SOLVED;
    }
    else if (st == 24) {
        //trace();
        return E_NO_SOLVED;
    }

    int ok = okmask(mask);

    int r = 0;
    ll _solve_key = 0;
    int _solve_next_combo = 0;
    int _solve_next_score = 0;
    ll _no_solve_key = 0;
    int _no_solve_next_combo = 0;
    int _no_solve_next_score = 0;
    
    //No.1 try pyramid
    for (int i=0; i<28; ++i) {
        int m = 1 << i;
        if (m & mask) continue;
        if (i >= 10 && (ok & m) == 0) continue;
        int card = pyramid[i];
        if (!is_next[head][card]) continue;

        int next_mask = mask | m;
        
        int __solve_next_combo = 0;
        int __solve_next_score = 0;
        int __no_solve_next_combo = 0;
        int __no_solve_next_score = 0;
        r |= solve(__solve_next_combo, __solve_next_score, __no_solve_next_combo, __no_solve_next_score, card, st, next_mask);
        if (is_debug) {
            printf("<1> r: %08x\n", r);
            printf("<1> __solve_next_combo: %d\n", __solve_next_combo);
            printf("<1> __solve_next_score: %d\n", __solve_next_score);
            printf("<1> __no_solve_next_combo: %d\n", __no_solve_next_combo);
            printf("<1> __no_solve_next_score: %d\n", __no_solve_next_score);
        }
        if (r & E_SOLVED) {
            __solve_next_score += 1 + __solve_next_combo * 2 + bonus(i, mask);
        }
        if (__solve_next_score > _solve_next_score) {
            _solve_key = _key(card, st, next_mask);
            _solve_next_combo = __solve_next_combo+1;
            _solve_next_score = __solve_next_score;
        }
        if (r & E_NO_SOLVED) {
            __no_solve_next_score += 1 + __no_solve_next_combo * 2 + bonus(i, mask);
        }
        if (__no_solve_next_score > _no_solve_next_score) {
            _no_solve_key = _key(card, st, next_mask);
            _no_solve_next_combo = __no_solve_next_combo+1;
            _no_solve_next_score = __no_solve_next_score;
        }
    }
    
    //No.2 try deck
    int __solve_next_combo = 0;
    int __solve_next_score = 0;
    int __no_solve_next_combo = 0;
    int __no_solve_next_score = 0;
    r |= solve(__solve_next_combo, __solve_next_score, __no_solve_next_combo, __no_solve_next_score, deck[st], st+1, mask);
    if (is_debug) {
        printf("<2> r: %08x\n", r);
        printf("<2> __solve_next_combo: %d\n", __solve_next_combo);
        printf("<2> __solve_next_score: %d\n", __solve_next_score);
        printf("<2> __no_solve_next_combo: %d\n", __no_solve_next_combo);
        printf("<2> __no_solve_next_score: %d\n", __no_solve_next_score);
    }
    if (__solve_next_score > _solve_next_score) {
        _solve_key = _key(deck[st], st+1, mask);
        _solve_next_combo = 0;
        _solve_next_score = __solve_next_score;
    }
    if (__no_solve_next_score > _no_solve_next_score) {
        _no_solve_key = _key(deck[st], st+1, mask);
        _no_solve_next_combo = 0;
        _no_solve_next_score = __no_solve_next_score;
    }
    
    m_solve_key[key] = _solve_key;
    m_solve_next_combo[key] = _solve_next_combo;
    m_solve_next_score[key] = _solve_next_score;
    m_no_solve_key[key] = _no_solve_key;
    m_no_solve_next_combo[key] = _no_solve_next_combo;
    m_no_solve_next_score[key] = _no_solve_next_score;

    solve_next_combo += m_solve_next_combo[key];
    solve_next_score = m_solve_next_score[key];
    no_solve_next_combo += m_no_solve_next_combo[key];
    no_solve_next_score = m_no_solve_next_score[key];
    
    m_flag_key[key] = r;
    return r;
}

void print_best_move(bool solved, int best_score)
{
    map<ll, ll> m_key = solved ? m_solve_key : m_no_solve_key;
    map<ll, int> m_next_combo = solved ? m_solve_next_combo : m_no_solve_next_combo;
    map<ll, int> m_next_score = solved ? m_solve_next_score : m_no_solve_next_score;
    ll key = _key(deck[0], 1, 0);
    int i = 0;
    printf("%s\n",std::string(50,'=').c_str());
    while (key)
    {
        printf("%d) ", 1+i);
        ll next_key = m_key[key];
        if (!next_key) {
            printf("key: %llx\n", key);
            printf("m_next_combo[key]: %d\n", m_next_combo[key]);
            printf("m_next_score[key]: %d\n", m_next_score[key]);
            break;
        }
        int head, st, mask, next_head, next_st, next_mask;
        render_key(key, head, st, mask);
        render_key(next_key, next_head, next_st, next_mask);
        if (st == next_st) {
            int maskdiff = mask - next_mask;
            int loc = __builtin_ctz(maskdiff);
            cout << "from pyramid " << _loc(loc) << " (" << num_single[pyramid[loc]] << ")";
            if (m_next_combo[key] == 1) {
                cout << " combo(" << m_next_combo[key] << ")"<< " score(" << (best_score - m_next_score[key] + 1) * 100 << ")";
            } else {
                cout << " combo(" << m_next_combo[key] << ")";
            }
        } else {
            cout << "turn deck (" << num_single[deck[st]] << ")";
        }
        cout << endl;
        
        key = next_key;
        i++;
    }
    if (solved) {
        cout << "<solved> score(" << best_score * 100 << ")" << endl;
    } else {
        cout << "<not solved> score(" << best_score * 100 << ")" << endl;
    }
    printf("%s\n",std::string(50,'=').c_str());
}

void usage() {
    printf("usage: TriPeaks_solver <game-file>\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        usage();
        return 0;
    }

    char _pyramid[29], _deck[25];

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        usage();
        return 0;
    }

    vector<char> tmp;
    while (1) {
        int ch = fgetc(fp);
        if (ch == EOF) break;

        int c = char_to_card_num(ch);
        if (c < 0) continue;
        
        //printf("%d ", c);
        tmp.push_back(c);
    }
    //printf("\n");
    if (tmp.size() != 52) {
        printf("invalid data: %d != 52\n", tmp.size());
        return 0;
    }

    // 0+3 -> pyramid[25:]
    // 3+6 -> pyramid[19:]
    // 9+9 -> pyramid[10:]
    // 18+10 -> pyramid[0:]
    pyramid.insert(pyramid.end(), tmp.begin()+18, tmp.begin()+28);
    pyramid.insert(pyramid.end(), tmp.begin()+9, tmp.begin()+18);
    pyramid.insert(pyramid.end(), tmp.begin()+3, tmp.begin()+9);
    pyramid.insert(pyramid.end(), tmp.begin(), tmp.begin()+3);
    // 28+24 -> deck
    deck.insert(deck.end(), tmp.begin()+28, tmp.end());

    cout << "pyramid: ";
    tr(it, pyramid) putchar(num_single[*it]);
    cout << endl;

    cout << "deck: ";
    tr(it, deck) putchar(num_single[*it]);
    cout << endl;

    if (pyramid.size() != 28 || deck.size() != 24) {
        printf("invalid data length (%d, %d)\n", (int)pyramid.size(), (int)deck.size());
        fclose(fp);
        return 0;
    }

    init();

    vector<int> cnt(13, 0);
    for (int i=0; i<28; ++i) ++cnt[pyramid[i]];
    for (int i=0; i<24; ++i) ++cnt[deck[i]];
    for (int i=0; i<13; ++i) {
        if (cnt[i] != 4) {
            printf("invalid data.\n");
            cout << cnt << endl;
            return 0;
        }
    }

    int solve_next_combo = 0;
    int solve_next_score = 0;
    int no_solve_next_combo = 0;
    int no_solve_next_score = 0;
    solve(solve_next_combo, solve_next_score, no_solve_next_combo, no_solve_next_score);
    printf("cycle: %lld, flag_size:%d\n", T, m_flag_key.size());
    printf("solved: %s\n", solved ? "yes" : "no");
    printf("solve_next_score: %d\n", solve_next_score * 100);
    printf("no_solve_next_score: %d\n", no_solve_next_score * 100);
    print_best_move(solved, solved ? solve_next_score : no_solve_next_score);
    if (solved)
        print_best_move(false, no_solve_next_score);

    fclose(fp);
    return 0;
}
// 8Q8AAKK0260739346Q96A227 437058A574K26JQQ439J05J89JK5
