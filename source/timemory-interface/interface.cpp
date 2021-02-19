
#include <timemory-interface/interface.h>

#include <timemory/library.h>
#include <timemory/timemory.h>
#include <timemory/tools/timemory-mallocp.h>
#include <timemory/tools/timemory-mpip.h>


TIMEMORY_DECLARE_COMPONENT(papi_network_vector)
TIMEMORY_SET_COMPONENT_API(component::papi_network_vector, tpls::papi, category::external,
                           category::hardware_counter, os::supports_linux)
TIMEMORY_STATISTICS_TYPE(component::papi_network_vector, std::vector<double>)
// TIMEMORY_DEFINE_CONCRETE_TRAIT(custom_serialization, component::papi_network_vector, true_type)

#if !defined(TIMEMORY_USE_PAPI)
TIMEMORY_DEFINE_CONCRETE_TRAIT(is_available, component::papi_network_vector, false_type)
#endif

namespace tim
{
namespace component
{
struct papi_network_vector
: public base<papi_network_vector, std::vector<long long>>
, private papi_common
{
    using size_type  = size_t;
    using event_list = std::vector<int>;
    using value_type = std::vector<long long>;
    using entry_type = long long;
    using this_type  = papi_network_vector;

    static void configure();
    static void global_init();
    static void global_finalize();

    papi_network_vector();

    ~papi_network_vector()                              = default;
    papi_network_vector(const papi_network_vector&)     = default;
    papi_network_vector(papi_network_vector&&) noexcept = default;
    papi_network_vector& operator=(const papi_network_vector&) = default;
    papi_network_vector& operator=(papi_network_vector&&) noexcept = default;

    static size_t     size();
    static value_type record();
    template <typename Tp = double>
    std::vector<Tp> get() const;

    void start();
    void stop();

public:
    static std::string label();
    static std::string description();

    entry_type get(int evt_type) const;

    template <typename Archive>
    static void extra_serialization(Archive& ar, const unsigned int);
    static std::vector<std::string> label_array();
    static std::vector<std::string> description_array();
    static std::vector<std::string> display_unit_array();
    static std::vector<int64_t> unit_array();
    std::string get_display() const;
    // friend std::ostream& operator<<(std::ostream& os, const this_type& obj);
};
}
}

using bundle_t     = tim::auto_tuple<tim::component::papi_network_vector>;
using custom_map_t = std::unordered_map<const char*, bundle_t>;

static std::map<std::string, int> network_event_codes_map{};
static bool                       papi_network_configured = false;
static int                        network_event_set       = PAPI_NULL;
static std::vector<std::string>   network_events{};
static std::vector<int>           network_event_codes{};
static custom_map_t               network_bundle{};

static void
create_network_event_set()
{
    PAPI_create_eventset(&network_event_set);

    assert(network_events.size() == network_event_codes.size());

    std::set<int> _valid{};
    for(size_t i = 0; i < network_event_codes.size(); ++i)
    {
        int retval = PAPI_add_event(network_event_set, network_event_codes.at(i));
        if(tim::papi::check(retval, std::string{ "Warning!! Failure to add event " } +
                                        network_events.at(i) + " to event set"))
            _valid.insert(network_event_codes.at(i));
    }

    std::vector<std::string> _network_events{};
    std::vector<int>         _network_event_codes{};
    for(size_t i = 0; i < network_event_codes.size(); ++i)
    {
        if(_valid.find(network_event_codes.at(i)) == _valid.end())
        {
            _network_events.emplace_back(network_events.at(i));
            _network_event_codes.emplace_back(network_event_codes.at(i));
        }
    }

    network_events      = _network_events;
    network_event_codes = _network_event_codes;
    assert(network_events.size() == network_event_codes.size());

    if(network_events.size() == 0)
        tim::trait::runtime_enabled<tim::component::papi_network_vector>::set(false);

    papi_network_configured = true;
}

extern "C"
{
    void set_papi_events(int nevents, const char** events)
    {
        PAPI_library_init(PAPI_VER_CURRENT);
        for(int i = 0; i < nevents; ++i)
        {
            int _evt_code = PAPI_NULL;
            auto ret = PAPI_event_name_to_code(events[i], &_evt_code);
            // tim::papi::check(ret, std::string{ "Warning!! Failure converting " } +
            // std::string{ events[i] } + " to enum value");
            //    continue;
            // std::cout << events[i] << " maps to " << network_event_codes[i] <<
            // std::endl;
            if(ret == PAPI_OK && _evt_code != PAPI_NULL)
                network_event_codes_map.emplace(events[i], _evt_code);
        }

        for(auto& itr : network_event_codes_map)
        {
            network_events.emplace_back(itr.first);
            network_event_codes.emplace_back(itr.second);
        }
    }

    void initialize(int* argc, char*** argv)
    {
        tim::settings::precision()         = 6;
        tim::settings::cout_output()       = false;
        tim::settings::flamegraph_output() = false;
        tim::settings::global_components() = "wall_clock, cpu_clock, peak_rss";
        tim::settings::papi_events() =
            "PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_RES_STL, PAPI_STL_CCY, PAPI_LD_INS, "
            "PAPI_SR_INS, PAPI_LST_INS";

        if(!tim::settings::papi_events().empty())
            tim::settings::global_components() += ", papi_array";

        timemory_init_library(*argc, *argv);
        tim::timemory_argparse(argc, argv);
        timemory_set_default(tim::settings::global_components().c_str());
        timemory_register_mpip();
    }

    void finalize()
    {
        timemory_deregister_mpip();
        timemory_finalize_library();
    }

    void push_region(const char* _region_name)
    {
        network_bundle.emplace(_region_name, bundle_t{ _region_name });
        timemory_push_region(_region_name);
    }

    void pop_region(const char* _region_name)
    {
        timemory_pop_region(_region_name);
        network_bundle.erase(_region_name);
    }
}


