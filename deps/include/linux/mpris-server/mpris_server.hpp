#ifndef MPRIS_SERVER_HPP_INCLUDED
#define MPRIS_SERVER_HPP_INCLUDED

#include <cstdio>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <memory>

#ifndef MPRIS_SERVER_NO_IMPL
#include <sdbus-c++/sdbus-c++.h>
#else

namespace sdbus {

struct Variant {
    Variant() = default;
    template <typename T>
    Variant(const T &) { }
};

struct ObjectPath { };
struct IConnection { };
struct IObject { };

} // namespace sdbus

#endif

namespace mpris {

using namespace std::literals::string_literals;

using StringList = std::vector<std::string>;
using Metadata   = std::map<std::string, sdbus::Variant>;

static const auto PREFIX      = "org.mpris.MediaPlayer2."s;
static const auto OBJECT_PATH = "/org/mpris/MediaPlayer2"s;
static const auto MP2         = "org.mpris.MediaPlayer2"s;
static const auto MP2P        = "org.mpris.MediaPlayer2.Player"s;
static const auto PROPS       = "org.freedesktop.DBus.Properties"s;

enum class PlaybackStatus { Playing, Paused, Stopped };
enum class LoopStatus     { None, Track, Playlist };

enum class Field {
    TrackId     , Length     , ArtUrl      , Album          ,
    AlbumArtist , Artist     , AsText      , AudioBPM       ,
    AutoRating  , Comment    , Composer    , ContentCreated ,
    DiscNumber  , FirstUsed  , Genre       , LastUsed       ,
    Lyricist    , Title      , TrackNumber , Url            ,
    UseCount    , UserRating
};

static const char *playback_status_strings[] = { "Playing"           , "Paused"              , "Stopped" };
static const char *loop_status_strings[]     = { "None"              , "Track"               , "Playlist" };
static const char *metadata_strings[]        = { "mpris:trackid"     , "mpris:length"        , "mpris:artUrl"      , "xesam:album"          ,
                                                 "xesam:albumArtist" , "xesam:artist"        , "xesam:asText"      , "xesam:audioBPM"       ,
                                                 "xesam:autoRating"  , "xesam:comment"       , "xesam:composer"    , "xesam:contentCreated" ,
                                                 "xesam:discNumber"  , "xesam:firstUsed"     , "xesam:genre"       , "xesam:lastUsed"       ,
                                                 "xesam:lyricist"    , "xesam:title"         , "xesam:trackNumber" , "xesam:url"            ,
                                                 "xesam:useCount"    , "xesam:userRating" };

namespace detail {

template <typename T, typename R, typename... Args>
std::function<R(Args...)> member_fn(T *obj, R (T::*fn)(Args...))
{
    return [=](Args&&... args) -> R { return (obj->*fn)(args...); };
}

template <typename T, typename R, typename... Args>
std::function<R(Args...)> member_fn(T *obj, R (T::*fn)(Args...) const)
{
    return [=](Args&&... args) -> R { return (obj->*fn)(args...); };
}

inline std::string playback_status_to_string(PlaybackStatus status) { return playback_status_strings[static_cast<int>(status)]; }
inline std::string loop_status_to_string(        LoopStatus status) { return     loop_status_strings[static_cast<int>(status)]; }
inline std::string field_to_string(                    Field entry) { return        metadata_strings[static_cast<int>( entry)]; }

} // namespace detail

class Server {
    std::string service_name;
    std::unique_ptr<sdbus::IConnection> connection;
    std::unique_ptr<sdbus::IObject> object;

    std::function<void(void)>               quit_fn;
    std::function<void(void)>               raise_fn;
    std::function<void(void)>               next_fn;
    std::function<void(void)>               previous_fn;
    std::function<void(void)>               pause_fn;
    std::function<void(void)>               play_pause_fn;
    std::function<void(void)>               stop_fn;
    std::function<void(void)>               play_fn;
    std::function<void(int64_t)>            seek_fn;
    std::function<void(int64_t)>            set_position_fn;
    std::function<void(std::string_view)>   open_uri_fn;
    std::function<void(bool)>               fullscreen_changed_fn;
    std::function<void(LoopStatus)>         loop_status_changed_fn;
    std::function<void(double)>             rate_changed_fn;
    std::function<void(bool)>               shuffle_changed_fn;
    std::function<void(double)>             volume_changed_fn;

