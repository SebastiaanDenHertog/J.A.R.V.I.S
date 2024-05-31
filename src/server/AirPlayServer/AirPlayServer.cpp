#include "AirPlayServer.h"
#include "lib/raop.h"
#include "lib/stream.h"
#include "lib/logger.h"
#include "lib/dnssd.h"
#include "renderers/video_renderer.h"
#include "renderers/audio_renderer.h"
#include <glib.h>
#include <csignal>

const unsigned char empty_image[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x25, 0xdb, 0x56,
    0xca, 0x00, 0x00, 0x00, 0x03, 0x50, 0x4c, 0x54, 0x45, 0x00, 0x00, 0x00, 0xa7, 0x7a, 0x3d, 0xda,
    0x00, 0x00, 0x00, 0x01, 0x74, 0x52, 0x4e, 0x53, 0x00, 0x40, 0xe6, 0xd8, 0x66, 0x00, 0x00, 0x00,
    0x0a, 0x49, 0x44, 0x41, 0x54, 0x08, 0xd7, 0x63, 0x60, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0xe2,
    0x21, 0xbc, 0x33, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};

static dnssd_t *dnssd = NULL;
static raop_t *raop = NULL;
static logger_t *render_logger = NULL;
videoflip_t videoflip[2] = {NONE, NONE};
static unsigned char mark[] = {0x00, 0x00, 0x00, 0x01};
static std::string coverart_filename = "";
static unsigned short display[5] = {0}, tcp[3] = {0}, udp[3] = {0};
static int nohold = 0;
static unsigned short raop_port;
static unsigned short airplay_port;
static std::vector<std::string> allowed_clients;
static std::vector<std::string> blocked_clients;
static bool restrict_clients;
static std::string keyfile = "";
static std::string mac_address = "";
static std::string dacpfile = "";
static std::string pairing_register = "";
static std::vector<std::string> registered_keys;

std::vector<char> server_hw_addr;
std::string config_file = "";

#define LOGD(...) log(LOGGER_DEBUG, __VA_ARGS__)
#define LOGI(...) log(LOGGER_INFO, __VA_ARGS__)
#define LOGW(...) log(LOGGER_WARNING, __VA_ARGS__)
#define LOGE(...) log(LOGGER_ERR, __VA_ARGS__)

#define MULTICAST 0
#define LOCAL 1
#define OCTETS 6

AirPlayServer::AirPlayServer(int port, const char *Name) : server_name(Name),
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
                                                           taper_volume(false),
                                 raop_port(port)
{
    std::fill(std::begin(display), std::end(display), 0);
    std::fill(std::begin(tcp), std::end(tcp), 0);
    std::fill(std::begin(udp), std::end(udp), 0);
}

static int log_level = LOGGER_INFO;

void log(int level, const char *format, ...)
{
    va_list vargs;
    if (level > log_level)
        return;
    switch (level)
    {
    case 0:
    case 1:
    case 2:
    case 3:
        printf("*** ERROR: ");
        break;
    case 4:
        printf("*** WARNING: ");
        break;
    default:
        break;
    }
    va_start(vargs, format);
    vprintf(format, vargs);
    printf("\n");
    va_end(vargs);
}

bool AirPlayServer::file_has_write_access(const char *filename)
{
    bool exists = false;
    bool write = false;
#ifdef _WIN32
    if ((exists = _access(filename, 0) != -1))
    {
        write = (_access(filename, 2) != -1);
    }
#else
    if ((exists = access(filename, F_OK) != -1))
    {
        write = (access(filename, W_OK) != -1);
    }
#endif
    if (!exists)
    {
        FILE *fp = fopen(filename, "w");
        if (fp)
        {
            write = true;
            fclose(fp);
            remove(filename);
        }
    }
    return write;
}

size_t write_coverart(const char *filename, const void *image, size_t len)
{
    FILE *fp = fopen(filename, "wb");
    size_t count = fwrite(image, 1, len, fp);
    fclose(fp);
    return count;
}

std::string AirPlayServer::find_uxplay_config_file()
{
    std::string no_config_file = "";
    const char *homedir = NULL;
    const char *uxplayrc = NULL;
    std::string config0, config1, config2;
    struct stat sb;
    uxplayrc = getenv("UXPLAYRC"); /* first look for $UXPLAYRC */
    if (uxplayrc)
    {
        config0 = uxplayrc;
        if (stat(config0.c_str(), &sb) == 0)
            return config0;
    }
    homedir = get_homedir();
    if (homedir)
    {
        config1 = homedir;
        config1.append("/.uxplayrc");
        if (stat(config1.c_str(), &sb) == 0)
            return config1; /* look for ~/.uxplayrc */
        config2 = homedir;
        config2.append("/.config/uxplayrc"); /* look for ~/.config/uxplayrc */
        if (stat(config2.c_str(), &sb) == 0)
            return config2;
    }
    return no_config_file;
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

void AirPlayServer::dump_audio_to_file(unsigned char *data, int datalen, unsigned char type)
{
    if (!audio_dumpfile && audio_type != previous_audio_type)
    {
        char suffix[20];
        std::string fn = audio_dumpfile_name;
        previous_audio_type = audio_type;
        audio_dumpfile_count++;
        audio_dump_count = 0;
        /* type 0x20 is lossless ALAC, type 0x80 is compressed AAC-ELD, type 0x10 is "other" */
        if (audio_type == 0x20)
        {
            snprintf(suffix, sizeof(suffix), ".%d.alac", audio_dumpfile_count);
        }
        else if (audio_type == 0x80)
        {
            snprintf(suffix, sizeof(suffix), ".%d.aac", audio_dumpfile_count);
        }
        else
        {
            snprintf(suffix, sizeof(suffix), ".%d.aud", audio_dumpfile_count);
        }
        fn.append(suffix);
        audio_dumpfile = fopen(fn.c_str(), "w");
        if (audio_dumpfile == NULL)
        {
            LOGE("could not open file %s for dumping audio frames", fn.c_str());
        }
    }

    if (audio_dumpfile)
    {
        fwrite(data, 1, datalen, audio_dumpfile);
        if (audio_dump_limit)
        {
            audio_dump_count++;
            if (audio_dump_count == audio_dump_limit)
            {
                fclose(audio_dumpfile);
                audio_dumpfile = NULL;
            }
        }
    }
}

void AirPlayServer::dump_video_to_file(unsigned char *data, int datalen)
{
    /*  SPS NAL has (data[4] & 0x1f) = 0x07  */
    if ((data[4] & 0x1f) == 0x07 && video_dumpfile && video_dump_limit)
    {
        fwrite(mark, 1, sizeof(mark), video_dumpfile);
        fclose(video_dumpfile);
        video_dumpfile = NULL;
        video_dump_count = 0;
    }

    if (!video_dumpfile)
    {
        std::string fn = video_dumpfile_name;
        if (video_dump_limit)
        {
            char suffix[20];
            video_dumpfile_count++;
            snprintf(suffix, sizeof(suffix), ".%d", video_dumpfile_count);
            fn.append(suffix);
        }
        fn.append(".h264");
        video_dumpfile = fopen(fn.c_str(), "w");
        if (video_dumpfile == NULL)
        {
            LOGE("could not open file %s for dumping h264 frames", fn.c_str());
        }
    }

    if (video_dumpfile)
    {
        if (video_dump_limit == 0)
        {
            fwrite(data, 1, datalen, video_dumpfile);
        }
        else if (video_dump_count < video_dump_limit)
        {
            video_dump_count++;
            fwrite(data, 1, datalen, video_dumpfile);
        }
    }
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

bool AirPlayServer::validate_mac(char *mac_address)
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
            if (strcmp(dmap_tag.c_str(), "asdt") == 0)
            {
                printf("Description: ");
            }
            else if (strcmp(dmap_tag.c_str(), "asfm") == 0)
            {
                printf("Format: ");
            }
            else if (strcmp(dmap_tag.c_str(), "asgn") == 0)
            {
                printf("Genre: ");
            }
            else if (strcmp(dmap_tag.c_str(), "asky") == 0)
            {
                printf("Keywords: ");
            }
            else if (strcmp(dmap_tag.c_str(), "aslc") == 0)
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
    else if (strcmp(dmap_tag.c_str(), "minm") == 0)
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

int AirPlayServer::parse_dmap_header(const unsigned char *metadata, char *tag, int *len)
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

char *AirPlayServer::create_pin_display(char *pin_str, int margin, int gap)
{
    char *ptr;
    char num[2] = {0};
    int w = 10;
    int h = 8;
    char digits[10][8][11] = {"0821111380", "2114005113", "1110000111", "1110000111", "1110000111", "1110000111", "5113002114", "0751111470",
                              "0002111000", "0021111000", "0000111000", "0000111000", "0000111000", "0000111000", "0000111000", "0011111110",
                              "0811112800", "2114005113", "0000000111", "0000082114", "0862111470", "2114700000", "1117000000", "1111111111",
                              "0821111380", "2114005113", "0000082114", "0000111170", "0000075130", "1110000111", "5113002114", "0751111470",
                              "0000211110", "0001401110", "0021401110", "0214001110", "2110001110", "1111111111", "0000001110", "0000001110",
                              "1111111110", "1110000000", "1110000000", "1112111380", "0000075113", "0000000111", "5113002114", "0711114700",
                              "0821111380", "2114005113", "1110000000", "1112111380", "1114075113", "1110000111", "5113002114", "0751111470",
                              "1111111111", "0000002114", "0000021140", "0000211400", "0002114000", "0021140000", "0211400000", "2114000000",
                              "0831111280", "2114002114", "5113802114", "0751111170", "8214775138", "1110000111", "5113002114", "0751111470",
                              "0821111380", "2114005113", "1110000111", "5113802111", "0751114111", "0000000111", "5113002114", "0751111470"};

    char pixels[9] = {' ', '8', 'd', 'b', 'P', 'Y', 'o', '"', '.'};
    /* Ascii art used here is derived from the FIGlet font "collosal" */

    int pin_val = (int)strtoul(pin_str, &ptr, 10);
    if (*ptr)
    {
        return NULL;
    }
    int len = strlen(pin_str);
    int *pin = (int *)calloc(len, sizeof(int));
    if (!pin)
    {
        return NULL;
    }

    for (int i = 0; i < len; i++)
    {
        pin[len - 1 - i] = pin_val % 10;
        pin_val = pin_val / 10;
    }

    int size = 4 + h * (margin + len * (w + gap + 1));
    char *pin_image = (char *)calloc(size, sizeof(char));
    if (!pin_image)
    {
        return NULL;
    }
    char *pos = pin_image;
    snprintf(pos, 2, "\n");
    pos++;

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < margin; j++)
        {
            snprintf(pos, 2, " ");
            pos++;
        }

        for (int j = 0; j < len; j++)
        {
            int l = pin[j];
            char *p = digits[l][i];
            for (int k = 0; k < w; k++)
            {
                char *ptr;
                strncpy(num, p++, 1);
                int r = (int)strtoul(num, &ptr, 10);
                snprintf(pos, 2, "%c", pixels[r]);
                pos++;
            }
            for (int n = 0; n < gap; n++)
            {
                snprintf(pos, 2, " ");
                pos++;
            }
        }
        snprintf(pos, 2, "\n");
        pos++;
    }
    snprintf(pos, 2, "\n");
    return pin_image;
}


