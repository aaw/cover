#include "logging.h"
#include "counters.h"
#include "flags.h"
#include "params.h"

#include <cctype>
#include <iomanip>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

DEFINE_PARAM(prefer_sharp, 0, "When branching, prefer items with a # prefix");

DEFINE_PARAM(prefer_unsharp, 0,
             "When branching, prefer items without a # prefix");

struct Node {
    std::string name;
    size_t ulink;
    size_t dlink;
    size_t llink;
    size_t rlink;
    size_t slack;
    size_t bound;
    int top_or_len;
    int color = 0;
};

#define NAME(i) (nodes[i].name)
#define LLINK(i) (nodes[i].llink)
#define RLINK(i) (nodes[i].rlink)
#define ULINK(i) (nodes[i].ulink)
#define DLINK(i) (nodes[i].dlink)
#define TOP(i) (nodes[i].top_or_len)
#define LEN(i) (nodes[i].top_or_len)
#define COLOR(i) (nodes[i].color)
#define SLACK(i) (nodes[i].slack)
#define BOUND(i) (nodes[i].bound)
#define MAX_LINE_SIZE (100000)

inline size_t monus(size_t x, size_t y) {
    return x > y ? x - y : 0;
}

struct MCC {
    std::vector<Node> nodes;
    std::vector<size_t> choice;
    std::vector<size_t> ft;
    std::vector<size_t> score;
    std::vector<std::string> colors;  // 1-indexed.
    size_t num_items;
    size_t num_primary_items;
    size_t num_options;

    std::string debug_nodes() {
        std::ostringstream oss;
        oss << std::endl;
        int i = 0;
        for (const Node& n : nodes) {
            oss << i << ": { " << n.name << " l: " << n.llink
                << " r: " << n.rlink << " u: " << n.ulink << " d: " << n.dlink
                << " t: " << n.top_or_len << " c: " << n.color << " s: "
                << n.slack << " b: " << n.bound << " }" << std::endl;
            ++i;
        }
        return oss.str();
    }

    int color_parse(char* ss, std::unordered_map<std::string, int>& colors,
                    int next_color, std::string* curr) {
        for (size_t i = 0; ss[i] != 0; ++i) {
            if (ss[i] == ':') {
                *curr = std::string(ss, i);
                std::string color(&ss[i+1]);
                CHECK(color.size() > 0) << "Bad color spec: " << ss;
                if (colors[color] == 0) { colors[color] = next_color; }
                return colors[color];
            }
        }
        *curr = ss;
        return 0;
    }

    void multiplicity_parse(std::string* curr, size_t* low, size_t* high) {
        size_t curr_size = curr->size();
        size_t state = 0;
        for (size_t i = 0; i < curr->size(); ++i) {
            char ch = (*curr)[i];
            if (state == 0 && ch == '[') {
                curr_size = i;
                ++state;
                *low = 0;
            } else if (state == 1) {
                CHECK(ch == ':' || isdigit(ch))
                    << "Expected ':' or digit after '[': " << *curr;
                if (ch == ':') { ++state; *high = 0; }
                else /* isdigit */ { *low *= 10; *low += ch - '0'; }
            } else if (state == 2) {
                CHECK(ch == ']' || isdigit(ch))
                    << "Expected ']' or digit after ':': " << *curr;
                if (ch == ']') { ++state; }
                else /* isdigit */ { *high *= 10; *high += ch - '0'; }
            } else if (state == 3) {
                CHECK(false) << "Expected no more input after ']': " << *curr;
            }
        }
        CHECK(*low >= 0) << "Bad low bound (" << *low << ") for " << *curr;
        CHECK(*high >= 1) << "Bad high bound (" << *high << ") for " << *curr;
        CHECK(state == 0 || state == 3)
            << "Incomplete input for option with multiplicity: " << *curr;
        *curr = curr->substr(0, curr_size);
    }

