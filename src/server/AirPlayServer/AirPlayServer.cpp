#include "AirPlayServer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#ifdef _WIN32
#include <unordered_map>
#include <winsock2.h>
#include <iphlpapi.h>
#else
#include <sys/utsname.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <pwd.h>
#include <sys/stat.h>
#endif

AirPlayServer::AirPlayServer() : server_name(DEFAULT_NAME),
                                 audio_sync(false),
                                 video_sync(true),
                                 audio_delay_alac(0),
                                 audio_delay_aac(0),
                                 relaunch_video(false),
                                 reset_loop(false),
                                 open_connections(0),
                                 videosink("autovideosink"),
                                 use_video(true),
                                 compression_type(0),
                                 audiosink("autoaudiosink"),
                                 audiodelay(-1),
                                 use_audio(true),
                                 new_window_closing_behavior(true),
                                 close_window(false),
                                 video_parser("h264parse"),
                                 video_decoder("decodebin"),
                                 video_converter("videoconvert"),
                                 show_client_FPS_data(false),
                                 max_ntp_timeouts(NTP_TIMEOUT_LIMIT),
                                 video_dumpfile(nullptr),
                                 video_dumpfile_name("videodump"),
                                 video_dump_limit(0),
                                 video_dumpfile_count(0),
                                 video_dump_count(0),
                                 dump_video(false),
                                 audio_dumpfile(nullptr),
                                 audio_dumpfile_name("audiodump"),
                                 audio_dump_limit(0),
                                 audio_dumpfile_count(0),
                                 audio_dump_count(0),
                                 dump_audio(false),
                                 audio_type(0x00),
                                 previous_audio_type(0x00),
                                 fullscreen(false),
                                 do_append_hostname(true),
                                 use_random_hw_addr(false),
                                 debug_log(DEFAULT_DEBUG_LOG),
                                 log_level(LOGGER_INFO),
                                 bt709_fix(false),
                                 max_connections(2),
                                 remote_clock_offset(0),
                                 restrict_clients(false),
                                 setup_legacy_pairing(false),
                                 require_password(false),
                                 pin(0),
                                 registration_list(false),
                                 db_low(-30.0),
                                 db_high(0.0),
                                 taper_volume(false)
{
    std::fill(std::begin(display), std::end(display), 0);
    std::fill(std::begin(tcp), std::end(tcp), 0);
    std::fill(std::begin(udp), std::end(udp), 0);
}

void AirPlayServer::initialize(int argc, char *argv[])
{
    std::string config_file = find_uxplay_config_file();
    if (!config_file.empty())
    {
        read_config_file(config_file);
    }
    parse_arguments(argc, argv);
    log_level = debug_log ? LOGGER_DEBUG : LOGGER_INFO;
}

void AirPlayServer::start()
{
    raop_callbacks_t raop_cbs;
    memset(&raop_cbs, 0, sizeof(raop_cbs));
    raop_cbs.conn_init = conn_init;
    raop_cbs.conn_destroy = conn_destroy;
    raop_cbs.conn_reset = conn_reset;
    raop_cbs.conn_teardown = conn_teardown;
    raop_cbs.audio_process = audio_process;
    raop_cbs.video_process = video_process;
    raop_cbs.audio_flush = audio_flush;
    raop_cbs.video_flush = video_flush;
    raop_cbs.video_pause = video_pause;
    raop_cbs.video_resume = video_resume;
    raop_cbs.audio_set_volume = audio_set_volume;
    raop_cbs.audio_get_format = audio_get_format;
    raop_cbs.video_report_size = video_report_size;
    raop_cbs.audio_set_metadata = audio_set_metadata;
    raop_cbs.audio_set_coverart = audio_set_coverart;
    raop_cbs.audio_set_progress = audio_set_progress;
    raop_cbs.report_client_request = report_client_request;
    raop_cbs.display_pin = display_pin;
    raop_cbs.register_client = register_client;
    raop_cbs.check_register = check_register;
    raop_cbs.export_dacp = export_dacp;

    raop = raop_init(&raop_cbs);
    if (raop == NULL)
    {
        LOGE("Error initializing raop!");
        return -1;
    }
    raop_set_log_callback(raop, log_callback, NULL);
    raop_set_log_level(raop, log_level);
    /* set max number of connections = 2 to protect against capture by new client */
    if (raop_init2(raop, max_connections, mac_address.c_str(), keyfile.c_str()))
    {
        LOGE("Error initializing raop (2)!");
        free(raop);
        return -1;
    }

    /* write desired display pixel width, pixel height, refresh_rate, max_fps, overscanned.  */
    /* use 0 for default values 1920,1080,60,30,0; these are sent to the Airplay client      */

    if (display[0])
        raop_set_plist(raop, "width", (int)display[0]);
    if (display[1])
        raop_set_plist(raop, "height", (int)display[1]);
    if (display[2])
        raop_set_plist(raop, "refreshRate", (int)display[2]);
    if (display[3])
        raop_set_plist(raop, "maxFPS", (int)display[3]);
    if (display[4])
        raop_set_plist(raop, "overscanned", (int)display[4]);

    if (show_client_FPS_data)
        raop_set_plist(raop, "clientFPSdata", 1);
    raop_set_plist(raop, "max_ntp_timeouts", max_ntp_timeouts);
    if (audiodelay >= 0)
        raop_set_plist(raop, "audio_delay_micros", audiodelay);
    if (require_password)
        raop_set_plist(raop, "pin", (int)pin);

    /* network port selection (ports listed as "0" will be dynamically assigned) */
    raop_set_tcp_ports(raop, tcp);
    raop_set_udp_ports(raop, udp);

    raop_port = raop_get_port(raop);
    raop_start(raop, &raop_port);
    raop_set_port(raop, raop_port);

    /* use raop_port for airplay_port (instead of tcp[2]) */
    airplay_port = raop_port;

    if (dnssd)
    {
        raop_set_dnssd(raop, dnssd);
    }
    else
    {
        LOGE("raop_set failed to set dnssd");
        return -2;
    }
    return 0;
}

