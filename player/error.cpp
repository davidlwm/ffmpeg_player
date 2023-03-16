//
// Created by apple on 14/1/2023.
//

#include "error.h"
extern "C"{
#include "IOTCAPIs.h"
#include "AVAPIs.h"
}

const char *GetConnectError(int err){
    switch (err) {
        case -12:
            return "IOTC_ER_NOT_INITIALIZED";
        case -13:
            return "IOTC_ER_TIMEOUT";
        case -90:
            return "IOTC_ER_DEVICE_OFFLINE";
        defaut:
            return "";
    }
    return "";
}
