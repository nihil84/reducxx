#ifndef REDUCPP_ACTION_HPP
#define REDUCPP_ACTION_HPP

namespace ReduCxx {
    class action;
}

class ReduCxx::action {
    virtual int type() const = 0;  
};


#endif // REDUCPP_ACTION_HPP