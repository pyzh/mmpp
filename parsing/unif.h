#ifndef UNIF_H
#define UNIF_H

#include <functional>
#include <unordered_map>
#include <map>

#include "parsing/parser.h"
#include "parsing/algos.h"

template< typename SymType, typename LabType >
using SubstMap = std::map< LabType, ParsingTree< SymType, LabType > >;

template< typename SymType, typename LabType >
using SubstMap2 = std::map< LabType, ParsingTree2< SymType, LabType > >;

template< typename SymType, typename LabType >
using SimpleSubstMap2 = std::map< LabType, LabType >;

/*template< typename SymType, typename LabType >
using SubstMap = std::unordered_map< LabType, ParsingTree< SymType, LabType > >;

template< typename SymType, typename LabType >
using SubstMap2 = std::unordered_map< LabType, ParsingTree2< SymType, LabType > >;

template< typename SymType, typename LabType >
using SimpleSubstMap2 = std::unordered_map< LabType, LabType >;*/

template< typename SymType, typename LabType >
SubstMap2< SymType, LabType > subst_to_subst2(const SubstMap< SymType, LabType > &subst) {
    SubstMap2< SymType, LabType > subst2;
    for (const auto &x : subst) {
        subst2.insert(std::make_pair(x.first, pt_to_pt2(x.second)));
    }
    return subst2;
}

template< typename SymType, typename LabType >
SubstMap< SymType, LabType > subst2_to_subst(const SubstMap2< SymType, LabType > &subst2) {
    SubstMap< SymType, LabType > subst;
    for (const auto &x : subst2) {
        subst.insert(std::make_pair(x.first, pt2_to_pt(x.second)));
    }
    return subst;
}

template< typename SymType, typename LabType >
ParsingTree< SymType, LabType > substitute(const ParsingTree< SymType, LabType > &pt,
                                           const std::function< bool(LabType) > &is_var,
                                           const SubstMap< SymType, LabType > &subst) {
    if (is_var(pt.label)) {
        assert(pt.children.empty());
        auto it = subst.find(pt.label);
        if (it == subst.end()) {
            return pt;
        } else {
            return it->second;
        }
    } else {
        ParsingTree< SymType, LabType > ret;
        ret.label = pt.label;
        ret.type = pt.type;
        for (auto &child : pt.children) {
            ret.children.push_back(substitute(child, is_var, subst));
        }
        return ret;
    }
}


template< typename SymType, typename LabType >
size_t substitute2_count(const ParsingTree2< SymType, LabType > &pt,
                         const std::function< bool(LabType) > &is_var,
                         const SubstMap2< SymType, LabType > &subst) {
    auto nodes_len = pt.get_nodes_len();
    const auto &nodes = pt.get_nodes();
    size_t ret = nodes_len;
    for (size_t i = 0; i < nodes_len; i++) {
        auto lab = nodes[i].label;
        if (is_var(lab)) {
            auto it = subst.find(lab);
            if (it != subst.end()) {
                ret += it->second.get_nodes_len() - 1;
            }
        }
    }
    return ret;
}

template< typename SymType, typename LabType >
ParsingTree2< SymType, LabType > substitute2(const ParsingTree2< SymType, LabType > &pt,
                                             const std::function< bool(LabType) > &is_var,
                                             const SubstMap2< SymType, LabType > &subst) {
    size_t final_size = substitute2_count(pt, is_var, subst);
    ParsingTree2Generator< SymType, LabType > gen;
    gen.reserve(final_size);
    auto it = pt.get_multi_iterator();
    bool discard_next_close = false;
    //for (auto x = it.next(); x.first != it.Finished; x = it.next()) {
    while (true) {
        const auto x = it.next();
        if (x.first == it.Finished) {
            break;
        }
        if (x.first == it.Open) {
            assert(!discard_next_close);
            if (is_var(x.second.label)) {
                auto it = subst.find(x.second.label);
                if (it != subst.end()) {
                    discard_next_close = true;
                    gen.copy_tree(it->second);
                } else {
                    gen.open_node(x.second.label, x.second.type);
                }
            } else {
                gen.open_node(x.second.label, x.second.type);
            }
        } else {
            assert(x.first == it.Close);
            if (discard_next_close) {
                discard_next_close = false;
            } else {
                gen.close_node();
            }
        }
    }
    ParsingTree2< SymType, LabType > ret = gen.get_parsing_tree();
#ifdef UNIFICATOR_SELF_TEST
    assert(ret == pt_to_pt2(substitute(pt2_to_pt(pt), is_var, subst2_to_subst(subst))));
#endif
    assert(final_size == ret.nodes_storage.size());
    return ret;
}

