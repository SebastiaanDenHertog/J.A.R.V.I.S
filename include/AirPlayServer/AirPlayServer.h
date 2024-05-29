#ifndef AIRPLAYSERVER_H
#define AIRPLAYSERVER_H

#include <string>
#include <vector>
#include <cstdio>

class AirPlayServer
{
public:
    AirPlayServer();
    void initialize(int argc, char *argv[]);
    void start();
    void stop();
    void reset();

private:
    void parse_arguments(int argc, char *argv[]);
    void read_config_file(const std::string &filename);
    std::string find_mac();
    void append_hostname();
    bool validate_mac(const std::string &mac_address);
    std::string random_mac();
    void process_metadata(int count, const std::string &dmap_tag, const unsigned char *metadata, int datalen);
    int parse_dmap_header(const unsigned char *metadata, std::string &tag, int &len);
    void log(int level, const char *format, ...);

    std::string server_name;
    bool audio_sync;
    bool video_sync;
    int64_t audio_delay_alac;
    int64_t audio_delay_aac;
    bool relaunch_video;
    bool reset_loop;
    unsigned int open_connections;
    std::string videosink;
    enum videoflip_t
    {
        NONE,
        HFLIP,
        VFLIP,
        INVERT,
        LEFT,
        RIGHT
    };
    videoflip_t videoflip[2];
    bool use_video;
    unsigned char compression_type;
    std::string audiosink;
    int audiodelay;
    bool use_audio;
    bool new_window_closing_behavior;
    bool close_window;
    std::string video_parser;
    std::string video_decoder;
    std::string video_converter;
    bool show_client_FPS_data;
    unsigned int max_ntp_timeouts;
    FILE *video_dumpfile;
    std::string video_dumpfile_name;
    int video_dump_limit;
    int video_dumpfile_count;
    int video_dump_count;
    bool dump_video;
    FILE *audio_dumpfile;
    std::string audio_dumpfile_name;
    int audio_dump_limit;
    int audio_dumpfile_count;
    int audio_dump_count;
    bool dump_audio;
    unsigned char audio_type;
    unsigned char previous_audio_type;
    bool fullscreen;
    std::string coverart_filename;
    bool do_append_hostname;
    bool use_random_hw_addr;
    unsigned short display[5];
    bool debug_log;
    int log_level;
    bool bt709_fix;
    int max_connections;
    unsigned short raop_port;
    unsigned short airplay_port;
    uint64_t remote_clock_offset;
    std::vector<std::string> allowed_clients;
    std::vector<std::string> blocked_clients;
    bool restrict_clients;
    bool setup_legacy_pairing;
    bool require_password;
    unsigned short pin;
    std::string keyfile;
    std::string mac_address;
    std::string dacpfile;
    bool registration_list;
    std::string pairing_register;
    std::vector<std::string> registered_keys;
    double db_low;
    double db_high;
    bool taper_volume;
    unsigned short tcp[3];
    unsigned short udp[3];

    // Define constants
    static constexpr const char *DEFAULT_NAME = "J.A.R.V.I.S.";
    static constexpr bool DEFAULT_DEBUG_LOG = false;
    static constexpr int LOGGER_INFO = 1;
    static constexpr unsigned int NTP_TIMEOUT_LIMIT = 5;
    static constexpr int SECOND_IN_USECS = 1000000;
    static constexpr int SECOND_IN_NSECS = 1000000000UL;
};

#endif // AIRPLAYSERVER_H
