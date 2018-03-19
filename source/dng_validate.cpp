#include "dng_host.h"
#include "dng_file_stream.h"
#include "dng_info.h"
#include "dng_negative.h"

#include <iostream>
#include <string>
#include <vector>
using namespace std;

int main(){
    vector<string> paths;
    string s;
    while(getline(cin, s)){
        paths.push_back(s);
    }

    for(size_t i = 0; i < paths.size(); i++){
        dng_file_stream stream(paths.at(i).c_str());
        dng_host host;
        
        dng_info info;
        info.Parse(host, stream);
        info.PostParse(host);
        if (!info.IsValidDNG ()){
            return dng_error_bad_format;
        }
        
        dng_negative* negative = host.Make_dng_negative();
        negative->Parse(host, stream, info);
        negative->PostParse(host, stream, info);
        negative->ReadStage1Image(host, stream, info);
        negative->ValidateRawImageDigest(host);
        
        delete negative;
        
        if(negative->IsDamaged()){
            cout << paths.at(i) << " is damaged." << endl;
        }
        else{
            cout << paths.at(i) << " is good." << endl;
        }

    }

    return 0;
}