/*template< typename SymType, typename LabType >
ParsingTree2< SymType, LabType > substitute2_its(const std::vector< std::pair< typename ParsingTreeMultiIterator< SymType, LabType >::Status, ParsingTreeNode< SymType, LabType > > > &its,
                                             const std::function< bool(LabType) > &is_var,
                                             const SubstMap2< SymType, LabType > &subst) {
    using Status = typename ParsingTreeMultiIterator< SymType, LabType >::Status;
    ParsingTree2Generator< SymType, LabType > gen;
    bool discard_next_close = false;
    for (const auto &x : its) {
        if (x.first == Status::Finished) {
            break;
        }
        if (x.first == Status::Open) {
            assert(!discard_next_close);
            if (is_var(x.second.label)) {
                auto it = subst.find(x.second.label);
                if (it != subst.end()) {
                    discard_next_close = true;
                    gen.copy_tree(it->second);
                } else {
                    gen.open_node(x.second.label, x.second.type);
                }
            } else {
                gen.open_node(x.second.label, x.second.type);
            }
        } else {
            assert(x.first == Status::Close);
            if (discard_next_close) {
                discard_next_close = false;
            } else {
                gen.close_node();
            }
        }
    }
    return gen.get_parsing_tree();
}*/

template< typename SymType, typename LabType >
ParsingTree2< SymType, LabType > substitute2_simple(const ParsingTree2< SymType, LabType > &pt,
                                             const std::function< bool(LabType) > &is_var,
                                             const SimpleSubstMap2< SymType, LabType > &subst) {
    ParsingTree2< SymType, LabType > ret = pt;
    ret.refresh();
    for (auto &x : ret.nodes_storage) {
        if (is_var(x.label)) {
            auto it = subst.find(x.label);
            if (it != subst.end()) {
                x.label = it->second;
            }
        }
    }
    return ret;
}

template< typename SymType, typename LabType >
SubstMap< SymType, LabType > compose(const SubstMap< SymType, LabType > &first, const SubstMap< SymType, LabType > &second, const std::function< bool(LabType) > &is_var) {
    // Algorithm described in Chang, Lee (Symbolic logic and mechanical theorem proving), section 5.3 Substitution and unification
    SubstMap< SymType, LabType > ret;
    for (auto &first_pair : first) {
        auto tmp = substitute(first_pair.second, is_var, second);
        if (is_var(tmp.label)) {
            assert(tmp.children.empty());
            // We skip trivial substitutions, both for efficiency and to avoid hiding actual sostitutions from the second map
            if (tmp.label == first_pair.first) {
                continue;
            }
        }
        ret.insert(std::make_pair(first_pair.first, tmp));
    }
    for (auto &second_pair : second) {
        // Substitutions from the second map are automatically discarded by unordered_set if they are hidden by the first one
        ret.insert(second_pair);
    }
    return ret;
}