#ifdef _WIN32
struct signal_handler
{
    GSourceFunc handler;
    gpointer user_data;
};

std::unordered_map<gint, signal_handler> u = {};

void SignalHandler(int signum)
{
    if (signum == SIGTERM || signum == SIGINT)
    {
        u[signum].handler(u[signum].user_data);
    }
}

guint g_unix_signal_add(gint signum, GSourceFunc handler, gpointer user_data)
{
    u[signum] = signal_handler{handler, user_data};
    (void)signal(signum, SignalHandler);
    return 0;
}
#endif

int AirPlayServer::parse_hw_addr(std::string str, std::vector<char> &hw_addr)
{
    for (int i = 0; i < (int)str.length(); i += 3)
    {
        hw_addr.push_back((char)stol(str.substr(i), NULL, 16));
    }
    return 0;
}

const char *AirPlayServer::get_homedir()
{
    const char *homedir = getenv("XDG_CONFIG_HOMEDIR");
    if (homedir == NULL)
    {
        homedir = getenv("HOME");
    }
#ifndef _WIN32
    if (homedir == NULL)
    {
        homedir = getpwuid(getuid())->pw_dir;
    }
#endif
    return homedir;
}

std::string AirPlayServer::find_uxplay_config_file()
{
    std::string no_config_file = "";
    const char *homedir = NULL;
    const char *uxplayrc = NULL;
    std::string config0, config1, config2;
    struct stat sb;
    uxplayrc = getenv("UXPLAYRC"); /* first look for $UXPLAYRC */
    if (uxplayrc)
    {
        config0 = uxplayrc;
        if (stat(config0.c_str(), &sb) == 0)
            return config0;
    }
    homedir = get_homedir();
    if (homedir)
    {
        config1 = homedir;
        config1.append("/.uxplayrc");
        if (stat(config1.c_str(), &sb) == 0)
            return config1; /* look for ~/.uxplayrc */
        config2 = homedir;
        config2.append("/.config/uxplayrc"); /* look for ~/.config/uxplayrc */
        if (stat(config2.c_str(), &sb) == 0)
            return config2;
    }
    return no_config_file;
}

#define MULTICAST 0
#define LOCAL 1
#define OCTETS 6

bool AirPlayServer::validate_mac(char *mac_address)
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

bool AirPlayServer::option_has_value(const int i, const int argc, std::string option, const char *next_arg)
{
    if (i >= argc - 1 || next_arg[0] == '-')
    {
        LOGE("invalid: \"%s\" had no argument", option.c_str());
        return false;
    }
    return true;
}

bool AirPlayServer::get_display_settings(std::string value, unsigned short *w, unsigned short *h, unsigned short *r)
{
    // assume str  = wxh@r is valid if w and h are positive decimal integers
    // with no more than 4 digits, r < 256 (stored in one byte).
    char *end;
    std::size_t pos = value.find_first_of("x");
    if (pos == std::string::npos)
        return false;
    std::string str1 = value.substr(pos + 1);
    value.erase(pos);
    if (value.length() == 0 || value.length() > 4 || value[0] == '-')
        return false;
    *w = (unsigned short)strtoul(value.c_str(), &end, 10);
    if (*end || *w == 0)
        return false;
    pos = str1.find_first_of("@");
    if (pos != std::string::npos)
    {
        std::string str2 = str1.substr(pos + 1);
        if (str2.length() == 0 || str2.length() > 3 || str2[0] == '-')
            return false;
        *r = (unsigned short)strtoul(str2.c_str(), &end, 10);
        if (*end || *r == 0 || *r > 255)
            return false;
        str1.erase(pos);
    }
    if (str1.length() == 0 || str1.length() > 4 || str1[0] == '-')
        return false;
    *h = (unsigned short)strtoul(str1.c_str(), &end, 10);
    if (*end || *h == 0)
        return false;
    return true;
}

bool AirPlayServer::get_value(const char *str, unsigned int *n)
{
    // if n > 0 str must be a positive decimal <= input value *n
    // if n = 0, str must be a non-negative decimal
    if (strlen(str) == 0 || strlen(str) > 10 || str[0] == '-')
        return false;
    char *end;
    unsigned long l = strtoul(str, &end, 10);
    if (*end)
        return false;
    if (*n && (l == 0 || l > *n))
        return false;
    *n = (unsigned int)l;
    return true;
}

bool AirPlayServer::get_ports(int nports, std::string option, const char *value, unsigned short *const port)
{
    /*valid entries are comma-separated values port_1,port_2,...,port_r, 0 < r <= nports */
    /*where ports are distinct, and are in the allowed range.                            */
    /*missing values are consecutive to last given value (at least one value needed).    */
    char *end;
    unsigned long l;
    std::size_t pos;
    std::string val(value), str;
    for (int i = 0; i <= nports; i++)
    {
        if (i == nports)
            break;
        pos = val.find_first_of(',');
        str = val.substr(0, pos);
        if (str.length() == 0 || str.length() > 5 || str[0] == '-')
            break;
        l = strtoul(str.c_str(), &end, 10);
        if (*end || l < LOWEST_ALLOWED_PORT || l > HIGHEST_PORT)
            break;
        *(port + i) = (unsigned short)l;
        for (int j = 0; j < i; j++)
        {
            if (*(port + j) == *(port + i))
                break;
        }
        if (pos == std::string::npos)
        {
            if (nports + *(port + i) > i + 1 + HIGHEST_PORT)
                break;
            for (int j = i + 1; j < nports; j++)
            {
                *(port + j) = *(port + j - 1) + 1;
            }
            return true;
        }
        val.erase(0, pos + 1);
    }
    LOGE("invalid \"%s %s\", all %d ports must be in range [%d,%d]",
         option.c_str(), value, nports, LOWEST_ALLOWED_PORT, HIGHEST_PORT);
    return false;
}

bool AirPlayServer::get_videoflip(const char *str, videoflip_t *videoflip)
{
    if (strlen(str) > 1)
        return false;
    switch (str[0])
    {
    case 'I':
        *videoflip = INVERT;
        break;
    case 'H':
        *videoflip = HFLIP;
        break;
    case 'V':
        *videoflip = VFLIP;
        break;
    default:
        return false;
    }
    return true;
}

