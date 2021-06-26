#include "logging.h"
#include "counters.h"
#include "flags.h"

#include <string>
#include <vector>

struct Node {
    std::string name;
    size_t ulink;
    size_t dlink;
    size_t llink;
    size_t rlink;
    size_t top_or_len;
};

#define NAME(i) (nodex[i].name)
#define LLINK(i) (nodes[i].llink)
#define RLINK(i) (nodes[i].rlink)
#define ULINK(i) (nodes[i].ulink)
#define DLINK(i) (nodes[i].dlink)
#define TOP(i) (nodes[i].top_or_len)
#define LEN(i) (nodes[i].top_or_len)
#define MAX_LINE_SIZE (10000)

struct XC {
    std::vector<Node> nodes;

    XC(const char* filename) {
        FILE* f = fopen(filename, "r");
        CHECK(f) << "Failed to open file: " << filename;
        char s[MAX_LINE_SIZE];
        char ss[MAX_LINE_SIZE];
        // I1. [Read the first line.]
        CHECK(fgets(s, MAX_LINE_SIZE, f) != NULL) <<
            "No header row listing items.";
        while (sscanf(s, " %s ", ss) > 0) {
            Node n;
            n.name = ss;
            n.llink = nodes.size() - 1;
            nodes.back().rlink = nodes.size();
            nodes.push_back(n);
        }
        // I2. [Finish the horizontal list.]

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