template< typename SymType, typename LabType >
SubstMap2< SymType, LabType > compose2(const SubstMap2< SymType, LabType > &first, const SubstMap2< SymType, LabType > &second, const std::function< bool(LabType) > &is_var) {
    // Algorithm described in Chang, Lee (Symbolic logic and mechanical theorem proving), section 5.3 Substitution and unification
    SubstMap2< SymType, LabType > ret;
    for (auto &first_pair : first) {
        auto tmp = substitute(first_pair.second, is_var, second);
        if (is_var(tmp.label)) {
            assert(tmp.children.empty());
            // We skip trivial substitutions, both for efficiency and to avoid hiding actual sostitutions from the second map
            if (tmp.label == first_pair.first) {
                continue;
            }
        }
        ret.insert(std::make_pair(first_pair.first, tmp));
    }
    for (auto &second_pair : second) {
        // Substitutions from the second map are automatically discarded by unordered_set if they are hidden by the first one
        ret.insert(second_pair);
    }
#ifdef UNIFICATOR_SELF_TEST
    assert(ret == subst_to_subst2(compose(subst2_to_subst(first), subst2_to_subst(second), is_var)));
#endif
    return ret;
}

template< typename SymType, typename LabType >
SubstMap< SymType, LabType > update(const SubstMap< SymType, LabType > &first, const SubstMap< SymType, LabType > &second, bool assert_disjoint = false) {
    SubstMap< SymType, LabType > ret = first;
    for (auto &second_pair : second) {
        bool inserted;
        std::tie(std::ignore, inserted) = ret.insert(second_pair);
        if (assert_disjoint) {
            assert(inserted);
        }
    }
    return ret;
}

template< typename SymType, typename LabType >
SubstMap2< SymType, LabType > update2(const SubstMap2< SymType, LabType > &first, const SubstMap2< SymType, LabType > &second, bool assert_disjoint = false) {
    SubstMap2< SymType, LabType > ret = first;
    for (auto &second_pair : second) {
        bool inserted;
        std::tie(std::ignore, inserted) = ret.insert(second_pair);
        if (assert_disjoint) {
            assert(inserted);
        }
    }
#ifdef UNIFICATOR_SELF_TEST
    assert(ret == subst_to_subst2(update(subst2_to_subst(first), subst2_to_subst(second))));
#endif
    return ret;
}

template< typename SymType, typename LabType >
bool contains_var(const ParsingTree< SymType, LabType > &pt, LabType var) {
    if (pt.label == var) {
        return true;
    }
    for (auto &child : pt.children) {
        if (contains_var(child, var)) {
            return true;
        }
    }
    return false;
}

template< typename SymType, typename LabType >
bool contains_var2(const ParsingTreeIterator< SymType, LabType > &it, LabType var) {
    if (it.get_node().label == var) {
        return true;
    }
    for (auto &child : it) {
        if (contains_var2(child, var)) {
            return true;
        }
    }
    return false;
}

template< typename SymType, typename LabType >
bool contains_var2(const ParsingTree2< SymType, LabType > &pt, LabType var) {
    return contains_var2(pt.get_root(), var);
}

template< typename SymType, typename LabType >
void collect_variables(const ParsingTree< SymType, LabType > &pt, const std::function< bool(LabType) > &is_var, std::set< LabType > &vars) {
    if (is_var(pt.label)) {
        vars.insert(pt.label);
    } else {
        for (const auto &child : pt.children) {
            collect_variables(child, is_var, vars);
        }
    }
}

template< typename SymType, typename LabType >
void collect_variables2(const ParsingTreeIterator< SymType, LabType > &it, const std::function< bool(LabType) > &is_var, std::set< LabType > &vars) {
    const auto tree = it.get_view();
    auto nodes_len = tree.get_nodes_len();
    const auto &nodes = tree.get_nodes();
    for (size_t i = 0; i < nodes_len; i++) {
        auto lab = nodes[i].label;
        if (is_var(lab)) {
            vars.insert(lab);
        }
    }
}

template< typename SymType, typename LabType >
void collect_variables2(const ParsingTree2< SymType, LabType > &pt, const std::function< bool(LabType) > &is_var, std::set< LabType > &vars) {
    return collect_variables2(pt.get_root(), is_var, vars);
}

// Unilateral unification