bool AirPlayServer::get_videorotate(const char *str, videoflip_t *videoflip)
{
    if (strlen(str) > 1)
        return false;
    switch (str[0])
    {
    case 'L':
        *videoflip = LEFT;
        break;
    case 'R':
        *videoflip = RIGHT;
        break;
    default:
        return false;
    }
    return true;
}

void AirPlayServer::append_hostname(std::string &server_name)
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

void AirPlayServer::parse_arguments(int argc, char *argv[])
{
    // Parse arguments
    for (int i = 1; i < argc; i++)
    {
        std::string arg(argv[i]);
        if (arg == "-allow")
        {
            if (!option_has_value(i, argc, arg, argv[i + 1]))
                exit(1);
            i++;
            allowed_clients.push_back(argv[i]);
        }
        else if (arg == "-block")
        {
            if (!option_has_value(i, argc, arg, argv[i + 1]))
                exit(1);
            i++;
            blocked_clients.push_back(argv[i]);
        }
        else if (arg == "-restrict")
        {
            if (i < argc - 1)
            {
                if (strlen(argv[i + 1]) == 2 && strncmp(argv[i + 1], "no", 2) == 0)
                {
                    restrict_clients = false;
                    i++;
                    continue;
                }
            }
            restrict_clients = true;
        }
        else if (arg == "-n")
        {
            if (!option_has_value(i, argc, arg, argv[i + 1]))
                exit(1);
            server_name = std::string(argv[++i]);
        }
        else if (arg == "-nh")
        {
            do_append_hostname = false;
        }
        else if (arg == "-async")
        {
            audio_sync = true;
            if (i < argc - 1)
            {
                if (strlen(argv[i + 1]) == 2 && strncmp(argv[i + 1], "no", 2) == 0)
                {
                    audio_sync = false;
                    i++;
                    continue;
                }
                char *end;
                int n = (int)(strtof(argv[i + 1], &end) * 1000);
                if (*end == '\0')
                {
                    i++;
                    if (n > -SECOND_IN_USECS && n < SECOND_IN_USECS)
                    {
                        audio_delay_alac = n * 1000; /* units are nsecs */
                    }
                    else
                    {
                        fprintf(stderr, "invalid -async %s: requested delays must be smaller than +/- 1000 millisecs\n", argv[i]);
                        exit(1);
                    }
                }
            }
        }
        else if (arg == "-vsync")
        {
            video_sync = true;
            if (i < argc - 1)
            {
                if (strlen(argv[i + 1]) == 2 && strncmp(argv[i + 1], "no", 2) == 0)
                {
                    video_sync = false;
                    i++;
                    continue;
                }
                char *end;
                int n = (int)(strtof(argv[i + 1], &end) * 1000);
                if (*end == '\0')
                {
                    i++;
                    if (n > -SECOND_IN_USECS && n < SECOND_IN_USECS)
                    {
                        audio_delay_aac = n * 1000; /* units are nsecs */
                    }
                    else
                    {
                        fprintf(stderr, "invalid -vsync %s: requested delays must be smaller than +/- 1000 millisecs\n", argv[i]);
                        exit(1);
                    }
                }
            }
        }
        else if (arg == "-s")
        {
            if (!option_has_value(i, argc, argv[i], argv[i + 1]))
                exit(1);
            std::string value(argv[++i]);
            if (!get_display_settings(value, &display[0], &display[1], &display[2]))
            {
                fprintf(stderr, "invalid \"-s %s\"; -s wxh : max w,h=9999; -s wxh@r : max r=255\n",
                        argv[i]);
                exit(1);
            }
        }
        else if (arg == "-fps")
        {
            if (!option_has_value(i, argc, arg, argv[i + 1]))
                exit(1);
            unsigned int n = 255;
            if (!get_value(argv[++i], &n))
            {
                fprintf(stderr, "invalid \"-fps %s\"; -fps n : max n=255, default n=30\n", argv[i]);
                exit(1);
            }
            display[3] = (unsigned short)n;
        }
        else if (arg == "-o")
        {
            display[4] = 1;
        }
        else if (arg == "-f")
        {
            if (!option_has_value(i, argc, arg, argv[i + 1]))
                exit(1);
            if (!get_videoflip(argv[++i], &videoflip[0]))
            {
                fprintf(stderr, "invalid \"-f %s\" , unknown flip type, choices are H, V, I\n", argv[i]);
                exit(1);
            }
        }
        else if (arg == "-r")
        {
            if (!option_has_value(i, argc, arg, argv[i + 1]))
                exit(1);
            if (!get_videoflip(argv[++i], &videoflip[0]))
            {
                fprintf(stderr, "invalid \"-r %s\" , unknown rotation  type, choices are R, L\n", argv[i]);
                exit(1);
            }
        }
        else if (arg == "-p")
        {
            if (i == argc - 1 || argv[i + 1][0] == '-')
            {
                tcp[0] = 7100;
                tcp[1] = 7000;
                tcp[2] = 7001;
                udp[0] = 7011;
                udp[1] = 6001;
                udp[2] = 6000;
                continue;
            }
            std::string value(argv[++i]);
            if (value == "tcp")
            {
                arg.append(" tcp");
                if (!get_ports(3, arg, argv[++i], tcp))
                    exit(1);
            }
            else if (value == "udp")
            {
                arg.append(" udp");
                if (!get_ports(3, arg, argv[++i], udp))
                    exit(1);
            }
            else
            {
                if (!get_ports(3, arg, argv[i], tcp))
                    exit(1);
                for (int j = 1; j < 3; j++)
                {
                    udp[j] = tcp[j];
                }
            }
        }
        else if (arg == "-m")
        {
            if (i < argc - 1 && *argv[i + 1] != '-')
            {
                if (validate_mac(argv[++i]))
                {
                    mac_address.erase();
                    mac_address = argv[i];
                    use_random_hw_addr = false;
                }
                else
                {
                    fprintf(stderr, "invalid mac address \"%s\": address must have form"
                                    " \"xx:xx:xx:xx:xx:xx\", x = 0-9, A-F or a-f\n",
                            argv[i]);
                    exit(1);
                }
            }
            else
            {
                use_random_hw_addr = true;
            }
        }
        else if (arg == "-a")
        {
            use_audio = false;
        }
        else if (arg == "-d")
        {
            debug_log = !debug_log;
        }
        else if (arg == "-vp")
        {
            if (!option_has_value(i, argc, arg, argv[i + 1]))
                exit(1);
            video_parser.erase();
            video_parser.append(argv[++i]);
        }
        else if (arg == "-vd")
        {
            if (!option_has_value(i, argc, arg, argv[i + 1]))
                exit(1);
            video_decoder.erase();
            video_decoder.append(argv[++i]);
        }
        else if (arg == "-vc")
        {
            if (!option_has_value(i, argc, arg, argv[i + 1]))
                exit(1);
            video_converter.erase();
            video_converter.append(argv[++i]);
        }
        else if (arg == "-vs")
        {
            if (!option_has_value(i, argc, arg, argv[i + 1]))
                exit(1);
            videosink.erase();
            videosink.append(argv[++i]);
        }
        else if (arg == "-as")
        {
            if (!option_has_value(i, argc, arg, argv[i + 1]))
                exit(1);
            audiosink.erase();
            audiosink.append(argv[++i]);
        }
        else if (arg == "-t")
        {
            fprintf(stderr, "The uxplay option \"-t\" has been removed: it was a workaround for an  Avahi issue.\n");
            fprintf(stderr, "The correct solution is to open network port UDP 5353 in the firewall for mDNS queries\n");
            exit(1);
        }
        else if (arg == "-nc")
        {
            new_window_closing_behavior = false;
        }
        else if (arg == "-avdec")
        {
            video_parser.erase();
            video_parser = "h264parse";
            video_decoder.erase();
            video_decoder = "avdec_h264";
            video_converter.erase();
            video_converter = "videoconvert";
        }
        else if (arg == "-v4l2")
        {
            video_decoder.erase();
            video_decoder = "v4l2h264dec";
            video_converter.erase();
            video_converter = "v4l2convert";
        }
        else if (arg == "-rpi" || arg == "-rpifb" || arg == "-rpigl" || arg == "-rpiwl")
        {
            fprintf(stderr, "*** -rpi* options do not apply to Raspberry Pi model 5, and have been removed\n");
            fprintf(stderr, "     For models 3 and 4, use their equivalents, if needed:\n");
            fprintf(stderr, "     -rpi   was equivalent to \"-v4l2\"\n");
            fprintf(stderr, "     -rpifb was equivalent to \"-v4l2 -vs kmssink\"\n");
            fprintf(stderr, "     -rpigl was equivalent to \"-v4l2 -vs glimagesink\"\n");
            fprintf(stderr, "     -rpiwl was equivalent to \"-v4l2 -vs waylandsink\"\n");
            fprintf(stderr, "     for GStreamer < 1.22, \"-bt709\" may also be needed\n");
            exit(1);
        }
        else if (arg == "-fs")
        {
            fullscreen = true;
        }
        else if (arg == "-FPSdata")
        {
            show_client_FPS_data = true;
        }
        else if (arg == "-reset")
        {
            max_ntp_timeouts = 0;
            if (!get_value(argv[++i], &max_ntp_timeouts))
            {
                fprintf(stderr, "invalid \"-reset %s\"; -reset n must have n >= 0,  default n = %d\n", argv[i], NTP_TIMEOUT_LIMIT);
                exit(1);
            }
        }
        else if (arg == "-vdmp")
        {
            dump_video = true;
            if (i < argc - 1 && *argv[i + 1] != '-')
            {
                unsigned int n = 0;
                if (get_value(argv[++i], &n))
                {
                    if (n == 0)
                    {
                        fprintf(stderr, "invalid \"-vdmp 0 %s\"; -vdmp n  needs a non-zero value of n\n", argv[i]);
                        exit(1);
                    }
                    video_dump_limit = n;
                    if (option_has_value(i, argc, arg, argv[i + 1]))
                    {
                        video_dumpfile_name.erase();
                        video_dumpfile_name.append(argv[++i]);
                    }
                }
                else
                {
                    video_dumpfile_name.erase();
                    video_dumpfile_name.append(argv[i]);
                }
                const char *fn = video_dumpfile_name.c_str();
                if (!file_has_write_access(fn))
                {
                    fprintf(stderr, "%s cannot be written to:\noption \"-vdmp <fn>\" must be to a file with write access\n", fn);
                }
            }
        }
        else if (arg == "-admp")
        {
            dump_audio = true;
            if (i < argc - 1 && *argv[i + 1] != '-')
            {
                unsigned int n = 0;
                if (get_value(argv[++i], &n))
                {
                    if (n == 0)
                    {
                        fprintf(stderr, "invalid \"-admp 0 %s\"; -admp n  needs a non-zero value of n\n", argv[i]);
                    }
                    audio_dump_limit = n;
                    if (option_has_value(i, argc, arg, argv[i + 1]))
                    {
                        audio_dumpfile_name.erase();
                        audio_dumpfile_name.append(argv[++i]);
                    }
                }
                else
                {
                    audio_dumpfile_name.erase();
                    audio_dumpfile_name.append(argv[i]);
                }
                const char *fn = audio_dumpfile_name.c_str();
                if (!file_has_write_access(fn))
                {
                    fprintf(stderr, "%s cannot be written to:\noption \"-admp <fn>\" must be to a file with write access\n", fn);
                }
            }
        }
        else if (arg == "-ca")
        {
            if (option_has_value(i, argc, arg, argv[i + 1]))
            {
                coverart_filename.erase();
                coverart_filename.append(argv[++i]);
                const char *fn = coverart_filename.c_str();
                if (!file_has_write_access(fn))
                {
                    fprintf(stderr, "%s cannot be written to:\noption \"-ca <fn>\" must be to a file with write access\n", fn);
                    exit(1);
                }
            }
            else
            {
                fprintf(stderr, "option -ca must be followed by a filename for cover-art output\n");
                exit(1);
            }
        }
        else if (arg == "-bt709")
        {
            bt709_fix = true;
        }
        else if (arg == "-nohold")
        {
            max_connections = 3;
        }
        else if (arg == "-al")
        {
            int n;
            char *end;
            if (i < argc - 1 && *argv[i + 1] != '-')
            {
                n = (int)(strtof(argv[++i], &end) * SECOND_IN_USECS);
                if (*end == '\0' && n >= 0 && n <= 10 * SECOND_IN_USECS)
                {
                    audiodelay = n;
                    continue;
                }
            }
            fprintf(stderr, "invalid -al %s: value must be a decimal time offset in seconds, range [0,10]\n"
                            "(like 5 or 4.8, which will be converted to a whole number of microseconds)\n",
                    argv[i]);
            exit(1);
        }
        else if (arg == "-pin")
        {
            setup_legacy_pairing = true;
            require_password = true;
            if (i < argc - 1 && *argv[i + 1] != '-')
            {
                unsigned int n = 9999;
                if (!get_value(argv[++i], &n))
                {
                    fprintf(stderr, "invalid \"-pin %s\"; -pin nnnn : max nnnn=9999, (4 digits)\n", argv[i]);
                    exit(1);
                }
                pin = n + 10000;
            }
        }
        else if (arg == "-reg")
        {
            registration_list = true;
            pairing_register.erase();
            if (i < argc - 1 && *argv[i + 1] != '-')
            {
                pairing_register.append(argv[++i]);
                const char *fn = pairing_register.c_str();
                if (!file_has_write_access(fn))
                {
                    fprintf(stderr, "%s cannot be written to:\noption \"-key <fn>\" must be to a file with write access\n", fn);
                    exit(1);
                }
            }
        }
        else if (arg == "-key")
        {
            keyfile.erase();
            if (i < argc - 1 && *argv[i + 1] != '-')
            {
                keyfile.append(argv[++i]);
                const char *fn = keyfile.c_str();
                if (!file_has_write_access(fn))
                {
                    fprintf(stderr, "%s cannot be written to:\noption \"-key <fn>\" must be to a file with write access\n", fn);
                    exit(1);
                }
            }
            else
            {
                //                fprintf(stderr, "option \"-key <fn>\" requires a path <fn> to a file for persistent key storage\n");
                // exit(1);
                keyfile.erase();
                keyfile.append("0");
            }
        }
        else if (arg == "-dacp")
        {
            dacpfile.erase();
            if (i < argc - 1 && *argv[i + 1] != '-')
            {
                dacpfile.append(argv[++i]);
                const char *fn = dacpfile.c_str();
                if (!file_has_write_access(fn))
                {
                    fprintf(stderr, "%s cannot be written to:\noption \"-dacp <fn>\" must be to a file with write access\n", fn);
                    exit(1);
                }
            }
            else
            {
                dacpfile.append(get_homedir());
                dacpfile.append("/.uxplay.dacp");
            }
        }
        else if (arg == "-taper")
        {
            taper_volume = true;
        }
        else if (arg == "-db")
        {
            bool db_bad = true;
            double db1, db2;
            char *start = NULL;
            if (i < argc - 1)
            {
                char *end1, *end2;
                start = argv[i + 1];
                db1 = strtod(start, &end1);
                if (end1 > start && *end1 == ':')
                {
                    db2 = strtod(++end1, &end2);
                    if (*end2 == '\0' && end2 > end1 && db1 < 0 && db1 < db2)
                    {
                        db_bad = false;
                    }
                }
                else if (*end1 == '\0' && end1 > start && db1 < 0)
                {
                    db_bad = false;
                    db2 = 0.0;
                }
            }
            if (db_bad)
            {
                fprintf(stderr, "invalid %s %s: db value must be \"low\" or \"low:high\", low < 0 and high > low are decibel gains\n", argv[i], start);
                exit(1);
            }
            i++;
            db_low = db1;
            db_high = db2;
            printf("db range %f:%f\n", db_low, db_high);
        }
        else
        {
            fprintf(stderr, "unknown option %s, stopping (for help use option \"-h\")\n", argv[i]);
            exit(1);
        }
    }
}