void AirPlayServer::stop()
{
    // Stop the AirPlay server
}

void AirPlayServer::reset()
{
    if (raop)
    {
        raop_destroy(raop);
        raop = NULL;
    }
    return;
}

std::string AirPlayServer::find_mac()
{
    /*  finds the MAC address of a network interface *
     *  in a Windows, Linux, *BSD or macOS system.   */
    std::string mac = "";
    char str[3];
#ifdef _WIN32
    ULONG buflen = sizeof(IP_ADAPTER_ADDRESSES);
    PIP_ADAPTER_ADDRESSES addresses = (IP_ADAPTER_ADDRESSES *)malloc(buflen);
    if (addresses == NULL)
    {
        return mac;
    }
    if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, addresses, &buflen) == ERROR_BUFFER_OVERFLOW)
    {
        free(addresses);
        addresses = (IP_ADAPTER_ADDRESSES *)malloc(buflen);
        if (addresses == NULL)
        {
            return mac;
        }
    }
    if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, addresses, &buflen) == NO_ERROR)
    {
        for (PIP_ADAPTER_ADDRESSES address = addresses; address != NULL; address = address->Next)
        {
            if (address->PhysicalAddressLength != 6                /* MAC has 6 octets */
                || (address->IfType != 6 && address->IfType != 71) /* Ethernet or Wireless interface */
                || address->OperStatus != 1)
            { /* interface is up */
                continue;
            }
            mac.erase();
            for (int i = 0; i < 6; i++)
            {
                snprintf(str, sizeof(str), "%02x", int(address->PhysicalAddress[i]));
                mac = mac + str;
                if (i < 5)
                    mac = mac + ":";
            }
            break;
        }
    }
    free(addresses);
    return mac;
#else
    struct ifaddrs *ifap, *ifaptr;
    int non_null_octets = 0;
    unsigned char octet[6];
    if (getifaddrs(&ifap) == 0)
    {
        for (ifaptr = ifap; ifaptr != NULL; ifaptr = ifaptr->ifa_next)
        {
            if (ifaptr->ifa_addr == NULL)
                continue;
#ifdef __linux__
            if (ifaptr->ifa_addr->sa_family != AF_PACKET)
                continue;
            struct sockaddr_ll *s = (struct sockaddr_ll *)ifaptr->ifa_addr;
            for (int i = 0; i < 6; i++)
            {
                if ((octet[i] = s->sll_addr[i]) != 0)
                    non_null_octets++;
            }
#else /* macOS and *BSD */
            if (ifaptr->ifa_addr->sa_family != AF_LINK)
                continue;
            unsigned char *ptr = (unsigned char *)LLADDR((struct sockaddr_dl *)ifaptr->ifa_addr);
            for (int i = 0; i < 6; i++)
            {
                if ((octet[i] = *ptr) != 0)
                    non_null_octets++;
                ptr++;
            }
#endif
            if (non_null_octets)
            {
                mac.erase();
                for (int i = 0; i < 6; i++)
                {
                    snprintf(str, sizeof(str), "%02x", octet[i]);
                    mac = mac + str;
                    if (i < 5)
                        mac = mac + ":";
                }
                break;
            }
        }
    }
    freeifaddrs(ifap);
#endif
    return mac;
}

void AirPlayServer::append_hostname()
{
#ifdef _WIN32 /*modification for compilation on Windows */
    char buffer[256] = "";
    unsigned long size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size))
    {
        std::string name = server_name;
        name.append("@");
        name.append(buffer);
        server_name = name;
    }
#else
    struct utsname buf;
    if (!uname(&buf))
    {
        std::string name = server_name;
        name.append("@");
        name.append(buf.nodename);
        server_name = name;
    }
#endif
}

bool AirPlayServer::validate_mac(const std::string &mac_address)
{
    char c;
    if (strlen(mac_address) != 17)
        return false;
    for (int i = 0; i < 17; i++)
    {
        c = *(mac_address + i);
        if (i % 3 == 2)
        {
            if (c != ':')
                return false;
        }
        else
        {
            if (c < '0')
                return false;
            if (c > '9' && c < 'A')
                return false;
            if (c > 'F' && c < 'a')
                return false;
            if (c > 'f')
                return false;
        }
    }
    return true;
}