template< typename SymType, typename LabType >
[[deprecated]]
bool unify_internal(const ParsingTree< SymType, LabType > &templ, const ParsingTree< SymType, LabType > &target,
                    const std::function< bool(LabType) > &is_var, SubstMap< SymType, LabType > &subst) {
    if (is_var(templ.label)) {
        assert(templ.children.empty());
        auto it = subst.find(templ.label);
        if (it == subst.end()) {
            subst.insert(std::make_pair(templ.label, target));
            return true;
        } else {
            return it->second == target;
        }
    } else {
        if (templ.label != target.label) {
            return false;
        } else {
            assert(templ.children.size() == target.children.size());
            for (size_t i = 0; i < templ.children.size(); i++) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                auto res = unify(templ.children[i], target.children[i], is_var, subst);
#pragma GCC diagnostic pop
                if (!res) {
                    return false;
                }
            }
            return true;
        }
    }
}

template< typename SymType, typename LabType >
[[deprecated]]
bool unify(const ParsingTree< SymType, LabType > &templ, const ParsingTree< SymType, LabType > &target,
           const std::function< bool(LabType) > &is_var, SubstMap< SymType, LabType > &subst) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    bool ret = unify_internal(templ, target, is_var, subst);
#pragma GCC diagnostic pop
#ifdef UNIFICATOR_SELF_TEST
    if (ret) {
        assert(substitute(templ, is_var, subst) == target);
    }
#endif
    return ret;
}

template< typename SymType, typename LabType >
[[deprecated]]
std::pair< bool, SubstMap< SymType, LabType > > unify(const ParsingTree< SymType, LabType > &templ,
                                                      const ParsingTree< SymType, LabType > &target,
                                                      const std::function< bool(LabType) > &is_var) {
    SubstMap< SymType, LabType > subst;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    bool res = unify(templ, target, is_var, subst);
#pragma GCC diagnostic pop
    if (!res) {
        subst = {};
    }
    return std::make_pair(res, subst);
}

template< typename SymType, typename LabType >
class UnilateralUnificator {
public:
    UnilateralUnificator(const std::function< bool(LabType) > &is_var) : is_var(is_var) {
        this->pt1.label = {};
        this->pt2.label = {};
        this->pt1.type = {};
        this->pt2.type = {};
    }

    void add_parsing_trees(const ParsingTree< SymType, LabType > &new_pt1, const ParsingTree< SymType, LabType > &new_pt2) {
        this->pt1.children.push_back(new_pt1);
        this->pt2.children.push_back(new_pt2);
    }

    bool is_unifiable() {
        return true;
    }

    std::pair< bool, SubstMap< SymType, LabType > > unify() {
        bool ret1;
        SubstMap< SymType, LabType > ret2;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        std::tie(ret1, ret2) = ::unify(this->pt1, this->pt2, this->is_var);
#pragma GCC diagnostic pop
        return std::make_pair(ret1, ret2);
    }

    void add_parsing_trees2(const ParsingTree2< SymType, LabType > &new_pt1, const ParsingTree2< SymType, LabType > &new_pt2) {
        this->add_parsing_trees(pt2_to_pt(new_pt1), pt2_to_pt(new_pt2));
    }

    std::pair< bool, SubstMap2< SymType, LabType > > unify2() {
        bool ret1;
        SubstMap< SymType, LabType > ret2;
        std::tie(ret1, ret2) = this->unify();
        return std::make_pair(ret1, subst_to_subst2(ret2));
    }

private:
    const std::function< bool(LabType) > &is_var;
    ParsingTree< SymType, LabType > pt1;
    ParsingTree< SymType, LabType > pt2;
};

// Slow bilateral unification

