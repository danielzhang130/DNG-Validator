#include "dng_host.h"
#include "dng_file_stream.h"
#include "dng_info.h"
#include "dng_negative.h"
#include "dng_errors.h"
#include "dng_exceptions.h"

#include <iostream>
#include <string>
#include <queue>
#include <syslog.h>
using namespace std;

string print(const dng_error_code value){
    const char* s;
    #define PROCESS_VAL(p) case(p): s = #p; break;
    switch(value){
        PROCESS_VAL(dng_error_none);
        PROCESS_VAL(dng_error_unknown);
        PROCESS_VAL(dng_error_not_yet_implemented);
        PROCESS_VAL(dng_error_silent);
        PROCESS_VAL(dng_error_user_canceled);        
        PROCESS_VAL(dng_error_host_insufficient);
        PROCESS_VAL(dng_error_memory);
        PROCESS_VAL(dng_error_bad_format);
        PROCESS_VAL(dng_error_matrix_math);
        PROCESS_VAL(dng_error_open_file);
        PROCESS_VAL(dng_error_read_file);
        PROCESS_VAL(dng_error_write_file);
        PROCESS_VAL(dng_error_end_of_file);
        PROCESS_VAL(dng_error_file_is_damaged);
        PROCESS_VAL(dng_error_image_too_big_dng);
        PROCESS_VAL(dng_error_image_too_big_tiff);
        PROCESS_VAL(dng_error_unsupported_dng);
    }
    return s;
}

int main(){
    queue<string> paths;
    string s;
    while(getline(cin, s)){
        paths.push(s);
    }

    while(!paths.empty()){
        string path = paths.front();
        paths.pop();
        dng_negative* negative = nullptr;
        try{
            dng_file_stream stream(path.c_str());
            dng_host host;
            
            dng_info info;
            info.Parse(host, stream);
            info.PostParse(host);
            if (!info.IsValidDNG ()){
                return dng_error_bad_format;
            }
            
            negative = host.Make_dng_negative();
            negative->Parse(host, stream, info);
            negative->PostParse(host, stream, info);
            negative->ReadStage1Image(host, stream, info);
            negative->ValidateRawImageDigest(host);
            
            if(negative->IsDamaged()){
                openlog("DNG Validator", LOG_PID|LOG_CONS, LOG_LOCAL0);
                syslog(LOG_INFO, "%s is damaged.\n", path.c_str());
                closelog();
            }
            else{
                 openlog("DNG Validator", LOG_PID|LOG_CONS, LOG_LOCAL0);
                syslog(LOG_INFO, "%s is good.\n", path.c_str());
                closelog();
            }
            
            delete negative;
            negative = nullptr;
        }
        catch(const dng_exception& e){
            openlog("DNG Validator", LOG_PID|LOG_CONS, LOG_LOCAL0);
            syslog(LOG_ERR, "%s Error processing %s. File is probably damaged.\n", print(e.ErrorCode()).c_str(), path.c_str());
            closelog();
            if(nullptr != negative){
                delete negative;
                negative = nullptr;
            }
            continue;
        }
        catch (...){
            openlog("DNG Validator", LOG_PID|LOG_CONS, LOG_LOCAL0);
            syslog(LOG_ERR, "Unknown error processing %s.\n", path.c_str());
            closelog();
            if(nullptr != negative){
                delete negative;
                negative = nullptr;
            }
            continue;
        }
    }

    return 0;
}