    MCC(const char* filename) {
        FILE* f = fopen(filename, "r");
        CHECK(f) << "Failed to open file: " << filename;
        char s[MAX_LINE_SIZE];
        char ss[MAX_LINE_SIZE];

        // I1. [Read the first line.]
        std::unordered_map<std::string, size_t> header;
        nodes.push_back(Node());  // Header
        num_primary_items = std::numeric_limits<size_t>::max();
        while(fgets(s, MAX_LINE_SIZE, f) != NULL) {
            CHECK(strlen(s) < MAX_LINE_SIZE-1 || s[strlen(s)-1] == '\n') <<
                "Input line too long. Recompile with larger MAX_LINE_SIZE.";
            int offset = 0, r = 0;
            std::string curr;
            while (sscanf(s + offset, " %s %n", ss, &r) > 0) {
                curr = ss;
                if (curr[0] == '/' && curr.size() > 1 && curr[1] == '/') break;
                if (curr == "\\") break;
                offset += r;
                size_t low = 1, high = 1;
                multiplicity_parse(&curr, &low, &high);
                CHECK(header.find(curr) == header.end()) <<
                    "Duplicate item name: '" << ss << "'";
                if (curr == "|") {
                    num_primary_items = nodes.size() - 1;
                    continue;
                }
                header[curr] = nodes.size();
                Node n;
                n.name = curr;
                n.llink = nodes.size() - 1;
                n.slack = high - low;
                n.bound = high;
                nodes.back().rlink = nodes.size();
                nodes.push_back(n);
            }
            if (curr != "\\" && !header.empty()) break;
        }
        num_items = header.size();

        // I2. [Finish the horizontal list.]
        if (num_primary_items == std::numeric_limits<size_t>::max()) {
            num_primary_items = nodes.size() - 1;
        } else {
            nodes[num_primary_items+1].llink = nodes.size()-1;
            nodes.back().rlink = num_primary_items+1;
        }
        nodes[num_primary_items].rlink = 0;
        nodes[0].llink = num_primary_items;
        LOG(1) << "Parsed " << num_items << " items (" << num_primary_items
               << " primary)";

        // I3. [Prepare for options.]
        for (size_t i = 1; i < nodes.size(); ++i) {
            LEN(i) = 0;
            ULINK(i) = DLINK(i) = i;
        }
        size_t m = 0;
        size_t p = nodes.size();
        nodes.push_back(Node());  // First spacer

        num_options = 0;
        std::unordered_set<std::string> seen;
        std::unordered_map<std::string, int> color_ids;
        int next_color = 1;
        size_t j = 0;

        while(fgets(s, MAX_LINE_SIZE, f) != NULL) {
            CHECK(strlen(s) < MAX_LINE_SIZE-1 || s[strlen(s)-1] == '\n') <<
                "Input line too long. Recompile with larger MAX_LINE_SIZE.";
            int offset = 0, r = 0, cnum = 0;
            std::string curr;
            while (sscanf(s + offset, " %s %n", ss, &r) > 0) {
                cnum = color_parse(ss, color_ids, next_color, &curr);
                next_color = std::max(next_color, cnum + 1);
                if (curr[0] == '/' && curr.size() > 1 && curr[1] == '/') break;
                if (curr == "\\") break;
                offset += r;
                ++j;
                size_t i = header[curr];
                CHECK(i > 0) << "Item '" << curr << "' not in header";
                CHECK(i >= num_primary_items || cnum == 0) <<
                    "Color can't be assigned to primary item (" << ss << ")";
                CHECK(seen.find(curr) == seen.end()) <<
                    "Duplicate item '" << curr << "'";
                seen.insert(curr);
                LEN(i)++;
                size_t q = ULINK(i);
                nodes.push_back(Node());
                CHECK(nodes.size() > p+j) << "Not enough nodes allocated. Want "
                                          << p+j << ", got " << nodes.size()-1;
                ULINK(p+j) = q;
                DLINK(q) = p+j;
                DLINK(p+j) = i;
                ULINK(i) = p+j;
                TOP(p+j) = i;
                COLOR(p+j) = cnum;
            }
            if (curr == "\\" || seen.empty()) continue;

            // I5. [Finish an option.]
            num_options++;
            m++;
            DLINK(p) = p+j;
            nodes.push_back(Node());
            CHECK(nodes.size()-1 == p+j+1) << "No room for spacer";
            p = p+j+1;
            TOP(p) = -m;
            ULINK(p) = p-j;
            seen.clear();
            j = 0;
        }
        LOG(1) << "Parsed " << color_ids.size() << " colors";
        LOG(1) << "Parsed " << num_options << " options";

        LOG(3) << "After parsing, memory is: " << debug_nodes();
        fclose(f);

        choice = std::vector<size_t>(num_options);
        ft = std::vector<size_t>(num_options);
        score = std::vector<size_t>(num_options);
        colors.resize(color_ids.size() + 1);  // colors are 1-indexed.
        for (const auto& kv : color_ids) { colors[kv.second] = kv.first; }
    }

    void hide(size_t p) {
        for(size_t q = p + 1; q != p;) {
            if (COLOR(q) < 0) { ++q; continue; }
            int x = TOP(q);
            size_t u = ULINK(q), d = DLINK(q);
            if (x <= 0) { q = u; continue; } // q was a spacer.
            DLINK(u) = d;
            ULINK(d) = u;
            LEN(x)--;
            ++q;
        }
    }

