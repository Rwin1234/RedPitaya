#include <iostream>
#include <getopt.h>
#include <vector>
#include <cstring>
#include <algorithm>

#include "options.h"

static struct option long_options_broadcast[] = {
        /* These options set a flag. */
        {"detect",       no_argument,       0, 'd'},
        {"port",         required_argument, 0, 'p'},
        {"timeout",      required_argument, 0, 't'},
        {0, 0, 0, 0}
};

static constexpr char optstring_broadcast[] = "dp:t:";

static struct option long_options_config[] = {
        /* These options set a flag. */
        {"config",       no_argument,       0, 'c'},
        {"hosts",        required_argument, 0, 'h'},
        {"port",         required_argument, 0, 'p'},
        {"get",          required_argument, 0, 'g'},
        {"set",          required_argument, 0, 's'},
        {"config_file",  required_argument, 0, 'f'},
        {"verbose",      no_argument,       0, 'v'},
        {0, 0, 0, 0}
};

static constexpr char optstring_config[] = "ch:p:g:s:f:v";

static struct option long_options_remote[] = {
        /* These options set a flag. */
        {"remote",       no_argument,       0, 'r'},
        {"hosts",        required_argument, 0, 'h'},
        {"port",         required_argument, 0, 'p'},
        {"mode",         required_argument, 0, 'm'},
        {"timeout",      required_argument, 0, 't'},
        {"verbose",      no_argument,       0, 'v'},
        {0, 0, 0, 0}
};

static constexpr char optstring_remote[] = "rh:p:m:t:v";

std::vector<std::string> split(const std::string& s, char seperator)
{
    std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );

        output.push_back(substring);

        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

    return output;
}

int get_int(int *value, const char *str,const char *message,int max_value)
{
    try
    {
        if ( str == NULL || *str == '\0' )
            throw std::invalid_argument("null string");
        auto checkstr = str;
        while(*checkstr)
        {
            if ( *checkstr < '0' || *checkstr > '9' )
                throw std::invalid_argument("invalid input string");
            ++checkstr;
        }
        *value = std::stoi(str);
    }
    catch (...)
    {
        fprintf(stderr, "%s: %s\n",message, str);
        return -1;
    }
    if (*value > max_value){
        fprintf(stderr, "%s: %s\n",message, str);
        return -1;
    }
    return 0;
}

auto check_ip( const std::string &value ) -> bool {
    int octet[4], l;
    int r = sscanf (value.c_str(), "%d.%d.%d.%d%n", octet, octet + 1, octet + 2, octet + 3, &l);
    if (r != 4 || l != (int)value.length ()) {
        return false;
    }
    for (int i = 0; i < 4; i++) {
        if (octet[i] < 0 || octet[i] >= 256) {
            return false;
        }
    }
    return true;
}

void remove_duplicates(std::vector<std::string>& vec)
{
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}