struct foo
{
    int32_t b;
    float c;
};

// implementation of component

namespace tim
{
namespace component
{
void
papi_network_vector::configure()
{
    if(!papi_network_configured)
        create_network_event_set();
}

void
papi_network_vector::global_init()
{
    configure();
}

void
papi_network_vector::global_finalize()
{
    if(network_events.empty())
        return;

    std::vector<long long> values(network_events.size(), 0);
    papi::stop(network_event_set, values.data());
    papi::remove_events(network_event_set, network_event_codes.data(), network_event_codes.size());
    papi::destroy_event_set(network_event_set);
    papi_common::finalize_papi();
}

papi_network_vector::papi_network_vector()
{
    value.resize(size(), 0);
    accum.resize(size(), 0);
}

size_t
papi_network_vector::size()
{
    return network_event_codes.size();
}

papi_network_vector::value_type
papi_network_vector::record()
{
    configure(); // always ensure it is configured
    value_type read_value(size(), 0);
    papi::read(network_event_set, read_value.data());
    return read_value;
}

template <typename Tp>
std::vector<Tp>
papi_network_vector::get() const
{
    std::vector<Tp> values{};
    auto&&          _data = base_type::load();
    values.reserve(_data.size());
    for(auto& itr : _data)
        values.emplace_back(itr);
    return values;
}

void
papi_network_vector::start()
{
    value = record();
}

void
papi_network_vector::stop()
{
    using namespace tim::component::operators;
    value = (record() - value);
    accum += value;
}

    std::string
    papi_network_vector::label()
    {
        return "papi_network_vector";
}

std::string
papi_network_vector::description()
{
    return "Dynamically allocated array of PAPI HW counters";
}

papi_network_vector::entry_type
papi_network_vector::get(int evt_type) const
{
    return accum.at(evt_type);
}

template <typename Archive>
void
papi_network_vector::extra_serialization(Archive& ar, const unsigned int)
{
    ar(cereal::make_nvp("event_names", network_events),
        cereal::make_nvp("event_set", network_event_set),
        cereal::make_nvp("event_codes", network_event_codes));
}

//----------------------------------------------------------------------------------//
// array of descriptions
//
std::vector<std::string>
papi_network_vector::label_array()
{
    return network_events;
}

//----------------------------------------------------------------------------------//
// array of labels
//
std::vector<std::string>
papi_network_vector::description_array()
{
    std::vector<std::string> arr(network_event_codes.size(), "");
    for(size_type i = 0; i < network_event_codes.size(); ++i)
        arr[i] = papi::get_event_info(network_event_codes[i]).long_descr;
    return arr;
}

//----------------------------------------------------------------------------------//
// array of unit
//
std::vector<std::string>
papi_network_vector::display_unit_array()
{
    std::vector<std::string> arr(network_event_codes.size(), "");
    for(size_type i = 0; i < network_event_codes.size(); ++i)
        arr[i] = papi::get_event_info(network_event_codes[i]).units;
    return arr;
}

//----------------------------------------------------------------------------------//
// array of unit values
//
std::vector<int64_t>
papi_network_vector::unit_array()
{
    std::vector<int64_t> arr(network_event_codes.size(), 0);
    for(size_type i = 0; i < network_event_codes.size(); ++i)
        arr[i] = 1;
    return arr;
}

//----------------------------------------------------------------------------------//

std::string papi_network_vector::get_display() const
{
    if(network_event_codes.empty())
        return "";
    auto val          = (is_transient) ? accum : value;
    auto _get_display = [&](std::ostream& os, size_type idx) {
        auto     _obj_value = val.at(idx);
        auto     _evt_type  = network_event_codes.at(idx);
        std::string _label     = papi::get_event_info(_evt_type).short_descr;
        std::string _disp      = papi::get_event_info(_evt_type).units;
        auto     _prec      = base_type::get_precision();
        auto     _width     = base_type::get_width();
        auto     _flags     = base_type::get_format_flags();

        std::stringstream ss;
        std::stringstream ssv;
        std::stringstream ssi;
        ssv.setf(_flags);
        ssv << std::setw(_width) << std::setprecision(_prec) << _obj_value;
        if(!_disp.empty())
            ssv << " " << _disp;
        if(!_label.empty())
            ssi << " " << _label;
        ss << ssv.str() << ssi.str();
        os << ss.str();
    };

    std::stringstream ss;
    for(size_type i = 0; i < network_event_codes.size(); ++i)
    {
        _get_display(ss, i);
        if(i + 1 < network_event_codes.size())
            ss << ", ";
    }
    return ss.str();
}
}
}

TIMEMORY_INITIALIZE_STORAGE(papi_network_vector)