    void unhide(size_t p) {
        for(size_t q = p - 1; q != p;) {
            if (COLOR(q) < 0) { --q; continue; }
            int x = TOP(q);
            size_t u = ULINK(q), d = DLINK(q);
            if (x <= 0) { q = d; continue; } // q was a spacer.
            DLINK(u) = q;
            ULINK(d) = q;
            LEN(x)++;
            --q;
        }
    }

    void cover(size_t i) {
        for (size_t p = DLINK(i); p != i;) {
            hide(p);
            p = DLINK(p);
        }
        size_t l = LLINK(i), r = RLINK(i);
        RLINK(l) = r;
        LLINK(r) = l;
    }

    void uncover(size_t i) {
        size_t l = LLINK(i), r = RLINK(i);
        RLINK(l) = i;
        LLINK(r) = i;
        for (size_t p = ULINK(i); p != i;) {
            unhide(p);
            p = ULINK(p);
        }
    }

    void purify(size_t p) {
        int c = COLOR(p), i = TOP(p);
        CHECK(i >= 0) << "Bad top value for " << p;
        COLOR(i) = c;
        for(size_t q = DLINK(i); q != static_cast<size_t>(i); q = DLINK(q)) {
            if (COLOR(q) == c) { COLOR(q) = -1; }
            else { hide(q); }
        }
    }

    void unpurify(size_t p) {
        int c = COLOR(p), i = TOP(p);
        CHECK(i >= 0) << "Bad top value for " << p;
        for(size_t q = ULINK(i); q != static_cast<size_t>(i); q = ULINK(q)) {
            if (COLOR(q) < 0) { COLOR(q) = c; }
            else { unhide(q); }
        }
    }

    void commit(size_t p, size_t j) {
        if (COLOR(p) == 0) cover(j);
        if (COLOR(p) > 0) purify(p);
    }

    void uncommit(size_t p, size_t j) {
        if (COLOR(p) == 0) uncover(j);
        if (COLOR(p) > 0) unpurify(p);
    }

    void tweak(size_t x, size_t p) {
        CHECK(x == DLINK(p));
        CHECK(p == ULINK(x));
        CHECK(COLOR(x) >= 0) << "Attempt to tweak non-primary?";
        if (BOUND(p) != 0) hide(x);
        size_t d = DLINK(x);
        DLINK(p) = d;
        ULINK(d) = p;
        --LEN(p);
    }

    void untweak(size_t a, size_t i) {
        bool special = BOUND(i) == 0;
        int p = a <= num_items ? a : TOP(a);
        size_t x = a, y = p;
        size_t z = DLINK(p);
        DLINK(p) = x;
        size_t k = 0;
        while (x != z) {
            ULINK(x) = y;
            ++k;
            if (!special) unhide(x);
            y = x;
            x = DLINK(x);
        }
        ULINK(z) = y;
        LEN(p) += k;
        if (special) uncover(p);
    }

    void try_option(size_t x) {
        for(size_t p = x + 1; p != x;) {
            size_t j = TOP(p);
            if (TOP(p) <= 0) {
                p = ULINK(p);
            } else if (j <= num_primary_items) {
                --BOUND(j);
                ++p;
                if (BOUND(j) == 0) cover(j);
            } else {
                commit(p, j);
                ++p;
            }
        }
    }

    void try_again(size_t x) {
        for(size_t p = x - 1; p != x;) {
            size_t j = TOP(p);
            if (TOP(p) <= 0) {
                p = DLINK(p);
            } else if (j <= num_primary_items) {
                ++BOUND(j);
                --p;
                if (BOUND(j) == 1) uncover(j);
            } else {
                uncommit(p, j);
                --p;
            }
        }
    }

    // Returns true iff backtracking was successful.
    bool backtrack(size_t& l, size_t& i) {
        while(true) {
            // M9. [Leave level l.]
            if (l == 0) return false;
            --l;
            if (choice[l] <= num_items) {
                i = choice[l];
                size_t p = LLINK(i);
                size_t q = RLINK(i);
                LLINK(q) = i;
                RLINK(p) = i;

                // M8. [Restore i.]
                if (BOUND(i) == 0 && SLACK(i) == 0) uncover(i);
                else untweak(ft[l], i);
                ++BOUND(i);  // -> M9
            } else {
                i = TOP(choice[l]);
                CHECK(static_cast<int>(i) == TOP(choice[l]));

                // M7. [Try again.]
                try_again(choice[l]);
                choice[l] = DLINK(choice[l]);
                return true;  // -> M5
            }
        }
    }

