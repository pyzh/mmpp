#include "workset.h"

#include "libs/json.h"

#include "reader.h"
#include "platform.h"
#include "jsonize.h"

using namespace std;
using namespace nlohmann;

Workset::Workset()
{
    this->root_step = this->step_backrefs.make_instance();
}

template< typename TokType >
std::vector< std::string > map_to_vect(const std::unordered_map< TokType, std::string > &m) {
    std::vector< std::string > ret;
    ret.resize(m.size()+1);
    for (auto &i : m) {
        ret[i.first] = i.second;
    }
    return ret;
}

json Workset::answer_api1(HTTPCallback &cb, std::vector< std::string >::const_iterator path_begin, std::vector< std::string >::const_iterator path_end, std::string method)
{
    (void) cb;
    (void) method;
    unique_lock< mutex > lock(this->global_mutex);
    if (path_begin == path_end) {
        json ret = json::object();
        return ret;
    }
    if (*path_begin == "load") {
        this->load_library(platform_get_resources_base() / "library.mm");
        json ret = { { "status", "ok" } };
        return ret;
    } else if (*path_begin == "get_context") {
        path_begin++;
        assert_or_throw< SendError >(path_begin == path_end, 404);
        json ret;
        ret["name"] = this->get_name();
        ret["root_step_id"] = this->root_step->get_id();
        if (this->library == NULL) {
            ret["status"] = "unloaded";
            return ret;
        }
        ret["status"] = "loaded";
        ret["symbols"] = map_to_vect(this->library->get_symbols());
        ret["labels"] = map_to_vect(this->library->get_labels());
        const auto &addendum = this->library->get_addendum();
        ret["addendum"] = jsonize(addendum);
        ret["max_number"] = this->library->get_max_number();
        return ret;
    } else if (*path_begin == "get_sentence") {
        path_begin++;
        assert_or_throw< SendError >(path_begin != path_end, 404);
        int tok = safe_stoi(*path_begin);
        try {
            const Sentence &sent = this->library->get_sentence(tok);
            json ret;
            ret["sentence"] = sent;
            return ret;
        } catch (out_of_range e) {
            (void) e;
            throw SendError(404);
        }
    } else if (*path_begin == "get_assertion") {
        path_begin++;
        assert_or_throw< SendError >(path_begin != path_end, 404);
        int tok = safe_stoi(*path_begin);
        try {
            const Assertion &ass = this->library->get_assertion(tok);
            assert_or_throw< SendError >(ass.is_valid(), 404);
            json ret;
            ret["assertion"] = jsonize(ass);
            return ret;
        } catch (out_of_range e) {
            (void) e;
            throw SendError(404);
        }
    } else if (*path_begin == "get_proof_tree") {
        path_begin++;
        assert_or_throw< SendError >(path_begin != path_end, 404);
        int tok = safe_stoi(*path_begin);
        try {
            const Assertion &ass = this->library->get_assertion(tok);
            assert_or_throw< SendError >(ass.is_valid(), 404);
            const auto &executor = ass.get_proof_executor(*this->library, true);
            executor->execute();
            const auto &proof_tree = executor->get_proof_tree();
            json ret;
            ret["proof_tree"] = jsonize(proof_tree);
            return ret;
        } catch (out_of_range e) {
            (void) e;
            throw SendError(404);
        }
    } else if (*path_begin == "get_root_step") {
        path_begin++;
        assert_or_throw< SendError >(path_begin == path_end, 404);
        return jsonize(*this->root_step);
    }
    throw SendError(404);
}

void Workset::load_library(boost::filesystem::path filename)
{
    FileTokenizer ft(filename);
    Reader p(ft, false, true);
    p.run();
    this->library = make_unique< LibraryImpl >(p.get_library());
}

const string &Workset::get_name()
{
    return this->name;
}

void Workset::set_name(const string &name)
{
    this->name = name;
}