template< typename SymType, typename LabType >
[[deprecated]]
std::tuple< bool, bool > unify2_slow_step(const ParsingTree< SymType, LabType > &pt1, const ParsingTree< SymType, LabType > &pt2,
                                                   const std::function< bool(LabType) > &is_var, SubstMap< SymType, LabType > &subst) {
    if (pt1.label == pt2.label) {
        assert(pt1.children.size() == pt2.children.size());
        for (size_t i = 0; i < pt1.children.size(); i++) {
            bool finished;
            bool success;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            std::tie(finished, success) = unify2_slow_step(pt1.children[i], pt2.children[i], is_var, subst);
#pragma GCC diagnostic pop
            if (!finished || !success) {
                return std::make_pair(finished, success);
            }
        }
    } else {
        if (is_var(pt1.label) && !contains_var(pt2, pt1.label)) {
            assert(pt1.children.empty());
            subst.insert(std::make_pair(pt1.label, pt2));
            return std::make_pair(false, true);
        } else if (is_var(pt2.label) && !contains_var(pt1, pt2.label)) {
            assert(pt2.children.empty());
            subst.insert(std::make_pair(pt2.label, pt1));
            return std::make_pair(false, true);
        } else {
            return std::make_pair(true, false);
        }
    }
    return std::make_pair(true, true);
}

template< typename SymType, typename LabType >
[[deprecated]]
bool unify2_slow(const ParsingTree< SymType, LabType > &pt1, const ParsingTree< SymType, LabType > &pt2,
                          const std::function< bool(LabType) > &is_var, SubstMap< SymType, LabType > &subst) {
    // Algorithm described in Chang, Lee (Symbolic logic and mechanical theorem proving), section 5.4 Unification algorithm
    // It seems to be rather inefficient, but it is also simple, so it easier to trust; it can be used
    // to check that other implementations are correct
    ParsingTree< SymType, LabType > pt1s = substitute(pt1, is_var, subst);
    ParsingTree< SymType, LabType > pt2s = substitute(pt2, is_var, subst);
    while (true) {
        bool finished;
        bool success;
        SubstMap< SymType, LabType > new_subst;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        std::tie(finished, success) = unify2_slow_step(pt1s, pt2s, is_var, new_subst);
#pragma GCC diagnostic pop
        if (finished) {
            return success;
        }
        pt1s = substitute(pt1s, is_var, new_subst);
        pt2s = substitute(pt2s, is_var, new_subst);
        subst = compose(subst, new_subst, is_var);
    }
}

// Quick bilateral unification

template< typename SymType, typename LabType >
[[deprecated]]
bool unify2_quick_process_tree(const ParsingTreeIterator< SymType, LabType > &pt1, const ParsingTreeIterator< SymType, LabType > &pt2,
                               const std::function< bool(LabType) > &is_var, SubstMap2< SymType, LabType > &subst,
                               NaiveIncrementalCycleDetector< LabType > &cycle_detector,
                               DisjointSet< LabType > &djs) {
    const auto &n1 = pt1.get_node();
    const auto &n2 = pt2.get_node();
    if (n1.label == n2.label) {
        auto it1 = pt1.begin();
        auto it2 = pt2.begin();
        while (it1 != pt1.end() && it2 != pt2.end()) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            bool res = unify2_quick_process_tree(*it1, *it2, is_var, subst, cycle_detector, djs);
#pragma GCC diagnostic pop
            if (!res) {
                return false;
            }
            ++it1;
            ++it2;
        }
        assert(it1 == pt1.end() && it2 == pt2.end());
        return true;
    } else {
        LabType var;
        ParsingTree2< SymType, LabType > pt_temp;
        bool v1 = is_var(n1.label);
        bool v2 = is_var(n2.label);
        assert(!(v1 && pt1.has_children()));
        assert(!(v2 && pt2.has_children()));
        if (v1 && v2) {
            // If both are variables, then we use the disjoint set structure to determine how to substitute
            djs.make_set(n1.label);
            djs.make_set(n2.label);
            LabType l1 = djs.find_set(n1.label);
            LabType l2 = djs.find_set(n2.label);
            if (l1 != l2) {
                bool res;
                LabType l3;
                std::tie(res, l3) = djs.union_set(l1, l2);
                assert(res);
                assert(l3 == l1 || l3 == l2);
                if (l3 == l1) {
                    pt_temp = var_parsing_tree(l1, n1.type);
                    var = l2;
                } else {
                    pt_temp = var_parsing_tree(l2, n2.type);
                    var = l1;
                }
            } else {
                return true;
            }
        } else if (v1) {
            var = n1.label;
            pt_temp = pt2.get_view();
        } else if (v2) {
            var = n2.label;
            pt_temp = pt1.get_view();
        } else {
            return false;
        }
        bool res;
        typename SubstMap2< SymType, LabType >::iterator it;
        // In general we cannot assume that the caller will retain pt1 and pt2 for a long time
        pt_temp.refresh();
        std::tie(it, res) = subst.insert(std::make_pair(var, pt_temp));
        cycle_detector.make_node(var);
        if (res) {
            std::set< LabType > vars;
            collect_variables2(pt_temp, is_var, vars);
            for (const auto &var2 : vars) {
                cycle_detector.make_edge(var, var2);
            }
            return true;
        } else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            return unify2_quick_process_tree(it->second.get_root(), pt_temp.get_root(), is_var, subst, cycle_detector, djs);
#pragma GCC diagnostic pop
        }
    }
}

