#ifndef REDUCXX_ACTION_HPP
#define REDUCXX_ACTION_HPP

namespace ReduCxx {
    class Action;
}

class ReduCxx::Action {
    virtual int type() const = 0;  
};


#endif //REDUCXX_ACTION_HPP