void AirPlayServer::process_metadata(int count, const char *dmap_tag, const unsigned char *metadata, int datalen)
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

int AirPlayServer::parse_dmap_header(const unsigned char *metadata, char *tag, int *len)
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

int AirPlayServer::register_dnssd()
{
    int dnssd_error;
    uint64_t features;

    if ((dnssd_error = dnssd_register_raop(dnssd, raop_port)))
    {
        if (dnssd_error == -65537)
        {
            LOGE("No DNS-SD Server found (DNSServiceRegister call returned kDNSServiceErr_Unknown)");
        }
        else if (dnssd_error == -65548)
        {
            LOGE("DNSServiceRegister call returned kDNSServiceErr_NameConflict");
            LOGI("Is another instance of %s running with the same DeviceID (MAC address) or using same network ports?",
                 DEFAULT_NAME);
            LOGI("Use options -m ... and -p ... to allow multiple instances of %s to run concurrently", DEFAULT_NAME);
        }
        else
        {
            LOGE("dnssd_register_raop failed with error code %d\n"
                 "mDNS Error codes are in range FFFE FF00 (-65792) to FFFE FFFF (-65537) "
                 "(see Apple's dns_sd.h)",
                 dnssd_error);
        }
        return -3;
    }
    if ((dnssd_error = dnssd_register_airplay(dnssd, airplay_port)))
    {
        LOGE("dnssd_register_airplay failed with error code %d\n"
             "mDNS Error codes are in range FFFE FF00 (-65792) to FFFE FFFF (-65537) "
             "(see Apple's dns_sd.h)",
             dnssd_error);
        return -4;
    }

    LOGD("register_dnssd: advertised AirPlay service with \"Features\" code = 0x%X",
         dnssd_get_airplay_features(dnssd));
    return 0;
}