    bool should_try(size_t l, size_t i) {
        // M5. [Possibly tweak x_l.]
        if (BOUND(i) == 0 && SLACK(i) == 0) {
            if  (choice[l] == i) return false;
        } else if ((BOUND(i) != 0 || SLACK(i) != 0) &&
                   LEN(i) <= (int)BOUND(i) - (int)SLACK(i)) {
            return false;
        } else if (choice[l] != i) {
            tweak(choice[l], i);
        } else if (BOUND(i) != 0) {
            size_t p = LLINK(i);
            size_t q = RLINK(i);
            RLINK(p) = q;
            LLINK(q) = p;
        }
        return true;
    }

    size_t choose_item(size_t l) {
        int theta = std::numeric_limits<int>::max();
        size_t i = RLINK(0);
        INC(choices);
        for(size_t p = RLINK(0); p != 0; p = RLINK(p)) {
            int s = monus(LEN(p) + 1, monus(BOUND(p), SLACK(p)));
            if ((PARAM_prefer_sharp && s > 1 && NAME(p)[0] != '#') ||
                (PARAM_prefer_unsharp && s > 1 && NAME(p)[0] == '#')) {
                s += num_options;
            }
            if (s < theta ||
                (s == theta && SLACK(p) < SLACK(i)) ||
                (s == theta && SLACK(p) == SLACK(i) && LEN(p) > LEN(i))) {
                theta = s;
                i = p;
                if (theta == 0) break;
            }
        }
        INC(score, theta);
        score[l] = theta;
        ft[l] = 0;
        LOG(2) << "Chose i=" << i << " (" << NAME(i) << ")";
        return i;
    }


    void visit(size_t l) {
        std::ostringstream oss;
        oss << "Solution: " << std::endl;
        for (size_t j = 0; j < l; ++j) {
            size_t r = choice[j];
            if (r <= num_items) continue;
            while (TOP(r) >= 0) ++r;
            oss << "  " << -TOP(r) << ": ";
            for(size_t p = ULINK(r); TOP(p) > 0; ++p) {
                size_t q = TOP(p);
                oss << NAME(q);
                if (COLOR(q) > 0) oss << ":" << colors[COLOR(q)];
                oss << " ";
            }
            oss << std::endl;
        }
        LOG(1) << oss.str();
    }

    double progress(size_t l) {
        double p = 0;
        double denom = 1;
        for (size_t j = 0; j < l; ++j) {
            size_t i = choice[j];
            size_t c = i <= num_items ? i : TOP(i);
            size_t k = 1;
            for(size_t q = ft[j] ? ft[j] : DLINK(c); q != choice[j];
                q = DLINK(q)) {
                ++k;
            }
            denom *= score[j];
            p += (k - 1) / denom;
        }
        p += 1.0 / (2.0 * denom);
        return p * 100;
    }

    void solve() {
        // M1. [Initialize.]
        INITCOUNTER(solutions);
        size_t l = 0;

        while (true) {
            // M3. [Choose i.]
            size_t i = choose_item(l);
            if (score[l] == 0) {
                INC(score_zero);
                if (!backtrack(l, i)) return;
            } else {
                // M4. [Prepare to branch on i.]
                choice[l] = DLINK(i);
                if (--BOUND(i) == 0) cover(i);
                if (BOUND(i) != 0 || SLACK(i) != 0) ft[l] = choice[l];
            }

            while (true) {
                LOG_EVERY_N_SECS_T(0, 1)
                    << "sols: " << GETCOUNTER(solutions) << " done: "
                    << std::setprecision(3) << progress(l) << "%";

                if (should_try(l, i)) {
                    // M6. [Try x_l.]
                    LOG(2) << "Trying x_" << l << " = " << choice[l];
                    if (choice[l] != i) try_option(choice[l]);
                    ++l;

                    // M2. [Enter level l.]
                    if (RLINK(0) != 0) break;  // -> M3
                    INC(solutions);
                    visit(l);
                } else {
                    // M8. [Restore i.]
                    if (BOUND(i) == 0 && SLACK(i) == 0) uncover(i);
                    else untweak(ft[l], i);
                    ++BOUND(i);
                }

                if (!backtrack(l, i)) return;
            }
        }
    }

};

int main(int argc, char** argv) {
    int oidx;
    CHECK(parse_flags(argc, argv, &oidx))
        << "Usage: " << argv[0] << " [-vV] <filename>\n"
        << "V: verbosity (>= 2 prints solutions)\n";
    CHECK(!PARAM_prefer_sharp || !PARAM_prefer_unsharp) <<
        "Both prefer_sharp and prefer_unsharp are set. Use only one.";
    init_counters();
    MCC(argv[oidx]).solve();
    return 0;
}