template< typename SymType, typename LabType >
class BilateralUnificator {
public:
    BilateralUnificator(const std::function< bool(LabType) > &is_var) : failed(false), is_var(&is_var) {
#ifdef UNIFICATOR_SELF_TEST
        this->pt1.label = {};
        this->pt2.label = {};
        this->pt1.type = {};
        this->pt2.type = {};
#endif
    }

    void add_parsing_trees(const ParsingTree< SymType, LabType > &pt1, const ParsingTree< SymType, LabType > &pt2) {
        this->add_parsing_trees2(pt_to_pt2(pt1), pt_to_pt2(pt2));
    }

    /*
     * Return an approximation from above of failer: is has_failed() returns true, then all unifications from now on will return false.
     * However, it is possible that has_failed() returns false, but unification fails anyway.
     */
    bool has_failed() {
        return this->failed;
    }

    void add_parsing_trees2(const ParsingTree2< SymType, LabType > &pt1, const ParsingTree2< SymType, LabType > &pt2) {
#ifdef UNIFICATOR_SELF_TEST
        this->pt1.children.push_back(pt2_to_pt(pt1));
        this->pt2.children.push_back(pt2_to_pt(pt2));
#endif
        if (this->failed) {
            return;
        }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        bool res = unify2_quick_process_tree(pt1.get_root(), pt2.get_root(), *this->is_var, this->subst, this->cycle_detector, this->djs);
#pragma GCC diagnostic pop
        if (!res) {
            this->fail();
        }
    }

    bool is_unifiable() {
        if (this->failed) {
            return false;
        }
        if (!this->cycle_detector.is_acyclic()) {
            this->fail();
            return false;
        }
        return true;
    }

    std::pair< bool, SubstMap< SymType, LabType > > unify() {
        auto ret = this->unify2();
        return std::make_pair(ret.first, subst2_to_subst(ret.second));
    }

