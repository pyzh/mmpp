
#include <iostream>
#include <random>

#include "test/test_env.h"
#include "toolbox.h"
#include "parsing/unif.h"
#include "utils/utils.h"

using namespace std;

void print_trace(const ProofTree &pt, const Library &lib, const Assertion &ass) {
    size_t essentials_num = 0;
    for (const auto &child : pt.children) {
        if (child.essential) {
            print_trace(child, lib, ass);
            essentials_num++;
        }
    }
    auto it = find(ass.get_ess_hyps().begin(), ass.get_ess_hyps().end(), pt.label);
    if (it == ass.get_ess_hyps().end()) {
        cout << lib.resolve_label(pt.label) << " " << essentials_num << " ";
    } else {
        cout << "_hyp" << (it - ass.get_ess_hyps().begin()) << " 0 ";
    }
}

int dissector_main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    auto &data = get_set_mm();
    auto &tb = data.tb;

    const Assertion &ass = tb.get_assertion(tb.get_label("fta"));
    UncompressedProof unc_proof = ass.get_proof_executor(tb, false)->uncompress();
    auto pe = unc_proof.get_executor(tb, ass, true);
    pe->execute();
    const ProofTree &pt = pe->get_proof_tree();
    print_trace(pt, tb, ass);
    cout << endl;

    return 0;
}
static_block {
    register_main_function("mmpp_dissector", dissector_main);
}

struct ProofStat {
    size_t proof_size;
    size_t ess_proof_size;
    size_t ess_hyp_num;
    size_t ess_hyp_steps;
};

ostream &operator<<(ostream &os, const ProofStat &stat) {
    return os << stat.proof_size << " " << stat.ess_proof_size << " " << stat.ess_hyp_num << " " << stat.ess_hyp_steps;
}

void proof_stat_unwind_tree(const ProofTree &pt, const Assertion &ass, ProofStat &stat) {
    if (pt.essential) {
        stat.ess_proof_size++;
        if (find(ass.get_ess_hyps().begin(), ass.get_ess_hyps().end(), pt.label) != ass.get_ess_hyps().end()) {
            stat.ess_hyp_steps++;
        }
    }
    for (const auto &child : pt.children) {
        proof_stat_unwind_tree(child, ass, stat);
    }
}

int proofs_stats_main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    auto &data = get_set_mm();
    auto &lib = data.lib;
    //auto &tb = data.tb;

    TextProgressBar tpb(100, lib.get_assertions().size());
    vector< pair< LabTok, ProofStat > > proofs_stats;
    for (const Assertion &ass : lib.get_assertions()) {
        if (!ass.is_valid()) {
            continue;
        }
        if (!ass.is_theorem()) {
            continue;
        }
        if (!ass.has_proof()) {
            continue;
        }
        auto uncomp_proof = ass.get_proof_executor(lib)->uncompress();
        auto uncomp_exec = uncomp_proof.get_executor(lib, ass, true);
        uncomp_exec->execute();

        ProofStat stat;
        bzero(&stat, sizeof(stat));
        stat.proof_size = uncomp_proof.get_labels().size();
        stat.ess_hyp_num = ass.get_ess_hyps().size();
        proof_stat_unwind_tree(uncomp_exec->get_proof_tree(), ass, stat);
        proofs_stats.push_back(make_pair(ass.get_thesis(), stat));
        tpb.report(ass.get_thesis());
    }
    tpb.finished();

    sort(proofs_stats.begin(), proofs_stats.end(), [](const auto &x, const auto &y) {
        return (x.second.ess_proof_size - x.second.ess_hyp_steps) < (y.second.ess_proof_size - y.second.ess_hyp_steps);
    });

    for (size_t i = 0; i < 20; i++) {
        cout << lib.resolve_label(proofs_stats[i].first) << ": " << proofs_stats[i].second << endl;
    }

    return 0;
}
static_block {
    register_main_function("proofs_stats", proofs_stats_main);
}

void gen_theorems(const BilateralUnificator< SymTok, LabTok > &unif,
                  const vector< ParsingTree< SymTok, LabTok > > &open_hyps,
                  const vector< LabTok > &steps,
                  size_t hyps_pos,
                  const vector< const Assertion* > &useful_asses,
                  const ParsingTree< SymTok, LabTok > &final_thesis,
                  LibraryToolbox &tb,
                  size_t depth,
                  const function< void(const ParsingTree< SymTok, LabTok >&, const vector< ParsingTree< SymTok, LabTok > >&, const vector< LabTok >&, LibraryToolbox&)> &callback) {
    if (depth == 0 || hyps_pos == open_hyps.size()) {
        auto unif2 = unif;
        SubstMap< SymTok, LabTok > subst;
        bool res;
        tie(res, subst) = unif2.unify();
        if (!res) {
            return;
        }
        auto thesis = substitute(final_thesis, tb.get_standard_is_var(), subst);
        vector< ParsingTree< SymTok, LabTok > > hyps;
        for (const auto &hyp : open_hyps) {
            hyps.push_back(substitute(hyp, tb.get_standard_is_var(), subst));
        }
        callback(thesis, hyps, steps, tb);
    } else {
        gen_theorems(unif, open_hyps, steps, hyps_pos+1, useful_asses, final_thesis, tb, depth, callback);
        for (const auto assp : useful_asses) {
            tb.new_temp_var_frame();
            const Assertion &ass = *assp;
            ParsingTree< SymTok, LabTok > thesis;
            vector< ParsingTree< SymTok, LabTok > > hyps;
            tie(hyps, thesis) = tb.refresh_assertion(ass);
            auto unif2 = unif;
            auto steps2 = steps;
            unif2.add_parsing_trees(open_hyps[hyps_pos], thesis);
            steps2.push_back(ass.get_thesis());
            if (unif2.is_unifiable()) {
                //cout << "Attaching " << tb.resolve_label(ass.get_thesis()) << " in position " << hyps_pos << endl;
                auto open_hyps2 = open_hyps;
                open_hyps2.erase(open_hyps2.begin() + hyps_pos);
                open_hyps2.insert(open_hyps2.end(), hyps.begin(), hyps.end());
                gen_theorems(unif2, open_hyps2, steps2, hyps_pos, useful_asses, final_thesis, tb, depth-1, callback);
            }
            tb.release_temp_var_frame();
        }
    }
}

