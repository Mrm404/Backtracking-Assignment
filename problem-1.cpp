#include <bits/stdc++.h>
using namespace std;

struct Schedule {
    string candidate;
    string interviewer;
    pair<int, int> slot;
};

vector<vector<Schedule>> all_schedules;

bool overlap(pair<int, int> a, pair<int, int> b) {
    return !(a.second <= b.first || b.second <= a.first);
}

void backtrack(
    vector<string>& candidates,
    unordered_map<string, vector<pair<int, int>>>& candidateSlots,
    unordered_map<string, vector<pair<int, int>>>& interviewerSlots,
    unordered_map<string, int>& candCount,
    unordered_map<string, int>& intCount,
    int maxPerPerson,
    int idx,
    vector<Schedule>& current
) {
    if (idx == candidates.size()) {
        all_schedules.push_back(current);
        return;
    }

    string cand = candidates[idx];

    for (auto& cSlot : candidateSlots[cand]) {
        for (auto& [interviewer, slots] : interviewerSlots) {
            if (candCount[cand] >= maxPerPerson || intCount[interviewer] >= maxPerPerson)
                continue;

            for (auto& iSlot : slots) {
                int start = max(cSlot.first, iSlot.first);
                int end = min(cSlot.second, iSlot.second);
                if (end - start >= 1) {
                    pair<int, int> meeting = {start, start + 1};

                    bool conflict = false;
                    for (auto& s : current) {
                        if ((s.candidate == cand || s.interviewer == interviewer) && overlap(s.slot, meeting)) {
                            conflict = true;
                            break;
                        }
                    }
                    if (conflict) continue;

                    current.push_back({cand, interviewer, meeting});
                    candCount[cand]++;
                    intCount[interviewer]++;

                    backtrack(candidates, candidateSlots, interviewerSlots,
                              candCount, intCount, maxPerPerson, idx + 1, current);

                    current.pop_back();
                    candCount[cand]--;
                    intCount[interviewer]--;
                }
            }
        }
    }

    backtrack(candidates, candidateSlots, interviewerSlots,
              candCount, intCount, maxPerPerson, idx + 1, current);
}

int main() {
    unordered_map<string, vector<pair<int, int>>> candidates = {
        {"Alice", {{9, 10}, {14, 16}, {17, 18}}},
        {"Bob", {{10, 12}, {15, 17}}},
        {"Charlie", {{9, 11}, {13, 15}}}
    };

    unordered_map<string, vector<pair<int, int>>> interviewers = {
        {"John", {{9, 12}, {14, 18}}},
        {"Sarah", {{10, 13}, {15, 17}}},
        {"Mike", {{11, 14}, {16, 18}}}
    };

    int duration = 1;
    int max_per_person = 2;

    vector<string> candList;
    for (auto& c : candidates) candList.push_back(c.first);

    unordered_map<string, int> candCount, intCount;
    for (auto& c : candidates) candCount[c.first] = 0;
    for (auto& i : interviewers) intCount[i.first] = 0;

    vector<Schedule> current;
    backtrack(candList, candidates, interviewers, candCount, intCount, max_per_person, 0, current);

    cout << "Total valid schedules found: " << all_schedules.size() << "\n\n";
    int n = 1;
    for (auto& sched : all_schedules) {
        cout << "Schedule " << n++ << ":\n";
        for (auto& s : sched) {
            cout << "  (" << s.candidate << ", " << s.interviewer << ", ("
                 << s.slot.first << ", " << s.slot.second << "))\n";
        }
        cout << "\n";
    }

    return 0;
}
