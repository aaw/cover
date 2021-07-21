#include "logging.h"
#include "counters.h"
#include "flags.h"

#include <iomanip>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
    size_t z;  // Index of last spacer node.
    size_t y;  // Index of last primary node.
    size_t num_items;
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
        y = std::numeric_limits<size_t>::max();
        while(fgets(s, MAX_LINE_SIZE, f) != NULL) {
            int offset = 0, r = 0;
            std::string curr;
            while (sscanf(s + offset, " %s %n", ss, &r) > 0) {
                curr = ss;
                if (curr[0] == '#') break;
                if (curr == "\\") break;
                offset += r;
                CHECK(header.find(curr) == header.end()) <<
                    "Duplicate item name: " << ss;
                header[curr] = nodes.size();
                if (curr == "|") {
                    y = nodes.size() - 1;
                    continue;
                }
                Node n;
                n.name = curr;
                n.llink = nodes.size() - 1;
                nodes.back().rlink = nodes.size();
                nodes.push_back(n);
            }
            if (curr[0] == '#' && !header.empty()) break;
        }
        num_items = header.size();
        LOG(1) << "Parsed " << num_items << " items";

        // I2. [Finish the horizontal list.]
        if (y == std::numeric_limits<size_t>::max()) {
            y = nodes.size() - 1;
        } else {
            nodes[y+1].llink = nodes.size()-1;
            nodes.back().rlink = y+1;
        }
        nodes[y].rlink = 0;
        nodes[0].llink = y;

        // I3. [Prepare for options.]
        for (size_t i = 1; i < nodes.size(); ++i) {
            LEN(i) = 0;
            ULINK(i) = DLINK(i) = i;
        }
        size_t m = 0;
        size_t p = nodes.size();
        nodes.push_back(Node());  // First spacer

        num_options = 0;
        while(fgets(s, MAX_LINE_SIZE, f) != NULL) {
            num_options++;
            std::unordered_set<std::string> seen;
            size_t j = 0;

            int offset = 0, r = 0;
            while (sscanf(s + offset, " %s %n", ss, &r) > 0) {
                offset += r;
                ++j;
                size_t i = header[ss];
                CHECK(i > 0) << "Item " << ss << " not in header";
                CHECK(seen.find(ss) == seen.end()) <<
                    "Duplicate item " << ss;
                seen.insert(ss);
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

            // I5. [Finish an option.]
            m++;
            DLINK(p) = p+j;
            nodes.push_back(Node());
            CHECK(nodes.size()-1 == p+j+1) << "No room for spacer";
            p = p+j+1;
            TOP(p) = -m;
            ULINK(p) = p-j;
            z = p;
        }
        LOG(1) << "Parsed " << num_options << " options";

        LOG(3) << "After parsing, memory is: " << debug_nodes();
        fclose(f);
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

    void visit(std::vector<size_t>& x, size_t l) {
        std::ostringstream oss;
        oss << "Solution: " << std::endl;
        for (size_t j = 0; j < l; ++j) {
            size_t r = x[j];
            while (TOP(r) >= 0) ++r;
            oss << "  " << -TOP(r) << ": ";
            for(size_t p = ULINK(r); TOP(p) > 0; ++p) {
                oss << NAME(TOP(p)) << " ";
            }
            oss << std::endl;
        }
        LOG(1) << oss.str();
    }

    double progress(std::vector<size_t>& x, size_t l) {
        double p = 0;
        double denom = 1;
        for (size_t j = 0; j < l; ++j) {
            size_t i = TOP(x[j]);
            size_t k = 1;
            for(size_t q = DLINK(i); q != i && q != x[j]; q = DLINK(q)) {
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
        size_t l = 0;
        std::vector<size_t> x(num_options);

        while (true) {
            // X3. [Choose i.]
            int theta = std::numeric_limits<int>::max();
            size_t i = RLINK(0);
            for(size_t p = RLINK(0); p != 0; p = RLINK(p)) {
                if (LEN(p) < theta) {
                    theta = LEN(p);
                    i = p;
                    if (theta == 0) break;
                }
            }
            LOG(2) << "Chose i=" << i << " (" << NAME(i) << ")";

            // X4. [Cover i.]
            cover(i);
            x[l] = DLINK(i);

            while (true) {
                LOG_EVERY_N_SECS_T(0, 1)
                    << "sols: " << GETCOUNTER(solutions) << " done: "
                    << std::setprecision(3) << progress(x,l) << "%";

                // X5. [Try x_l.]
                LOG(2) << "Trying x_" << l << " = " << x[l];
                if (x[l] == i) {
                    // X7. [Backtrack.]
                    uncover(i);
                    // X8. [Leave level l.]
                    if (l == 0) return;
                    --l;
                } else {
                    for(size_t p = x[l] + 1; p != x[l];) {
                        int j = TOP(p);
                        if (j <= 0) { p = ULINK(p); }
                        else {
                            cover(j);
                            ++p;
                        }
                    }
                    ++l;
                    // X2. [Enter level l.]
                    if (RLINK(0) != 0) break; // -> X3
                    INC(solutions);
                    visit(x, l);
                    // X8. [Leave level l.]
                    if (l == 0) return;
                    --l;
                }

                // X6 [Try again.]
                for(size_t p = x[l] - 1; p != x[l];) {
                    int j = TOP(p);
                    if (j <= 0) { p = DLINK(p); }
                    else {
                        uncover(j);
                        --p;
                    }
                }
                i = TOP(x[l]);
                x[l] = DLINK(x[l]);
            }
        }
    }

};

int main(int argc, char** argv) {
    int oidx;
    CHECK(parse_flags(argc, argv, &oidx))
        << "Usage: " << argv[0] << " [-vV] <filename>\n"
        << "V: verbosity (>= 2 prints solutions)\n";
    init_counters();
    XC(argv[oidx]).solve();
    return 0;
}
