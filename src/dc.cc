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

// TODO: fix int vs. size_t without (int) casts, we only need int for item to represent negatives
struct OptionNode {
    int item;
    size_t loc;
    size_t color = 0;
};

struct TrailNode {
    size_t i;
    size_t size;
};

#define SIZE(i) (set_[(i)-1])
#define POS(i) (set_[(i)-2])
#define NAME(i) (name_[set_[(i)-2]])
// TODO: explain how our set layout differs from Knuth's: he's got LNAME/RNAME so a 4-word
// padding in each cell but we only have 2-word padding and store names externally.
#define SET_PAD 2
#define ITM(i) (node_[i].item)
#define LOC(i) (node_[i].loc)
#define CLR(i) (node_[i].color)
#define MAX_LINE_SIZE (100000)

struct DC {
    std::vector<OptionNode> node_;
    std::vector<std::string> name_;
    std::vector<std::string> color_;
    std::vector<size_t> item_;
    std::vector<size_t> set_;
    size_t active_;
    size_t oactive_;
    size_t second_;
    size_t num_primary_;
    size_t flag_;

    size_t color_parse(char* ss, std::unordered_map<std::string, size_t>& colors,
                       size_t next_color, std::string* curr) {
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

    DC(const char* filename) {
        FILE* f = fopen(filename, "r");
        CHECK(f) << "Failed to open file: " << filename;
        char s[MAX_LINE_SIZE];
        char ss[MAX_LINE_SIZE];

        // I1. [Read the first line.]
        std::unordered_map<std::string, size_t> iname;
        num_primary_ = std::numeric_limits<size_t>::max();
        second_ = std::numeric_limits<size_t>::max();
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
                CHECK(iname.find(curr) == iname.end()) <<
                    "Duplicate item name: '" << ss << "'";
                if (curr == "|") {
                    num_primary_ = iname.size();
                    second_ = iname.size() + 1;
                    continue;
                }
                iname[curr] = name_.size();
                name_.push_back(curr);
            }
            if (curr != "\\" && !iname.empty()) break;
        }
        set_.resize(SET_PAD * name_.size(), 0);
        item_.resize(name_.size(), 0);
        OptionNode spacer = {.item = 0, .loc = 0, .color = 0};
        node_.push_back(spacer);
        active_ = name_.size();
        if (num_primary_ == std::numeric_limits<size_t>::max()) {
            num_primary_ = name_.size();
        }
        LOG(1) << "Parsed " << name_.size() << " items (" << num_primary_
               << " primary)";

        // I2. [Read an option.]
        size_t num_options = 0;
        std::unordered_set<std::string> seen;
        std::unordered_map<std::string, size_t> color_ids;
        size_t next_color = 1;

        int n = 0;
        while(fgets(s, MAX_LINE_SIZE, f) != NULL) {
            CHECK(strlen(s) < MAX_LINE_SIZE-1 || s[strlen(s)-1] == '\n') <<
                "Input line too long. Recompile with larger MAX_LINE_SIZE.";
            int offset = 0, r = 0;
            size_t cnum = 0;
            std::string curr;
            while (sscanf(s + offset, " %s %n", ss, &r) > 0) {
                cnum = color_parse(ss, color_ids, next_color, &curr);
                next_color = std::max(next_color, cnum + 1);
                if (curr[0] == '/' && curr.size() > 1 && curr[1] == '/') break;
                if (curr == "\\") break;
                offset += r;
                auto itr = iname.find(curr);
                CHECK(itr != iname.end()) << "Item '" << curr << "' not in header";
                int i = itr->second + 1;
                CHECK(i >= (int)num_primary_ || cnum == 0) <<
                    "Color can't be assigned to primary item (" << ss << ")";
                CHECK(seen.find(curr) == seen.end()) <<
                    "Duplicate item '" << curr << "'";
                seen.insert(curr);
                OptionNode node = {.item = i, .loc = SIZE(SET_PAD * i), .color = cnum};
                node_.push_back(node);
                ++n;
                SIZE(2*i)++;
            }
            if (curr == "\\" || seen.empty()) continue;

            // Adjust LOC for the previous spacer.
            node_[node_.size() - n - 1].loc = n;
            OptionNode spacer = {.item = -n, .loc = 0, .color = 0};
            node_.push_back(spacer);
            seen.clear();
            ++num_options;
            n = 0;
        }
        color_.resize(next_color);
        for (const auto& kv: color_ids) { color_[kv.second] = kv.first; }