    bool fullscreen                  = false;
    std::string identity             = "";
    std::string desktop_entry        = "";
    StringList supported_uri_schemes = {};
    StringList supported_mime_types  = {};
    PlaybackStatus playback_status   = PlaybackStatus::Stopped;
    LoopStatus loop_status           = LoopStatus::None;
    double rate                      = 1.0;
    bool shuffle                     = false;
    Metadata metadata                = {};
    double volume                    = 0.0;
    int64_t position                 = 0;
    double maximum_rate              = 1.0;
    double minimum_rate              = 1.0;

    void prop_changed(const std::string &interface, const std::string &name, sdbus::Variant value);
    void control_props_changed(auto&&... args);

    bool can_control()     const { return bool(loop_status_changed_fn) && bool(shuffle_changed_fn)
                                       && bool(volume_changed_fn)      && bool(stop_fn);                }
    bool can_go_next()     const { return can_control() && bool(next_fn);                               }
    bool can_go_previous() const { return can_control() && bool(previous_fn);                           }
    bool can_play()        const { return can_control() && bool(play_fn)      && bool(play_pause_fn);   }
    bool can_pause()       const { return can_control() && bool(pause_fn)     && bool(play_pause_fn);   }
    bool can_seek()        const { return can_control() && bool(seek_fn)      && bool(set_position_fn); }

    void set_fullscreen_external(bool value);
    void set_loop_status_external(const std::string &value);
    void set_rate_external(double value);
    void set_shuffle_external(bool value);
    void set_volume_external(double value);
    void set_position_method(sdbus::ObjectPath id, int64_t pos);
    void open_uri(const std::string &uri);

public:
    static std::unique_ptr<Server> make(std::string_view name);

    explicit Server(std::string_view player_name);
    void start_loop();
    void start_loop_async();

    void on_quit                ( auto &&fn) { quit_fn                = fn; prop_changed(MP2, "CanQuit", sdbus::Variant(true));                                    }
    void on_raise               ( auto &&fn) { raise_fn               = fn; prop_changed(MP2, "CanQuit", sdbus::Variant(true));                                    }
    void on_next                ( auto &&fn) { next_fn                = fn; control_props_changed("CanGoNext");                                                    }
    void on_previous            ( auto &&fn) { previous_fn            = fn; control_props_changed("CanGoPrevious");                                                }
    void on_pause               ( auto &&fn) { pause_fn               = fn; control_props_changed("CanPause");                                                     }
    void on_play_pause          ( auto &&fn) { play_pause_fn          = fn; control_props_changed("CanPlay", "CanPause");                                          }
    void on_stop                ( auto &&fn) { stop_fn                = fn; control_props_changed("CanGoNext", "CanGoPrevious", "CanPause", "CanPlay", "CanSeek"); }
    void on_play                ( auto &&fn) { play_fn                = fn; control_props_changed("CanPlay");                                                      }
    void on_seek                ( auto &&fn) { seek_fn                = fn; control_props_changed("CanSeek");                                                      }
    void on_set_position        ( auto &&fn) { set_position_fn        = fn; control_props_changed("CanSeek");                                                      }
    void on_open_uri            ( auto &&fn) { open_uri_fn            = fn;                                                                                        }
    void on_fullscreen_changed  ( auto &&fn) { fullscreen_changed_fn  = fn; prop_changed(MP2, "CanSetFullscreen", sdbus::Variant(true));                           }
    void on_loop_status_changed ( auto &&fn) { loop_status_changed_fn = fn; control_props_changed("CanGoNext", "CanGoPrevious", "CanPause", "CanPlay", "CanSeek"); }
    void on_rate_changed        ( auto &&fn) { rate_changed_fn        = fn;                                                                                        }
    void on_shuffle_changed     ( auto &&fn) { shuffle_changed_fn     = fn; control_props_changed("CanGoNext", "CanGoPrevious", "CanPause", "CanPlay", "CanSeek"); }
    void on_volume_changed      ( auto &&fn) { volume_changed_fn      = fn; control_props_changed("CanGoNext", "CanGoPrevious", "CanPause", "CanPlay", "CanSeek"); }