/** Print usage information */
auto ClientOpt::usage(char const* progName) -> void{
#ifdef _WIN32
    auto arr = split(std::string(progName),'\\');
#else
    auto arr = split(std::string(progName),'/');
#endif

    std::string name = "";
    if (arr.size() > 0)
        name = arr[arr.size()-1];
    const char *format =
            "\n"
            "Usage: %s\n"
            "\n"
            "Detect Mode:\n"
            "\tThis mode allows you to determine the IP addresses that are in the network in streaming mode. By default, the search takes 5 seconds.\n"
            "\n"
            "\tOptions:\n"
            "\t\t%s -d [-p PORT] [-t SEC]\n"
            "\t\t%s --detect [--port=PORT] [--timeout=SEC]\n"
            "\n"
            "\t\t--detect            -d           Enable broadcast search.\n"
            "\t\t--port=PORT         -p PORT      Port for broadcast (Default: 8902).\n"
            "\t\t--timeout=SEC       -t SEC       Timeout(Default: 5 sec).\n"
            "\n"
            "Configuration Mode:\n"
            "\tThis mode allows you to get or set the configuration on the boards.\n"
            "\n"
            "\tOptions:\n"
            "\t\t%s -c -h IPs [-p PORT] -g V|VV|F [-v]\n"
            "\t\t%s -c -h IPs [-p PORT] -s M|F [-f FILE] [-v]\n"
            "\t\t%s --config --hosts=IPs [--port=PORT] --get=V|VV|F [--verbose]\n"
            "\t\t%s --config --hosts=IPs [--port=PORT] --set=M|F [--config_file=FILE] [--verbose]\n"
            "\n"
            "\t\t--config            -c           Enable config mode.\n"
            "\t\t--hosts=IP;..       -h IP;...    You can specify one or more board IP addresses through a separator - ','\n"
            "\t\t                                 Example: --hosts=127.0.0.1 or --hosts=127.0.0.1,127.0.0.2\n"
            "\t\t                                           -p 127.0.0.1     or  -p 127.0.0.1,127.0.0.2,127.0.0.3\n"
            "\t\t--port=PORT         -p PORT      Port for configuration server (Default: 8901).\n"
            "\t\t--get=V|V1|VV|F     -g V|V1|VV|F Requests configurations from all boards.\n"
            "\t\t                                 Keys: V  = Displays on the screen in json format.\n"
            "\t\t                                       V1 = Displays on the screen in json format (only data).\n"
            "\t\t                                       VV = Displays on the screen in a format with decoding values.\n"
            "\t\t                                       F  = Saves to a config files.\n"
            "\t\t--set=M|F           -s M|F       Sets configurations for all boards.\n"
            "\t\t                                 Keys: M  = Sets values only to memory without saving to file.\n"
            "\t\t                                       F  = Sets configuration and saves to file on remote boards.\n"
            "\t\t--config_file=FILE  -f FILE      Configuration file for setting values on boards (Default: config.json).\n"
            "\t\t--verbose           -v           Displays service information.\n"
            "\n"
            "Remote control Mode:\n"
            "\tThis mode allows you to control streaming as a client.\n"
            "\n"
            "\tOptions:\n"
            "\t\t%s -r -h IPs [-p PORT] -m start|stop|start_stop [-w] [-t SEC] [-v]\n"
            "\t\t%s --remote --hosts=IPs [--port=PORT] --mode=start|stop|start_stop [--timeout=SEC] [--verbose]\n"
            "\n"
            "\t\t--remote            -r           Enable remote control mode.\n"
            "\t\t--hosts=IP;..       -h IP;...    You can specify one or more board IP addresses through a separator - ','\n"
            "\t\t                                 Example: --hosts=127.0.0.1 or --hosts=127.0.0.1,127.0.0.2\n"
            "\t\t                                           -p 127.0.0.1     or  -p 127.0.0.1,127.0.0.2,127.0.0.3\n"
            "\t\t--port=PORT         -p PORT      Port for configuration server (Default: 8901).\n"
            "\t\t--mode=MODE         -m MODE      Commands for managing servers.\n"
            "\t\t                                 Keys: start = Starts the server.\n"
            "\t\t                                       stop = Stop the server.\n"
            "\t\t                                       start_stop = Sends a start command at the end of the timeout sends a stop command.\n"
            "\t\t--timeout=MSEC      -t MSEC      Timeout (Default: 1000 ms). Used only in conjunction with the start_stop command.\n"
            "\t\t--verbose           -v           Displays service information.\n"
            "\n"
            "\tStreaming Mode:\n"
            "\t\tThis mode allows you to control streaming as a client, and also captures data in network streaming mode.\n"
            "\n"
            "\n";
    auto n = name.c_str();
    fprintf( stderr, format, n,n,n,n,n,n,n,n,n);
}

