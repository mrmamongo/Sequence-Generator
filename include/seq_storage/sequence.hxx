//
// Created by MrMam on 14.09.2022.
//

#ifndef SEQ_GENERATOR_SEQUENCE_HXX
#define SEQ_GENERATOR_SEQUENCE_HXX

#include <string>

typedef unsigned long long counter;

class sequence {
public:
    sequence() = default;
    /// \brief Конструктор с параметрами
    /// \param start Начальное значение
    /// \param step Шаг
    sequence(counter start, counter step);
    /// \brief Возвращает следующее число последовательности
    /// \return
    counter next();
    counter get_current() const;
    counter get_step() const;
private:
    counter step = 0;
    counter current = UINT64_MAX;
};


#endif //SEQ_GENERATOR_SEQUENCE_HXX