    void set_fullscreen(bool value)                         { fullscreen            = value; prop_changed(MP2,  "Fullscreen"          , sdbus::Variant(fullscreen));                                         }
    void set_identity(std::string_view value)               { identity              = value; prop_changed(MP2,  "Identity"            , sdbus::Variant(identity));                                           }
    void set_desktop_entry(std::string_view value)          { desktop_entry         = value; prop_changed(MP2,  "DesktopEntry"        , sdbus::Variant(desktop_entry));                                      }
    void set_supported_uri_schemes(const StringList &value) { supported_uri_schemes = value; prop_changed(MP2,  "SupportedUriSchemes" , sdbus::Variant(supported_uri_schemes));                              }
    void set_supported_mime_types(const StringList &value)  { supported_mime_types  = value; prop_changed(MP2,  "SupportedMimeTypes"  , sdbus::Variant(supported_mime_types));                               }
    void set_playback_status(PlaybackStatus value)          { playback_status       = value; prop_changed(MP2P, "PlaybackStatus"      , sdbus::Variant(detail::playback_status_to_string(playback_status))); }
    void set_loop_status(LoopStatus value)                  { loop_status           = value; prop_changed(MP2P, "LoopStatus"          , sdbus::Variant(detail::loop_status_to_string(loop_status)));         }
    void set_shuffle(bool value)                            { shuffle               = value; prop_changed(MP2P, "Shuffle"             , sdbus::Variant(shuffle));                                            }
    void set_volume(double value)                           { volume                = value; prop_changed(MP2P, "Volume"              , sdbus::Variant(volume));                                             }
    void set_position(int64_t value)                        { position              = value;                                                                                                                 }

    void set_rate(double value)
    {
        if (value == 0.0) {
            fprintf(stderr, "warning: rate value must not be 0.0.\n");
            return;
        }
        if (value < minimum_rate || value > maximum_rate) {
            fprintf(stderr, "warning: rate value not in range.\n");
            return;
        }
        rate = value;
        prop_changed(MP2P, "Rate", sdbus::Variant(rate));
    }

    void set_metadata(const std::map<Field, sdbus::Variant> &value)
    {
        metadata.clear();
        for (auto [k, v] : value)
            metadata[detail::field_to_string(k)] = v;
        prop_changed(MP2P, "Metadata", sdbus::Variant(metadata));
    }

    void set_minimum_rate(double value)
    {
        if (value > 1.0) {
            fprintf(stderr, "warning: minimum value should always be 1.0 or lower\n");
            return;
        }
        minimum_rate = value;
        prop_changed(MP2P, "MinimumRate", sdbus::Variant(minimum_rate));
    }

    void set_maximum_rate(double value)
    {
        if (value < 1.0) {
            fprintf(stderr, "warning: maximum rate should always be 1.0 or higher\n");
            return;
        }
        maximum_rate = value;
        prop_changed(MP2P, "MaximumRate", sdbus::Variant(maximum_rate));
    }