    std::pair< bool, SubstMap2< SymType, LabType > > unify2() {
#ifdef UNIFICATOR_SELF_TEST
        SubstMap< SymType, LabType > subst2;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        bool res2 = unify2_slow(this->pt1, this->pt2, *this->is_var, subst2);
#pragma GCC diagnostic pop
#endif
        if (this->failed) {
#ifdef UNIFICATOR_SELF_TEST
            assert(!res2);
#endif
            return std::make_pair(false, this->subst);
        }
        //std::cerr << "Final graph has " << this->cycle_detector.get_node_num() << " nodes and " << this->cycle_detector.get_edge_num() << " edges (load factor: " << ((double) this->cycle_detector.get_edge_num()) / ((double) this->cycle_detector.get_node_num()) << ")" << std::endl;
        bool res;
        std::vector< LabType > topo_sort;
        tie(res, topo_sort) = this->cycle_detector.find_topo_sort();
        if (!res) {
#ifdef UNIFICATOR_SELF_TEST
            assert(!res2);
#endif
            this->fail();
            return std::make_pair(false, this->subst);
        }
        SubstMap2< SymType, LabType > actual_subst;
        for (const LabType &lab : topo_sort) {
            actual_subst[lab] = substitute2(this->subst[lab], *this->is_var, actual_subst);
        }
#ifdef UNIFICATOR_SELF_TEST
        assert(res2);
        auto s1 = substitute(this->pt1, *this->is_var, subst2_to_subst(actual_subst));
        auto s2 = substitute(this->pt2, *this->is_var, subst2_to_subst(actual_subst));
        auto s3 = substitute(this->pt1, *this->is_var, subst2);
        auto s4 = substitute(this->pt2, *this->is_var, subst2);
        assert(s1 == s2);
        assert(s3 == s4);
#endif
        return std::make_pair(true, actual_subst);
    }

private:
    void fail() {
        // Once we have failed, there is not turning back: we can directly release resources
        this->failed = true;
        this->subst.clear();
        this->djs.clear();
        this->cycle_detector.clear();
    }

    bool failed;
    const std::function< bool(LabType) > *is_var;
    SubstMap2< SymType, LabType > subst;
    DisjointSet< LabType > djs;
    NaiveIncrementalCycleDetector< LabType > cycle_detector;

#ifdef UNIFICATOR_SELF_TEST
    ParsingTree< SymType, LabType > pt1;
    ParsingTree< SymType, LabType > pt2;
#endif
};

template< typename SymType, typename LabType >
[[deprecated]]
std::pair< bool, SubstMap< SymType, LabType > > unify2_quick(const ParsingTree< SymType, LabType > &pt1, const ParsingTree< SymType, LabType > &pt2,
                  const std::function< bool(LabType) > &is_var) {
    BilateralUnificator unif(is_var);
    unif.add_parsing_trees(pt1, pt2);
    return unif.unify();
}

template< typename SymType, typename LabType >
[[deprecated]]
bool unify2_quick_adapter(const ParsingTree< SymType, LabType > &pt1, const ParsingTree< SymType, LabType > &pt2,
                  const std::function< bool(LabType) > &is_var, SubstMap< SymType, LabType > &subst) {
    ParsingTree< SymType, LabType > pt1s = substitute(pt1, is_var, subst);
    ParsingTree< SymType, LabType > pt2s = substitute(pt2, is_var, subst);
    bool ret;
    SubstMap< SymType, LabType > new_subst;
    std::tie(ret, new_subst) = unify2_quick(pt1s, pt2s, is_var);
    if (ret) {
        subst = compose(subst, new_subst, is_var);
    }
    return ret;
}

template< typename SymType, typename LabType >
[[deprecated]]
bool unify2(const ParsingTree< SymType, LabType > &pt1, const ParsingTree< SymType, LabType > &pt2,
            const std::function< bool(LabType) > &is_var, SubstMap< SymType, LabType > &subst) {
#ifdef UNIFICATOR_SELF_TEST
    SubstMap< SymType, LabType > subst2 = subst;
#endif
    bool ret = unify2_quick_adapter(pt1, pt2, is_var, subst);
#ifdef UNIFICATOR_SELF_TEST
    bool ret2 = unify2_slow(pt1, pt2, is_var, subst2);
    assert(ret == ret2);
    if (ret) {
        auto s1 = substitute(pt1, is_var, subst);
        auto s2 = substitute(pt2, is_var, subst);
        auto s3 = substitute(pt1, is_var, subst2);
        auto s4 = substitute(pt2, is_var, subst2);
        assert(s1 == s2);
        assert(s3 == s4);
    }
#endif
    return ret;
}

#endif // UNIF_H