void AirPlayServer::unregister_dnssd()
{
    if (dnssd)
    {
        dnssd_unregister_raop(dnssd);
        dnssd_unregister_airplay(dnssd);
    }
    return;
}

void AirPlayServer::stop_dnssd()
{
    if (dnssd)
    {
        unregister_dnssd();
        dnssd_destroy(dnssd);
        dnssd = NULL;
        return;
    }
}

int AirPlayServer::start_dnssd(std::vector<char> hw_addr, std::string name)
{
    int dnssd_error;
    int require_pw = (require_password ? 1 : 0);
    if (dnssd)
    {
        LOGE("start_dnssd error: dnssd != NULL");
        return 2;
    }
    dnssd = dnssd_init(name.c_str(), strlen(name.c_str()), hw_addr.data(), hw_addr.size(), &dnssd_error, require_pw);
    if (dnssd_error)
    {
        LOGE("Could not initialize dnssd library!: error %d", dnssd_error);
        return 1;
    }

    /* after dnssd starts, reset the default feature set here
    /* (overwrites features set in dnssdint.h */
    /* default: FEATURES_1 = 0x5A7FFEE6, FEATURES_2 = 0 */

    dnssd_set_airplay_features(dnssd, 0, 0); // AirPlay video supported
    dnssd_set_airplay_features(dnssd, 1, 1); // photo supported
    dnssd_set_airplay_features(dnssd, 2, 1); // video protected with FairPlay DRM
    dnssd_set_airplay_features(dnssd, 3, 0); // volume control supported for videos

    dnssd_set_airplay_features(dnssd, 4, 0); // http live streaming (HLS) supported
    dnssd_set_airplay_features(dnssd, 5, 1); // slideshow supported
    dnssd_set_airplay_features(dnssd, 6, 1); //
    dnssd_set_airplay_features(dnssd, 7, 1); // mirroring supported

    dnssd_set_airplay_features(dnssd, 8, 0);  // screen rotation  supported
    dnssd_set_airplay_features(dnssd, 9, 1);  // audio supported
    dnssd_set_airplay_features(dnssd, 10, 1); //
    dnssd_set_airplay_features(dnssd, 11, 1); // audio packet redundancy supported

    dnssd_set_airplay_features(dnssd, 12, 1); // FaiPlay secure auth supported
    dnssd_set_airplay_features(dnssd, 13, 1); // photo preloading  supported
    dnssd_set_airplay_features(dnssd, 14, 1); // Authentication bit 4:  FairPlay authentication
    dnssd_set_airplay_features(dnssd, 15, 1); // Metadata bit 1 support:   Artwork

    dnssd_set_airplay_features(dnssd, 16, 1); // Metadata bit 2 support:  Soundtrack  Progress
    dnssd_set_airplay_features(dnssd, 17, 1); // Metadata bit 0 support:  Text (DAACP) "Now Playing" info.
    dnssd_set_airplay_features(dnssd, 18, 1); // Audio format 1 support:
    dnssd_set_airplay_features(dnssd, 19, 1); // Audio format 2 support: must be set for AirPlay 2 multiroom audio

    dnssd_set_airplay_features(dnssd, 20, 1); // Audio format 3 support: must be set for AirPlay 2 multiroom audio
    dnssd_set_airplay_features(dnssd, 21, 1); // Audio format 4 support:
    dnssd_set_airplay_features(dnssd, 22, 1); // Authentication type 4: FairPlay authentication
    dnssd_set_airplay_features(dnssd, 23, 0); // Authentication type 1: RSA Authentication

    dnssd_set_airplay_features(dnssd, 24, 0); //
    dnssd_set_airplay_features(dnssd, 25, 1); //
    dnssd_set_airplay_features(dnssd, 26, 0); // Has Unified Advertiser info
    dnssd_set_airplay_features(dnssd, 27, 1); // Supports Legacy Pairing

    dnssd_set_airplay_features(dnssd, 28, 1); //
    dnssd_set_airplay_features(dnssd, 29, 0); //
    dnssd_set_airplay_features(dnssd, 30, 1); // RAOP support: with this bit set, the AirTunes service is not required.
    dnssd_set_airplay_features(dnssd, 31, 0); //

    for (int i = 32; i < 64; i++)
    {
        dnssd_set_airplay_features(dnssd, i, 0);
    }

    /*  bits 32-63 are  not used here: see  https://emanualcozzi.net/docs/airplay2/features
    dnssd_set_airplay_features(dnssd, 32, 0); // isCarPlay when ON,; Supports InitialVolume when OFF
    dnssd_set_airplay_features(dnssd, 33, 0); // Supports Air Play Video Play Queue
    dnssd_set_airplay_features(dnssd, 34, 0); // Supports Air Play from cloud (requires that bit 6 is ON)
    dnssd_set_airplay_features(dnssd, 35, 0); // Supports TLS_PSK

    dnssd_set_airplay_features(dnssd, 36, 0); //
    dnssd_set_airplay_features(dnssd, 37, 0); //
    dnssd_set_airplay_features(dnssd, 38, 0); //  Supports Unified Media Control (CoreUtils Pairing and Encryption)
    dnssd_set_airplay_features(dnssd, 39, 0); //

    dnssd_set_airplay_features(dnssd, 40, 0); // Supports Buffered Audio
    dnssd_set_airplay_features(dnssd, 41, 0); // Supports PTP
    dnssd_set_airplay_features(dnssd, 42, 0); // Supports Screen Multi Codec
    dnssd_set_airplay_features(dnssd, 43, 0); // Supports System Pairing

    dnssd_set_airplay_features(dnssd, 44, 0); // is AP Valeria Screen Sender
    dnssd_set_airplay_features(dnssd, 45, 0); //
    dnssd_set_airplay_features(dnssd, 46, 0); // Supports HomeKit Pairing and Access Control
    dnssd_set_airplay_features(dnssd, 47, 0); //

    dnssd_set_airplay_features(dnssd, 48, 0); // Supports CoreUtils Pairing and Encryption
    dnssd_set_airplay_features(dnssd, 49, 0); //
    dnssd_set_airplay_features(dnssd, 50, 0); // Metadata bit 3: "Now Playing" info sent by bplist not DAACP test
    dnssd_set_airplay_features(dnssd, 51, 0); // Supports Unified Pair Setup and MFi Authentication

    dnssd_set_airplay_features(dnssd, 52, 0); // Supports Set Peers Extended Message
    dnssd_set_airplay_features(dnssd, 53, 0); //
    dnssd_set_airplay_features(dnssd, 54, 0); // Supports AP Sync
    dnssd_set_airplay_features(dnssd, 55, 0); // Supports WoL

    dnssd_set_airplay_features(dnssd, 56, 0); // Supports Wol
    dnssd_set_airplay_features(dnssd, 57, 0); //
    dnssd_set_airplay_features(dnssd, 58, 0); // Supports Hangdog Remote Control
    dnssd_set_airplay_features(dnssd, 59, 0); // Supports AudioStreamConnection setup

    dnssd_set_airplay_features(dnssd, 60, 0); // Supports Audo Media Data Control
    dnssd_set_airplay_features(dnssd, 61, 0); // Supports RFC2198 redundancy
    */

    /* bit 27 of Features determines whether the AirPlay2 client-pairing protocol will be used (1) or not (0) */
    dnssd_set_airplay_features(dnssd, 27, (int)setup_legacy_pairing);
    return 0;
}

bool AirPlayServer::check_client(char *deviceid)
{
    bool ret = false;
    int list = allowed_clients.size();
    for (int i = 0; i < list; i++)
    {
        if (!strcmp(deviceid, allowed_clients[i].c_str()))
        {
            ret = true;
            break;
        }
    }
    return ret;
}

bool AirPlayServer::check_blocked_client(char *deviceid)
{
    bool ret = false;
    int list = blocked_clients.size();
    for (int i = 0; i < list; i++)
    {
        if (!strcmp(deviceid, blocked_clients[i].c_str()))
        {
            ret = true;
            break;
        }
    }
    return ret;
}

// Server callbacks
void AirPlayServer::conn_init(void *cls)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    server->open_connections++;
    LOGD("Open connections: %i", server->open_connections);
    // video_renderer_update_background(1);
}