    void send_seeked_signal(int64_t position);
};

#ifndef MPRIS_SERVER_NO_IMPL

inline void Server::prop_changed(const std::string &interface, const std::string &name, sdbus::Variant value)
{
    std::map<std::string, sdbus::Variant> d;
    d[name] = value;
    object->emitSignal("PropertiesChanged").onInterface("org.freedesktop.DBus.Properties").withArguments(interface, d, std::vector<std::string>{});
}

inline void Server::control_props_changed(auto&&... args)
{
    auto f = [&] (std::string_view name) {
        if (name == "CanGoNext")     return can_go_next();
        if (name == "CanGoPrevious") return can_go_previous();
        if (name == "CanPause")      return can_pause();
        if (name == "CanPlay")       return can_play();
        if (name == "CanSeek")       return can_seek();
        return false;
    };
    std::map<std::string, sdbus::Variant> d;
    auto g = [&] (const std::string &v) { if (f(v)) d[v] = sdbus::Variant(true); };
    (g(args), ...);
    object->emitSignal("PropertiesChanged").onInterface("org.freedesktop.DBus.Properties").withArguments(MP2P, d, std::vector<std::string>{});
}

inline void Server::set_fullscreen_external(bool value)
{
    if (fullscreen_changed_fn)
        throw sdbus::Error(sdbus::Error::Name{service_name + ".Error"}, "Cannot set Fullscreen (CanSetFullscreen is false).");
    set_fullscreen(value);
    fullscreen_changed_fn(fullscreen);
}

inline void Server::set_loop_status_external(const std::string &value)
{
    for (auto i = 0u; i < std::size(loop_status_strings); i++) {
        if (value == loop_status_strings[i]) {
            if (!can_control())
                throw sdbus::Error(sdbus::Error::Name{service_name + ".Error"}, "Cannot set loop status (CanControl is false).");
            set_loop_status(static_cast<LoopStatus>(i));
            loop_status_changed_fn(loop_status);
        }
    }
}

inline void Server::set_rate_external(double value)
{
    if (value <= minimum_rate || value > maximum_rate)
        throw sdbus::Error(sdbus::Error::Name{service_name + ".Error"}, "Rate value not in range.");
    if (value == 0.0)
        throw sdbus::Error(sdbus::Error::Name{service_name + ".Error"}, "Rate value must not be 0.0.");
    set_rate(value);
    if (rate_changed_fn)
        rate_changed_fn(rate);
}

inline void Server::set_shuffle_external(bool value)
{
    if (!can_control())
        throw sdbus::Error(sdbus::Error::Name{service_name + ".Error"}, "Cannot set shuffle (CanControl is false).");
    set_shuffle(value);
    shuffle_changed_fn(shuffle);
}

inline void Server::set_volume_external(double value)
{
    if (!can_control())
        throw sdbus::Error(sdbus::Error::Name{service_name + ".Error"}, "Cannot set volume (CanControl is false).");
    set_volume(value);
    volume_changed_fn(volume);
}

inline void Server::set_position_method(sdbus::ObjectPath id, int64_t pos)
{
    if (!can_seek())
        return;
    auto tid = metadata.find(detail::field_to_string(Field::TrackId));
    if (tid == metadata.end() || tid->second.get<std::string>() != id)
        return;
    set_position_fn(pos);
}

inline void Server::open_uri(const std::string &uri)
{
    if (open_uri_fn)
        open_uri_fn(uri);
}

inline std::unique_ptr<Server> Server::make(std::string_view name)
{
    try {
        auto s = std::make_unique<Server>(name);
        return s;
    } catch (const sdbus::Error &error) {
        return nullptr;
    }
}

inline Server::Server(std::string_view name)
    : service_name(PREFIX + std::string(name))
{
    connection = sdbus::createSessionBusConnection();
    connection->requestName(sdbus::ServiceName{service_name});
    object = sdbus::createObject(*connection, sdbus::ObjectPath{OBJECT_PATH});

#define M(f) detail::member_fn(this, &Server::f)
    object->addVTable(sdbus::registerMethod("Raise").implementedAs([&] { if (raise_fn) raise_fn(); })
                    , sdbus::registerMethod("Quit") .implementedAs([&] { if (quit_fn)  quit_fn();  })

                    , sdbus::registerProperty("CanQuit")            .withGetter([&] { return bool(quit_fn); })
                    , sdbus::registerProperty("Fullscreen")         .withGetter([&] { return fullscreen; }).withSetter(M(set_fullscreen_external))
                    , sdbus::registerProperty("CanSetFullscreen")   .withGetter([&] { return bool(fullscreen_changed_fn); })
                    , sdbus::registerProperty("CanRaise")           .withGetter([&] { return bool(raise_fn); })
                    , sdbus::registerProperty("HasTrackList")       .withGetter([&] { return false; })
                    , sdbus::registerProperty("Identity")           .withGetter([&] { return identity; })
                    , sdbus::registerProperty("DesktopEntry")       .withGetter([&] { return desktop_entry; })
                    , sdbus::registerProperty("SupportedUriSchemes").withGetter([&] { return supported_uri_schemes; })
                    , sdbus::registerProperty("SupportedMimeTypes") .withGetter([&] { return supported_mime_types; })
                    ).forInterface(MP2);

    object->addVTable(sdbus::registerMethod("Next")       .implementedAs([&] { if (can_go_next())             next_fn();       })
                    , sdbus::registerMethod("Previous")   .implementedAs([&] { if (can_go_previous())         previous_fn();   })
                    , sdbus::registerMethod("Pause")      .implementedAs([&] { if (can_pause())               pause_fn();      })
                    , sdbus::registerMethod("PlayPause")  .implementedAs([&] { if (can_play() || can_pause()) play_pause_fn(); })
                    , sdbus::registerMethod("Stop")       .implementedAs([&] { if (can_control())             stop_fn();       })
                    , sdbus::registerMethod("Play")       .implementedAs([&] { if (can_play())                play_fn();       })
                    , sdbus::registerMethod("Seek")       .implementedAs([&] (int64_t n) { if (can_seek()) seek_fn(n); }).withInputParamNames("Offset")
                    , sdbus::registerMethod("SetPosition").implementedAs(M(set_position_method))                         .withInputParamNames("TrackId", "Position")
                    , sdbus::registerMethod("OpenUri")    .implementedAs(M(open_uri))                                    .withInputParamNames("Uri")

                    , sdbus::registerProperty("PlaybackStatus").withGetter([&] { return detail::playback_status_to_string(playback_status); })
                    , sdbus::registerProperty("LoopStatus")    .withGetter([&] { return detail::loop_status_to_string(loop_status); }).withSetter(M(set_loop_status_external))
                    , sdbus::registerProperty("Rate")          .withGetter([&] { return rate; }).withSetter(M(set_rate_external))
                    , sdbus::registerProperty("Shuffle")       .withGetter([&] { return shuffle; }).withSetter(M(set_shuffle_external))
                    , sdbus::registerProperty("Metadata")      .withGetter([&] { return metadata; })
                    , sdbus::registerProperty("Volume")        .withGetter([&] { return volume; }).withSetter(M(set_volume_external))
                    , sdbus::registerProperty("Position")      .withGetter([&] { return position; })
                    , sdbus::registerProperty("MinimumRate")   .withGetter([&] { return minimum_rate; })
                    , sdbus::registerProperty("MaximumRate")   .withGetter([&] { return maximum_rate; })
                    , sdbus::registerProperty("CanGoNext")     .withGetter(M(can_go_next))
                    , sdbus::registerProperty("CanGoPrevious") .withGetter(M(can_go_previous))
                    , sdbus::registerProperty("CanPlay")       .withGetter(M(can_play))
                    , sdbus::registerProperty("CanPause")      .withGetter(M(can_pause))
                    , sdbus::registerProperty("CanSeek")       .withGetter(M(can_seek))
                    , sdbus::registerProperty("CanControl")    .withGetter(M(can_control))

                    , sdbus::registerSignal("Seeked").withParameters<int64_t>("Position")
                    ).forInterface(MP2P);
#undef M
}

inline void Server::start_loop()       { connection->enterEventLoop(); }
inline void Server::start_loop_async() { connection->enterEventLoopAsync(); }

inline void Server::send_seeked_signal(int64_t position)
{
    object->emitSignal("Seeked").onInterface(MP2P).withArguments(position);
}

#else

inline void Server::prop_changed(const std::string &interface, const std::string &name, sdbus::Variant value) { }
inline void Server::control_props_changed(auto&&... args) { }
inline void Server::set_fullscreen_external(bool value) { }
inline void Server::set_loop_status_external(const std::string &value) { }
inline void Server::set_rate_external(double value) { }
inline void Server::set_shuffle_external(bool value) { }
inline void Server::set_volume_external(double value) { }
inline void Server::set_position_method(sdbus::ObjectPath id, int64_t pos) { }
inline void Server::open_uri(const std::string &uri) { }
inline std::unique_ptr<Server> Server::make(std::string_view name) { return std::make_unique<Server>(name); }
inline Server::Server(std::string_view name) { }
inline void Server::start_loop() { }
inline void Server::start_loop_async() { }
inline void Server::send_seeked_signal(int64_t position) { }

#endif

} // namespace mpris

#endif