std::string AirPlayServer::random_mac()
{
    char str[3];
    int octet = rand() % 64;
    octet = (octet << 1) + LOCAL;
    octet = (octet << 1) + MULTICAST;
    snprintf(str, 3, "%02x", octet);
    std::string mac_address(str);
    for (int i = 1; i < OCTETS; i++)
    {
        mac_address = mac_address + ":";
        octet = rand() % 256;
        snprintf(str, 3, "%02x", octet);
        mac_address = mac_address + str;
    }
    return mac_address;
}

void AirPlayServer::process_metadata(int count, const std::string &dmap_tag, const unsigned char *metadata, int datalen)
{
    int dmap_type = 0;
    /* DMAP metadata items can be strings (dmap_type = 9); other types are byte, short, int, long, date, and list.  *
     * The DMAP item begins with a 4-character (4-letter) "dmap_tag" string that identifies the type.               */

    if (debug_log)
    {
        printf("%d: dmap_tag [%s], %d\n", count, dmap_tag, datalen);
    }

    /* UTF-8 String-type DMAP tags seen in Apple Music Radio are processed here.   *
     * (DMAP tags "asal", "asar", "ascp", "asgn", "minm" ). TODO expand this */

    if (datalen == 0)
    {
        return;
    }

    if (dmap_tag[0] == 'a' && dmap_tag[1] == 's')
    {
        dmap_type = 9;
        switch (dmap_tag[2])
        {
        case 'a':
            switch (dmap_tag[3])
            {
            case 'a':
                printf("Album artist: "); /*asaa*/
                break;
            case 'l':
                printf("Album: "); /*asal*/
                break;
            case 'r':
                printf("Artist: "); /*asar*/
                break;
            default:
                dmap_type = 0;
                break;
            }
            break;
        case 'c':
            switch (dmap_tag[3])
            {
            case 'm':
                printf("Comment: "); /*ascm*/
                break;
            case 'n':
                printf("Content description: "); /*ascn*/
                break;
            case 'p':
                printf("Composer: "); /*ascp*/
                break;
            case 't':
                printf("Category: "); /*asct*/
                break;
            default:
                dmap_type = 0;
                break;
            }
            break;
        case 's':
            switch (dmap_tag[3])
            {
            case 'a':
                printf("Sort Artist: "); /*assa*/
                break;
            case 'c':
                printf("Sort Composer: "); /*assc*/
                break;
            case 'l':
                printf("Sort Album artist: "); /*assl*/
                break;
            case 'n':
                printf("Sort Name: "); /*assn*/
                break;
            case 's':
                printf("Sort Series: "); /*asss*/
                break;
            case 'u':
                printf("Sort Album: "); /*assu*/
                break;
            default:
                dmap_type = 0;
                break;
            }
            break;
        default:
            if (strcmp(dmap_tag, "asdt") == 0)
            {
                printf("Description: ");
            }
            else if (strcmp(dmap_tag, "asfm") == 0)
            {
                printf("Format: ");
            }
            else if (strcmp(dmap_tag, "asgn") == 0)
            {
                printf("Genre: ");
            }
            else if (strcmp(dmap_tag, "asky") == 0)
            {
                printf("Keywords: ");
            }
            else if (strcmp(dmap_tag, "aslc") == 0)
            {
                printf("Long Content Description: ");
            }
            else
            {
                dmap_type = 0;
            }
            break;
        }
    }
    else if (strcmp(dmap_tag, "minm") == 0)
    {
        dmap_type = 9;
        printf("Title: ");
    }

    if (dmap_type == 9)
    {
        char *str = (char *)calloc(1, datalen + 1);
        memcpy(str, metadata, datalen);
        printf("%s", str);
        free(str);
    }
    else if (debug_log)
    {
        for (int i = 0; i < datalen; i++)
        {
            if (i > 0 && i % 16 == 0)
                printf("\n");
            printf("%2.2x ", (int)metadata[i]);
        }
    }
    printf("\n");
}

int AirPlayServer::parse_dmap_header(const unsigned char *metadata, std::string &tag, int &len)
{
    const unsigned char *header = metadata;

    bool istag = true;
    for (int i = 0; i < 4; i++)
    {
        tag[i] = (char)*header;
        if (!isalpha(tag[i]))
        {
            istag = false;
        }
        header++;
    }

    *len = 0;
    for (int i = 0; i < 4; i++)
    {
        *len <<= 8;
        *len += (int)*header;
        header++;
    }
    if (!istag || *len < 0)
    {
        return 1;
    }
    return 0;
}

void AirPlayServer::log(int level, const char *format, ...)
{
    // Implement logging logic
}

int main(int argc, char *argv[])
{
    AirPlayServer server;
    server.initialize(argc, argv);
    server.start();
    // Run the server loop
    server.stop();
    return 0;
}
