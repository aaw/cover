#include "logging.h"
#include "counters.h"
#include "flags.h"

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
        CHECK(fgets(s, MAX_LINE_SIZE, f) != NULL) <<
            "No header row listing items.";
        nodes.push_back(Node());  // Header
        int offset = 0, r = 0;
        while (sscanf(s + offset, " %s %n", ss, &r) > 0) {
            offset += r;
            LOG(0) << "Got " << ss;
            CHECK(header.find(ss) == header.end()) <<
                "Duplicate item name: " << ss;
            header[ss] = nodes.size();
            Node n;
            n.name = ss;
            n.llink = nodes.size() - 1;
            nodes.back().rlink = nodes.size();
            nodes.push_back(n);
        }
        num_items = header.size();

        // I2. [Finish the horizontal list.]
        nodes.back().rlink = 0;
        nodes[0].llink = nodes.size()-1;

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

        LOG(3) << "After parsing, memory is: " << debug_nodes();
        fclose(f);
    }

    void hide(size_t p) {
        for(size_t q = p + 1; q != p; ++q) {
            size_t x = TOP(q), u = ULINK(q), d = DLINK(q);
            if (x <= 0) { q = u; continue; } // q was a spacer.
            DLINK(u) = d;
            ULINK(d) = u;
            LEN(x)--;
        }
    }

    void unhide(size_t p) {
        for(size_t q = p - 1; q != p; --q) {
            size_t x = TOP(q), u = ULINK(q), d = DLINK(q);
            if (x <= 0) { q = d; continue; } // q was a spacer.
            DLINK(u) = q;
            ULINK(d) = q;
            LEN(x)++;
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

    void solve() {
        // X1. [Initialize.]
        size_t l = 0;
        std::vector<size_t> x(num_options);

        while (true) {
            // X3. [Choose i.]
            size_t i = RLINK(0);  // TODO: use MRV heuristic.
            LOG(2) << "Chose i=" << i << " (" << NAME(i) << ")";

            // X4. [Cover i.]
            cover(i);
            x[l] = DLINK(i);

            while (true) {
                // X5. [Try x_l.]
                if (x[l] == i) {
                    // X7. [Backtrack.]
                    uncover(i);
                    // X8. [Leave level l.]
                    if (l == 0) return;
                    --l;
                } else {
                    for(size_t p = x[l] + 1; p != x[l];) {
                        size_t j = TOP(p);
                        if (j <= 0) { p = ULINK(p); }
                        else {
                            cover(j);
                            ++p;
                        }
                    }
                    ++l;
                    // X2. [Enter level l.]
                    if (RLINK(0) != 0) break; // -> X3
                    // TODO: visit
                    LOG(0) << "Solution!";
                    // X8. [Leave level l.]
                    if (l == 0) return;
                    --l;
                }

                // X6 [Try again.]
                for(size_t p = x[l] - 1; p != x[l];) {
                    size_t j = TOP(p);
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
