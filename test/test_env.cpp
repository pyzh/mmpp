
#include "test/test_env.h"

#include "reader.h"
#include "platform.h"
#include "utils/utils.h"

using namespace std;

TestEnvironmentInner::TestEnvironmentInner(const string &filename, const string &cache_filename)
{
    cout << "Reading database from file " << filename << " using cache in file " << cache_filename << endl;
    FileTokenizer ft(filename);
    Reader p(ft, false, true);
    p.run();
    this->lib = new LibraryImpl(p.get_library());
    shared_ptr< ToolboxCache > cache = make_shared< FileToolboxCache >(cache_filename);
    this->tb = new LibraryToolbox(*this->lib, "|-", true, cache);
    cout << this->lib->get_symbols_num() << " symbols and " << this->lib->get_labels_num() << " labels" << endl;
    cout << "Memory usage after loading the library: " << size_to_string(platform_get_current_rss()) << endl << endl;
}

TestEnvironmentInner::~TestEnvironmentInner() {
    delete this->lib;
    delete this->tb;
}

TestEnvironment::TestEnvironment(const string &filename, const string &cache_filename) :
    inner(filename, cache_filename), lib(*inner.lib), tb(*inner.tb)
{
}

const TestEnvironment &get_set_mm() {
    static TestEnvironment data("../set.mm/set.mm", "../set.mm/set.mm.cache");
    return data;
}