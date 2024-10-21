#include <libindi/baseclient.h>
#include <libindi/basedevice.h>
#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <string>
#include <mutex>
#include <thread>

static FILE *error_stream = NULL;
#define LOG(...) do { if( error_stream) { \
    struct timeval tv; gettimeofday(&tv,nullptr); \
    int now = tv.tv_sec  % (24*3600); \
    fprintf( error_stream,"%02d:%02d:%02d.%03d  ",now/3600,(now/60)%60,now%60,int(tv.tv_usec / 1000)); \
    fprintf( error_stream,__VA_ARGS__); fflush(error_stream); \
}} while(0)


typedef std::unique_lock<std::recursive_mutex> guard_type;
static void log_property(char const *src,INDI::Property const &p) 
{
    if(!error_stream)
        return;
    char const *perms[]={"ro","wo","rw"};
    LOG("%3s %s:%s (%s) %s type %s\n",src,p.getDeviceName(), p.getName(), p.getStateAsString(), perms[p.getPermission()], p.getTypeAsString());
    switch(p.getType()) {
        case INDI_NUMBER:
            {
                INDI::PropertyNumber v(p);
                for(size_t i=0;i<v.size();i++) {
                    fprintf(error_stream,"  %2d %s/%s val=%f min=%f max=%f step=%f\n",int(i),v[i].getName(),v[i].getLabel(),v[i].getValue(),v[i].getMin(),v[i].getMax(),v[i].getStep());
                }
            }
            break;
        case INDI_TEXT:
            {
                INDI::PropertyText v(p);
                for(size_t i=0;i<v.size();i++) {
                    fprintf(error_stream,"  %2d %s/%s val=%s\n",int(i),v[i].getName(),v[i].getLabel(),v[i].getText());
                }
            }
            break;
        case INDI_SWITCH:
            {
                INDI::PropertySwitch v(p);
                for(size_t i=0;i<v.size();i++) {
                    fprintf(error_stream,"  %2d %s/%s %s\n",int(i),v[i].getName(),v[i].getLabel(),(v[i].getState() == ISS_ON ? "ON " : "OFF"));
                }
            }
            break;
        case INDI_LIGHT:
            {
                INDI::PropertyLight v(p);
                for(size_t i=0;i<v.size();i++) {
                    fprintf(error_stream,"  %2d %s/%s %s\n",int(i),v[i].getName(),v[i].getLabel(),v[i].getStateAsString());
                }
            }
            break;
        case INDI_BLOB:
            {
                INDI::PropertyBlob v(p);
                for(size_t i=0;i<v.size();i++) {
                    fprintf(error_stream,"  %2d %s/%s ptr=%p size=%d\n",int(i),v[i].getName(),v[i].getLabel(),v[i].getBlob(),v[i].getSize());
                }
            }
            break;
        default:
            ;
    }
}


class MountClient : public INDI::BaseClient {
public:
    MountClient(char *port)
    {
        if(port)
            port_ = port;
        setServer("localhost",7624);
        if(!connectServer()) {
            throw std::runtime_error("Faild to connect");
        }
    }

    void sendNewProperty(INDI::Property prop)
    {
        log_property("SND",prop);
        INDI::BaseClient::sendNewProperty(prop);
    }

    virtual void updateProperty(INDI::Property prop) override
    {
        guard_type g(lock_);
        handle(true,prop);
    }
    virtual void newProperty(INDI::Property prop) override
    {
        guard_type g(lock_);
        handle(false,prop);
    }
    void handle(bool is_new,INDI::Property prop)
    {
        log_property(is_new?"new":"upd",prop);
        if(prop.isNameMatch("DRIVER_INFO") && name_.empty()) {
            INDI::PropertyText p(prop);
            auto interface = p.findWidgetByName("DRIVER_INTERFACE");
            if(interface && atoi(interface->getText()) & INDI::BaseDevice::TELESCOPE_INTERFACE) {
                LOG("!!!!!!!!!!!!!!!!!!!!!!!!!!!! DERECTED %s\n",prop.getDeviceName());
                name_ = prop.getDeviceName();
                on_detected(prop.getBaseDevice());
            }
        }
        if(prop.isNameMatch("CONNECTION") && prop.isDeviceNameMatch(name_)) {
            handle_connection(prop);
        }
        else if(prop.isNameMatch("CONNECTION_MODE")) {
            handle_mode(prop);
        }
        else if(prop.isNameMatch("EQUATORIAL_EOD_COORD")) {
            log_ra_de(prop);
        }
    }
    