        // I3. [Initialize ITEM.]
        size_t j = SET_PAD;
        for (size_t k = 0; k < active_; ++k) {
            item_[k] = j;
            j += SET_PAD + SIZE(SET_PAD * (k+1));
        }
        if (name_.size() == num_primary_) {  // if no secondaries
            second_ = j;
        }

        // I4. [Expand SET.]
        set_.resize(j, 0);
        for (size_t k = active_; k > 0; --k) {
            size_t j = item_[k-1];
            SIZE(j) = SIZE(SET_PAD * k);
            POS(j) = k-1;
            CHECK(SIZE(j) != 0 || k > num_primary_) << "Primary item " << name_[k-1] << " has no options.";
            if (k == second_) { LOG(3) << "setting second_ = " << j; second_ = j; }
        }

        // I5. [Adjust NODE.]
        for (size_t x = 1; x < node_.size(); ++x) {
            if (ITM(x) < 0) continue;
            size_t i = item_[ITM(x)-1];
            size_t j = i + LOC(x);
            ITM(x) = i;
            LOC(x) = j;
            set_[j] = x;
        }

        LOG(1) << "Parsed " << color_ids.size() << " colors";
        LOG(1) << "Parsed " << num_options << " options";

        LOG(3) << "After parsing, memory is: \n" << debug_memory();
        fclose(f);
    }

    std::string debug_item() {
        std::ostringstream oss;
        oss << "ITEM: \n";
        for (size_t i = 0; i < item_.size(); ++i) {
            oss << "[" << i << "]: " << item_[i] << " ";
        }
        return oss.str();
    }

    std::string debug_set() {
        std::ostringstream oss;
        std::unordered_set<size_t> items;
        for (const auto& item : item_) items.insert(item);
        oss << "SET:";
        bool ignore = false;
        for (size_t i = 0; i < set_.size(); ++i) {
            std::string sep = " ";
            auto itr = std::find(item_.begin(), item_.end(), i+2);
            if (itr != item_.end()) {
                size_t index = std::distance(item_.begin(), itr);
                ignore = index >= active_;
                sep = ignore ? " *POS " : " POS ";
                oss << "\n" << NAME(i+2) << ": ";
            } else if (items.find(i+1) != items.end()) {
                sep = "SIZE ";
            }
            oss << "[" << i << "]: " << sep << set_[i] << " ";
        }
        return oss.str();
    }

    std::string debug_node() {
        std::ostringstream oss;
        oss << "NODE:";
        for (size_t i = 0; i < node_.size(); ++i) {
            if (node_[i].item <= 0) oss << "\n";
            oss << " [" << i << "]" << ": {itm: " << node_[i].item << ", loc: " << node_[i].loc << ", clr: " << node_[i].color << "} ";
        }
        return oss.str();
    }

    std::string debug_name() {
        std::ostringstream oss;
        oss << "NAME:";
        for (size_t i = 0; i < name_.size(); ++i) { oss << "[" << i << "] " << name_[i] << " "; }
        return oss.str();
    }

    std::string debug_memory() {
        std::ostringstream oss;
        oss << debug_name() << "\n" << debug_item() << "\n" << debug_set() << "\n" << debug_node();
        return oss.str();
    }

    void hide(size_t i, size_t color) {
        LOG(3) << debug_set();
        LOG(3) << "Hiding " << NAME(i) << ", i = " << i << ", set_[i-2] = set[" << i-2 << "] = " << set_[i-2];
        for(size_t j = 0; j < SIZE(i); ++j) {
            LOG(3) << "set_[" << i << "+" << j << "] = " << set_[i+j];
            size_t x = set_[i+j];
            if (color != 0 && color == CLR(x)) continue;
            // for all siblings sib of x:
            LOG(3) << "Looking at siblings of " << x;
            for(size_t sib = x + 1; sib != x; ++sib) {
                int ip = ITM(sib);
                LOG(3) << "  ITM(" << sib << ") = " << ip;
                if (ip < 0) { sib += (ip - 1); continue; } // sibling wrap-around: -1 is b/c for loop will inc again
                if (POS(ip) >= oactive_) continue;
                size_t sp = SIZE(ip) - 1;
                if (sp == 0 && flag_ == 0 && ip < (int)second_ && POS(ip) < active_) {
                    flag_ = 1;
                    return;
                } else {
                    size_t xpp = set_[ip + sp];
                    SIZE(ip) = sp;
                    set_[ip + sp] = sib;
                    size_t ipp = LOC(sib);
                    LOG(3) << "  swapping set[" << ipp << "] and set[" << ip << "+" << sp << "]";
                    LOG(3) << "  swapping LOC(" << sib << ") and LOC(" << xpp << ")";
                    LOC(sib) = ip + sp;
                    set_[ipp] = xpp;
                    LOC(xpp) = ipp;
                }
            }
        }
    }

    std::string print_solution(const std::vector<size_t>& xs) {
        std::ostringstream oss;
        // For each option in the solution:
        for (size_t x : xs) {
            oss << "  " << x << ": ";
            // Rewind to the start of the option.
            size_t ni = x;
            while (ITM(ni) > 0) { ++ni; }
            ni += ITM(ni);
            // Print the items in the option.
            while (ITM(ni) > 0) {
                oss << NAME(ITM(ni));
                if (CLR(ni) > 0) { oss << ":" << color_[CLR(ni)]; }
                oss << " "; ++ni;
            }
            oss << "\n";
        }
        return oss.str();
    }

    void solve() {
        // C1. [Initialize.]
        INITCOUNTER(solutions);
        std::vector<size_t> xs, ys;
        std::vector<TrailNode> trail;
        ys.push_back(0);
        // TODO: remove l entirely, just use xs.size()
        // TODO: don't init xs,ys,trail
        size_t l = 0;
        size_t t = 0;
        size_t j = 0; // TODO: never mentioned that we need to init like this?
        size_t k = 0; // TODO: also never mentioned?

        while (true) {
            LOG(3) << "choosing, memory is: " << debug_memory();
            // C2. [Choose i.]
            size_t theta = std::numeric_limits<size_t>::max();
            int i = std::numeric_limits<int>::max();
            for (size_t k = 0; k < active_; ++k) {
                LOG(3) << "considering k = " << k << ", item_[" << k << "] = " << item_[k]
                       << ", second_ = " << second_;
                if (item_[k] >= second_) continue;
                size_t lambda = SIZE(item_[k]);
                LOG(3) << "lambda = " << lambda << ", theta = " << theta << ", item_["
                       << k << "] = " << item_[k] << ", i = " << i;
                if (lambda == theta && ((int)item_[k]) < i) i = item_[k];
                if (lambda < theta) {
                    i = item_[k];
                    theta = lambda;
                    if (lambda == 1) break;
                }
            }
            if (theta != std::numeric_limits<size_t>::max()) LOG(3) << "Chose " << NAME(i) << "(i = " << i << ")";

            if (theta == std::numeric_limits<size_t>::max()) {
                // C9. [Visit a solution.]
                LOG(1) << "Solution:\n" << print_solution(xs);
                INC(solutions);

                // TODO: this should be a do {} while () around C10.
                while (true) {
                    // C10. [Leave level l.]
                    LOG(3) << "Leaving level " << l;
                    if (l == 0) return;
                    --l;
                    i = ITM(xs.back());
                    j = LOC(xs.back());
                    xs.pop_back();

                    // C11. [Try again?]
                    if (j+1 >= i + SIZE(i)) continue; // -> C10
                    for (size_t k = ys[l]; k < ys[l+1]; ++k) {
                        TrailNode tn = trail[k];
                        SIZE(tn.i) = tn.size;
                    }
                    t = ys[l+1];
                    active_ = t - ys[l];
                    ++j;
                    break; // -> C6
                }
            } else {
                // C3. [Deactivate i.]
                LOG(3) << debug_set();
                LOG(3) << "Deactivating " << NAME(i) << "(i=" << i << ")";
                size_t kp = active_ - 1;
                active_ = kp;
                size_t ip = item_[kp];
                k = POS(i);
                LOG(3) << NAME(i) << " is at POS " << k;
                item_[kp] = i;
                item_[k] = ip;
                POS(ip) = k;
                POS(i) = kp;
                std::swap(name_[kp], name_[k]);
                LOG(3) << debug_set();

                // C4. [Hide i.]
                LOG(3) << "Hiding " << NAME(i) << "(i=" << i << ")";
                oactive_ = active_;
                flag_ = -1;
                hide(i, 0);
                j = i;

                LOG(3) << "After hiding, " << debug_memory();

                // C5. [Trail the sizes.]
                trail.resize(t + active_);
                for (size_t kk = 0; kk < active_; ++kk) {
                    LOG(3) << "setting trail[" << t+kk << "]";
                    trail[t+kk] = {.i = item_[kk], .size = SIZE(item_[kk])};
                }
                t += active_;
                LOG(3) << "ys size: " << ys.size() << ", want to set ys[" << l+1;
                ys.resize(l+2); ys[l+1] = t;
            }

            LOG(0) << "coming from above";
            while (true) {
                // C6. [Try SET[j].]
                LOG(0) << "xs size: " << xs.size() << ", want to set xs[" << l << "]: " << (l - (int)xs.size()+1);
                //xs.resize(l+1); xs[l] = set_[j];
                xs.push_back(set_[j]);
                k = oactive_ = active_;
                // for all siblings sib of x[l]:
                for(size_t sib = xs.back() + 1; sib != xs.back(); ++sib) {
                    int ip = ITM(sib);
                    if (ip < 0) { sib += (ip - 1); continue; } // sibling wrap-around: -1 b/c for loop incs after continue
                    size_t kp = POS(ip);
                    if (kp < k) {
                        --k;
                        size_t ipp = item_[k];
                        item_[k] = ip;
                        item_[kp] = ipp;
                        POS(ipp) = kp;
                        POS(ip) = k;
                        std::swap(name_[k], name_[kp]);
                    }
                }
                active_ = k;

                // C7. [Hide SET[j].]
                flag_ = 0;
                for(size_t sib = xs.back() + 1; sib != xs.back(); ++sib) {
                    int ip = ITM(sib);
                    if (ip < 0) { sib += (ip - 1); continue; } // sibling wrap-around: -1 b/c for loop incs after continue
                    if (ip < (int)second_ || POS(ip) < oactive_) {
                        hide(ip, CLR(sib));
                        if (flag_ == 1) break;
                    }
                }
                if (flag_ == 1) {
                    // C11. [Try again?]
                    while (j + 1 >= i + SIZE(i)) {
                        // C10. [Leave level l.]
                        if (l == 0) return;
                        --l;
                        xs.pop_back();
                        i = ITM(xs.back());
                        j = LOC(xs.back());
                    }
                    LOG(3) << "ys size: " << ys.size();
                    LOG(3) << "looking at ys from [" << ys[l] << "," << ys[l+1] << ")";
                    for (size_t k = ys[l]; k < ys[l+1]; ++k) {
                        TrailNode tn = trail[k];
                        LOG(3) << "popped trail node " << k << ": (" << tn.i << ", " << tn.size << ")";
                        SIZE(tn.i) = tn.size;
                    }
                    t = ys[l+1];
                    active_ = t - ys[l];
                    ++j;
                    LOG(0) << "jumping to C6";
                    xs.pop_back();
                    continue; // -> C6
                }
                break;
            }

            // C8. [Advance to the next level.]
            LOG(3) << "Level " << l << " -> " << l+1;
            ++l; // -> C2
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
    DC(argv[oidx]).solve();
    return 0;
}