void AirPlayServer::conn_destroy(void *cls)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    // video_renderer_update_background(-1);
    server->open_connections--;
    LOGD("Open connections: %i", server->open_connections);
    if (server->open_connections == 0)
    {
        server->remote_clock_offset = 0;
        if (server->use_audio)
        {
            audio_renderer_stop();
        }
        if (server->dacpfile.length())
        {
            remove(server->dacpfile.c_str());
        }
    }
}

void AirPlayServer::conn_reset(void *cls, int timeouts, bool reset_video)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    LOGI("***ERROR lost connection with client (network problem?)");
    if (timeouts)
    {
        LOGI("   Client no-response limit of %d timeouts (%d seconds) reached:", timeouts, 3 * timeouts);
        LOGI("   Sometimes the network connection may recover after a longer delay:\n"
             "   the default timeout limit n = %d can be changed with the \"-reset n\" option",
             NTP_TIMEOUT_LIMIT);
    }
    printf("reset_video %d\n", (int)reset_video);
    server->close_window = reset_video; /* leave "frozen" window open if reset_video is false */
    raop_stop(server->raop);
    server->reset_loop = true;
}

void AirPlayServer::conn_teardown(void *cls, bool *teardown_96, bool *teardown_110)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    if (*teardown_110 && server->close_window)
    {
        server->reset_loop = true;
    }
}

void AirPlayServer::audio_process(void *cls, raop_ntp_t *ntp, audio_decode_struct *data)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    if (server->dump_audio)
    {
        server->dump_audio_to_file(data->data, data->data_len, (data->data)[0] & 0xf0);
    }
    if (server->use_audio)
    {
        if (!server->remote_clock_offset)
        {
            server->remote_clock_offset = data->ntp_time_local - data->ntp_time_remote;
        }
        data->ntp_time_remote = data->ntp_time_remote + server->remote_clock_offset;
        switch (data->ct)
        {
        case 2:
            if (server->audio_delay_alac)
            {
                data->ntp_time_remote = (uint64_t)((int64_t)data->ntp_time_remote + server->audio_delay_alac);
            }
            break;
        case 4:
        case 8:
            if (server->audio_delay_aac)
            {
                data->ntp_time_remote = (uint64_t)((int64_t)data->ntp_time_remote + server->audio_delay_aac);
            }
            break;
        default:
            break;
        }
        audio_renderer_render_buffer(data->data, &(data->data_len), &(data->seqnum), &(data->ntp_time_remote));
    }
}

void AirPlayServer::video_process(void *cls, raop_ntp_t *ntp, h264_decode_struct *data)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    if (server->dump_video)
    {
        server->dump_video_to_file(data->data, data->data_len);
    }
    if (server->use_video)
    {
        if (!server->remote_clock_offset)
        {
            server->remote_clock_offset = data->ntp_time_local - data->ntp_time_remote;
        }
        data->ntp_time_remote = data->ntp_time_remote + server->remote_clock_offset;
        video_renderer_render_buffer(data->data, &(data->data_len), &(data->nal_count), &(data->ntp_time_remote));
    }
}

void AirPlayServer::video_pause(void *cls)
{
#ifdef GST_124
    return; // pause/resume changes in GStreamer-1.24 break this code
#endif
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    if (server->use_video)
    {
        video_renderer_pause();
    }
}

void AirPlayServer::video_resume(void *cls)
{
#ifdef GST_124
    return; // pause/resume changes in GStreamer-1.24 break this code
#endif
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    if (server->use_video)
    {
        video_renderer_resume();
    }
}

void AirPlayServer::audio_flush(void *cls)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    if (server->use_audio)
    {
        audio_renderer_flush();
    }
}

void AirPlayServer::video_flush(void *cls)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    if (server->use_video)
    {
        video_renderer_flush();
    }
}

void AirPlayServer::audio_set_volume(void *cls, float volume)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    double db, db_flat, frac, gst_volume;
    if (!server->use_audio)
    {
        return;
    }
    /* convert from AirPlay dB  volume in range {-30dB : 0dB}, to GStreamer volume */
    if (volume == -144.0f)
    { /* AirPlay "mute" signal */
        frac = 0.0;
    }
    else if (volume < -30.0f)
    {
        LOGE(" invalid AirPlay volume %f", volume);
        frac = 0.0;
    }
    else if (volume > 0.0f)
    {
        LOGE(" invalid AirPlay volume %f", volume);
        frac = 1.0;
    }
    else if (volume == -30.0f)
    {
        frac = 0.0;
    }
    else if (volume == 0.0f)
    {
        frac = 1.0;
    }
    else
    {
        frac = (double)((30.0f + volume) / 30.0f);
        frac = (frac > 1.0) ? 1.0 : frac;
    }

    /* frac is length of volume slider as fraction of max length */
    /* also (steps/16) where steps is number of discrete steps above mute (16 = full volume) */
    if (frac == 0.0)
    {
        gst_volume = 0.0;
    }
    else
    {
        /* flat rescaling of decibel range from {-30dB : 0dB} to {db_low : db_high} */
        db_flat = server->db_low + (server->db_high - server->db_low) * frac;
        if (server->taper_volume)
        {
            /* taper the volume reduction by the (rescaled) Airplay {-30:0} range so each reduction of
             * the remaining slider length by 50% reduces the perceived volume by 50% (-10dB gain)
             * (This is the "dasl-tapering" scheme offered by shairport-sync) */
            db = server->db_high + 10.0 * (log10(frac) / log10(2.0));
            db = (db > db_flat) ? db : db_flat;
        }
        else
        {
            db = db_flat;
        }
        /* conversion from (gain) decibels to GStreamer's linear volume scale */
        gst_volume = pow(10.0, 0.05 * db);
    }
    audio_renderer_set_volume(gst_volume);
}

void AirPlayServer::audio_get_format(void *cls, unsigned char *ct, unsigned short *spf, bool *usingScreen, bool *isMedia, uint64_t *audioFormat)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    unsigned char type;
    LOGI("ct=%d spf=%d usingScreen=%d isMedia=%d  audioFormat=0x%lx", *ct, *spf, *usingScreen, *isMedia, (unsigned long)*audioFormat);
    switch (*ct)
    {
    case 2:
        type = 0x20;
        break;
    case 8:
        type = 0x80;
        break;
    default:
        type = 0x10;
        break;
    }
    if (server->audio_dumpfile && type != server->audio_type)
    {
        fclose(server->audio_dumpfile);
        server->audio_dumpfile = NULL;
    }
    server->audio_type = type;

    if (server->use_audio)
    {
        audio_renderer_start(ct);
    }

    if (server->coverart_filename.length())
    {
        write_coverart(server->coverart_filename.c_str(), (const void *)empty_image, sizeof(empty_image));
    }
}

void AirPlayServer::video_report_size(void *cls, float *width_source, float *height_source, float *width, float *height)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    if (server->use_video)
    {
        video_renderer_size(width_source, height_source, width, height);
    }
}

void AirPlayServer::audio_set_coverart(void *cls, const void *buffer, int buflen)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    if (buffer && server->coverart_filename.length())
    {
        write_coverart(server->coverart_filename.c_str(), buffer, buflen);
        LOGI("coverart size %d written to %s", buflen, server->coverart_filename.c_str());
    }
}

extern "C" void AirPlayServer::audio_set_progress(void *cls, unsigned int start, unsigned int curr, unsigned int end)
{
    int duration = (int)(end - start) / 44100;
    int position = (int)(curr - start) / 44100;
    int remain = duration - position;
    printf("audio progress (min:sec): %d:%2.2d; remaining: %d:%2.2d; track length %d:%2.2d\n",
           position / 60, position % 60, remain / 60, remain % 60, duration / 60, duration % 60);
}

extern "C" void AirPlayServer::display_pin(void *cls, char *pin)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    int margin = 10;
    int spacing = 3;
    char *image = server->create_pin_display(pin, margin, spacing);
    if (!image)
    {
        LOGE("create_pin_display could not create pin image, pin = %s", pin);
    }
    else
    {
        LOGI("%s\n", image);
        free(image);
    }
}
void AirPlayServer::audio_set_metadata(void *cls, const void *buffer, int buflen)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    char dmap_tag[5] = {0x0};
    const unsigned char *metadata = (const unsigned char *)buffer;
    int datalen;
    int count = 0;

    printf("==============Audio Metadata=============\n");

    if (buflen < 8)
    {
        LOGE("received invalid metadata, length %d < 8", buflen);
        return;
    }
    else if (server->parse_dmap_header(metadata, dmap_tag, &datalen))
    {
        LOGE("received invalid metadata, tag [%s]  datalen %d", dmap_tag, datalen);
        return;
    }
    metadata += 8;
    buflen -= 8;

    if (strcmp(dmap_tag, "mlit") != 0 || datalen != buflen)
    {
        LOGE("received metadata with tag %s, but is not a DMAP listingitem, or datalen = %d !=  buflen %d",
             dmap_tag, datalen, buflen);
        return;
    }
    while (buflen >= 8)
    {
        count++;
        if (server->parse_dmap_header(metadata, dmap_tag, &datalen))
        {
            LOGE("received metadata with invalid DMAP header:  tag = [%s],  datalen = %d", dmap_tag, datalen);
            return;
        }
        metadata += 8;
        buflen -= 8;
        server->process_metadata(count, (const char *)dmap_tag, metadata, datalen);
        metadata += datalen;
        buflen -= datalen;
    }
    if (buflen != 0)
    {
        LOGE("%d bytes of metadata were not processed", buflen);
    }
}

