#include "APIManager.h"
#include "Utility.h"

SmoothCamAPI::IVSmoothCam2* g_SmoothCam;

void APIs::RequestAPIs() {
    if (!SmoothCam) {
        if (!SmoothCamAPI::RegisterInterfaceLoaderCallback(
                SKSE::GetMessagingInterface(),
                [](void* interfaceInstance, SmoothCamAPI::InterfaceVersion interfaceVersion) {
                    if (interfaceVersion == SmoothCamAPI::InterfaceVersion::V2) {
                        SmoothCam = reinterpret_cast<SmoothCamAPI::IVSmoothCam2*>(interfaceInstance);
                        g_SmoothCam = SmoothCam;
                        logger::info("Obtained SmoothCamAPI");
                    } else {
                        logger::error("Unable to acquire requested SmoothCamAPI interface version");
                    }
                })) {
            logger::warn("SmoothCamAPI::RegisterInterfaceLoaderCallback reported an error");
        }

        if (!SmoothCamAPI::RequestInterface(SKSE::GetMessagingInterface(), SmoothCamAPI::InterfaceVersion::V2)) {
            logger::warn("SmoothCamAPI::RequestInterface reported an error");
        }
    }
}