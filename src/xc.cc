#include "logging.h"
#include "counters.h"
#include "flags.h"
#include "params.h"

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
    int top_or_len;
};

#define NAME(i) (nodes[i].name)
#define LLINK(i) (nodes[i].llink)
#define RLINK(i) (nodes[i].rlink)
#define ULINK(i) (nodes[i].ulink)
#define DLINK(i) (nodes[i].dlink)
#define TOP(i) (nodes[i].top_or_len)
#define LEN(i) (nodes[i].top_or_len)
#define MAX_LINE_SIZE (10000)

struct XC {
    std::vector<Node> nodes;
    std::vector<size_t> choice;
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
                << " t: " << n.top_or_len << " }" << std::endl;
            ++i;
        }
        return oss.str();
    }

    XC(const char* filename) {
        FILE* f = fopen(filename, "r");
        CHECK(f) << "Failed to open file: " << filename;
        char s[MAX_LINE_SIZE];
        char ss[MAX_LINE_SIZE];

        // I1. [Read the first line.]
        std::unordered_map<std::string, size_t> header;
        nodes.push_back(Node());  // Header
        num_primary_items = std::numeric_limits<size_t>::max();
        while(fgets(s, MAX_LINE_SIZE, f) != NULL) {
            int offset = 0, r = 0;
            std::string curr;
            while (sscanf(s + offset, " %s %n", ss, &r) > 0) {
                curr = ss;
                if (curr[0] == '/' && curr.size() > 1 && curr[1] == '/') break;
                if (curr == "\\") break;
                offset += r;
                CHECK(header.find(curr) == header.end()) <<
                    "Duplicate item name: " << ss;
                header[curr] = nodes.size();
                if (curr == "|") {
                    num_primary_items = nodes.size() - 1;
                    continue;
                }
                Node n;
                n.name = curr;
                n.llink = nodes.size() - 1;
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
            nodes[num_primary_items + 1].llink = nodes.size() - 1;
            nodes.back().rlink = num_primary_items + 1;
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
        size_t j = 0;

        while(fgets(s, MAX_LINE_SIZE, f) != NULL) {
            int offset = 0, r = 0;
            std::string curr;
            while (sscanf(s + offset, " %s %n", ss, &r) > 0) {
                curr = ss;
                if (curr[0] == '/' && curr.size() > 1 && curr[1] == '/') break;
                if (curr == "\\") break;
                offset += r;
                ++j;
                size_t i = header[curr];
                CHECK(i > 0) << "Item " << curr << " not in header";
                CHECK(seen.find(curr) == seen.end()) <<
                    "Duplicate item " << curr;
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
        LOG(1) << "Parsed " << num_options << " options";

        LOG(3) << "After parsing, memory is: " << debug_nodes();
        fclose(f);

        choice = std::vector<size_t>(num_options);
    }

    void hide(size_t p) {
        for(size_t q = p + 1; q != p;) {
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

    void try_once(size_t l) {
        for(size_t p = choice[l] + 1; p != choice[l];) {
            int j = TOP(p);
            if (j <= 0) { p = ULINK(p); }
            else {
                cover(j);
                ++p;
            }
        }
    }

    void try_again(size_t l) {
        for(size_t p = choice[l] - 1; p != choice[l];) {
            int j = TOP(p);
            if (j <= 0) { p = DLINK(p); }
            else {
                uncover(j);
                --p;
            }
        }
    }

    size_t choose() {
        int theta = std::numeric_limits<int>::max();
        size_t i = RLINK(0);
        for(size_t p = RLINK(0); p != 0; p = RLINK(p)) {
            int lambda = LEN(p);
            if ((PARAM_prefer_sharp && lambda > 1 && NAME(p)[0] != '#') ||
                (PARAM_prefer_unsharp && lambda > 1 && NAME(p)[0] == '#')) {
                lambda += num_options;
            }
            if (lambda < theta) {
                theta = lambda;
                i = p;
                if (theta == 0) break;
            }
        }
        LOG(2) << "Chose i=" << i << " (" << NAME(i) << ")";
        return i;
    }

    void visit(size_t l) {
        std::ostringstream oss;
        oss << "Solution: " << std::endl;
        for (size_t j = 0; j < l; ++j) {
            size_t r = choice[j];
            while (TOP(r) >= 0) ++r;
            oss << "  " << -TOP(r) << ": ";
            for(size_t p = ULINK(r); TOP(p) > 0; ++p) {
                oss << NAME(TOP(p)) << " ";
            }
            oss << std::endl;
        }
        LOG(1) << oss.str();
    }

    double progress(size_t l) {
        double p = 0;
        double denom = 1;
        for (size_t j = 0; j < l; ++j) {
            size_t i = TOP(choice[j]);
            size_t k = 1;
            for(size_t q = DLINK(i); q != i && q != choice[j]; q = DLINK(q)) {
                ++k;
            }
            denom *= LEN(i);
            p += (k - 1) / denom;
        }
        p += 1.0 / (2.0 * denom);
        return p * 100;
    }

    void solve() {
        // X1. [Initialize.]
        INITCOUNTER(solutions);
        size_t l = 0;

        while (true) {
            // X3. [Choose i.]
            size_t i = choose();

            // X4. [Cover i.]
            cover(i);
            choice[l] = DLINK(i);

            while (true) {
                LOG_EVERY_N_SECS_T(0, 1)
                    << "sols: " << GETCOUNTER(solutions) << " done: "
                    << std::setprecision(3) << progress(l) << "%";

                // X5. [Try x_l.]
                LOG(2) << "Trying x_" << l << " = " << choice[l];
                if (choice[l] == i) {
                    // X7. [Backtrack.]
                    uncover(i);
                } else {
                    try_once(l);
                    ++l;
                    // X2. [Enter level l.]
                    if (RLINK(0) != 0) break; // -> X3
                    INC(solutions);
                    visit(l);
                }

                // X8. [Leave level l.]
                if (l == 0) return;
                --l;

                // X6 [Try again.]
                try_again(l);
                i = TOP(choice[l]);
                choice[l] = DLINK(choice[l]);
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
    XC(argv[oidx]).solve();
    return 0;
}