void AirPlayServer::register_client(void *cls, const char *device_id, const char *client_pk, const char *client_name)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    if (!server->registration_list)
    {
        /* we are not maintaining a list of registered clients */
        return;
    }
    LOGI("registered new client: %s DeviceID = %s PK = \n%s", client_name, device_id, client_pk);
    server->registered_keys.push_back(client_pk);
    if (strlen(server->pairing_register.c_str()))
    {
        FILE *fp = fopen(server->pairing_register.c_str(), "a");
        if (fp)
        {
            fprintf(fp, "%s,%s,%s\n", client_pk, device_id, client_name);
            fclose(fp);
        }
    }
}

bool AirPlayServer::check_register(void *cls, const char *client_pk)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    if (!server->registration_list)
    {
        /* we are not maintaining a list of registered clients */
        return true;
    }
    LOGD("check returning client's pairing registration");
    std::string pk = client_pk;
    if (std::find(server->registered_keys.rbegin(), server->registered_keys.rend(), pk) != server->registered_keys.rend())
    {
        LOGD("registration found: PK=%s", client_pk);
        return true;
    }
    else
    {
        LOGE("returning client's pairing registration not found: PK=%s", client_pk);
        return false;
    }
}

extern "C" void log_callback(void *cls, int level, const char *msg)
{
    switch (level)
    {
    case LOGGER_DEBUG:
    {
        LOGD("%s", msg);
        break;
    }
    case LOGGER_WARNING:
    {
        LOGW("%s", msg);
        break;
    }
    case LOGGER_INFO:
    {
        LOGI("%s", msg);
        break;
    }
    case LOGGER_ERR:
    {
        LOGE("%s", msg);
        break;
    }
    default:
        break;
    }
}

extern "C" void AirPlayServer::export_dacp(void *cls, const char *active_remote, const char *dacp_id)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    if (server->dacpfile.length())
    {
        FILE *fp = fopen(server->dacpfile.c_str(), "w");
        if (fp)
        {
            fprintf(fp, "%s\n%s\n", dacp_id, active_remote);
            fclose(fp);
        }
        else
        {
            LOGE("failed to open DACP export file \"%s\"", server->dacpfile.c_str());
        }
    }
}

extern "C" void AirPlayServer::report_client_request(void *cls, char *deviceid, char *model, char *name, bool *admit)
{
    AirPlayServer *server = static_cast<AirPlayServer *>(cls);
    LOGI("connection request from %s (%s) with deviceID = %s\n", name, model, deviceid);
    if (server->restrict_clients)
    {
        *admit = server->check_client(deviceid);
        if (*admit == false)
        {
            LOGI("client connections have been restricted to those with listed deviceID,\nuse \"-allow %s\" to allow this client to connect.\n",
                 deviceid);
        }
    }
    else
    {
        *admit = true;
    }
    if (server->check_blocked_client(deviceid))
    {
        *admit = false;
        LOGI("*** attempt to connect by blocked client (clientID %s): DENIED\n", deviceid);
    }
}

int AirPlayServer::start_raop_server(unsigned short display[5], unsigned short tcp[3], unsigned short udp[3], bool debug_log)
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

void AirPlayServer::stop_raop_server()
{
    if (raop)
    {
        raop_destroy(raop);
        raop = NULL;
    }
    return;
}

void AirPlayServer::read_config_file(const char *filename, const char *uxplay_name)
{
    std::string config_file = filename;
    std::string option_char = "-";
    std::vector<std::string> options;
    options.push_back(uxplay_name);
    std::ifstream file(config_file);
    if (file.is_open())
    {
        fprintf(stdout, "UxPlay: reading configuration from  %s\n", config_file.c_str());
        std::string line;
        while (std::getline(file, line))
        {
            if (line[0] == '#')
                continue;
            //  first process line into separate option items with '\0' as delimiter
            bool is_part_of_item, in_quotes;
            char endchar;
            is_part_of_item = false;
            for (int i = 0; i < (int)line.size(); i++)
            {
                if (is_part_of_item == false)
                {
                    if (line[i] == ' ')
                    {
                        line[i] = '\0';
                    }
                    else
                    {
                        // start of new item
                        is_part_of_item = true;
                        switch (line[i])
                        {
                        case '\'':
                        case '\"':
                            endchar = line[i];
                            line[i] = '\0';
                            in_quotes = true;
                            break;
                        default:
                            in_quotes = false;
                            endchar = ' ';
                            break;
                        }
                    }
                }
                else
                {
                    /* previous character was inside this item */
                    if (line[i] == endchar)
                    {
                        if (in_quotes)
                        {
                            /* cases where endchar is inside quoted item */
                            if (i > 0 && line[i - 1] == '\\')
                                continue;
                            if (i + 1 < (int)line.size() && line[i + 1] != ' ')
                                continue;
                        }
                        line[i] = '\0';
                        is_part_of_item = false;
                    }
                }
            }

            // now tokenize the processed line
            std::istringstream iss(line);
            std::string token;
            bool first = true;
            while (std::getline(iss, token, '\0'))
            {
                if (token.size() > 0)
                {
                    if (first)
                    {
                        options.push_back(option_char + token.c_str());
                        first = false;
                    }
                    else
                    {
                        options.push_back(token.c_str());
                    }
                }
            }
        }
        file.close();
    }
    else
    {
        fprintf(stderr, "UxPlay: failed to open configuration file at %s\n", config_file.c_str());
    }
    if (options.size() > 1)
    {
        int argc = options.size();
        char **argv = (char **)malloc(sizeof(char *) * argc);
        for (int i = 0; i < argc; i++)
        {
            argv[i] = (char *)options[i].c_str();
        }
        parse_arguments(argc, argv);
        free(argv);
    }
}

void AirPlayServer::initialize(int argc, char *argv[])
{
    std::string config_file = find_uxplay_config_file();
    if (!config_file.empty())
    {
        read_config_file(config_file.c_str(), argv[0]);
    }
    parse_arguments(argc, argv);
    log_level = debug_log ? LOGGER_DEBUG : LOGGER_INFO;
}

