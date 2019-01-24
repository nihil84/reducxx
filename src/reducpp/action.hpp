#ifndef REDUCPP_ACTION_HPP
#define REDUCPP_ACTION_HPP

namespace reducpp {
    class action;
}

class reducpp::action {
    virtual int type() const = 0;  
};


#endif // REDUCPP_ACTION_HPP