auto ClientOpt::parse(int argc, char* argv[]) -> ClientOpt::Options{
    Options opt;
    opt.mode = Mode::ERROR;
    if (argc < 2) return opt;
    /* getopt_long stores the option index here. */
    int option_index = 0;
    int ch = -1;
    if ((strcmp(argv[1],"-d") == 0) || (strcmp(argv[1],"--detect") == 0)) {
        while ((ch = getopt_long(argc, argv, optstring_broadcast, long_options_broadcast, &option_index)) != -1) {
            switch (ch) {

                case 'd':
                    opt.mode = Mode::SEARCH;
                    break;

                case 'p': {
                    int port = 0;
                    if (get_int(&port, optarg, "Error get port number", 65535) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.port = optarg;
                    break;
                }

                case 't': {
                    int t_out = 0;
                    if (get_int(&t_out, optarg, "Error get timout", 100000) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.timeout = t_out;
                    break;
                }
                default: {
                    if (opt.mode == Mode::SEARCH) {
                        fprintf(stderr, "[ERROR] Unknown parameter\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
        if (opt.mode != Mode::ERROR){
            return opt;
        }
    }

    option_index = 0;
    ch = -1;
    if ((strcmp(argv[1],"-c") == 0) || (strcmp(argv[1],"--config") == 0)) {
        while ((ch = getopt_long(argc, argv, optstring_config, long_options_config, &option_index)) != -1) {
            switch (ch) {

                case 'c':
                    opt.mode = Mode::CONFIG;
                    break;

                case 'v':
                    opt.verbous = true;
                    break;

                case 'p': {
                    int port = 0;
                    if (get_int(&port, optarg, "Error get port number", 65535) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.port = optarg;
                    break;
                }

                case 'h': {
                    opt.hosts = split(optarg, ',');
                    remove_duplicates(opt.hosts);
                    if (opt.hosts.size() > 0) {
                        for (auto &s:opt.hosts) {
                            if (!check_ip(s)) {
                                fprintf(stderr, "Error parse ip address: %s\n", s.c_str());
                                opt.mode = Mode::ERROR_PARAM;
                                return opt;
                            }
                        }
                    } else {
                        fprintf(stderr, "Error parse ip address: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'g': {
                    if (strcmp(optarg, "V") == 0) {
                        opt.conf_get = ConfGet::VERBOUS_JSON;
                    } else if (strcmp(optarg, "V1") == 0) {
                        opt.conf_get = ConfGet::VERBOUS_JSON_DATA;
                    } else if (strcmp(optarg, "VV") == 0) {
                        opt.conf_get = ConfGet::VERBOUS;
                    } else if (strcmp(optarg, "F") == 0) {
                        opt.conf_get = ConfGet::FILE;
                    } else {
                        fprintf(stderr, "Error key --get: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 't': {
                    int t_out = 0;
                    if (get_int(&t_out, optarg, "Error get timeout", 100000) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.timeout = t_out;
                    break;
                }

                default: {
                    if (opt.mode == Mode::CONFIG) {
                        fprintf(stderr, "[ERROR] Unknown parameter\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }

        if (opt.mode != Mode::ERROR){
            if (opt.mode == Mode::CONFIG && (opt.hosts.size() == 0 || (opt.conf_get == ConfGet::NONE && opt.conf_set == ConfSet::NONE))){
                fprintf(stderr,"[ERROR] Missing required key in configuration mode\n");
                exit( EXIT_FAILURE );
            }
            return opt;
        }
    }

    option_index = 0;
    ch = -1;
    if ((strcmp(argv[1],"-r") == 0) || (strcmp(argv[1],"--remote") == 0)) {
        while ((ch = getopt_long(argc, argv, optstring_remote, long_options_remote, &option_index)) != -1) {
            switch (ch) {

                case 'r':
                    opt.mode = Mode::REMOTE;
                    break;

                case 'v':
                    opt.verbous = true;
                    break;

                case 'p': {
                    int port = 0;
                    if (get_int(&port, optarg, "Error get port number", 65535) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.port = optarg;
                    break;
                }

                case 'h': {
                    opt.hosts = split(optarg, ',');
                    remove_duplicates(opt.hosts);
                    if (opt.hosts.size() > 0) {
                        for (auto &s:opt.hosts) {
                            if (!check_ip(s)) {
                                fprintf(stderr, "Error parse ip address: %s\n", s.c_str());
                                opt.mode = Mode::ERROR_PARAM;
                                return opt;
                            }
                        }
                    } else {
                        fprintf(stderr, "Error parse ip address: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 'm': {
                    if (strcmp(optarg, "start") == 0) {
                        opt.remote_mode = RemoteMode::START;
                    } else if (strcmp(optarg, "stop") == 0) {
                        opt.remote_mode = RemoteMode::STOP;
                    } else if (strcmp(optarg, "start_stop") == 0) {
                        opt.remote_mode = RemoteMode::START_STOP;
                    } else {
                        fprintf(stderr, "Error key --mode: %s\n", optarg);
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    break;
                }

                case 't': {
                    int t_out = 0;
                    if (get_int(&t_out, optarg, "Error get timeout", 0xFFFFFFF) != 0) {
                        opt.mode = Mode::ERROR_PARAM;
                        return opt;
                    }
                    opt.timeout = t_out;
                    break;
                }

                default: {
                    if (opt.mode == Mode::CONFIG) {
                        fprintf(stderr, "[ERROR] Unknown parameter\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }

        if (opt.mode != Mode::ERROR){
            if (opt.mode == Mode::REMOTE && opt.remote_mode ==RemoteMode::NONE){
                fprintf(stderr,"[ERROR] Missing required key in remote mode\n");
                exit( EXIT_FAILURE );
            }
            return opt;
        }
    }

    return opt;
}