    void go_to(double RA,double DEC)
    {
        guard_type g(lock_);
        /*for(auto p : device_.getProperties()) {
            log_property("GTO",p);
        }*/
        INDI::PropertySwitch on_set = device_.getProperty("ON_COORD_SET");
        if(!on_set.isValid()) {
            LOG("No ON_COORD_SET\n");
            return;
        }
        INDI::PropertyNumber coord = device_.getProperty("EQUATORIAL_EOD_COORD");
        if(!coord.isValid()) {
            LOG("No EQUATORIAL_EOD_COORD\n");
            return;
        }
        auto on_set_w = on_set.findWidgetByName("TRACK");
        auto ra_w = coord.findWidgetByName("RA");
        auto dec_w = coord.findWidgetByName("DEC");
        
        if(!on_set_w || !ra_w || !dec_w) {
            LOG("No widgets\n");
            return;
        }
        if(on_set_w->getState() != ISS_ON) {
            on_set_w->setState(ISS_ON);
            sendNewProperty(on_set);
        }
        ra_w->setValue(RA);
        dec_w->setValue(DEC);
        sendNewProperty(coord);
    }
    void log_ra_de(INDI::PropertyNumber prop) 
    {
        if(!prop.isValid())
            return;
        auto ra = prop.findWidgetByName("RA");
        auto de = prop.findWidgetByName("DEC");
        if(!ra || !de)
            return;
        double RA=ra->getValue();
        double DEC=de->getValue();

        int rh = int(RA);
        int rs = int(RA*3600) % 3600;
        int rm = rs / 60;
        rs = rs % 60;
        
        char s='+';
        if(DEC < 0) {
            s='-';
            DEC=-DEC;
        }
        int dd = int(DEC);
        int ds = int(DEC*3600) % 3600;
        int dm = ds / 60;
        ds = ds % 60;
        printf("RA=%02d:%02d:%02d DE=%c%02d:%02d:%02d\n",rh,rm,rs,s,dd,dm,ds);
    }
    void handle_mode(INDI::PropertySwitch p)
    {
        if(!p.isValid()) {
            LOG("Property is not valid\n");
            return;
        }
        char const *wname = port_.empty() ? "CONNECTION_TCP" : "CONNECTION_SERIAL";
        printf("Connecting to %s\n",wname);
        bool update = false;
        for(int i=0;i<p.count();i++) {
            if(p[i].isNameMatch(wname)) {
                if(p[i].getState() != ISS_ON) {
                    p[i].setState(ISS_ON);
                    update = true;
                }
            }
            else {
                if(p[i].getState() != ISS_OFF) {
                    p[i].setState(ISS_OFF);
                    update = true;
                }
            }
        }
        if(update) {
            sendNewProperty(p);
        }
    }
    void on_detected(INDI::BaseDevice d)
    {
        device_ = d;
        watchDevice(name_.c_str());
        connectDevice(name_.c_str());
    }
    void handle_connection(INDI::PropertySwitch p)
    {
        auto ptr = p.findWidgetByName("CONNECT");
        if(!ptr) {
            LOG("No CONNECT in connection\n");
            return;
        }
        if(ptr->getState() != ISS_ON) {
            LOG("Connecting to %s\n",name_.c_str());
            connectDevice(name_.c_str());
        }
        else {
            LOG("Connected to camera %s\n",name_.c_str());
            connected_ = true;
        }
    }


private:
    std::string port_;
    std::recursive_mutex lock_;
    bool connected_ = false;
    std::string name_;
    INDI::BaseDevice device_;
};


double parseRA(int h,int m,int s)
{
    double hours = (h + (60 * m + s) / 3600.0 );
    return hours;
}

double parseDEC(int d,int m,int s)
{
    int sig=1;
    if(d < 0) {
        sig = -1;
        d = -d;
    }
    return sig*(d + (60 * m + s) / 3600.0);
}


#ifdef INDI_AS_LIBRARY

int indiserver_main(std::vector<std::string> drivers);

#endif


int main(int argc,char **argv)
{
    if(argc>=2 && strcmp(argv[1],"-l") == 0)  {
        error_stream = fopen("/tmp/log.txt","w");
        argc--;
        argv++;
    }
    else
        error_stream = stderr;
#ifdef INDI_AS_LIBRARY
    std::vector<std::string> libs;
    while(argc >= 2 && strstr(argv[1],"lib") != nullptr) {
        libs.push_back(argv[1]);
        argv++;
        argc--;
    }
    if(!libs.empty()) {
        std::thread server([=]() {
            indiserver_main(libs);
        });
        server.detach();
        usleep(500000);
    }
#endif    
    MountClient client(argv[1]);
    usleep(100000);
    std::string line;
    while(std::getline(std::cin,line)) {
        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;
        if(cmd=="end")
            break;
        if(cmd=="goto") {
            int rh,rm,rs,dh,dm,ds;
            char c;
            ss >> rh >>c >> rm >> c >> rs >> dh>>c>>dm>>c>>ds;
            if(!ss) {
                std::cout << "Bad format" << std::endl;
                continue;
            }
            client.go_to(parseRA(rh,rm,rs),parseDEC(dh,dm,ds));
        }
    }
}
