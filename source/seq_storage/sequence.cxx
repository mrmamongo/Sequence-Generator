//
// Created by MrMam on 14.09.2022.
//

#include <seq_storage/sequence.hxx>

counter sequence::next() {
    counter result = current;
    current += step;
    return result;
}

sequence::sequence(counter start, counter step):
        step(step),
        current(start),
        valid(true)
        {}

counter sequence::get_current() const {
    return current;
}

counter sequence::get_step() const {
    return step;
}

sequence::operator bool() const {
    return valid;
}