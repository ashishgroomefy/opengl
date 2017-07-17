#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <curl/curl.h>

//#include <cstdint>
#include <iostream>
#include <memory>
#include <json/json.h>

std::string data; //will hold the url's contents

namespace {
std::size_t callback(
    const char* in,
    std::size_t size,
    std::size_t num,
    std::string* out)
{

    for (int c = 0; c < size * num; c++) {
        data.push_back(in[c]);
    }

    const std::size_t totalBytes(size * num);
    out->append(in, totalBytes);
    return totalBytes;
}
}

int getLandmarks(const char* theFileName)
{
    CURL* curl;

    CURLM* multi_handle;
    int still_running;

    struct curl_httppost* formpost = NULL;
    struct curl_httppost* lastptr = NULL;
    struct curl_slist* headerlist = NULL;
    static const char buf[] = "Expect:";

    /* Fill in the file upload field. This makes libcurl load data from
     the given file name when curl_easy_perform() is called. */
    curl_formadd(&formpost,
        &lastptr,
        CURLFORM_COPYNAME, "face",
        CURLFORM_FILE, theFileName,
        CURLFORM_END);

    curl = curl_easy_init();

    /* what URL that receives this POST */
    curl_easy_setopt(curl, CURLOPT_URL, "http://facedetection.groomefy.com/get68landmarks_new");
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); //tell curl to output its progress

    //    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    // Response information.
    int httpCode(0);
    std::unique_ptr<std::string> httpData(new std::string());

    // Hook up data handling function.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

    // Hook up data container (will be passed as the last parameter to the
    // callback handling function).  Can be any pointer type, since it will
    // internally be passed as a void pointer.
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

    // Run our HTTP GET command, capture the HTTP response code, and clean up.
    curl_easy_perform(curl);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    /* always cleanup */
    curl_easy_cleanup(curl);

    /* then cleanup the formpost chain */
    curl_formfree(formpost);

    /* free slist */
    curl_slist_free_all(headerlist);

    if (httpCode == 200) {
        std::cout << "\nGot successful response from " << data << std::endl;

        // Response looks good - done using Curl now.  Try to parse the results
        // and print them out.
        Json::Value jsonData;
        Json::Reader jsonReader;
        if (jsonReader.parse(*httpData, jsonData)) {
            std::cout << "Successfully parsed JSON data" << std::endl;
            std::cout << "\nJSON data received:" << std::endl;
            std::cout << jsonData.toStyledString() << std::endl;

            //                               {
            //                                 "time": "02:44:56 PM",
            //                                 "milliseconds_since_epoch": 1500216296332,
            //                                 "date": "07-16-2017"
            //                               }
            //                               const std::string dateString(jsonData["date"].asString());
            //                                          const std::size_t unixTimeMs(
            //                                                  jsonData["milliseconds_since_epoch"].asUInt64());
            //                                          const std::string timeString(jsonData["time"].asString());

            Json::Value rightEyebrow= jsonData["right-eyebrow"];
            for( Json::ValueIterator itr = rightEyebrow.begin() ; itr != rightEyebrow.end() ; itr++ ) {
                std::string x =  (*itr)["x"].asString();
                std::cout<<"value of x  "<<x<< std::endl;

            }
        }
        else {
            std::cout << "Could not parse HTTP data as JSON" << std::endl;
            std::cout << "HTTP data was:\n" << *httpData.get() << std::endl;
            return 1;
        }
    }
    else {
        std::cout << "Couldn't GET from "
                  << " - exiting" << std::endl;
        return 1;
    }

    return 0;
}