void AirPlayServer::run(int argc, char *argv[])
{
    raop_callbacks_t raop_cbs;
    memset(&raop_cbs, 0, sizeof(raop_cbs));
    raop_cbs.conn_init = &AirPlayServer::conn_init;
    raop_cbs.conn_destroy = &AirPlayServer::conn_destroy;
    raop_cbs.conn_reset = &AirPlayServer::conn_reset;
    raop_cbs.conn_teardown = &AirPlayServer::conn_teardown;
    raop_cbs.audio_process = &AirPlayServer::audio_process;
    raop_cbs.video_process = &AirPlayServer::video_process;
    raop_cbs.audio_flush = &AirPlayServer::audio_flush;
    raop_cbs.video_flush = &AirPlayServer::video_flush;
    raop_cbs.video_pause = &AirPlayServer::video_pause;
    raop_cbs.video_resume = &AirPlayServer::video_resume;
    raop_cbs.audio_set_volume = &AirPlayServer::audio_set_volume;
    raop_cbs.audio_get_format = &AirPlayServer::audio_get_format;
    raop_cbs.video_report_size = &AirPlayServer::video_report_size;
    raop_cbs.audio_set_metadata = &AirPlayServer::audio_set_metadata;
    raop_cbs.audio_set_coverart = &AirPlayServer::audio_set_coverart;
    raop_cbs.audio_set_progress = &AirPlayServer::audio_set_progress;
    raop_cbs.report_client_request = &AirPlayServer::report_client_request;
    raop_cbs.display_pin = &AirPlayServer::display_pin;
    raop_cbs.register_client = &AirPlayServer::register_client;
    raop_cbs.check_register = &AirPlayServer::check_register;
    raop_cbs.export_dacp = &AirPlayServer::export_dacp;
    raop_cbs.cls = this;

    raop = raop_init(&raop_cbs);
    if (raop == NULL)
    {
        LOGE("Error initializing raop!");
    }
    raop_set_log_callback(raop, log_callback, NULL);
    raop_set_log_level(raop, log_level);
    /* set max number of connections = 2 to protect against capture by new client */
    if (raop_init2(raop, max_connections, mac_address.c_str(), keyfile.c_str()))
    {
        LOGE("Error initializing raop (2)!");
        free(raop);
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
    }
#ifdef GST_MACOS
/* workaround for GStreamer >= 1.22 "Official Builds" on macOS */
#include <TargetConditionals.h>
#include <gst/gstmacos.h>

    printf("*=== Using gst_macos_main wrapper for GStreamer >= 1.22 on macOS ===*\n");
    return gst_macos_main((GstMainFunc)real_main, argc, argv, NULL);

#endif

#ifdef SUPPRESS_AVAHI_COMPAT_WARNING
    // suppress avahi_compat nag message.  avahi emits a "nag" warning (once)
    // if  getenv("AVAHI_COMPAT_NOWARN") returns null.
    char avahi_compat_nowarn[] = "AVAHI_COMPAT_NOWARN=1";
    if (!getenv("AVAHI_COMPAT_NOWARN"))
        putenv(avahi_compat_nowarn);
#endif

    config_file = find_uxplay_config_file();
    if (config_file.length())
    {
        read_config_file(config_file.c_str(), argv[0]);
    }
    parse_arguments(argc, argv);

    log_level = (debug_log ? LOGGER_DEBUG : LOGGER_INFO);

#ifdef _WIN32 /*  use utf-8 terminal output; don't buffer stdout in WIN32 when debug_log = false */
    SetConsoleOutputCP(CP_UTF8);
    if (!debug_log)
    {
        setbuf(stdout, NULL);
    }
#endif
    if (audiosink == "0")
    {
        use_audio = false;
        dump_audio = false;
    }
    if (dump_video)
    {
        if (video_dump_limit > 0)
        {
            printf("dump video using \"-vdmp %d %s\"\n", video_dump_limit, video_dumpfile_name.c_str());
        }
        else
        {
            printf("dump video using \"-vdmp %s\"\n", video_dumpfile_name.c_str());
        }
    }
    if (dump_audio)
    {
        if (audio_dump_limit > 0)
        {
            printf("dump audio using \"-admp %d %s\"\n", audio_dump_limit, audio_dumpfile_name.c_str());
        }
        else
        {
            printf("dump audio using \"-admp %s\"\n", audio_dumpfile_name.c_str());
        }
    }

#if __APPLE__
    /* force use of -nc option on macOS */
    LOGI("macOS detected: use -nc option as workaround for GStreamer problem");
    new_window_closing_behavior = false;
#endif

    if (videosink == "0")
    {
        use_video = false;
        videosink.erase();
        videosink.append("fakesink");
        LOGI("video_disabled");
        display[3] = 1; /* set fps to 1 frame per sec when no video will be shown */
    }

    if (fullscreen && use_video)
    {
        if (videosink == "waylandsink" || videosink == "vaapisink")
        {
            videosink.append(" fullscreen=true");
        }
    }

    if (videosink == "d3d11videosink" && use_video)
    {
        if (fullscreen)
        {
            videosink.append(" fullscreen-toggle-mode=property fullscreen=true");
        }
        else
        {
            videosink.append(" fullscreen-toggle-mode=alt-enter");
        }
        LOGI("d3d11videosink is being used with option fullscreen-toggle-mode=alt-enter\n"
             "Use Alt-Enter key combination to toggle into/out of full-screen mode");
    }

    if (bt709_fix && use_video)
    {
        video_parser.append(" ! ");
        video_parser.append(BT709_FIX);
    }

    if (require_password && registration_list)
    {
        if (pairing_register == "")
        {
            const char *homedir = get_homedir();
            if (homedir)
            {
                pairing_register = homedir;
                pairing_register.append("/.uxplay.register");
            }
        }
    }

    /* read in public keys that were previously registered with pair-setup-pin */
    if (require_password && registration_list && strlen(pairing_register.c_str()))
    {
        size_t len = 0;
        std::string key;
        int clients = 0;
        std::ifstream file(pairing_register);
        if (file.is_open())
        {
            std::string line;
            while (std::getline(file, line))
            {
                /*32 bytes pk -> base64 -> strlen(pk64) = 44 chars = line[0:43]; add '\0' at line[44] */
                line[44] = '\0';
                std::string pk = line.c_str();
                registered_keys.push_back(key.assign(pk));
                clients++;
            }
            if (clients)
            {
                LOGI("Register %s lists %d pin-registered clients", pairing_register.c_str(), clients);
            }
            file.close();
        }
    }

    if (require_password && keyfile == "0")
    {
        const char *homedir = get_homedir();
        if (homedir)
        {
            keyfile.erase();
            keyfile = homedir;
            keyfile.append("/.uxplay.pem");
        }
        else
        {
            LOGE("could not determine $HOME: public key wiil not be saved, and so will not be persistent");
        }
    }

    if (keyfile != "")
    {
        LOGI("public key storage (for persistence) is in %s", keyfile.c_str());
    }

    if (do_append_hostname)
    {
        append_hostname(server_name);
    }

    if (!gstreamer_init())
    {
        LOGE("stopping");
    }

    render_logger = logger_init();
    logger_set_callback(render_logger, log_callback, NULL);
    logger_set_level(render_logger, log_level);

    if (use_audio)
    {
        audio_renderer_init(render_logger, audiosink.c_str(), &audio_sync, &video_sync);
    }
    else
    {
        LOGI("audio_disabled");
    }

    if (use_video)
    {
        video_renderer_init(render_logger, server_name.c_str(), videoflip, video_parser.c_str(),
                            video_decoder.c_str(), video_converter.c_str(), videosink.c_str(), &fullscreen, &video_sync);
        video_renderer_start();
    }

    if (udp[0])
    {
        LOGI("using network ports UDP %d %d %d TCP %d %d %d", udp[0], udp[1], udp[2], tcp[0], tcp[1], tcp[2]);
    }

    if (!use_random_hw_addr)
    {
        if (strlen(mac_address.c_str()) == 0)
        {
            mac_address = find_mac();
            LOGI("using system MAC address %s", mac_address.c_str());
        }
        else
        {
            LOGI("using user-set MAC address %s", mac_address.c_str());
        }
    }
    if (mac_address.empty())
    {
        srand(time(NULL) * getpid());
        mac_address = random_mac();
        LOGI("using randomly-generated MAC address %s", mac_address.c_str());
    }
    parse_hw_addr(mac_address, server_hw_addr);

    if (coverart_filename.length())
    {
        LOGI("any AirPlay audio cover-art will be written to file  %s", coverart_filename.c_str());
        write_coverart(coverart_filename.c_str(), (const void *)empty_image, sizeof(empty_image));
    }
}

void AirPlayServer::stop()
{
    if (use_audio)
    {
        audio_renderer_destroy();
    }
    if (use_video)
    {
        video_renderer_destroy();
    }
    logger_destroy(render_logger);
    render_logger = NULL;
    if (audio_dumpfile)
    {
        fclose(audio_dumpfile);
    }
    if (video_dumpfile)
    {
        fwrite(mark, 1, sizeof(mark), video_dumpfile);
        fclose(video_dumpfile);
    }
    if (coverart_filename.length())
    {
        remove(coverart_filename.c_str());
    }
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

void AirPlayServer::restart()
{
    if (start_dnssd(server_hw_addr, server_name))
    {
        stop();
    }
    if (start_raop_server(display, tcp, udp, debug_log))
    {
        stop_dnssd();
        stop();
    }
    if (register_dnssd())
    {
        stop_raop_server();
        stop_dnssd();
        stop();
    }
}

void AirPlayServer::reconnect()
{
    compression_type = 0;
    close_window = new_window_closing_behavior;
    if (relaunch_video || reset_loop)
    {
        if (reset_loop)
        {
            reset_loop = false;
        }
        else
        {
            raop_stop(raop);
        }
        if (use_audio)
            audio_renderer_stop();
        if (use_video && close_window)
        {
            video_renderer_destroy();
            video_renderer_init(render_logger, server_name.c_str(), videoflip, video_parser.c_str(),
                                video_decoder.c_str(), video_converter.c_str(), videosink.c_str(), &fullscreen,
                                &video_sync);
            video_renderer_start();
        }
        if (relaunch_video)
        {
            unsigned short port = raop_get_port(raop);
            raop_start(raop, &port);
            raop_set_port(raop, port);
            reconnect();
        }
        else
        {
            LOGI("Re-launching RAOP server...");
            stop_raop_server();
            stop_dnssd();
            restart();
        }
    }
    else
    {
        LOGI("Stopping...");
        stop_raop_server();
        stop_dnssd();
    }
}