size_t count = 0;
void print_theorem(const ParsingTree< SymTok, LabTok > &thesis, const vector< ParsingTree< SymTok, LabTok > >&hyps, const vector< LabTok > &steps, LibraryToolbox &tb) {
    ::count++;
    if (::count % 10000 == 0) {
        cout << ::count << endl;
        cout << tb.print_sentence(thesis) << endl;
        cout << "with the hypotheses:" << endl;
        for (const auto &hyp : hyps) {
            cout << " * " << tb.print_sentence(hyp) << endl;
        }
        cout << "Proved with steps: " << tb.print_proof(steps) << endl;
    }
    if (::count == 100000) {
        exit(0);
    }
}

int gen_random_theorems_main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    random_device rand_dev;
    mt19937 rand_mt;
    rand_mt.seed(rand_dev());

    auto &data = get_set_mm();
    auto &lib = data.lib;
    auto &tb = data.tb;

    vector< const Assertion* > useful_asses;
    for (const auto &ass : lib.get_assertions()) {
        if (ass.is_valid() && lib.get_sentence(ass.get_thesis()).at(0) == tb.get_turnstile()) {
            if (ass.is_theorem() && ass.has_proof() && ass.get_proof_executor(lib)->is_trivial()) {
                //cout << "Proof for " << lib.resolve_label(ass.get_thesis()) << " is trivial" << endl;
            } else {
                useful_asses.push_back(&ass);
            }
        }
    }
    cout << "There are " << useful_asses.size() << " useful theorems" << endl;

    BilateralUnificator< SymTok, LabTok > unif(tb.get_standard_is_var());
    vector< ParsingTree< SymTok, LabTok > > open_hyps;
    LabTok th_label;
    tie(th_label, ignore) = tb.new_temp_var(tb.get_turnstile_alias());
    ParsingTree< SymTok, LabTok > final_thesis;
    final_thesis.label = th_label;
    final_thesis.type = tb.get_turnstile_alias();
    final_thesis = tb.get_parsed_sents()[tb.get_label("absmod0")];
    open_hyps.push_back(final_thesis);
    vector< LabTok > steps;

    gen_theorems(unif, open_hyps, steps, 0, useful_asses, final_thesis, tb, 2, print_theorem);
    return 0;

    for (size_t i = 0; i < 5; i++) {
        if (open_hyps.empty()) {
            cout << "Terminating early" << endl;
            break;
        }
        while (true) {
            // Select a random hypothesis, a random open hypothesis and let them match
            size_t ass_idx = uniform_int_distribution< size_t >(0, useful_asses.size()-1)(rand_mt);
            size_t hyp_idx = uniform_int_distribution< size_t >(0, open_hyps.size()-1)(rand_mt);
            const Assertion &ass = *useful_asses[ass_idx];
            if (i == 0 && ass.get_ess_hyps().size() == 0) {
                continue;
            }
            if (i > 1 && ass.get_ess_hyps().size() > 0) {
                continue;
            }
            ParsingTree< SymTok, LabTok > thesis;
            vector< ParsingTree< SymTok, LabTok > > hyps;
            tie(hyps, thesis) = tb.refresh_assertion(ass);
            auto unif2 = unif;
            unif2.add_parsing_trees(open_hyps[hyp_idx], thesis);
            if (unif2.unify().first) {
                cout << "Attaching " << tb.resolve_label(ass.get_thesis()) << " in position " << hyp_idx << endl;
                unif = unif2;
                open_hyps.erase(open_hyps.begin() + hyp_idx);
                open_hyps.insert(open_hyps.end(), hyps.begin(), hyps.end());
                break;
            }
        }
    }

    SubstMap< SymTok, LabTok > subst;
    bool res;
    tie(res, subst) = unif.unify();

    if (res) {
        cout << "Unification succedeed and proved:" << endl;
        cout << tb.print_sentence(substitute(final_thesis, tb.get_standard_is_var(), subst)) << endl;
        cout << "with the hypotheses:" << endl;
        for (const auto &hyp : open_hyps) {
            cout << " * " << tb.print_sentence(substitute(hyp, tb.get_standard_is_var(), subst)) << endl;
        }
    } else {
        cout << "Unification failed" << endl;
    }

    return 0;
}
static_block {
    register_main_function("mmpp_gen_random_theorems", gen_random_theorems_main);
}
