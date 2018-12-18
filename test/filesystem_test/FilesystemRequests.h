#include "Requests.h"

class FilesystemRequests : public filesystem::Requests {
public:
    void log(const cstd::string& s) override {} // do nothing
};