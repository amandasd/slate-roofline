
#include <interface.h>

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

static bool papi_network_configured = false;
static int network_event_set = PAPI_NULL;
static std::vector<std::string> network_events{};
static std::vector<int> network_event_codes{};

static void create_network_event_set()
{
    PAPI_library_init(PAPI_VER_CURRENT);
    PAPI_create_eventset(&network_event_set);

    std::set<int> _valid{};
    for(size_t i = 0; i < network_event_codes.size(); ++i)
    {
        int  retval   = PAPI_add_event(network_event_set, network_event_codes.at(i));
        if(tim::papi::check(retval, std::string{ "Warning!! Failure to add event " } + network_events.at(i) + " to event set"))
            _valid.insert(network_event_codes.at(i));
    }

    std::vector<std::string> _network_events{};
    std::vector<int> _network_event_codes{};
    for(size_t i = 0; i < network_event_codes.size(); ++i)
    {
        if(_valid.find(network_event_codes.at(i)) == _valid.end())
        {
            _network_events.emplace_back(network_events.at(i));
            _network_event_codes.emplace_back(network_event_codes.at(i));
        }
    }

    network_events = _network_events;
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
        network_event_codes.resize(nevents, 0);
        for(int i = 0; i < nevents; ++i)
        {
            auto ret = PAPI_event_name_to_code(events[i], &(network_event_codes[i]));
            // tim::papi::check(ret, std::string{ "Warning!! Failure converting " } + std::string{ events[i] } + " to enum value");
            //    continue;
            // std::cout << events[i] << " maps to " << network_event_codes[i] << std::endl;
            if(ret == PAPI_OK)
                network_events.emplace_back(events[i]);
        }

    }

    void initialize(int* argc, char*** argv)
    {
        tim::settings::precision()         = 6;
        tim::settings::cout_output()       = false;
        tim::settings::flamegraph_output() = false;
        tim::settings::global_components() =
            "wall_clock, cpu_clock, peak_rss, papi_array";

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

    void push_region(const char* _region_name) { timemory_push_region(_region_name); }

    void pop_region(const char* _region_name) { timemory_pop_region(_region_name); }
}


// implementation of component

namespace tim
{
namespace component
{
struct papi_network_vector
: public base<papi_network_vector, std::vector<long long>>
, private papi_common
{
    template <typename Tp>
    using vector_t = std::vector<Tp>;

    using size_type         = size_t;
    using event_list        = vector_t<int>;
    using value_type        = vector_t<long long>;
    using entry_type        = long long;
    using this_type         = papi_network_vector;

    //----------------------------------------------------------------------------------//

    static void  configure()
    {
        if(!papi_network_configured)
            create_network_event_set();
    }

    static void global_init() { configure(); }
    static void global_finalize()
    {
        std::vector<long long> values(network_events.size(), 0);
        papi::stop(network_event_set, values.data());
        papi::remove_events(network_event_set, network_event_codes.data(), network_event_codes.size());
        papi::destroy_event_set(network_event_set);
        papi_common::finalize_papi();
    }

    //----------------------------------------------------------------------------------//

    papi_network_vector()
    {
        value.resize(size(), 0);
        accum.resize(size(), 0);
    }

    ~papi_network_vector()                      = default;
    papi_network_vector(const papi_network_vector&)     = default;
    papi_network_vector(papi_network_vector&&) noexcept = default;
    papi_network_vector& operator=(const papi_network_vector&) = default;
    papi_network_vector& operator=(papi_network_vector&&) noexcept = default;

    static size_t size() { return network_event_codes.size(); }
    static value_type record()
    {
        configure(); // always ensure it is configured
        value_type read_value(size(), 0);
        papi::read(network_event_set, read_value.data());
        return read_value;
    }

    template <typename Tp = double>
    vector_t<Tp> get() const
    {
        std::vector<Tp> values{};
        auto&&          _data = base_type::load();
        values.reserve(_data.size());
        for(auto& itr : _data)
            values.emplace_back(itr);
        return values;
    }

    void start()
    {
        value = record();
    }

    void stop()
    {
        using namespace tim::component::operators;
        value = (record() - value);
        accum += value;
    }

    //----------------------------------------------------------------------------------//
    /*
    this_type& operator+=(const this_type& rhs)
    {
        value += rhs.value;
        accum += rhs.accum;
        if(rhs.is_transient)
            is_transient = rhs.is_transient;
        return *this;
    }

    //----------------------------------------------------------------------------------//

    this_type& operator-=(const this_type& rhs)
    {
        value -= rhs.value;
        accum -= rhs.accum;
        if(rhs.is_transient)
            is_transient = rhs.is_transient;
        return *this;
    }*/

public:
    //==================================================================================//
    //
    //      data representation
    //
    //==================================================================================//

    static std::string label()
    {
        return "papi_network_vector";
    }

    static std::string description()
    {
        return "Dynamically allocated array of PAPI HW counters";
    }

    entry_type get(int evt_type) const
    {
        return accum.at(evt_type);
    }

    template <typename Archive>
    static void extra_serialization(Archive& ar, const unsigned int)
    {
        ar(cereal::make_nvp("event_names", network_events),
           cereal::make_nvp("event_set", network_event_set),
           cereal::make_nvp("event_codes", network_event_codes));
    }

    //----------------------------------------------------------------------------------//
    // array of descriptions
    //
    static vector_t<std::string> label_array()
    {
        return network_events;
    }

    //----------------------------------------------------------------------------------//
    // array of labels
    //
    static vector_t<std::string> description_array()
    {
        vector_t<std::string> arr(network_event_codes.size(), "");
        for(size_type i = 0; i < network_event_codes.size(); ++i)
            arr[i] = papi::get_event_info(network_event_codes[i]).long_descr;
        return arr;
    }

    //----------------------------------------------------------------------------------//
    // array of unit
    //
    static vector_t<std::string> display_unit_array()
    {
        vector_t<std::string> arr(network_event_codes.size(), "");
        for(size_type i = 0; i < network_event_codes.size(); ++i)
            arr[i] = papi::get_event_info(network_event_codes[i]).units;
        return arr;
    }

    //----------------------------------------------------------------------------------//
    // array of unit values
    //
    static vector_t<int64_t> unit_array()
    {
        vector_t<int64_t> arr(network_event_codes.size(), 0);
        for(size_type i = 0; i < network_event_codes.size(); ++i)
            arr[i] = 1;
        return arr;
    }

    //----------------------------------------------------------------------------------//

    std::string get_display() const
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

    //----------------------------------------------------------------------------------//

    friend std::ostream& operator<<(std::ostream& os, const this_type& obj)
    {
        if(network_event_codes.empty())
            return os;
        // output the metrics
        auto _value = obj.get_display();
        os << _value;
        return os;
    }
};
}
}

TIMEMORY_INITIALIZE_STORAGE(papi_network_vector)
